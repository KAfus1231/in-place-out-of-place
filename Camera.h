#pragma once
#include "includes.h"
#include "Timer.h"
#include "Player.h"

#define SQUARE_DISTANCE_THRESHOLD 9
#define WIDTH_DISTANCE_THRESHOLD 30
#define HEIGHT_DISTANCE_THRESHOLD 27

struct Detection
{
	int classId;
	float confidence;

	struct Distance
	{
		int square;
		int width;
		int height;

		bool isInPlace() const
		{
			return (square >= SQUARE_DISTANCE_THRESHOLD &&
				width >= WIDTH_DISTANCE_THRESHOLD &&
				height >= HEIGHT_DISTANCE_THRESHOLD);
		}
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
	explicit Camera(std::filesystem::path p);
	~Camera() = default;

	void preparingModel(const cv::Mat& img);
	void drawBoxes(cv::Mat& img);
	bool inPlaceOrNot(cv::Mat& img);

	PlaceStatus placeStatus = PlaceStatus::IN_PLACE;
private:
	std::unique_ptr<Player> player_;
	Timer timer_;

	std::filesystem::path path_;

	bool cudaAvailable = true;
	bool warningShown = false;
	bool alarmPlaying = false;

	cv::dnn::Net net;

	cv::Mat imgBGR_, imgResized_;
	cv::Mat blob_;

	std::vector<Detection> detections_;
	std::vector<int> indices_;

	std::vector<cv::Rect> boxes_;
	std::vector<float> confidences_;

	const int personClassId = 0;
	const int inpW = 640, inpH = 640;
	const float confThreshold = 0.7f, nmsThreshold = 0.5f;

	std::chrono::steady_clock::time_point warningStartTime;
	std::chrono::seconds warningDurationFirst = std::chrono::seconds(1);
	std::chrono::seconds warningDurationSecond = std::chrono::seconds(5);

	// тхкэрп ьслнб
	const int framesForTrigger = 3;
	int consecutiveNotInPlace = 0;
	int consecutiveInPlace = 0;
};