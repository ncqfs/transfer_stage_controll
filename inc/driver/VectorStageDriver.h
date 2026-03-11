#ifndef _VECTOR_STAGE_H_
#define _VECTOR_STAGE_H_

// 打开串口
void open_com(const wchar_t comm_number_string[]);

// 初始化串口参数
void init_comm(void);

// 关闭串口
void close_comm(void);

// 读写字节
int ComRdByte(void);
void ComRd(int numb_of_byte);
void ComWrtByte(int Byte);
void trans_byte(int Byte);

// 电机控制
int set_distance(int channel, int distance);       // 移动指定距离
int set_speed(int channel, int speed);            // 设定运行速度
int set_max_speed(int channel, int speed);        // 设定最大速度

// 获取电机状态
void get_stage_state(void);

// 任意速度运动
void SetArbitrarySpeedData(int channel, int total_seg, int duration[700], int acceleration[700], int repeat);
void TriggerArbitrarySpeedProcedure(int Channels);

#endif
