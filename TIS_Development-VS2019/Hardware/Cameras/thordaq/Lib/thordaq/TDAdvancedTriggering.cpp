/*++

Copyright (c)2023 Thorlabs, Inc.  All rights reserved.

Module Name: TDAdvancedTriggering.cpp
Created by DZimmerman

Abstract:

	Defines the Class for advanced programmable triggering
	See:
	  TIS_Development\Documents\Design\ThorDAQ\AdvancedTriggeringConfigTruthTable.xlsx
	  TIS_Development\Documents\Design\ThorDAQ_AdvancedTriggeringSchematic.pub
Environment:

	user mode only.

Style:
	Google C++ coding style.
Note:
	Move implementations of API functions to this file, out of thordaq.cpp
--*/

#include "stdafx.h"
#pragma once
#include "thordaq.h"

//#include "TDAdvancedTriggering.h"


CThordaq::ProgrammableTrigger::ProgrammableTrigger(CThordaq& TbBrd, signed char chan)  // constructor
{
//	THORDAQ_STATUS status;
	// initialize all shadow register names according to channel
	if (chan == -1) // ImageAcqTrigger
	{
		sprintf_s(_PT_DO_WaveformIN_SEL, PT_SHADOW_REG_STRLEN, "%s", "ImageAcqPT_DO_WaveformIN_SEL");
		sprintf_s(_PT_HW_In2_SEL, PT_SHADOW_REG_STRLEN, "%s", "ImageAcqPT_HW_In2_SEL");
		sprintf_s(_PT_HW_In1_SEL, PT_SHADOW_REG_STRLEN, "%s", "ImageAcqPT_HW_In1_SEL");
		sprintf_s(_PT_SW_ArmASSERT, PT_SHADOW_REG_STRLEN, "%s", "GIGCR0_STOP_RUN");  // Legacy SW Arm/Trigger name
		sprintf_s(_PT_HWSW_SEL, PT_SHADOW_REG_STRLEN, "%s", "ImageAcqPT_HWSW_SEL");
		sprintf_s(_PT_InputCfgIndx, PT_SHADOW_REG_STRLEN, "%s", "ImageAcqPT_InputCfgIndx");
		sprintf_s(_PT_OutCfgFn, PT_SHADOW_REG_STRLEN, "%s", "ImageAcqPT_OutCfgFn");
		sprintf_s(_PT_CfgWriteSTROBE, PT_SHADOW_REG_STRLEN, "%s", "ImageAcqPT_CfgWriteSTROBE");

		_PT_ImageAcqTrig_SEL[0] = 0; // invalid shadow regs for this PT
		_PT_TruthTableCfg_SEL[0] = 0;
	}
	else // WaveformControlTrigger (chan 0-13)
	{
		sprintf_s(_PT_DO_WaveformIN_SEL, PT_SHADOW_REG_STRLEN, "WaveformControlPT_DO_WaveformIN_SEL_chan%d", chan); // 
		sprintf_s(_PT_HW_In2_SEL, PT_SHADOW_REG_STRLEN, "WaveformControlPT_HW_In2_SEL_chan%d", chan);
		sprintf_s(_PT_HW_In1_SEL, PT_SHADOW_REG_STRLEN, "WaveformControlPT_HW_In1_SEL_chan%d", chan);
		sprintf_s(_PT_SW_ArmASSERT, PT_SHADOW_REG_STRLEN, "WaveformControlPT_SW_ArmASSERT_chan%d", chan);
		sprintf_s(_PT_HWSW_SEL, PT_SHADOW_REG_STRLEN, "WaveformControlPT_HWSW_SEL_chan%d", chan);
		// these registers shared by all channels
		sprintf_s(_PT_InputCfgIndx, PT_SHADOW_REG_STRLEN, "%s", "WaveformControlPT_InputCfgIndx");
		sprintf_s(_PT_OutCfgFn, PT_SHADOW_REG_STRLEN, "%s", "WaveformControlPT_OutCfgFn");
		sprintf_s(_PT_CfgWriteSTROBE, PT_SHADOW_REG_STRLEN, "%s", "WaveformControlPT_CfgWriteSTROBE");
		sprintf_s(_PT_TruthTableCfg_SEL, PT_SHADOW_REG_STRLEN, "%s", "WaveformControlPT_TruthTableCfg_SEL");

		sprintf_s(_PT_ImageAcqTrig_SEL, PT_SHADOW_REG_STRLEN, "WaveformControlPT_ImageAcqTrig_SEL_chan%d", chan); // when 0, WaveformControl by ImageAcqTrigger
	}
	// set default config (don't expect errors in Constructor)
	if (chan == -1)
	{
		ProgrammableTriggerConfig(TbBrd, -1, 0);
	}
	else // WaveformControlTrig
	{
		// for ALL WaveformControl Trigs, first switch default chan control away from ImageAcqTrig
		TbBrd.FPGAregisterWrite(_PT_ImageAcqTrig_SEL, (int)strlen(_PT_ImageAcqTrig_SEL), 0x01); // Waveform PT asserts 

	}
}

THORDAQ_STATUS CThordaq::ProgrammableTrigger::ProgrammableTrigger::ProgrammableTriggerConfig(CThordaq& TbBrd, signed char chan, int AppConfig)  // default config sets all WaveformControl PTs to be sync'd with ImageAcqTrigger
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	switch (AppConfig)
	{
	case 0: // DEFAULT
		// default config is all triggering by ImageAcqTrigger - Software ArmASSERT so the GlobalStopRun bit (written by user SW) starts image acq
		// set up a default hardware trigger with as auto-reset rising edge pulse:   MUX external signal "DI_HW_Trigger_1"
		status = ProgrammableTriggerTruthTableSetup(TbBrd, chan, AppConfig);
		break;

	default:
		// do nothing
		status = THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
		break;
	}

	return status;
}

THORDAQ_STATUS CThordaq::ProgrammableTrigger::ProgrammableTrigger::ProgrammableTriggerTruthTableSetup(CThordaq& TbBrd, signed char chan, int AppConfig)
{
	THORDAQ_STATUS status = THORDAQ_STATUS::STATUS_SUCCESSFUL;
	int iErr = 0;
	switch (AppConfig)
	{
	case 0: // the DEFAULT config - must contain a switch for EACH chan involved in the AppConfig
		switch (chan) // 
		{
		case -1:  // ImageAcqTrigger
			// set a HW default of In3 PT with auto reset of rising edge latch
			iErr |= TbBrd.FPGAregisterWrite( _PT_InputCfgIndx,  (int)strlen(_PT_InputCfgIndx), 0x08); // In3
			iErr |= TbBrd.FPGAregisterWrite(_PT_OutCfgFn, (int)strlen(_PT_OutCfgFn), 0x3); // PT_Reset asserts, PT_Out asserts
			// assert STROBE to write to table...
			iErr |= TbBrd.FPGAregisterWrite(_PT_CfgWriteSTROBE, (int)strlen(_PT_CfgWriteSTROBE), 0x1); // write when 0 -> 1
			iErr |= TbBrd.FPGAregisterWrite(_PT_CfgWriteSTROBE, (int)strlen(_PT_CfgWriteSTROBE), 0x0); // Reset STOBE
			// Assert PT_Out by user SW
			iErr |= TbBrd.FPGAregisterWrite(_PT_HWSW_SEL, (int)strlen(_PT_HWSW_SEL), 0x0); // Reset STOBE

			break;
		}
		break;
	default:
		status = THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;
		break;
	}
	if (iErr != 0) status = THORDAQ_STATUS::STATUS_PARAMETER_SETTINGS_ERROR;

	return status;
}

// An FPGA software Arm/ASSERT register (e.g. BAR3, offset 0, Bit 0 Legacy "GIGCR0_STOP_RUN"
// or "WaveformControlPT_SW_ArmASSERT_chan0") has two functions.  If Trig mode is SOFTWARE, asserting
// this signal asserts the trigger itself.  It Trig mode is HARDWARE, asserting this ARMS the Hardware
// signalling defined by the Programmable Trigger.  Note that the ProgrammableTrigger has an option
// to bypass ARM SW function and run purely off hardware signalling
void CThordaq::ProgrammableTrigger::ProgrammableTrigger::ProgrammableTriggerArmASSERT(CThordaq& TbBrd, signed char chan, bool Assert)
{
	UINT64 Value = (Assert) ? 1 : 0;
	TbBrd.FPGAregisterWrite(_PT_SW_ArmASSERT, (int)strlen(_PT_SW_ArmASSERT), Value);
}

CThordaq::ProgrammableTrigger::ProgrammableTrigger::~ProgrammableTrigger()  // destructor
{
	// free heap memory


}
