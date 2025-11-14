#pragma once
#include "includes.h"

struct Detection
{
	int classId;
	float confidence;

	struct Distance
	{
		int square;
		int width;
		int height;
	} distance;

	cv::Rect box;
};

class Camera
{
public:
	Camera();
	~Camera();

	void preparingModel(cv::Mat& img);
	void drawBoxes(cv::Mat& img);
	bool inPlaceOrNot(cv::Mat& img);

private:
	bool cudaAvailable = true;
	bool timerIsWorking = false;

	cv::dnn::Net net;
	std::vector<Detection> detections;
	std::vector<int> indices;

	int personClassId = 0;
	const int inpW = 640, inpH = 640;
	const float confThreshold = 0.7f, nmsThreshold = 0.5f;
};