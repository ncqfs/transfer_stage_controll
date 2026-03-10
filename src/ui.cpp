#include "transfer_stage_gui.h"
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>

// 控件句柄定义（全局）
HWND hComboPort = nullptr;
HWND hBtnRefreshPort = nullptr;
HWND hBtnConnectStage = nullptr;
HWND hBtnDisconnectStage = nullptr;
HWND hBtnConnectCamera = nullptr;
HWND hBtnDisconnectCamera = nullptr;
HWND hBtnCapture = nullptr;
HWND hBtnStop = nullptr;
HWND hStaticStageStatus = nullptr;
HWND hStaticCameraStatus = nullptr;
HWND hStaticCameraView = nullptr;
HWND hBtnConnectZ = nullptr;
HWND hBtnDisconnectZ = nullptr;
HWND hStaticZStatus = nullptr;
HWND hComboPortZ = nullptr;
HWND hBtnRefreshPortZ = nullptr;
HWND hBtnMoveUpZ = nullptr;
HWND hBtnMoveDownZ = nullptr;
HWND hBtnStopZ = nullptr;
HWND hSliderSpeedZ = nullptr;
HWND hEditXDistance = nullptr;
HWND hEditXSpeed = nullptr;
HWND hBtnXPos = nullptr;
HWND hBtnXNeg = nullptr;
HWND hEditYDistance = nullptr;
HWND hEditYSpeed = nullptr;
HWND hBtnYPos = nullptr;
HWND hBtnYNeg = nullptr;

void CreateControls(HWND hWnd)
{
    // 将原 CreateControls 中的全部代码复制过来
    // 注意：原代码中使用了 hInst 全局变量，这里仍然有效（在 main.cpp 中定义）
    // 控件创建代码保持不变...
    // 记得在最后调用 RefreshSerialPorts() 和 RefreshSerialPortsZ()，以及初始禁用状态
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