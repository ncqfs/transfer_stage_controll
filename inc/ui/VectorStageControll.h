#pragma once
#include"driver/VectorStageDriver.h"

constexpr int CHANNEL_X = 0;
constexpr int CHANNEL_Y = 1;


void RefreshSerialPorts();
void ConnectStage();
void DisconnectStage();
void StopStage();
void UpdateStageControlsState(bool enabled);
void MoveStage(int channel, bool positive);
void MoveSpeed(int channel, bool positive);
void SpeedRefresh(LPARAM lParam);
void BoundaryCheck(int channel, bool positive, int& distance);