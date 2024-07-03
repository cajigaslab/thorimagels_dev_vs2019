#pragma once
#include "thordaq.h"

class ProgrammableTrigger
{
#define PT_SHADOW_REG_STRLEN 32
private:
	char _PT_HW_In1_SEL[PT_SHADOW_REG_STRLEN];
	char _PT_HW_In2_SEL[PT_SHADOW_REG_STRLEN];
	char _PT_SW_ArmASSERT[PT_SHADOW_REG_STRLEN];
	char _PT_HWSW_SEL[PT_SHADOW_REG_STRLEN];
	char _PT_InputCfgIndx[PT_SHADOW_REG_STRLEN];  // The "truth table" 5 bit lookup table entry "address"
	char _PT_OutCfgFn[PT_SHADOW_REG_STRLEN];      // The "truth table" 2 bit output function  "data" for the table entry
	char _PT_CfgWriteSTROBE[PT_SHADOW_REG_STRLEN];   // causes InputCfgIndx+OutCfgFn to be written to 
	char _PT_DO_WaveformIN_SEL[PT_SHADOW_REG_STRLEN]; // Selects one of the 16 possible DO Waveforms as INput to PT

	char _PT_ImageAcqTrig_SEL[PT_SHADOW_REG_STRLEN]; // undefined for ImageAcqTrigger, selects ImageAcq or WaveformControl PT
	char _PT_TruthTableCfg_SEL[PT_SHADOW_REG_STRLEN];  // only for WaveformControl PT - selects TruthTable by channel

public:
	ProgrammableTrigger(CThordaq& TbBrd, signed char chan); // constructor
	  // AppConfig is the enum for pre-defined Programmable Triggers

	THORDAQ_STATUS ProgrammableTriggerConfig(CThordaq& TbBrd, signed char chan, int AppConfig); 	// chan -1 for ImageAcqTrigger, 0-13 for WaveformControlTrigger
	THORDAQ_STATUS ProgrammableTriggerTruthTableSetup(CThordaq& TbBrd, signed char chan, int AppConfig);
	void ProgrammableTriggerArmASSERT(CThordaq& TbBrd, signed char chan, bool Assert); // Software Arm/TRIGGER FPGA register write

public:
	~ProgrammableTrigger();  // destructor
};
