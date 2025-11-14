#include "includes.h"
#include "Camera.h"


int main()
{
	setlocale(LC_ALL, "Russian");

	Camera camera;
	cv::Mat test = cv::imread("assets/test/2.jpg");

	cv::VideoCapture cap(0);
	if (!cap.isOpened())
		std::cerr << RED << "Error: Could not open video stream." << STANDART << std::endl;
		
	cv::Mat frame;
	while (true)
	{
		cap >> frame;
		if (frame.empty()) break;

		camera.preparingModel(frame);
		camera.drawBoxes(frame);
		camera.inPlaceOrNot(frame);

		cv::resize(frame, frame, cv::Size(640, 500));

		cv::imshow("Video", frame);

		if (cv::waitKey(1) == 27) break;
	}

	cap.release();
	cv::destroyAllWindows();
	return 0;
}