#include"ui/callback.h"
#include"ui/ui_setup.h"
#include"ui/CameraManage.h"
#include"ui/VectorStageControll.h"
#include"ui/ZAxisControll.h"
#include"ui/main.h"
#include"framework.h"
#include <windowsx.h>  // 需要 GET_X_LPARAM

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
