#include "VectorStage.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//************************ 全局变量 ****************************
HANDLE pdv_hCom;
COMMTIMEOUTS pdv_commtimeouts;
DCB pdv_dcb;

int x_min, x_max, y_min, y_max, z_min, z_max, l_min, l_max; //限位
int x_home, y_home, z_home, l_home; //零位状态
int x_pos, y_pos, z_pos, l_pos;     //位置值
unsigned char io_buffer[512];       //串口缓冲区

//************************ 打开RS232串口 ****************************
void open_com(const wchar_t comm_number_string[])
{
    pdv_hCom = CreateFile(comm_number_string,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (pdv_hCom == INVALID_HANDLE_VALUE)
        MessageBox(NULL, L"初始化串口失败!", L"错误", MB_OK);
}

//************************ 设置RS232串口参数 *******************
void init_comm(void)
{
    GetCommState(pdv_hCom, &pdv_dcb);
    pdv_dcb.BaudRate = CBR_57600;
    pdv_dcb.fParity = 0;
    pdv_dcb.ByteSize = 8;
    pdv_dcb.Parity = NOPARITY;
    pdv_dcb.StopBits = ONESTOPBIT;
    SetCommState(pdv_hCom, &pdv_dcb);

    SetupComm(pdv_hCom, 32767, 512);

    pdv_commtimeouts.ReadIntervalTimeout = 100;
    pdv_commtimeouts.ReadTotalTimeoutMultiplier = 50;
    pdv_commtimeouts.ReadTotalTimeoutConstant = 100;
    pdv_commtimeouts.WriteTotalTimeoutMultiplier = 50;
    pdv_commtimeouts.WriteTotalTimeoutConstant = 100;
    SetCommTimeouts(pdv_hCom, &pdv_commtimeouts);

    PurgeComm(pdv_hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
}

//*************************** 读1字节 *************************
int ComRdByte(void)
{
    DWORD rCount;
    ReadFile(pdv_hCom, io_buffer, 1, &rCount, NULL);
    return io_buffer[0];
}

//************************ 读多个字节 *******************
void ComRd(int numb_of_byte)
{
    DWORD rCount;
    ReadFile(pdv_hCom, io_buffer, numb_of_byte, &rCount, NULL);
}

//**************************** 写1字节 ***********************
void ComWrtByte(int Byte)
{
    DWORD wCount;
    io_buffer[0] = Byte;
    WriteFile(pdv_hCom, io_buffer, 1, &wCount, NULL);
}

//**************************** 写1字节，等待读回1字节 *********************
void trans_byte(int Byte)
{
    ComWrtByte(Byte);
    ComRdByte();
}

//**************************** 关闭通讯端口 **************************
void close_comm(void)
{
    CloseHandle(pdv_hCom);
}

//************************** 使电机运行指定的距离 **************************
int set_distance(int channel, int distance)
{
    int chnl = channel;
    int dist = distance;
    char cmd[32] = { 0 };
    char temp[32] = { 0 };

    if (chnl > 3) chnl = 3;
    if (chnl < 0) chnl = 0;
    if (dist > 2147000000) dist = 2147000000;
    if (dist < -2147000000) dist = -2147000000;

    if (chnl == 0) strncpy_s(cmd, sizeof(cmd), "DX", _TRUNCATE);
    else if (chnl == 1) strncpy_s(cmd, sizeof(cmd), "DY", _TRUNCATE);
    else if (chnl == 2) strncpy_s(cmd, sizeof(cmd), "DZ", _TRUNCATE);
    else strncpy_s(cmd, sizeof(cmd), "DL", _TRUNCATE);

    sprintf_s(temp, sizeof(temp), "%d", dist);
    strncat_s(cmd, sizeof(cmd), temp, _TRUNCATE);
    strncat_s(cmd, sizeof(cmd), ";", _TRUNCATE);

    int cmd_size = (int)strlen(cmd);
    for (int i = 0; i < cmd_size; i++)
        trans_byte(cmd[i]);

    return distance;
}



//****************************** 设置最高运行速度 **************************
int set_max_speed(int channel, int speed)
{
    int chnl = channel;
    int spd = speed;
    char cmd[32] = { 0 };
    char temp[32] = { 0 };

    if (chnl > 3) chnl = 3;
    if (chnl < 0) chnl = 0;

    if (chnl == 0) strncpy_s(cmd, sizeof(cmd), "MX", _TRUNCATE);
    else if (chnl == 1) strncpy_s(cmd, sizeof(cmd), "MY", _TRUNCATE);
    else if (chnl == 2) strncpy_s(cmd, sizeof(cmd), "MZ", _TRUNCATE);
    else strncpy_s(cmd, sizeof(cmd), "ML", _TRUNCATE);

    sprintf_s(temp, sizeof(temp), "%d", spd);
    strncat_s(cmd, sizeof(cmd), temp, _TRUNCATE);
    strncat_s(cmd, sizeof(cmd), ";", _TRUNCATE);

    int cmd_size = (int)strlen(cmd);
    for (int i = 0; i < cmd_size; i++)
        trans_byte(cmd[i]);

    return spd;
}

//****************************** 使电机以指定速度运行 **************************
int set_speed(int channel, int speed)
{
    int chnl = channel;
    int spd = speed;
    char cmd[32] = { 0 };
    char temp[32] = { 0 };

    if (chnl > 3) chnl = 3;
    if (chnl < 0) chnl = 0;

    if (chnl == 0) strncpy_s(cmd, sizeof(cmd), "SX", _TRUNCATE);
    else if (chnl == 1) strncpy_s(cmd, sizeof(cmd), "SY", _TRUNCATE);
    else if (chnl == 2) strncpy_s(cmd, sizeof(cmd), "SZ", _TRUNCATE);
    else strncpy_s(cmd, sizeof(cmd), "SL", _TRUNCATE);

    sprintf_s(temp, sizeof(temp), "%d", spd);
    strncat_s(cmd, sizeof(cmd), temp, _TRUNCATE);
    strncat_s(cmd, sizeof(cmd), ";", _TRUNCATE);

    int cmd_size = (int)strlen(cmd);
    for (int i = 0; i < cmd_size; i++)
        trans_byte(cmd[i]);

    return spd;
}


//*************** 获取限位信息、零位信息、显示的位置数据 ************
void get_stage_state(void)
{
    char cmd[32] = { 0 };    // 扩大缓冲区
    char string[16] = { 0 }; // 足够存储9位数字和结束符
    int s, i;

    // 限位
    strncpy_s(cmd, sizeof(cmd), "US;", _TRUNCATE);
    int cmd_size = (int)strlen(cmd);
    for (i = 0; i < cmd_size; i++) trans_byte(cmd[i]);
    ComRd(1);
    s = io_buffer[0];

    x_min = (s & 1);
    x_max = (s & 2) / 2;
    y_min = (s & 4) / 4;
    y_max = (s & 8) / 8;
    z_min = (s & 16) / 16;
    z_max = (s & 32) / 32;
    l_min = (s & 64) / 64;
    l_max = (s & 128) / 128;

    // 零位
    strncpy_s(cmd, sizeof(cmd), "UH;", _TRUNCATE);
    cmd_size = (int)strlen(cmd);
    for (i = 0; i < cmd_size; i++) trans_byte(cmd[i]);
    ComRd(1);
    s = io_buffer[0];

    x_home = (s & 16) / 16;
    y_home = (s & 32) / 32;
    z_home = (s & 64) / 64;
    l_home = (s & 128) / 128;

    // 显示位置
    const char axes[4] = { 'X', 'Y', 'Z', 'L' };
    int* pos[4] = { &x_pos, &y_pos, &z_pos, &l_pos };
    for (int a = 0; a < 4; a++)
    {
        sprintf_s(cmd, sizeof(cmd), "U%c;", axes[a]);
        cmd_size = (int)strlen(cmd);
        for (i = 0; i < cmd_size; i++) trans_byte(cmd[i]);
        ComRd(9);
        memset(string, 0, sizeof(string));
        for (i = 0; i < 9; i++) string[i] = io_buffer[i];
        *pos[a] = atoi(string);
    }
}


//******************* 设置任意速度运动数据 *******************************
void SetArbitrarySpeedData(int channel, int total_seg, int duration[700], int acceleration[700], int repeat)
{
    char cmd[32] = { 0 };
    int cmd_size, i, j;

    // (1) 发送轴和时段数
    if (channel == 0) sprintf_s(cmd, sizeof(cmd), "NX%d;", total_seg);
    else if (channel == 1) sprintf_s(cmd, sizeof(cmd), "NY%d;", total_seg);
    else if (channel == 2) sprintf_s(cmd, sizeof(cmd), "NZ%d;", total_seg);
    else sprintf_s(cmd, sizeof(cmd), "NL%d;", total_seg);

    cmd_size = (int)strlen(cmd);
    for (j = 0; j < cmd_size; j++) trans_byte(cmd[j]);

    // (2) 发送时长和加速度
    for (i = 0; i < total_seg; i++)
    {
        sprintf_s(cmd, sizeof(cmd), "LT%d;", duration[i]);
        cmd_size = (int)strlen(cmd);
        for (j = 0; j < cmd_size; j++) trans_byte(cmd[j]);

        sprintf_s(cmd, sizeof(cmd), "LA%d;", acceleration[i]);
        cmd_size = (int)strlen(cmd);
        for (j = 0; j < cmd_size; j++) trans_byte(cmd[j]);
    }

    // (3) 发送重复次数
    if (channel == 0) sprintf_s(cmd, sizeof(cmd), "RX%d;", repeat);
    else if (channel == 1) sprintf_s(cmd, sizeof(cmd), "RY%d;", repeat);
    else if (channel == 2) sprintf_s(cmd, sizeof(cmd), "RZ%d;", repeat);
    else sprintf_s(cmd, sizeof(cmd), "RL%d;", repeat);

    cmd_size = (int)strlen(cmd);
    for (j = 0; j < cmd_size; j++) trans_byte(cmd[j]);
}

//************************ 启动任意速度运动 *******************************
void TriggerArbitrarySpeedProcedure(int Channels)
{
    char cmd[32] = { 0 };
    sprintf_s(cmd, sizeof(cmd), "TM%d;", Channels);
    int cmd_size = (int)strlen(cmd);
    for (int i = 0; i < cmd_size; i++) trans_byte(cmd[i]);
}
