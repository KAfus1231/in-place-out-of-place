#include "Camera.h"

VOID ShowBalloon(LPCWSTR title, LPCWSTR msg)
{
	NOTIFYICONDATAW nid = {};
	nid.cbSize = sizeof(nid);
	nid.hWnd = GetConsoleWindow();
	nid.uID = 1;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = WM_USER + 1;
	nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	lstrcpy(nid.szTip, L"IN PLACE");

	Shell_NotifyIconW(NIM_ADD, &nid);
	nid.uVersion = NOTIFYICON_VERSION_4;
	Shell_NotifyIconW(NIM_SETVERSION, &nid);

	NOTIFYICONDATAW modify = nid;
	modify.uFlags = NIF_INFO;
	modify.dwInfoFlags = NIIF_INFO;
	lstrcpy(modify.szInfoTitle, title);
	lstrcpy(modify.szInfo, msg);

	Shell_NotifyIconW(NIM_MODIFY, &modify);

	std::thread([nid]() mutable
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));
			Shell_NotifyIconW(NIM_DELETE, &nid);
		}).detach();
}

Camera::Camera(std::filesystem::path p)
	: path_(std::move(p))
{
	try
	{
		this->net = cv::dnn::readNetFromONNX(path_.string());
		std::cout << GREEN << "Model loaded successfully." << STANDART << std::endl;
		
		if (cudaAvailable)
		{
			net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
			net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
		}
		else
		{
			net.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
			net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
		}
	}
	catch (const cv::Exception& e)
	{
		throw std::runtime_error("Failed to load ONNX model: " + path_.string() + "\n" + e.what());
	}
}

void Camera::preparingModel(const cv::Mat& img)
{
	cv::cvtColor(img, imgBGR_, cv::COLOR_BGRA2BGR);
	cv::resize(imgBGR_, imgResized_, cv::Size(inpW, inpH), 0, 0, cv::INTER_LINEAR);

	blob_ = cv::dnn::blobFromImage(imgResized_, 1 / 255.0, cv::Size(inpW, inpH), cv::Scalar(0, 0, 0), true, false);
	net.setInput(blob_);

	std::vector<cv::Mat> outputs;
	net.forward(outputs, net.getUnconnectedOutLayersNames());

	cv::Mat detMat = outputs[0];
	detMat = detMat.reshape(1, detMat.size[1]).t();
	
	detections_.clear();
	for (int i = 0; i < detMat.rows; ++i)
	{
		auto* data = detMat.ptr<float>(i);

		cv::Mat scores(1, detMat.cols - 4, CV_32FC1, data + 4);
		cv::Point classIdPoint;
		double maxClassScore;
		cv::minMaxLoc(scores, nullptr, &maxClassScore, nullptr, &classIdPoint);
		float conf = (float)maxClassScore;

		if (!(classIdPoint.x == personClassId && conf > confThreshold)) continue;
	
		float cx = data[0] / inpW,
			cy = data[1] / inpH,
			w = data[2] / inpW,
			h = data[3] / inpH;

		const auto scaleSquare = 10000;
		const auto scaleWidth = 10;
		const auto scaleHeight = 10;

		int left = int((cx - w / 2) * img.cols);
		int top = int((cy - h / 2) * img.rows);
		int width = int(w * img.cols);
		int height = int(h * img.rows);
		int square = int(width * height) / scaleSquare;

		detections_.push_back({ classIdPoint.x, conf,
			{square, width / scaleWidth, height / scaleHeight},
			cv::Rect(left, top, width, height) });
	}
}

void Camera::drawBoxes(cv::Mat& img)
{
	for (const auto& d : detections_)
	{
		if (d.box.width <= 0 || d.box.height <= 0)
		{
			std::cerr << "ÏÐÎÏÓÑÊÀÅÌ ÎÏÀÑÍÛÅ ÁÎÊÑÛ: " << d.box << std::endl;
			continue;
		}

		if (!std::isfinite(d.confidence))
		{
			std::cerr << "ÏÐÎÏÓÑÊÀÅÌ ÎÏÀÑÍÛÅ ÓÂÅÐÅÍÍÎÑÒÈ" << std::endl;
			continue;
		}

		boxes_.push_back(d.box);
		confidences_.push_back(d.confidence);
	}

	indices_.clear();
	if (boxes_.size() == confidences_.size() && !boxes_.empty())
		cv::dnn::NMSBoxes(boxes_, confidences_, confThreshold, nmsThreshold, indices_);

	cv::Point center(IMG_WIDTH / 2, IMG_HEIGHT / 2);

	for (const auto& idx : indices_)
	{
		const auto& d = detections_[idx];

		cv::Point boxCenter(d.box.x + d.box.width / 2, d.box.y + d.box.height / 2);
		cv::putText(img,
			cv::format("Chelik: %.2f", d.confidence),
			cv::Point(d.box.x, d.box.y - 10),
			cv::FONT_HERSHEY_SIMPLEX,
			0.5,
			cv::Scalar(0, 255, 0),
			1);
		cv::putText(img,
			cv::format("Square: %d", d.distance.square),
			cv::Point(d.box.x + 100, d.box.y - 10),
			cv::FONT_HERSHEY_SIMPLEX,
			0.5,
			cv::Scalar(255, 0, 0),
			1);
		cv::putText(img,
			cv::format("Width: %d", d.distance.width),
			cv::Point(d.box.x + 200, d.box.y - 10),
			cv::FONT_HERSHEY_SIMPLEX,
			0.5,
			cv::Scalar(255, 0, 0),
			1);
		cv::putText(img,
			cv::format("Height: %d", d.distance.height),
			cv::Point(d.box.x + 290, d.box.y - 10),
			cv::FONT_HERSHEY_SIMPLEX,
			0.5,
			cv::Scalar(255, 0, 0),
			1);
		cv::rectangle(img, d.box,
			[=]() {
				switch (placeStatus)
				{
				case PlaceStatus::IN_PLACE:		  return cv::Scalar(0, 255, 0);
				case PlaceStatus::LEFT_THE_PLACE: return cv::Scalar(0, 255, 255);
				case PlaceStatus::OUT_OF_PLACE:	  return cv::Scalar(0, 0, 255);
				}
			}(), 2);
	}
	boxes_.clear();
	confidences_.clear();
}

bool Camera::inPlaceOrNot(cv::Mat& img)
{
	bool personDetected = !indices_.empty();
	bool inPlace = true;

	if (personDetected)
	{
		auto& d = detections_[indices_[0]];
		inPlace = d.distance.isInPlace();
	}
	else
	{
		inPlace = false; 
		cv::putText(img,
			"NO PERSON DETECTED",
			cv::Point(10, 30),
			cv::FONT_HERSHEY_SIMPLEX,
			1.0,
			cv::Scalar(0, 0, 255),
			2);
	}

	if (inPlace)
	{
		consecutiveInPlace++;
		consecutiveNotInPlace = 0;
	}
	else
	{
		consecutiveNotInPlace++;
		consecutiveInPlace = 0;
	}

	bool stableInPlace = (consecutiveInPlace >= framesForTrigger);
	bool stableNotInPlace = (consecutiveNotInPlace >= framesForTrigger);

	auto now = std::chrono::steady_clock::now();

	switch (placeStatus)
	{
		case PlaceStatus::IN_PLACE:
		{
			if (!timer_.isRunning()) timer_.start();

			if (stableNotInPlace)
			{
				placeStatus = PlaceStatus::LEFT_THE_PLACE;
				warningStartTime = now;
				warningShown = false;
				std::cout << RED << "LEFT THE PLACE: WARNING STARTED" << STANDART << std::endl;
			}
			break;
		}
		case PlaceStatus::LEFT_THE_PLACE:
		{
			if (stableInPlace)
			{
				placeStatus = PlaceStatus::IN_PLACE;
				player_.reset();
				alarmPlaying = false;
				break;
			}

			if (!alarmPlaying && !player_)
			{
				player_ = std::make_unique<Player>();
				player_->play();
				alarmPlaying = true;
			}

			auto elepsed = std::chrono::duration_cast<std::chrono::seconds>(now - warningStartTime);

			if (elepsed > warningDurationFirst && elepsed <= warningDurationSecond && !warningShown)
			{
				ShowBalloon(L"Warning", L"You have left the designated area. Please return.");
				warningShown = true;
			}

			if (elepsed > warningDurationSecond)
			{
				placeStatus = PlaceStatus::OUT_OF_PLACE;
				int focusSeconds = timer_.elepsed();

				{
					std::wstringstream ss;
					ss << L"Focus session length: " << focusSeconds << L" s";
					ShowBalloon(L"Focus ended", ss.str().c_str());
				}

				timer_.reset();
			}
			break;
		}
		case PlaceStatus::OUT_OF_PLACE:
		{
			if (alarmPlaying)
			{
				player_.reset();
				alarmPlaying = false;
			}
			
			if (stableInPlace)
			{
				placeStatus = PlaceStatus::IN_PLACE;
				timer_.stop();
				timer_.reset();
				ShowBalloon(L"Welcome back", L"You are back in the designated area.");
			}
			break;
		}
	}

	return (placeStatus == PlaceStatus::IN_PLACE);
}

Camera::~Camera()
{

}

