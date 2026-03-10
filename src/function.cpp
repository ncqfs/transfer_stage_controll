#include "transfer_stage_gui.h"
#include <opencv2/opencv.hpp>
#include <uEye.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <commctrl.h>
#include <windowsx.h>  // 可能用到 GET_X_LPARAM 等，但可在 callbacks 中使用

// 全局变量定义（与硬件相关的）
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
bool stageConnected = false;
int stageComPort = 3;
BluetoothMotorController Zctrl;
bool Zconnected = false;
bool zBtnHeld = false;
int zMoveDirection = 0;
// 注意：还有 x_min, x_max 等外部变量，可能在 VectorStage.h 中定义，用 extern 引用即可

// 包含其他必要的头文件
#include "VectorStage.h"
extern HANDLE pdv_hCom;
extern int x_min, x_max, y_min, y_max, z_min, z_max, l_min, l_max;
extern int x_home, y_home, z_home, l_home;
extern int x_pos, y_pos, z_pos, l_pos;
const int MAX_POS_X = 60000;
const int MIN_POS_X = -115000;
const int MAX_POS_Y = 100000;
const int MIN_POS_Y = -85000;
const int EPS = 5000;



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
