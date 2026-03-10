#include "BluetoothMotorController.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <cstdint>

#ifdef _WIN32
#include <tchar.h>
#pragma comment(lib, "Advapi32.lib")  // 用于注册表操作
#endif

using namespace std;

// CRC16表定义
const uint16_t BluetoothMotorController::crc16_table[256] = {
0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,0xC601, 0x06C0,

0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,0xCC01, 0x0CC0, 0x0D80, 0xCD41,

0x0F00, 0xCFC1, 0xCE81, 0x0E40,0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0,

0x0880, 0xC841,0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,0x1400, 0xD4C1,

0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,0xD201, 0x12C0, 0x1380, 0xD341,

0x1100, 0xD1C1, 0xD081, 0x1040,0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1,

0xF281, 0x3240,0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,0xFA01, 0x3AC0,

0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,0x2800, 0xE8C1, 0xE981, 0x2940,

0xEB01, 0x2BC0, 0x2A80, 0xEA41,0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1,

0xEC81, 0x2C40,0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,0xA001, 0x60C0,

0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,0x6600, 0xA6C1, 0xA781, 0x6740,

0xA501, 0x65C0, 0x6480, 0xA441,0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0,

0x6E80, 0xAE41,0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,0xBE01, 0x7EC0,

0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,0xB401, 0x74C0, 0x7580, 0xB541,

0x7700, 0xB7C1, 0xB681, 0x7640,0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0,

0x7080, 0xB041,0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,0x9C01, 0x5CC0,

0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,0x5A00, 0x9AC1, 0x9B81, 0x5B40,

0x9901, 0x59C0, 0x5880, 0x9841,0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1,

0x8A81, 0x4A40,0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,0x8201, 0x42C0,

0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};



// 辅助函数实现
std::string wstringToString(const std::wstring& wstr) {
#ifdef _WIN32
	if (wstr.empty()) {
		return std::string();
	}

	int size = WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(),
		NULL, 0, NULL, NULL);
	std::string ret = std::string(size, 0);
	WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(),
		&ret[0], size, NULL, NULL);
	return ret;
#else
	// Linux版本（如果需要跨平台）
	std::string result;
	for (wchar_t wc : wstr) {
		result += static_cast<char>(wc & 0xFF);
	}
	return result;
#endif
}

// 枚举系统中的串口设备（Windows）
bool enumDetailsSerialPorts(std::vector<SerialPortInfo>& portInfoList)
{
#ifdef _WIN32
	// 注册表句柄
	HKEY hKey;

	// 注册表字符串最大长度
	const int MAX_VALUE_NAME = 16383;

	// 注册表值名（如 \Device\Serial0）
	TCHAR achValue[MAX_VALUE_NAME];

	// 注册表值数据（如 COM3）
	TCHAR strDSName[MAX_VALUE_NAME];

	// 打开串口映射注册表
	if (ERROR_SUCCESS == RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,
		_T("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
		0,
		KEY_READ,
		&hKey))
	{
		DWORD cKeyNum = 0;

		// 查询注册表中值的数量
		if (ERROR_SUCCESS == RegQueryInfoKey(
			hKey,
			NULL, NULL, NULL,
			NULL, NULL, NULL,
			&cKeyNum,
			NULL, NULL, NULL, NULL))
		{
			// 清空输出列表
			portInfoList.clear();

			// 枚举所有串口项
			for (DWORD i = 0; i < cKeyNum; i++)
			{
				DWORD cchValue = MAX_VALUE_NAME;
				DWORD nBuffLen = MAX_VALUE_NAME;

				if (ERROR_SUCCESS == RegEnumValue(
					hKey,
					i,
					achValue,
					&cchValue,
					NULL,
					NULL,
					(LPBYTE)strDSName,
					&nBuffLen))
				{
					SerialPortInfo info;

#ifdef UNICODE
					info.portName = wstringToString(strDSName);
#else
					info.portName = std::string(strDSName);
#endif
					portInfoList.push_back(info);
				}
			}
		}

		RegCloseKey(hKey);
		return !portInfoList.empty();
	}

	return false;
#else
	// 非 Windows 平台未实现
	return false;
#endif
}


// 类成员函数实现
BluetoothMotorController::BluetoothMotorController()
	: isConnected(false), pulseNumPerCircle(32768) {
#ifdef _WIN32
	hSerial = INVALID_HANDLE_VALUE;
#endif
}

BluetoothMotorController::~BluetoothMotorController() {
	stopMotor();
	disconnect();
}

std::string BluetoothMotorController::workMode()
{
	return this->mode;
}

bool BluetoothMotorController::connect(const std::string& portName, uint32_t baudRate) {
#ifdef _WIN32
	// 1. 构造完整的设备名
	std::string fullPortName = "\\\\.\\" + portName;

	// 2. 打开串口
	hSerial = CreateFileA(fullPortName.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);

	if (hSerial == INVALID_HANDLE_VALUE) {
		cerr << "无法打开串口 " << portName
			<< "，错误代码: " << GetLastError() << endl;
		return false;
	}

	// 3. 配置串口参数
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	if (!GetCommState(hSerial, &dcbSerialParams)) {
		cerr << "获取串口状态失败" << endl;
		CloseHandle(hSerial);
		hSerial = INVALID_HANDLE_VALUE;
		return false;
	}

	// 设置波特率等参数
	dcbSerialParams.BaudRate = baudRate;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	dcbSerialParams.fBinary = TRUE;
	dcbSerialParams.fOutxCtsFlow = FALSE;
	dcbSerialParams.fOutxDsrFlow = FALSE;
	dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;
	dcbSerialParams.fRtsControl = RTS_CONTROL_ENABLE;
	dcbSerialParams.fOutX = FALSE;
	dcbSerialParams.fInX = FALSE;

	if (!SetCommState(hSerial, &dcbSerialParams)) {
		cerr << "设置串口参数失败" << endl;
		CloseHandle(hSerial);
		hSerial = INVALID_HANDLE_VALUE;
		return false;
	}

	// 4. 设置超时
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	if (!SetCommTimeouts(hSerial, &timeouts)) {
		cerr << "设置串口超时失败" << endl;
		CloseHandle(hSerial);
		hSerial = INVALID_HANDLE_VALUE;
		return false;
	}

	// 5. 清空缓冲区
	PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

	isConnected = true;
	cout << "成功连接到串口: " << portName << endl;
	return true;
#else
	cerr << "非Windows平台暂不支持" << endl;
	return false;
#endif
}

void BluetoothMotorController::disconnect() {
#ifdef _WIN32
	if (hSerial != INVALID_HANDLE_VALUE) {
		CloseHandle(hSerial);
		hSerial = INVALID_HANDLE_VALUE;
		isConnected = false;
		cout << "已断开串口连接" << endl;
	}
#endif
}

bool BluetoothMotorController::connected() const {
#ifdef _WIN32
	return isConnected && (hSerial != INVALID_HANDLE_VALUE);
#else
	return false;
#endif
}

bool BluetoothMotorController::sendData(const uint8_t* data, size_t length) {
#ifdef _WIN32
	if (!connected()) {
		cerr << "未连接到串口" << endl;
		return false;
	}

	DWORD dwBytesWrite = static_cast<DWORD>(length);
	DWORD dwErrorFlags;
	COMSTAT comStat;
	OVERLAPPED m_osWrite;

	memset(&m_osWrite, 0, sizeof(m_osWrite));
	m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, L"WriteEvent");

	ClearCommError(hSerial, &dwErrorFlags, &comStat);
	BOOL bWriteStat = WriteFile(hSerial, data, dwBytesWrite, &dwBytesWrite, &m_osWrite);

	if (!bWriteStat) {
		if (GetLastError() == ERROR_IO_PENDING) {
			WaitForSingleObject(m_osWrite.hEvent, 1000);
		}
		else {
			cerr << "发送失败" << endl;
			ClearCommError(hSerial, &dwErrorFlags, &comStat);
			CloseHandle(m_osWrite.hEvent);
			return false;
		}
	}
	cout << "发送成功\n";
	CloseHandle(m_osWrite.hEvent);
	return true;
#else
	return false;
#endif
}

void BluetoothMotorController::extractByte(uint32_t value, uint8_t* bytes) {
	bytes[0] = (value >> 8) & 0xFF;
	bytes[1] = (value >> 0) & 0xFF;
	bytes[2] = (value >> 24) & 0xFF;
	bytes[3] = (value >> 16) & 0xFF;
}

void BluetoothMotorController::crc16_modbus(const uint8_t* data, size_t length, uint8_t* storage) {
	uint16_t crc = 0xFFFF;

	for (size_t i = 0; i < length; i++) {
		uint8_t index = (crc ^ data[i]) & 0xFF;
		uint16_t table_value = crc16_table[index];
		crc = (crc >> 8) ^ table_value;
	}

	storage[1] = (crc >> 8) & 0xFF;
	storage[0] = crc & 0xFF;
}

void BluetoothMotorController::checksum(const uint8_t* data, size_t length, uint8_t* storage) {
	uint8_t cksum = 0;
	for (size_t i = 0; i < length; i++) {
		cksum += data[i];
	}
	*storage = cksum;
}

bool BluetoothMotorController::rotateAngle(double degree) {
	if (!connected()) {
		cerr << "设备未连接" << endl;
		return false;
	}

	if (mode != "position") {
		setPositionMode();
	}

	uint32_t pulseNum = static_cast<uint32_t>(round(degree * pulseNumPerCircle / 360));
	//int pulseNum = static_cast<int>(round(degree * pulseNumPerCircle / 360));
	//if (pulseNum < 0) {
	//    pulseNum = static_cast<uint32_t>(-pulseNum);
	//    pulseNum = ~pulseNum + 1;
	//}
	//else {
	//    pulseNum = static_cast<uint32_t>(pulseNum);
	//}
	uint8_t cmd[16] = { 0xaa, 0x10, 0x01, 0x10, 0x00, 0x0c, 0x00, 0x02, 0x04 };
	extractByte(pulseNum, &cmd[9]);
	crc16_modbus(&cmd[2], 11, &cmd[13]);
	checksum(cmd, 15, &cmd[15]);

	return sendData(cmd, 16);
}

//bool BluetoothMotorController::rotatePulse(int pulseNum)
//{
//    if (!connected()) {
//        cerr << "设备未连接" << endl;
//        return false;
//    }
//
//    if (mode != "position") {
//        setPositionMode();
//    }
//
//    uint8_t cmd[16] = { 0xaa, 0x10, 0x01, 0x10, 0x00, 0x0c, 0x00, 0x02, 0x04 };
//    extractByte(static_cast<uint32_t>(pulseNum), &cmd[9]);
//    crc16_modbus(&cmd[2], 11, &cmd[13]);
//    checksum(cmd, 15, &cmd[15]);
//
//    return sendData(cmd, 16);
//}

bool BluetoothMotorController::rotateSpeed(short speed)
{
	if (mode != "speed") {
		setSpeedMode();
	}
	uint8_t speedCmd[11] = { 0xaa, 0x0b, 0x01, 0x06, 0x00, 0x02 };
	uint16_t u_speed = static_cast<uint16_t>(speed);
	speedCmd[7] = u_speed & 0xFF;
	speedCmd[6] = (u_speed >> 8) & 0xFF;


	crc16_modbus(&speedCmd[2], 6, &speedCmd[8]);
	checksum(speedCmd, 10, &speedCmd[10]);
	return sendData(speedCmd, sizeof(speedCmd));
}

// 可以添加更多控制函数
bool BluetoothMotorController::stopMotor() {
	// 实现停止指令
	if (mode != "speed") {
		setSpeedMode();
	}
	uint8_t stopCmd[] = { 0xaa, 0x0b, 0x01, 0x06, 0x00, 0x02, 0x00, 0x00, 0x28, 0x0A, 0xf0 }; // 示例指令
	return sendData(stopCmd, sizeof(stopCmd));
}

bool BluetoothMotorController::setPositionMode()
{
	if (mode == "position") return false;
	uint8_t cmd[] = { 0xAA, 0x0B, 0x01, 0x06, 0x00, 0x19, 0x00, 0x00, 0x58, 0x0D, 0x3A };
	bool success = sendData(cmd, 11);
	if (success) mode = "position";
	return success;
}

bool BluetoothMotorController::setSpeedMode()
{
	if (mode == "speed") return false;
	uint8_t cmd[] = { 0xAA, 0x0B, 0x01, 0x06, 0x00, 0x19, 0x00, 0x03, 0x18, 0x0C, 0xFC };
	bool success = sendData(cmd, 11);
	if (success) mode = "speed";
	return success;
}
