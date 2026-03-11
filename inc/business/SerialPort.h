#pragma once
#include <string>
#include <vector>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

// 串口信息结构体
extern struct SerialPortInfo {
	std::string portName;
	std::string description;
};

// 辅助函数声明
std::string wstringToString(const std::wstring& wstr);
/**
 * @brief 枚举所有可用的串口
 * @param portInfoList 列出的串口信息保存在这里
 * @return 是否有可用的串口
 */
bool enumDetailsSerialPorts(std::vector<SerialPortInfo>& portInfoList);