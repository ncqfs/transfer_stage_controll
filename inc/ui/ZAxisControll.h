#pragma once
#include"driver/ZAxisDriver.h"

extern ZAxisDriver Zctrl;
extern bool Zconnected;
extern bool zBtnHeld;
extern int zMoveDirection;

void RefreshSerialPortsZ();
void ConnectZ();
void DisconnectZ();
void UpdateZControlsState(bool enabled);
void StopZ();
