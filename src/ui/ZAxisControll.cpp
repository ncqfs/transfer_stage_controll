#include"driver/ZAxisDriver.h"
#include"ui/ZAxisControll.h"
#include"ui/ui_setup.h"
#include"business/SerialPort.h"


ZAxisDriver Zctrl;
bool Zconnected = false;
bool zBtnHeld = false;
int zMoveDirection = 0;


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
