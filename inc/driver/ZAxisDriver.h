/*
	Last modified 2025.12.14 18:03
	Version 1.0a

	更新内容：
	(1.0a)
	1.rotateAngle 和 rotateSpeed可以传入负数用来控制反转；
	2.rotateSpeed 的参数类型从原来的 int 改为 short;
	3.连接到串口时不再默认设置为速度模式.

*/

#ifndef BLUETOOTHMOTORCONTROLLER_H
#define BLUETOOTHMOTORCONTROLLER_H

#include <string>
#include <vector>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif


// 主控制类
class ZAxisDriver {
public:
	ZAxisDriver();
	~ZAxisDriver();

	// 查询工作模式-----------------------------------------
	/**
	 * @brief 查询工作模式
	 * @return position（位置模式） speed（速度模式）
	 */
	std::string workMode();

	// 连接管理-----------------------

	/**
	 * @brief 连接指定串口
	 * @param portName 接口名
	 * @param baudRate 波特率，对于蓝牙JDY-31-SPP，采用默认设置9600
	 * @return 是否连接成功
	 */
	bool connect(const std::string& portName, uint32_t baudRate = 9600);

	/**
	 * @brief 断开连接（对象析构时自动调用）
	 */
	void disconnect();

	/**
	 * @brief 查询是否处于连接状态
	 * @return 连接中，返回true；未连接，返回false
	 */
	bool connected() const;

	// 电机控制

	/**
	 * @brief 控制电机旋转到指定角度
	 * @param degree 要旋转的角度（单位是度）
	 * @return 是否成功发送指令
	 */
	bool rotateAngle(double degree);

	/**
	 * @brief 控制电机旋转指定个数脉冲
	 * @param pulseNum 需要旋转的脉冲数
	 * @return 是否成功发送指令
	 */
	 //bool rotatePulse(int pulseNum);

	 /**
	  * @brief 按照制定速度旋转（单位：rpm，范围：-3000~3000）
	  * @param speed 目标速度
	  * @return 是否成功发送指令
	  */
	bool rotateSpeed(short speed);

	/**
	 * @brief 电机停转
	 * @return 是否成功发送指令
	 */
	bool stopMotor();  // 可以添加更多控制方法

	// 设置工作模式

	/**
	 * @brief 设置为位置模式
	 * @return 是否成功发送指令
	 */
	bool setPositionMode();

	/**
	 * @brief 设置为速度模式
	 * @return 是否成功发送指令
	 */
	bool setSpeedMode();

private:

	// 串口通信
	/**
	 * @brief 发送一段数据
	 * @param data 数据地址
	 * @param length 数据长度
	 * @return 是否成功发送
	 */
	bool sendData(const uint8_t* data, size_t length);

	// CRC校验
	/**
	 * @brief 计算16位modbusCRC校验，保存在storage开始的2字节中
	 * @param data 需要计算校验的数据
	 * @param length 这段数据的长度
	 * @param storage 保存的地址
	 */

	void crc16_modbus(const uint8_t* data, size_t length, uint8_t* storage);
	/**
	 * @brief 计算校验和
	 * @param data 需要计算的数据
	 * @param length 这段数据的长度
	 * @param storage 校验和保存的位置
	 */
	void checksum(const uint8_t* data, size_t length, uint8_t* storage);

	// 数据处理
	/**
	 * @brief 将32位整数切分为8~15位，0~7位，24~31位，16~23位，保存在bytes开始的四个字节中
	 * @param value 需要拆分的数字
	 * @param bytes 保存的起始地址
	 */
	void extractByte(uint32_t value, uint8_t* bytes);

	// 常量
	static const uint16_t crc16_table[256];

	// 成员变量
#ifdef _WIN32
	HANDLE hSerial;
#endif
	bool isConnected;
	const int pulseNumPerCircle;
	std::string mode;

	// 复制和赋值禁用
	ZAxisDriver(const ZAxisDriver&) = delete;
	ZAxisDriver& operator=(const ZAxisDriver&) = delete;
};

#endif // BLUETOOTHMOTORCONTROLLER_H