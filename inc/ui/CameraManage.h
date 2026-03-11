#pragma once
#include<opencv2/opencv.hpp>


#define TIMER_CAMERA_UPDATE     1


extern bool cameraConnected;

void ConnectCamera();
void DisconnectCamera();
void UpdateCameraFrame();
void CaptureImage();
void UpdateCameraControlsState(bool enabled);
cv::Mat GetCurrentFrame();
void DisplayFrame(const cv::Mat& frame);
