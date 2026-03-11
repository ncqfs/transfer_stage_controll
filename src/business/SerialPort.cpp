#include"business/SerialPort.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <cstdint>

#ifdef _WIN32
#include <tchar.h>
#pragma comment(lib, "Advapi32.lib")  // 用于注册表操作
#endif

using namespace std;


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
