#pragma once
#include"..\res\Resource.h"
#include <windows.h>
#include <atomic>
#include <string>
#include<opencv2/opencv.hpp>
#include<uEye.h>



// 控件句柄定义（全局）
extern HWND hComboPort;
extern HWND hBtnRefreshPort;
extern HWND hBtnConnectStage;
extern HWND hBtnDisconnectStage;
extern HWND hBtnConnectCamera;
extern HWND hBtnDisconnectCamera;
extern HWND hBtnCapture;
extern HWND hBtnStop;
extern HWND hStaticStageStatus;
extern HWND hStaticCameraStatus;
extern HWND hStaticCameraView;
extern HWND hBtnConnectZ;
extern HWND hBtnDisconnectZ;
extern HWND hStaticZStatus;
extern HWND hComboPortZ;
extern HWND hBtnRefreshPortZ;
extern HWND hBtnMoveUpZ;
extern HWND hBtnMoveDownZ;
extern HWND hBtnStopZ;
extern HWND hSliderSpeedZ;
extern HWND hEditXDistance;
extern HWND hEditXSpeed;
extern HWND hBtnXPos;
extern HWND hBtnXNeg;
extern HWND hEditYDistance;
extern HWND hEditYSpeed;
extern HWND hBtnYPos;
extern HWND hBtnYNeg;

// 控件ID定义（可以保留在头文件中，也可单独放）
// 但为了避免重复，建议保留在 resource.h 或本头文件中
// 你原来的 #define 都在 cpp 中，可以移到头文件
// 
#define MAX_LOADSTRING 100

// 控件 ID 定义
#define IDC_COMBO_PORT          1001
#define IDC_BTN_REFRESH_PORT    1002
#define IDC_BTN_CONNECT_STAGE   1003
#define IDC_BTN_DISCONNECT_STAGE 1004
#define IDC_BTN_CONNECT_CAMERA  1005
#define IDC_BTN_DISCONNECT_CAMERA 1006
#define IDC_BTN_CAPTURE         1007

#define IDC_BTN_STOP            1013
#define IDC_STATIC_STAGE_STATUS 1014
#define IDC_STATIC_CAMERA_STATUS 1015
#define IDC_STATIC_CAMERA_VIEW  1017

#define IDC_COMBO_PORT_Z 1020
#define IDC_STATIC_Z_STATUS 1021
#define IDC_BTN_CONNECT_Z 1022
#define IDC_BTN_DISCONNECT_Z 1023
#define IDC_BTN_REFRESH_PORT_Z 1024
#define IDC_BTN_MOVEUP_Z 1025
#define IDC_BTN_MOVEDOWN_Z 1026
#define IDC_BTN_STOP_Z 1027
#define IDC_SLIDER_SPEED_Z 1028
#define IDC_STATIC_SPEED_VALUE_Z 1029

// X 轴
#define IDC_EDIT_X_SPEED        1201
#define IDC_EDIT_X_VALUE_SPEED  1202
#define IDC_EDIT_X_DISTANCE     1203
#define IDC_BTN_X_POS           1204
#define IDC_BTN_X_NEG           1205

#define IDC_BTN_X_POS2          2203
#define IDC_BTN_X_NEG2          2204
// Y 轴
#define IDC_EDIT_Y_SPEED        1211
#define IDC_EDIT_Y_VALUE_SPEED  1212
#define IDC_EDIT_Y_DISTANCE     1213
#define IDC_BTN_Y_POS           1214
#define IDC_BTN_Y_NEG           1215

#define IDC_BTN_Y_POS2          2213
#define IDC_BTN_Y_NEG2          2214

void CreateControls(HWND hWnd);
