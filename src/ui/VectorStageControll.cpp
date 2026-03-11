#include"framework.h"
#include"ui/VectorStageControll.h"
#include"ui/ui_setup.h"
#include"business/SerialPort.h"
#include"driver/VectorStageDriver.h"

bool stageConnected = false;
int stageComPort = 3;
extern HANDLE pdv_hCom;
extern int x_min, x_max, y_min, y_max, z_min, z_max, l_min, l_max;
extern int x_home, y_home, z_home, l_home;
extern int x_pos, y_pos, z_pos, l_pos;
const int MAX_POS_X = 60000;
const int MIN_POS_X = -115000;
const int MAX_POS_Y = 100000;
const int MIN_POS_Y = -85000;
const int EPS = 5000;


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
