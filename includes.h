#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/ximgproc.hpp>

#include <Windows.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <atomic>
#include <shellapi.h>
#include <filesystem>

#define PATH_TO_MODEL "assets/models/yolov8n.onnx"

#define IMG_WIDTH 1930
#define IMG_HEIGHT 1020

#define GREEN "\033[32m"
#define RED "\033[31m"
#define STANDART "\033[0m"