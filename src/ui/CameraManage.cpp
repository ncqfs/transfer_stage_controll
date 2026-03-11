#include<opencv2/opencv.hpp>
#include<ueye.h>
#include <atomic>

#include"ui/CameraManage.h"
#include"ui/ui_setup.h"

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
    double scale = (std::min)((double)displayWidth / frame.cols, (double)displayHeight / frame.rows);
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

void UpdateCameraControlsState(bool enabled)
{
    EnableWindow(hBtnConnectCamera, !enabled);
    EnableWindow(hBtnDisconnectCamera, enabled);
    EnableWindow(hBtnCapture, enabled);
}
