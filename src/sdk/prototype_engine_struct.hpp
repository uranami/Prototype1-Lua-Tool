#pragma once

class SimulationDriver_t
{
public:
	char pad_0000[48]; //0x0000
	float N00000528; //0x0030
	char pad_0034[32]; //0x0034
	float float_timer; //0x0054
	char pad_0058[4]; //0x0058
	float DebugTimeDilation; //0x005C
	char pad_0060[24]; //0x0060
	float timerv2; //0x0078
	char pad_007C[196]; //0x007C
}; //Size: 0x1040