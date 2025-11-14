#include "Camera.h"

//днаюбхрэ PADDED RESIZE letterbox_image

VOID ShowBalloon(LPCWSTR title, LPCWSTR msg)
{
	NOTIFYICONDATA nid = {};
	nid.cbSize = sizeof(nid);
	nid.hWnd = GetConsoleWindow();
	nid.uID = 1;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = WM_USER + 1;
	nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);
	lstrcpy(nid.szTip, L"IN PLACE");
	lstrcpy(nid.szInfo, msg);
	lstrcpy(nid.szInfoTitle, title);
	nid.dwInfoFlags = NIIF_INFO;

	NOTIFYICONDATA localNid = nid;

	std::thread([localNid]() mutable
	{
		Shell_NotifyIcon(NIM_ADD, &localNid);
		Sleep(3000);
		Shell_NotifyIcon(NIM_DELETE, &localNid);
	}).detach();
}

std::atomic<int> runTime{ 0 };
void timer()
{
	std::thread([]()
		{
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				runTime++;
				std::cout << RED << runTime.load() << STANDART << std::endl;
			}

		}).detach();
}

Camera::Camera()
{
	try
	{
		this->net = cv::dnn::readNetFromONNX(PATH_TO_MODEL);
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
		std::cerr << RED << "Error loading model: " << e.what() << STANDART << std::endl;
	}
}

void Camera::preparingModel(cv::Mat& img)
{
	cv::Mat imgBGR, imgResized;

	cv::cvtColor(img, imgBGR, cv::COLOR_BGRA2BGR);
	cv::resize(imgBGR, imgResized, cv::Size(inpW, inpH), 0, 0, cv::INTER_LINEAR);

	cv::Mat blob = cv::dnn::blobFromImage(imgResized, 1 / 255.0, cv::Size(inpW, inpH), cv::Scalar(0, 0, 0), true, false);
	net.setInput(blob);

	std::vector<cv::Mat> outputs;
	net.forward(outputs, net.getUnconnectedOutLayersNames());

	cv::Mat detMat = outputs[0];
	detMat = detMat.reshape(1, detMat.size[1]).t();
	
	detections.clear();
	for (int i = 0; i < detMat.rows; ++i)
	{
		float* data = (float*)detMat.ptr(i);

		cv::Mat scores(1, detMat.cols - 4, CV_32FC1, data + 4);
		cv::Point classIdPoint;
		double maxClassScore;
		cv::minMaxLoc(scores, nullptr, &maxClassScore, nullptr, &classIdPoint);
		float conf = (float)maxClassScore;

		if (classIdPoint.x != personClassId) continue;
		if (conf < confThreshold) continue;

		float cx = data[0] / 640, 
			  cy = data[1] / 640,
			  w = data[2] / 640, 
			  h = data[3] / 640;

		int left = int((cx - w / 2) * img.cols);
		int top = int((cy - h / 2) * img.rows);
		int width = int(w * img.cols);
		int height = int(h * img.rows);
		int square = int(width * height) / 10000;

		detections.push_back({ classIdPoint.x, conf, {square, width / 100, height / 100}, cv::Rect(left, top, width, height)});
	}
}

void Camera::drawBoxes(cv::Mat& img)
{
	cv::Mat imgDrawn = img.clone();
	std::vector<cv::Rect> boxes;
	std::vector<float> confidences;
	for (auto& d : detections)
	{
		if (d.classId != personClassId) continue;

		if (d.box.width <= 0 || d.box.height <= 0)
		{
			std::cerr << "опносяйюел ноюямше анйяш: " << d.box << std::endl;
			continue;
		}

		if (!std::isfinite(d.confidence))
		{
			std::cerr << "опносяйюел ноюямше сбепеммнярх" << std::endl;
			continue;
		}

		boxes.push_back(d.box);
		confidences.push_back(d.confidence);
	}

	indices.clear();
	if (boxes.size() == confidences.size() && !boxes.empty())
		cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

	cv::Point center(IMG_WIDTH / 2, IMG_HEIGHT / 2);

	for (int idx : indices)
	{
		auto& d = detections[idx];

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
			cv::Point(d.box.x + 270, d.box.y - 10),
			cv::FONT_HERSHEY_SIMPLEX,
			0.5,
			cv::Scalar(255, 0, 0),
			1);
		cv::rectangle(img, d.box, cv::Scalar(0, 0, 255), 2);
	}
}

bool Camera::inPlaceOrNot(cv::Mat& img)
{
	if (!indices.empty())
	{
		for (int idx : indices)
		{
			auto& d = detections[idx];

			if (d.distance.square < DISTANCE_THRESHOLD)
			{
				if (!timerIsWorking)
				{
					timerIsWorking = true;
					timer();
					std::cout << "Person not in place!" << std::endl;
				}
				/*ShowBalloon(L"IN PLACE", L"Person not in place!");*/
				return false;
			}
			else
			{
				/*ShowBalloon(L"IN PLACE", L"Person in place.");*/
				timerIsWorking = false;
				return true;
			}
		}
	}
	else
	{
		std::cout << RED << "No persons detected." << STANDART << std::endl;
	}
}

Camera::~Camera()
{

}

