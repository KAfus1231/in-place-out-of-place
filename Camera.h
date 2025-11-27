#pragma once
#include "includes.h"
#include "Timer.h"

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

enum class PlaceStatus
{
	IN_PLACE, // мю леяре
	LEFT_THE_PLACE, // онйхмск леярн онксвюер опедсопефдемхе
	OUT_OF_PLACE // бме леярю
};

class Camera
{
public:
	Camera();
	~Camera();

	void preparingModel(cv::Mat& img);
	void drawBoxes(cv::Mat& img);
	bool inPlaceOrNot(cv::Mat& img, Timer& timer);

private:
	PlaceStatus placeStatus = PlaceStatus::IN_PLACE;

	bool cudaAvailable = true;
	bool warningShown = false;

	cv::dnn::Net net;
	std::vector<Detection> detections;
	std::vector<int> indices;

	int personClassId = 0;
	const int inpW = 640, inpH = 640;
	const float confThreshold = 0.7f, nmsThreshold = 0.5f;

	std::chrono::steady_clock::time_point warningStartTime;
	std::chrono::seconds warningDuration = std::chrono::seconds(4);

	// тхкэрп ьслнб
	int framesForTrigger = 3;
	int consecutiveNotInPlace = 0;
	int consecutiveInPlace = 0;
};