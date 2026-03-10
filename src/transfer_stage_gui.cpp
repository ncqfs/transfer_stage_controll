// transfer_stage_gui.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "transfer_stage_gui.h"
#include "BluetoothMotorController.h"
#include <opencv2/opencv.hpp>
#include <uEye.h>
#include "VectorStage.h"
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <sstream>
#include <iomanip>
#include <windowsx.h>
#include <commctrl.h>
#include <iostream>

#pragma comment(lib, "comctl32.lib")  // 链接库


extern HANDLE pdv_hCom;

extern int x_min, x_max, y_min, y_max, z_min, z_max, l_min, l_max; //限位
extern int x_home, y_home, z_home, l_home; //零位状态
extern int x_pos, y_pos, z_pos, l_pos;     //位置值

const int MAX_POS_X = 60000;
const int MIN_POS_X = -115000;
const int MAX_POS_Y = 100000;
const int MIN_POS_Y = -85000;

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

// 定时器 ID
#define TIMER_CAMERA_UPDATE     1

// 全局变量:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// 控件句柄
HWND hComboPort;
HWND hBtnRefreshPort;
HWND hBtnConnectStage;
HWND hBtnDisconnectStage;
HWND hBtnConnectCamera;
HWND hBtnDisconnectCamera;
HWND hBtnCapture;
HWND hBtnStop;
HWND hStaticStageStatus;
HWND hStaticCameraStatus;
HWND hStaticCameraView;

HWND hBtnConnectZ;
HWND hBtnDisconnectZ;
HWND hStaticZStatus;
HWND hComboPortZ;
HWND hBtnRefreshPortZ;
HWND hBtnMoveUpZ;
HWND hBtnMoveDownZ;
HWND hBtnStopZ;
HWND hSliderSpeedZ;

// ===== 位移台 X/Y 控件句柄 =====
HWND hEditXDistance = nullptr;
HWND hEditXSpeed ;
HWND hBtnXPos;
HWND hBtnXNeg;
HWND hEditYDistance = nullptr;
HWND hEditYSpeed;
HWND hBtnYPos;
HWND hBtnYNeg;


// 相机相关全局变量
HIDS hCam = 0;
int camWidth = 0;
int camHeight = 0;
int camBitsPerPixel = 0;
int camColorMode = 0;
char* pCamMem = nullptr;
int camMemID = 0;
bool cameraConnected = false;
int photoCount = 0;
HBITMAP hCameraBitmap = nullptr;
std::atomic<bool> cameraRunning(false);

// 位移台相关全局变量
bool stageConnected = false;
int stageComPort = 3;   // 示例：COM3

// 位移台通道定义
constexpr int CHANNEL_X = 0;
constexpr int CHANNEL_Y = 1;

// 临界值
const int EPS = 5000;

// Z轴相关全局变量
BluetoothMotorController Zctrl ;
bool Zconnected = false;
bool zBtnHeld = false;
int zMoveDirection = 0; // 0 停止 1 上升 -1 下降


// 函数前向声明
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ButtonSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass,
    DWORD_PTR dwRefData);


void CreateControls(HWND hWnd);
void RefreshSerialPorts();
void ConnectStage();
void DisconnectStage();
void ConnectCamera();
void DisconnectCamera();
void UpdateCameraFrame();
void CaptureImage();
void StopStage();
void UpdateStageControlsState(bool enabled);
void UpdateCameraControlsState(bool enabled);
cv::Mat GetCurrentFrame();
void DisplayFrame(const cv::Mat& frame);
void MoveStage(int channel, bool positive);
void MoveSpeed(int channel, bool positive);
void RefreshSerialPortsZ();
void ConnectZ();
void DisconnectZ();
void UpdateZControlsState(bool enabled);
void StopZ();
void SpeedRefresh(LPARAM lParam);


void BoundaryCheck(int channel, bool positive, int& distance);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TRANSFERSTAGEGUI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TRANSFERSTAGEGUI));
    MSG msg;

    // 主消息循环
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRANSFERSTAGEGUI));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TRANSFERSTAGEGUI);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, L"相机与位移台控制系统 v1.0",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 1650, 750,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    // 创建控件
    CreateControls(hWnd);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

void CreateControls(HWND hWnd)
{
    HFONT hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");

    HFONT hTitleFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑");

    // 左侧 - 相机显示区域
    HWND hCameraTitle = CreateWindowW(L"STATIC", L"相机实时预览",
        WS_CHILD | WS_VISIBLE,
        20, 20, 700, 30, hWnd, nullptr, hInst, nullptr);
    SendMessage(hCameraTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

    hStaticCameraView = CreateWindowW(L"STATIC", L"相机未连接",
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE | WS_BORDER,
        20, 60, 700, 520, hWnd, (HMENU)IDC_STATIC_CAMERA_VIEW, hInst, nullptr);
    SendMessage(hStaticCameraView, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 相机控制按钮
    hBtnConnectCamera = CreateWindowW(L"BUTTON", L"连接相机",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 590, 120, 35, hWnd, (HMENU)IDC_BTN_CONNECT_CAMERA, hInst, nullptr);
    SendMessage(hBtnConnectCamera, WM_SETFONT, (WPARAM)hFont, TRUE);

    hBtnDisconnectCamera = CreateWindowW(L"BUTTON", L"断开相机",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        150, 590, 120, 35, hWnd, (HMENU)IDC_BTN_DISCONNECT_CAMERA, hInst, nullptr);
    SendMessage(hBtnDisconnectCamera, WM_SETFONT, (WPARAM)hFont, TRUE);
    //EnableWindow(hBtnDisconnectCamera, FALSE);

    hBtnCapture = CreateWindowW(L"BUTTON", L"拍照保存(S)",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        280, 590, 140, 35, hWnd, (HMENU)IDC_BTN_CAPTURE, hInst, nullptr);
    SendMessage(hBtnCapture, WM_SETFONT, (WPARAM)hFont, TRUE);
    //EnableWindow(hBtnCapture, FALSE);

    hStaticCameraStatus = CreateWindowW(L"STATIC", L"相机状态: 未连接",
        WS_CHILD | WS_VISIBLE,
        20, 635, 700, 25, hWnd, (HMENU)IDC_STATIC_CAMERA_STATUS, hInst, nullptr);
    SendMessage(hStaticCameraStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 右侧 - 位移台控制区域
    HWND hStageTitle = CreateWindowW(L"STATIC", L"位移台控制",
        WS_CHILD | WS_VISIBLE,
        750, 20, 400, 30, hWnd, nullptr, hInst, nullptr);
    SendMessage(hStageTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

    // 串口连接组
    CreateWindowW(L"BUTTON", L"位移台连接",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        750, 60, 420, 150, hWnd, nullptr, hInst, nullptr);

    CreateWindowW(L"STATIC", L"串口:",
        WS_CHILD | WS_VISIBLE,
        770, 90, 60, 25, hWnd, nullptr, hInst, nullptr);

    hComboPort = CreateWindowW(L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        830, 90, 200, 200, hWnd, (HMENU)IDC_COMBO_PORT, hInst, nullptr);
    SendMessage(hComboPort, WM_SETFONT, (WPARAM)hFont, TRUE);

    hBtnRefreshPort = CreateWindowW(L"BUTTON", L"刷新",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        1040, 85, 110, 30, hWnd, (HMENU)IDC_BTN_REFRESH_PORT, hInst, nullptr);
    SendMessage(hBtnRefreshPort, WM_SETFONT, (WPARAM)hFont, TRUE);

    hBtnConnectStage = CreateWindowW(L"BUTTON", L"连接位移台",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        770, 130, 180, 35, hWnd, (HMENU)IDC_BTN_CONNECT_STAGE, hInst, nullptr);
    SendMessage(hBtnConnectStage, WM_SETFONT, (WPARAM)hFont, TRUE);

    hBtnDisconnectStage = CreateWindowW(L"BUTTON", L"断开位移台",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        960, 130, 190, 35, hWnd, (HMENU)IDC_BTN_DISCONNECT_STAGE, hInst, nullptr);
    SendMessage(hBtnDisconnectStage, WM_SETFONT, (WPARAM)hFont, TRUE);
    //EnableWindow(hBtnDisconnectStage, FALSE);

    hStaticStageStatus = CreateWindowW(L"STATIC", L"位移台状态: 未连接",
        WS_CHILD | WS_VISIBLE,
        770, 175, 300, 25, hWnd, (HMENU)IDC_STATIC_STAGE_STATUS, hInst, nullptr);
    SendMessage(hStaticStageStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

    // X/Y轴点动控制
    CreateWindowW(L"BUTTON", L"X/Y轴点动控制",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        750, 230, 420, 140, hWnd, nullptr, hInst, nullptr);

    // ===== X 轴 =====
    CreateWindowW(L"STATIC", L"位移:",
        WS_CHILD | WS_VISIBLE,
        770, 260, 50, 25, hWnd, nullptr, hInst, nullptr);

    hEditXDistance = CreateWindowW(L"EDIT", L"10000",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        820, 260, 90, 28, hWnd,
        (HMENU)IDC_EDIT_X_DISTANCE, hInst, nullptr);

    CreateWindowW(L"BUTTON", L"+X",
        WS_CHILD | WS_VISIBLE,
        940, 260, 90, 28, hWnd,
        (HMENU)IDC_BTN_X_POS, hInst, nullptr);

    CreateWindowW(L"BUTTON", L"-X",
        WS_CHILD | WS_VISIBLE,
        1050, 260, 90, 28, hWnd,
        (HMENU)IDC_BTN_X_NEG, hInst, nullptr);

    // ===== Y 轴 =====
    //CreateWindowW(L"STATIC", L"Y轴 速度:",
    /*    WS_CHILD | WS_VISIBLE,
        770, 345, 80, 25, hWnd, nullptr, hInst, nullptr);

    hEditYSpeed = CreateWindowW(L"EDIT", L"1000",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        850, 343, 80, 28, hWnd,
        (HMENU)IDC_EDIT_Y_SPEED, hInst, nullptr);*/

    CreateWindowW(L"STATIC", L"位移:",
        WS_CHILD | WS_VISIBLE,
        770, 320, 50, 25, hWnd, nullptr, hInst, nullptr);

    hEditYDistance = CreateWindowW(L"EDIT", L"10000",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        820, 320, 90, 28, hWnd,
        (HMENU)IDC_EDIT_Y_DISTANCE, hInst, nullptr);

    CreateWindowW(L"BUTTON", L"+Y",
        WS_CHILD | WS_VISIBLE,
        940, 320, 90, 28, hWnd,
        (HMENU)IDC_BTN_Y_POS, hInst, nullptr);

    CreateWindowW(L"BUTTON", L"-Y",
        WS_CHILD | WS_VISIBLE,
        1050, 320, 90, 28, hWnd,
        (HMENU)IDC_BTN_Y_NEG, hInst, nullptr);

    // XY轴速度控制
    CreateWindowW(L"BUTTON", L"X/Y轴速度控制",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        750, 395, 420, 220, hWnd, nullptr, hInst, nullptr);

    hBtnXPos = CreateWindowW(L"BUTTON", L"+X",
        WS_CHILD | WS_VISIBLE,
        940, 425, 90, 28, hWnd,
        (HMENU)IDC_BTN_X_POS2, hInst, nullptr);


    hBtnXNeg = CreateWindowW(L"BUTTON", L"-X",
        WS_CHILD | WS_VISIBLE,
        1050, 425, 90, 28, hWnd,
        (HMENU)IDC_BTN_X_NEG2, hInst, nullptr);

    // 绑定子类窗口化
    SetWindowSubclass(hBtnXPos, ButtonSubclassProc, IDC_BTN_X_POS2, 0);
    SetWindowSubclass(hBtnXNeg, ButtonSubclassProc, IDC_BTN_X_NEG2, 0);

    // 速度滑块
    CreateWindowW(L"STATIC", L"速度:",
        WS_CHILD | WS_VISIBLE,
        765, 465, 50, 25, hWnd, nullptr, hInst, nullptr);

    hEditXSpeed = CreateWindowW(
        TRACKBAR_CLASSW,           // 滑动条类名
        L"",                       // 文本（通常为空）
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS | TBS_TOOLTIPS,
        815, 465,                  // 位置
        300, 40,                   // 宽度和高度
        hWnd,                // 父窗口
        (HMENU)IDC_EDIT_X_SPEED,   // 控件ID
        hInst,                     // 实例句柄
        nullptr
    );
    SendMessageW(hEditXSpeed, TBM_SETRANGEMIN, FALSE, 0);      // 最小值 0
    SendMessageW(hEditXSpeed, TBM_SETRANGEMAX, TRUE, 100);     // 最大值 100
    SendMessageW(hEditXSpeed, TBM_SETPOS, TRUE, 0);           // 初始位置 50
    SendMessageW(hEditXSpeed, TBM_SETTICFREQ, 10, 0);          // 每10个单位一个刻度
    SendMessageW(hEditXSpeed, TBM_SETPAGESIZE, 0, 10);



    // 速度显示
    CreateWindowW(L"STATIC", L"0",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        1115, 465, 40, 25, hWnd, (HMENU)IDC_EDIT_X_VALUE_SPEED, hInst, nullptr);

    //Y轴
    hBtnYPos = CreateWindowW(L"BUTTON", L"+Y",
        WS_CHILD | WS_VISIBLE,
        940, 515, 90, 28, hWnd,
        (HMENU)IDC_BTN_Y_POS2, hInst, nullptr);


    hBtnYNeg = CreateWindowW(L"BUTTON", L"-Y",
        WS_CHILD | WS_VISIBLE,
        1050, 515, 90, 28, hWnd,
        (HMENU)IDC_BTN_Y_NEG2, hInst, nullptr);

    // 绑定子类窗口化
    SetWindowSubclass(hBtnYPos, ButtonSubclassProc, IDC_BTN_Y_POS2, 0);
    SetWindowSubclass(hBtnYNeg, ButtonSubclassProc, IDC_BTN_Y_NEG2, 0);

    // 速度滑块
    CreateWindowW(L"STATIC", L"速度:",
        WS_CHILD | WS_VISIBLE,
        765, 555, 50, 25, hWnd, nullptr, hInst, nullptr);

    hEditYSpeed = CreateWindowW(
        TRACKBAR_CLASSW,           // 滑动条类名
        L"",                       // 文本（通常为空）
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS | TBS_TOOLTIPS,
        815, 555,                    // 位置
        300, 40,                   // 宽度和高度
        hWnd,                // 父窗口
        (HMENU)IDC_EDIT_Y_SPEED,   // 控件ID
        hInst,                     // 实例句柄
        nullptr
    );
    SendMessageW(hEditYSpeed, TBM_SETRANGEMIN, FALSE, 0);      // 最小值 0
    SendMessageW(hEditYSpeed, TBM_SETRANGEMAX, TRUE, 100);     // 最大值 100
    SendMessageW(hEditYSpeed, TBM_SETPOS, TRUE, 0);           // 初始位置 50
    SendMessageW(hEditYSpeed, TBM_SETTICFREQ, 10, 0);          // 每10个单位一个刻度
    SendMessageW(hEditYSpeed, TBM_SETPAGESIZE, 0, 10);


    // 速度显示
    CreateWindowW(L"STATIC", L"0",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        1115, 555, 40, 25, hWnd, (HMENU)IDC_EDIT_Y_VALUE_SPEED, hInst, nullptr);

    // 停止按钮
    hBtnStop = CreateWindowW(L"BUTTON", L"停止位移台",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        750, 630, 420, 40, hWnd, (HMENU)IDC_BTN_STOP, hInst, nullptr);
    SendMessage(hBtnStop, WM_SETFONT, (WPARAM)hFont, TRUE);
    //EnableWindow(hBtnStop, FALSE);

    // z轴串口连接

    HWND hFocusTitle = CreateWindowW(L"STATIC", L"准焦",
        WS_CHILD | WS_VISIBLE,
        1200, 20, 400, 30, hWnd, nullptr, hInst, nullptr);
    SendMessage(hFocusTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);


    CreateWindowW(L"BUTTON", L"Z轴连接",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        1200, 60, 420, 150, hWnd, nullptr, hInst, nullptr);

    CreateWindowW(L"STATIC", L"串口:",
        WS_CHILD | WS_VISIBLE,
        1220, 90, 60, 25, hWnd, nullptr, hInst, nullptr);

    hComboPortZ = CreateWindowW(L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        1280, 90, 200, 200, hWnd, (HMENU)IDC_COMBO_PORT_Z, hInst, nullptr);
    SendMessage(hComboPortZ, WM_SETFONT, (WPARAM)hFont, TRUE);

    hBtnRefreshPortZ = CreateWindowW(L"BUTTON", L"刷新",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        1490, 85, 110, 30, hWnd, (HMENU)IDC_BTN_REFRESH_PORT_Z, hInst, nullptr);
    SendMessage(hBtnRefreshPortZ, WM_SETFONT, (WPARAM)hFont, TRUE);

    hBtnConnectZ = CreateWindowW(L"BUTTON", L"连接Z轴",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        1220, 130, 180, 35, hWnd, (HMENU)IDC_BTN_CONNECT_Z, hInst, nullptr);
    SendMessage(hBtnConnectZ, WM_SETFONT, (WPARAM)hFont, TRUE);

    hBtnDisconnectZ = CreateWindowW(L"BUTTON", L"断开Z轴",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        1410, 130, 190, 35, hWnd, (HMENU)IDC_BTN_DISCONNECT_Z, hInst, nullptr);
    SendMessage(hBtnDisconnectZ, WM_SETFONT, (WPARAM)hFont, TRUE);
    //EnableWindow(hBtnDisconnectZ, FALSE);

    hStaticZStatus = CreateWindowW(L"STATIC", L"Z轴状态: 未连接",
        WS_CHILD | WS_VISIBLE,
        1220, 175, 300, 25, hWnd, (HMENU)IDC_STATIC_Z_STATUS, hInst, nullptr);
    SendMessage(hStaticZStatus, WM_SETFONT, (WPARAM)hFont, TRUE);

    // z轴控制
    CreateWindowW(L"BUTTON", L"Z轴电动控制",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        1200, 230, 420, 140, hWnd, nullptr, hInst, nullptr);

    hBtnMoveUpZ = CreateWindowW(L"BUTTON", L"+Z",
        WS_CHILD | WS_VISIBLE,
        1390, 260, 90, 28, hWnd,
        (HMENU)IDC_BTN_MOVEUP_Z, hInst, nullptr);


    hBtnMoveDownZ = CreateWindowW(L"BUTTON", L"-Z",
        WS_CHILD | WS_VISIBLE,
        1500, 260, 90, 28, hWnd,
        (HMENU)IDC_BTN_MOVEDOWN_Z, hInst, nullptr);

    // 绑定子类窗口化
    SetWindowSubclass(hBtnMoveUpZ, ButtonSubclassProc, IDC_BTN_MOVEUP_Z, 0);
    SetWindowSubclass(hBtnMoveDownZ, ButtonSubclassProc, IDC_BTN_MOVEDOWN_Z, 0);


    hBtnStopZ = CreateWindowW(L"BUTTON", L"停止Z轴",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        1200, 390, 420, 40, hWnd, (HMENU)IDC_BTN_STOP_Z, hInst, nullptr);
    SendMessage(hBtnStopZ, WM_SETFONT, (WPARAM)hFont, TRUE);
 

    // 速度滑块
    CreateWindowW(L"STATIC", L"速度:",
        WS_CHILD | WS_VISIBLE,
        1215, 300, 50, 25, hWnd, nullptr, hInst, nullptr);

    hSliderSpeedZ = CreateWindowW(
        TRACKBAR_CLASSW,           // 滑动条类名
        L"",                       // 文本（通常为空）
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS | TBS_TOOLTIPS,
        1265, 300,                    // 位置
        300, 40,                   // 宽度和高度
        hWnd,                // 父窗口
        (HMENU)IDC_SLIDER_SPEED_Z,   // 控件ID
        hInst,                     // 实例句柄
        nullptr
    );
    SendMessageW(hSliderSpeedZ, TBM_SETRANGEMIN, FALSE, 0);      // 最小值 0
    SendMessageW(hSliderSpeedZ, TBM_SETRANGEMAX, TRUE, 100);     // 最大值 100
    SendMessageW(hSliderSpeedZ, TBM_SETPOS, TRUE, 0);           // 初始位置 50
    SendMessageW(hSliderSpeedZ, TBM_SETTICFREQ, 10, 0);          // 每10个单位一个刻度
    SendMessageW(hSliderSpeedZ, TBM_SETPAGESIZE, 0, 10);



    // 速度显示
    CreateWindowW(L"STATIC", L"0",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        1565, 300, 40, 25, hWnd, (HMENU)IDC_STATIC_SPEED_VALUE_Z, hInst, nullptr);

    // 刷新串口列表
    RefreshSerialPorts();
    RefreshSerialPortsZ();
	//禁用控件
    UpdateStageControlsState(false);
    UpdateZControlsState(false);
    UpdateCameraControlsState(false);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDC_BTN_REFRESH_PORT:
            RefreshSerialPorts();
            break;
        case IDC_BTN_CONNECT_STAGE:
            ConnectStage();
            break;
        case IDC_BTN_DISCONNECT_STAGE:
            DisconnectStage();
            break;
        case IDC_BTN_CONNECT_CAMERA:
            ConnectCamera();
            break;
        case IDC_BTN_DISCONNECT_CAMERA:
            DisconnectCamera();
            break;
        case IDC_BTN_CAPTURE:
            CaptureImage();
            break;
        case IDC_BTN_STOP:
            StopStage();
            break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDC_BTN_X_POS: MoveStage(CHANNEL_X, true);  break;
        case IDC_BTN_X_NEG: MoveStage(CHANNEL_X, false); break;
        case IDC_BTN_Y_POS: MoveStage(CHANNEL_Y, true);  break;
        case IDC_BTN_Y_NEG: MoveStage(CHANNEL_Y, false); break;
        case IDC_BTN_CONNECT_Z: 
            ConnectZ();
            break;
        case IDC_BTN_DISCONNECT_Z:
            DisconnectZ();
            break;
        case IDC_BTN_REFRESH_PORT_Z:
            RefreshSerialPortsZ();
            break;
        case IDC_BTN_STOP_Z:
            StopZ();
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_TIMER:
        if (wParam == TIMER_CAMERA_UPDATE)
        {
            UpdateCameraFrame();
        }
        break;

    case WM_KEYDOWN:
        if (wParam == 'S' && cameraConnected)
        {
            CaptureImage();
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_HSCROLL:
    {
        // 处理滑动条消息，更新速度显示
		SpeedRefresh(lParam);
        break;
    }
    case WM_DESTROY:
        DisconnectCamera();
        DisconnectStage();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK ButtonSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) 
{
    HWND hParent = GetParent(hWnd);
    short speed = 0;
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
        // 鼠标左键按下
    {
        switch (uIdSubclass)
        {
		case IDC_BTN_MOVEUP_Z:
            speed = (short)SendMessageW(hSliderSpeedZ, TBM_GETPOS, 0, 0);
            zMoveDirection = 1;
            Zctrl.rotateSpeed(speed);
            break;
		case IDC_BTN_MOVEDOWN_Z:
            speed = -(short)SendMessageW(hSliderSpeedZ, TBM_GETPOS, 0, 0);
            zMoveDirection = -1;
            Zctrl.rotateSpeed(speed);
            break;
		case IDC_BTN_X_POS2:
			MoveSpeed(CHANNEL_X, 1);
            break;
		case IDC_BTN_X_NEG2:
            MoveSpeed(CHANNEL_X, 0);
            break;
		case IDC_BTN_Y_POS2:
            MoveSpeed(CHANNEL_Y, 1);
			break;
        case IDC_BTN_Y_NEG2:
            MoveSpeed(CHANNEL_Y, 0);
            break;
        }
        // 标记按钮被按住
        zBtnHeld = true;
        // 捕获鼠标，确保能收到 WM_LBUTTONUP 消息
        SetCapture(hWnd);
        // 改变按钮外观（可选）
        SendMessageW(hWnd, BM_SETSTATE, TRUE, 0);
    }
    break;

    case WM_LBUTTONUP:
        // 鼠标左键松开
        if (zBtnHeld)
        {
            // 停止移动
            Zctrl.stopMotor();
            set_speed(CHANNEL_X, 0);
            set_speed(CHANNEL_Y, 0);

            // 重置状态
            zBtnHeld = false;
            zMoveDirection = 0;

            // 释放鼠标捕获
            ReleaseCapture();

            // 恢复按钮外观（可选）
            SendMessageW(hWnd, BM_SETSTATE, FALSE, 0);
        }
        break;

    case WM_MOUSEMOVE:
        // 鼠标移动

        if (zBtnHeld)
        {
            // 检查鼠标是否还在按钮区域内
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            RECT rc;
            GetClientRect(hWnd, &rc);

            // 如果鼠标移出按钮区域，也停止
            if (!PtInRect(&rc, pt))
            {
                // 但不要立即停止，等待鼠标松开
                // 这样可以允许用户在按钮外松开鼠标
            }
        }
        break;

    
    case WM_NCDESTROY:
        // 窗口被销毁时，移除子类化
        RemoveWindowSubclass(hWnd, ButtonSubclassProc, uIdSubclass);
        break;
    }

    // 调用原始窗口过程，确保按钮的正常行为
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// 实现功能函数
void RefreshSerialPorts()
{
    SendMessage(hComboPort, CB_RESETCONTENT, 0, 0);

    std::vector<SerialPortInfo> ports;
    if (enumDetailsSerialPorts(ports) && !ports.empty())
    {
        for (const auto& port : ports)
        {
            std::wstring wPortName(port.portName.begin(), port.portName.end());
            SendMessage(hComboPort, CB_ADDSTRING, 0, (LPARAM)wPortName.c_str());
        }
        SendMessage(hComboPort, CB_SETCURSEL, 0, 0);
    }
    else
    {
        SendMessage(hComboPort, CB_ADDSTRING, 0, (LPARAM)L"未找到串口");
        SendMessage(hComboPort, CB_SETCURSEL, 0, 0);
        MessageBoxW(GetParent(hComboPort), L"未找到可用串口,请检查设备连接", L"提示", MB_OK | MB_ICONINFORMATION);
    }
}

void ConnectStage()
{
    // 实现位移台连接逻辑
    int index = SendMessage(hComboPort, CB_GETCURSEL, 0, 0);
    if (index == CB_ERR)
    {
        MessageBoxW(GetParent(hComboPort), L"请选择串口", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    wchar_t portName[256];
    SendMessage(hComboPort, CB_GETLBTEXT, index, (LPARAM)portName);

    if (wcscmp(portName, L"未找到串口") == 0)
    {
        MessageBoxW(GetParent(hComboPort), L"没有可用的串口", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    std::wstring wPortName(portName);

    // 使用 VectorStage API 连接位移台
    open_com(portName);
    if (pdv_hCom != INVALID_HANDLE_VALUE)
    {
        init_comm();
        stageConnected = true;
        SetWindowTextW(hStaticStageStatus, L"位移台状态: 已连接");
        UpdateStageControlsState(true);
        std::wstring msg = L"位移台连接成功！\n\n串口: " + wPortName;
        MessageBoxW(GetParent(hComboPort), msg.c_str(), L"成功", MB_OK | MB_ICONINFORMATION);

        // 设置最大速度
        set_max_speed(CHANNEL_X, 1000);
        set_max_speed(CHANNEL_Y, 1000);

        // 获取位移台基础状态
        get_stage_state();
        //// 打印
        //std::wstringstream wss;
        //wss << L"位移台状态\n\n"
        //    << L"x_pos: " << x_pos << L"; " << L"y_pos: " << y_pos << std::endl;
        //MessageBoxW(GetParent(hBtnConnectStage), wss.str().c_str(), L"确定", MB_OK | MB_ICONINFORMATION);
        //
        int _dist = 0;
        BoundaryCheck(CHANNEL_X, true, _dist);
        _dist = 0;
        BoundaryCheck(CHANNEL_Y, true, _dist);


    }
}

void DisconnectStage()
{
    // 位移台断开逻辑
     if (stageConnected)
     {
         // 停止位移台运动
         StopStage();
         // 关闭串口连接
         close_comm();
         stageConnected = false;
         SetWindowTextW(hStaticStageStatus, L"位移台状态: 未连接");
         UpdateStageControlsState(false);
     }

    stageConnected = false;
    SetWindowTextW(hStaticStageStatus, L"位移台状态: 未连接");
    UpdateStageControlsState(false);
}

void ConnectCamera()
{
    INT nRet = is_InitCamera(&hCam, NULL);
    if (nRet != IS_SUCCESS)
    {
        MessageBoxW(GetParent(hBtnConnectCamera),
            L"相机初始化失败！\n\n可能的原因:\n1. 相机未连接\n2. 驱动未安装\n3. 相机已被其他程序占用\n4. USB接口问题",
            L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    // 设置颜色模式
    nRet = is_SetColorMode(hCam, IS_CM_BGRA8_PACKED);
    if (nRet != IS_SUCCESS)
    {
        nRet = is_SetColorMode(hCam, IS_CM_BGR8_PACKED);
        if (nRet != IS_SUCCESS)
        {
            is_ExitCamera(hCam);
            MessageBoxW(GetParent(hBtnConnectCamera), L"无法设置相机颜色模式", L"错误", MB_OK | MB_ICONERROR);
            return;
        }
        camColorMode = IS_CM_BGR8_PACKED;
    }
    else
    {
        camColorMode = IS_CM_BGRA8_PACKED;
    }

    SENSORINFO sInfo;
    is_GetSensorInfo(hCam, &sInfo);
    camWidth = sInfo.nMaxWidth;
    camHeight = sInfo.nMaxHeight;

    IS_RECT rect = { 0, 0, camWidth, camHeight };
    is_AOI(hCam, IS_AOI_IMAGE_SET_AOI, (void*)&rect, sizeof(rect));

    camBitsPerPixel = (camColorMode == IS_CM_BGRA8_PACKED) ? 32 : 24;

    nRet = is_AllocImageMem(hCam, camWidth, camHeight, camBitsPerPixel, &pCamMem, &camMemID);
    if (nRet != IS_SUCCESS)
    {
        is_ExitCamera(hCam);
        MessageBoxW(GetParent(hBtnConnectCamera), L"无法分配相机内存", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    is_SetImageMem(hCam, pCamMem, camMemID);

    nRet = is_CaptureVideo(hCam, IS_WAIT);
    if (nRet != IS_SUCCESS)
    {
        is_FreeImageMem(hCam, pCamMem, camMemID);
        is_ExitCamera(hCam);
        MessageBoxW(GetParent(hBtnConnectCamera), L"无法启动视频采集", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    cameraConnected = true;
    cameraRunning = true;

    std::wstringstream wss;
    wss << L"相机状态: 已连接 (" << camWidth << L"x" << camHeight << L" @ " << camBitsPerPixel << L"位)";
    SetWindowTextW(hStaticCameraStatus, wss.str().c_str());

    UpdateCameraControlsState(true);

    // 启动定时器更新相机画面
    SetTimer(GetParent(hBtnConnectCamera), TIMER_CAMERA_UPDATE, 16, nullptr);

    std::wstringstream msg;
    msg << L"相机连接成功！\n\n分辨率: " << camWidth << L"x" << camHeight
        << L"\n颜色模式: " << (camColorMode == IS_CM_BGRA8_PACKED ? L"BGRA8" : L"BGR8")
        << L"\n位深: " << camBitsPerPixel << L"位";
    MessageBoxW(GetParent(hBtnConnectCamera), msg.str().c_str(), L"成功", MB_OK | MB_ICONINFORMATION);
}

void DisconnectCamera()
{
    if (cameraConnected)
    {
        cameraRunning = false;
        KillTimer(GetParent(hBtnDisconnectCamera), TIMER_CAMERA_UPDATE);

        is_StopLiveVideo(hCam, IS_WAIT);
        if (pCamMem != nullptr)
        {
            is_FreeImageMem(hCam, pCamMem, camMemID);
        }
        is_ExitCamera(hCam);

        if (hCameraBitmap)
        {
            DeleteObject(hCameraBitmap);
            hCameraBitmap = nullptr;
        }

        hCam = 0;
        pCamMem = nullptr;
        cameraConnected = false;

        SetWindowTextW(hStaticCameraView, L"相机未连接");
        InvalidateRect(hStaticCameraView, nullptr, TRUE);
        SetWindowTextW(hStaticCameraStatus, L"相机状态: 未连接");
        UpdateCameraControlsState(false);
    }
}

cv::Mat GetCurrentFrame()
{
    if (!cameraConnected) return cv::Mat();

    char* pLastMem = nullptr;
    if (is_GetImageMem(hCam, (void**)&pLastMem) != IS_SUCCESS || pLastMem == nullptr)
    {
        return cv::Mat();
    }

    int openCVType = (camColorMode == IS_CM_BGRA8_PACKED) ? CV_8UC4 : CV_8UC3;
    INT nPixelSize = camBitsPerPixel / 8;
    cv::Mat frame(camHeight, camWidth, openCVType, pLastMem, camWidth * nPixelSize);

    if (frame.empty()) return cv::Mat();

    cv::Mat displayFrame;
    if (camColorMode == IS_CM_BGRA8_PACKED)
    {
        cv::cvtColor(frame, displayFrame, cv::COLOR_BGRA2BGR);
    }
    else
    {
        displayFrame = frame.clone();
    }

    return displayFrame;
}

void DisplayFrame(const cv::Mat& frame)
{
    if (frame.empty() || !IsWindow(hStaticCameraView)) return;

    // 获取显示区域大小
    RECT rect;
    GetClientRect(hStaticCameraView, &rect);
    int displayWidth = rect.right - rect.left;
    int displayHeight = rect.bottom - rect.top;

    // 缩放图像以适应显示区域（保持宽高比）
    cv::Mat resized;
    double scale = std::min((double)displayWidth / frame.cols, (double)displayHeight / frame.rows);
    int newWidth = static_cast<int>(frame.cols * scale);
    int newHeight = static_cast<int>(frame.rows * scale);
    cv::resize(frame, resized, cv::Size(newWidth, newHeight));

    cv::Mat bgra;
    cv::cvtColor(resized, bgra, cv::COLOR_BGR2BGRA);

    // 创建 BITMAP
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bgra.cols;
    bmi.bmiHeader.biHeight = -bgra.rows; // 负值表示自顶向下
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdc = GetDC(hStaticCameraView);

    if (hCameraBitmap)
    {
        DeleteObject(hCameraBitmap);
    }

    hCameraBitmap = CreateDIBitmap(hdc, &bmi.bmiHeader, CBM_INIT, bgra.data, &bmi, DIB_RGB_COLORS);

    HDC hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, hCameraBitmap);

    // 计算居中位置
    int offsetX = (displayWidth - newWidth) / 2;
    int offsetY = (displayHeight - newHeight) / 2;

    BitBlt(hdc, offsetX, offsetY, newWidth, newHeight, hdcMem, 0, 0, SRCCOPY);

    DeleteDC(hdcMem);
    ReleaseDC(hStaticCameraView, hdc);
}

void UpdateCameraFrame()
{
    if (!cameraConnected || !cameraRunning) return;

    INT nRet = is_FreezeVideo(hCam, IS_WAIT);
    if (nRet == IS_SUCCESS)
    {
        cv::Mat frame = GetCurrentFrame();
        if (!frame.empty())
        {
            DisplayFrame(frame);
        }
    }
}

void CaptureImage()
{
    if (!cameraConnected)
    {
        MessageBoxW(GetParent(hBtnCapture), L"相机未连接", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    cv::Mat frame = GetCurrentFrame();
    if (frame.empty())
    {
        MessageBoxW(GetParent(hBtnCapture), L"无法获取图像", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    std::ostringstream oss;
    oss << "photo_" << std::setw(4) << std::setfill('0') << photoCount++ << ".jpg";
    std::string filename = oss.str();

    if (cv::imwrite(filename, frame))
    {
        std::wstringstream wss;
        wss << L"图像已保存！\n\n文件名: " << filename.c_str()
            << L"\n分辨率: " << camWidth << L"x" << camHeight;
        MessageBoxW(GetParent(hBtnCapture), wss.str().c_str(), L"成功", MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        MessageBoxW(GetParent(hBtnCapture), L"图像保存失败", L"错误", MB_OK | MB_ICONERROR);
    }
}

void StopStage()
{
    if (!stageConnected)
    {
        MessageBoxW(GetParent(hBtnStop), L"位移台未连接", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    set_distance(CHANNEL_X, 0);
    set_distance(CHANNEL_Y, 0);
    set_speed(CHANNEL_X, 0);
    set_speed(CHANNEL_Y, 0);

    // 实现停止位移台运动的逻辑
    // if (停止成功)
    // {
    //     MessageBoxW(GetParent(hBtnStop), L"位移台已停止", L"成功", MB_OK | MB_ICONINFORMATION);
    // }
    // else
    // {
    //     MessageBoxW(GetParent(hBtnStop), L"停止位移台失败", L"错误", MB_OK | MB_ICONERROR);
    // }
}

void UpdateStageControlsState(bool enabled)
{
    EnableWindow(hComboPort, !enabled);
    EnableWindow(hBtnRefreshPort, !enabled);
    EnableWindow(hBtnConnectStage, !enabled);
    EnableWindow(hBtnDisconnectStage, enabled);
    EnableWindow(hBtnStop, enabled);

    // 启用/禁用位移台控制按钮
    EnableWindow(hEditXDistance, enabled);
    EnableWindow(hEditYDistance, enabled);
    EnableWindow(hBtnXPos, enabled);
	EnableWindow(hBtnXNeg, enabled);
	EnableWindow(hBtnYPos, enabled);
	EnableWindow(hBtnYNeg, enabled);
    EnableWindow(hEditXSpeed, enabled);
	EnableWindow(hEditYSpeed, enabled);


    HWND hBtnXPos = GetDlgItem(GetParent(hBtnStop), IDC_BTN_X_POS);
    HWND hBtnXNeg = GetDlgItem(GetParent(hBtnStop), IDC_BTN_X_NEG);
    HWND hBtnYPos = GetDlgItem(GetParent(hBtnStop), IDC_BTN_Y_POS);
    HWND hBtnYNeg = GetDlgItem(GetParent(hBtnStop), IDC_BTN_Y_NEG);

    if (hBtnXPos) EnableWindow(hBtnXPos, enabled);
    if (hBtnXNeg) EnableWindow(hBtnXNeg, enabled);
    if (hBtnYPos) EnableWindow(hBtnYPos, enabled);
    if (hBtnYNeg) EnableWindow(hBtnYNeg, enabled);
}

void UpdateCameraControlsState(bool enabled)
{
    EnableWindow(hBtnConnectCamera, !enabled);
    EnableWindow(hBtnDisconnectCamera, enabled);
    EnableWindow(hBtnCapture, enabled);
}

void MoveStage(int channel, bool positive)
{
    if (!stageConnected)
    {
        MessageBoxW(nullptr, L"位移台未连接", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    wchar_t buf[64];
    int distance = 0;

    // 获取位移台状态
    get_stage_state();
    /*std::wstringstream wss;
    wss << L"位移台状态\n\n"
        << L"x_max: " << x_max << L"; " << L"x_min: " << x_min << std::endl
        << L"y_max: " << y_max << L"; " << L"y_min: " << y_min << std::endl
        << L"x_home: " << x_home << L"; " << L"y_home: " << y_home << std::endl
        << L"x_pos: " << x_pos << L"; " << L"y_pos: " << y_pos << std::endl;
    MessageBoxW(GetParent(hBtnStop), wss.str().c_str(), L"确定", MB_OK | MB_ICONINFORMATION);*/

    if (channel == CHANNEL_X)
    {
        GetWindowTextW(hEditXDistance, buf, 64);
        distance = _wtoi(buf);
    }
    else if (channel == CHANNEL_Y)
    {
        GetWindowTextW(hEditYDistance, buf, 64);
        distance = _wtoi(buf);
    }

    BoundaryCheck(channel, positive, distance);

    if (!positive)
        distance = -distance; 

    // 实现位移台移动逻辑
    // 使用 VectorStage API:
    set_distance(channel, distance);
}

void MoveSpeed(int channel, bool positive)
{
    //if (!stageConnected)
    //{
    //    MessageBoxW(nullptr, L"位移台未连接", L"错误", MB_OK | MB_ICONERROR);
    //    return;
    //}
    
    int speed = 0;

    // 获取位移台状态
    get_stage_state();
    /*std::wstringstream wss;
    wss << L"位移台状态\n\n"
        << L"x_max: " << x_max << L"; " << L"x_min: " << x_min << std::endl
        << L"y_max: " << y_max << L"; " << L"y_min: " << y_min << std::endl
        << L"x_home: " << x_home << L"; " << L"y_home: " << y_home << std::endl
        << L"x_pos: " << x_pos << L"; " << L"y_pos: " << y_pos << std::endl;
    MessageBoxW(GetParent(hBtnStop), wss.str().c_str(), L"确定", MB_OK | MB_ICONINFORMATION);*/

    if (channel == CHANNEL_X)
    {
        speed = (short)SendMessageW(hEditXSpeed, TBM_GETPOS, 0, 0) + 1;
    }
    else if (channel == CHANNEL_Y)
    {
        speed = (short)SendMessageW(hEditYSpeed, TBM_GETPOS, 0, 0) + 1;
    }


    if (!positive)
        speed = -speed;

    // 实现位移台移动逻辑
    // 使用 VectorStage API:
    set_speed(channel, speed);
}

void BoundaryCheck(int channel, bool positive, int& distance)
{
    HWND hBtnXPos = GetDlgItem(GetParent(hBtnStop), IDC_BTN_X_POS);
    HWND hBtnXNeg = GetDlgItem(GetParent(hBtnStop), IDC_BTN_X_NEG);
    HWND hBtnYPos = GetDlgItem(GetParent(hBtnStop), IDC_BTN_Y_POS);
    HWND hBtnYNeg = GetDlgItem(GetParent(hBtnStop), IDC_BTN_Y_NEG);

    if (channel == CHANNEL_X) {
        int predX = x_pos + (positive ? 1 : -1) * distance;
        if (predX < MIN_POS_X + EPS) {
            distance = x_pos - (MIN_POS_X + EPS);
            EnableWindow(hBtnXNeg, false);
        }
        else if (predX > MAX_POS_X - EPS) {
            distance = MAX_POS_X - EPS - x_pos;
            EnableWindow(hBtnXPos, false);
        }
        else {
            EnableWindow(hBtnXNeg, true);
            EnableWindow(hBtnXPos, true);
        }
    }
    else {
        int predY = y_pos + (positive ? 1 : -1) * distance;
        if (predY < MIN_POS_Y + EPS) {
            distance = y_pos - (MIN_POS_Y + EPS);
            EnableWindow(hBtnYNeg, false);
        }
        else if (predY > MAX_POS_Y - EPS) {
            distance = MAX_POS_Y - EPS - y_pos;
            EnableWindow(hBtnYPos, false);
        }
        else {
            EnableWindow(hBtnYNeg, true);
            EnableWindow(hBtnYPos, true);
        }
    }
}


void RefreshSerialPortsZ()
{
    SendMessage(hComboPortZ, CB_RESETCONTENT, 0, 0);

    std::vector<SerialPortInfo> ports;
    if (enumDetailsSerialPorts(ports) && !ports.empty())
    {
        for (const auto& port : ports)
        {
            std::wstring wPortName(port.portName.begin(), port.portName.end());
            SendMessage(hComboPortZ, CB_ADDSTRING, 0, (LPARAM)wPortName.c_str());
        }
        SendMessage(hComboPortZ, CB_SETCURSEL, 0, 0);
    }
    else
    {
        SendMessage(hComboPortZ, CB_ADDSTRING, 0, (LPARAM)L"未找到串口");
        SendMessage(hComboPortZ, CB_SETCURSEL, 0, 0);
        MessageBoxW(GetParent(hComboPortZ), L"未找到可用串口,请检查设备连接", L"提示", MB_OK | MB_ICONINFORMATION);
    }
}

void ConnectZ()
{
    // 实现Z轴连接逻辑
    int index = SendMessage(hComboPortZ, CB_GETCURSEL, 0, 0);
    if (index == CB_ERR)
    {
        MessageBoxW(GetParent(hComboPortZ), L"请选择串口", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    wchar_t portName[256];
    SendMessage(hComboPortZ, CB_GETLBTEXT, index, (LPARAM)portName);

    if (wcscmp(portName, L"未找到串口") == 0)
    {
        MessageBoxW(GetParent(hComboPortZ), L"没有可用的串口", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    std::wstring wPortName(portName);
    std::string PortNameStr = wstringToString(wPortName);

    Zctrl.connect(PortNameStr);

    Zconnected = true;
    SetWindowTextW(hStaticZStatus, L"Z轴状态: 已连接");
    UpdateZControlsState(true);
    std::wstring msg = L"Z轴连接成功！\n\n串口: " + wPortName + L"\n\n请检查蓝牙指示灯：\n常亮：已连接\n闪烁：未连接";
    MessageBoxW(GetParent(hComboPortZ), msg.c_str(), L"成功", MB_OK | MB_ICONINFORMATION);
}

void DisconnectZ()
{
    // 位移台断开逻辑
    if (Zconnected)
    {
        // 停止位移台运动
        Zctrl.stopMotor();
        // 关闭串口连接
        Zctrl.disconnect();
        Zconnected = false;
        SetWindowTextW(hStaticZStatus, L"Z轴台状态: 未连接");
        UpdateZControlsState(false);
    }

    Zconnected = false;
    SetWindowTextW(hStaticZStatus, L"位移台状态: 未连接");
    UpdateZControlsState(false);
}

void UpdateZControlsState(bool enabled)
{
    EnableWindow(hComboPortZ, !enabled);
    EnableWindow(hBtnRefreshPortZ, !enabled);
    EnableWindow(hBtnConnectZ, !enabled);
    EnableWindow(hBtnDisconnectZ, enabled);
    EnableWindow(hBtnStopZ, enabled);
    EnableWindow(hBtnMoveUpZ, enabled);
    EnableWindow(hBtnMoveDownZ, enabled);
    EnableWindow(hSliderSpeedZ, enabled);
}



void StopZ()
{
    if (!Zconnected)
    {
        MessageBoxW(GetParent(hBtnStopZ), L"Z轴未连接", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    Zctrl.stopMotor();
}

void SpeedRefresh(LPARAM lParam) {
    HWND hCtrl = (HWND)lParam;
    int ctrlId = GetDlgCtrlID(hCtrl);
    int speed = 0;
    HWND hStaticSpeed = 0;
    switch (ctrlId) {
    case IDC_SLIDER_SPEED_Z:
        speed = (int)SendMessageW(hSliderSpeedZ, TBM_GETPOS, 0, 0);
        hStaticSpeed = GetDlgItem(GetParent(hSliderSpeedZ), IDC_STATIC_SPEED_VALUE_Z);
        break;
    case IDC_EDIT_X_SPEED:
        speed = (int)SendMessageW(hEditXSpeed, TBM_GETPOS, 0, 0);
        hStaticSpeed = GetDlgItem(GetParent(hEditXSpeed), IDC_EDIT_X_VALUE_SPEED);
        break;
    case IDC_EDIT_Y_SPEED:
        speed = (int)SendMessageW(hEditYSpeed, TBM_GETPOS, 0, 0);
        hStaticSpeed = GetDlgItem(GetParent(hEditYSpeed), IDC_EDIT_Y_VALUE_SPEED);
        break;
    }
    if (hStaticSpeed)
    {
        wchar_t buffer[16];
        swprintf(buffer, 16, L"%d", speed);
        SetWindowTextW(hStaticSpeed, buffer);
    }
}
