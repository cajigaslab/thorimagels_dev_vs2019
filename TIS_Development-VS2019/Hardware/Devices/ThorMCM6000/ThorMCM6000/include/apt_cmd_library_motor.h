#pragma once
#ifndef APT_CMD_LIBRARY_MOTOR_H
#define APT_CMD_LIBRARY_MOTOR_H

#define Motor_Control_Messages
#ifdef Motor_Control_Messages

#ifndef APT_CMD_EXPORTS
#define APT_CMD_STATIC
#endif // !APT_CMD_EXPORTS

#ifdef APT_CMD_STATIC
#define APT_CMD_API extern "C"
#else
#define APT_CMD_API extern "C" __declspec(dllexport)
#endif // !APT_CMD_STATIC

APT_CMD_API int fnAPT_DLL_SetTSTScalerB(int hDevice, unsigned long encCnt);
APT_CMD_API int fnAPT_DLL_SetStageType(int hDevice, int stageID);
APT_CMD_API int fnAPT_DLL_MOT_MoveJog(int hDevice, unsigned char direction, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_MoveHome(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_MoveStop(int hDevice, unsigned char stop_mode, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_MoveAbsolute_S(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_MoveAbsolute(int hDevice, double distance, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_MoveVelocity(int hDevice, unsigned char direction, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_MoveRelative(int hDevice, double distance, unsigned char* msg);
///
/// this function actually send the MGMSG_MOT_REQ_DCSTATUSUPDATE message,for some historic reason, now can not change the old function's name.
APT_CMD_API int fnAPT_DLL_MOT_Req_StatusUpdate(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_DCStatusUpdate(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_DCStatusUpdate(int hdl, char* msg, unsigned char& source, unsigned short &scid, double &positon,
	double &velocity, unsigned long &statusbits);
APT_CMD_API int fnAPT_DLL_MOT_Set_VelParams(int hDevice, double min_vel, double acceleration, double max_vel, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_VelParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_VelParams(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid, double &min_vel,
	double &acc, double &max_vel);
APT_CMD_API int fnAPT_DLL_MOT_Set_HomeParams(int hDevice, unsigned short home_dir, unsigned short limit_switch, double home_vel,
	double offset_dis, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_HomeParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_HomeParams(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	unsigned short & home_dir, unsigned short &limit_switch, double &home_vel, double &offset_dis);
APT_CMD_API int fnAPT_DLL_MOT_Set_JogParams(int hDevice, unsigned short jog_mode, double jog_step_size, double jog_min_vel,
	double jog_acc, double jog_max_vel, unsigned short jog_stop_mode, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_JogParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_JogParams(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	unsigned short &jog_mode, double &jog_step_size, double &jog_min_vel, double &jog_acc,
	double &jog_max_vel, unsigned short &jog_stop_mode);
APT_CMD_API int fnAPT_DLL_MOT_Set_GenMoveParams(int hDevice, double backlashdistance, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_GenMoveParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_GenMoveParams(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	double &backlash_distance);
APT_CMD_API int fnAPT_DLL_MOT_Set_LimitSwitchParams(int hDevice, unsigned short cw_hardlimit, unsigned short ccw_hardlimit,
	double cw_softlimit, double ccw_softlimit, unsigned short limit_mode, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_LimitSwitchParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_LimitSwitchParams(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	unsigned short &cw_hardlimit, unsigned short &ccw_hardlimit,
	double &cw_softlimit, double &ccw_softlimit, unsigned short &limit_mode);
APT_CMD_API int fnAPT_DLL_MOT_Set_PotParamas(int hDevice, unsigned short zero_wnd, double vel1, unsigned short wnd1, double vel2,
	unsigned short wnd2, double vel3, unsigned short wnd3, double vel4, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_PotParamas(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_PotParamas(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	unsigned short &zero_wnd, double &vel1, unsigned short &wnd1,
	double &vel2, unsigned short &wnd2, double &vel3, unsigned short &wnd3, double &vel4);
APT_CMD_API int fnAPT_DLL_MOT_Set_AVmodes(int hDevice, unsigned short modebites, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_AVmodes(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_AVmodes(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	unsigned short &modebites);
APT_CMD_API int fnAPT_DLL_MOT_Set_ButtonParams(int hDevice, unsigned short mode, double Postion1,
	double Postion2, unsigned short timeout, unsigned short notused, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_ButtonParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_ButtonParams(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	unsigned short &mode, double &Postion1, double &Postion2,
	unsigned short &timeout, unsigned short &notused);
APT_CMD_API int fnAPT_DLL_MOT_Set_EEPromParams(int hDevice, unsigned short msgID, unsigned char* msg);

APT_CMD_API int fnAPT_DLL_MOT_Set_PMDPositionLoopParams(int hDevice, unsigned short KpPos, unsigned short Integral,
	unsigned long ILimPos, unsigned short Differential, unsigned short KdTimePos, unsigned short KoutPos,
	unsigned short KvffPos, unsigned short KaffPos, unsigned long PosErrLim, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_PMDPositionLoopParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_PMDPositionLoopParams(int hdl, char *b, unsigned char& source, unsigned short &scid,
	unsigned short &KpPos, unsigned short& Integral, unsigned long& ILimPos, unsigned short& Differential,
	unsigned short& KdTimePos, unsigned short& KoutPos, unsigned short& KvffPos, unsigned short& KaffPos, unsigned long& PosErrLim);
APT_CMD_API int fnAPT_DLL_MOT_Set_PMDMotorOutputParams(int hDevice, double ContCurrentLim, double EnergyLim, double MotorLim,
	double MotorBias, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_PMDMotorOutputParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_PMDMotorOutputParams(int hdl, char *b, unsigned char& source, unsigned short &scid,
	double &ContCurrentLim, double &EnergyLim, double &MotorLim, double &MotorBias);
APT_CMD_API int fnAPT_DLL_MOT_Set_PMDTrackSettleParams(int hDevice, unsigned short time, unsigned short SettleWindow,
	unsigned short TrackWindow, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_PMDTrackSettleParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_PMDTrackSettleParams(int hdl, char* b, unsigned char& source, unsigned short &scid,
	unsigned short &time, unsigned short &SettleWindow, unsigned short &TrackWindow);
APT_CMD_API int fnAPT_DLL_MOT_Set_PMDProfileModeParams(int hDevice, unsigned short mode, double jerk, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_PMDProfileModeParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_PMDProfileModeParams(int hdl, char *b, unsigned char& source, unsigned short &scid,
	unsigned short &mode, double &jerk);
APT_CMD_API int fnAPT_DLL_MOT_Set_PMDJoyStickParams(int hDevice, double JSGearLowMaxVel, double JSGearHighMaxVel,
	double JSGearLowAccn, double JSGearHighAccn, unsigned short DirSense, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_PMDJoyStickParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_PMDJoyStickParams(int hdl, char* b, unsigned char& source, unsigned short &scid,
	double &JSGearLowMaxVel, double &JSGearHighMaxVel, double &JSGearLowAccn, double &JSGearHighAccn, unsigned short &DirSense);
APT_CMD_API int fnAPT_DLL_MOT_Set_PMDCurrentLoopParams(int hDevice, unsigned short Phase, unsigned short KpCurrent,
	unsigned short KiCurrent, unsigned short ILimCurrent, unsigned short IDeadBand, unsigned short Kff, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_PMDCurrentLoopParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_PMDCurrentLoopParams(int hdl, char* b, unsigned char& source, unsigned short &scid,
	unsigned short &Phase, unsigned short &KpCurrent, unsigned short &KiCurrent,
	unsigned short &ILimCurrent, unsigned short &IDeadBand, unsigned short &Kff);
APT_CMD_API int fnAPT_DLL_MOT_Set_PMDSettleDCurrentLoopParams(int hDevice, unsigned short Phase, unsigned short KpSettled,
	unsigned short KiSettled, unsigned short ILimSettled, unsigned short IDeadBandSettled,
	unsigned short KffSettled, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_PMDSettleDCurrentLoopParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_PMDSettleDCurrentLoopParams(int hdl, char* b, unsigned char& source, unsigned short &scid,
	unsigned short &Phase, unsigned short &KpSettled, unsigned short &KiSettled,
	unsigned short &ILimSettled, unsigned short &IDeadBandSettled, unsigned short &KffSettled);
APT_CMD_API int fnAPT_DLL_MOT_Set_PMDStageAxisParams(int hDevice, unsigned short StageID, unsigned short AxisID, char* PartNoAxis,
	unsigned long SerialNum, unsigned long CntsPerUnit, double MinPos, double MaxPos,
	double MaxAccn, double MaxDec, double MaxVel, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_PMDStageAxisParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_PMDStageAxisParams(int hdl, char* b, unsigned char& source, unsigned short &scid,
	unsigned short &StageID, unsigned short &AxisID, char* PartNoAxis, unsigned long &SerialNum, unsigned long &CntsPerUnit,
	double &MinPos, double &MaxPos, double &MaxAccn, double &MaxDec, double &MaxVel);


/// restfactor/movefactor range: 1~100
APT_CMD_API int fnAPT_DLL_MOT_Set_PowerParams(int hDevice, unsigned short restfactor, unsigned short movefactor, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_PowerParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_PowerParams(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	unsigned short &restfactor, unsigned short &movefactor);
APT_CMD_API int fnAPT_DLL_MOT_Set_Poscounter(int hDevice, double Postion, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_Poscounter(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_Poscounter(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid, double &Postion);
APT_CMD_API int fnAPT_DLL_MOT_Set_Enccounter(int hDevice, double encoder_counter, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_Enccounter(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_Enccounter(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	double &encoder_counter);


APT_CMD_API int fnAPT_DLL_MOT_Req_ADCInputs(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_ADCInputs(int hDevice, char *msg, unsigned char& source, unsigned short& ACDInput1);

APT_CMD_API int fnAPT_DLL_MOT_Set_BowIndex(int hDevice, unsigned short bowindex, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_BowIndex(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_BowIndex(int hDevice, char* msg, unsigned char& source, unsigned short &scid,
	unsigned short& bowindex);

APT_CMD_API int fnAPT_DLL_MOT_Set_MoveRELParams(int hDevice, double relative_distance, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_MoveRELParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_MoveRELParams(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	double &relative_distance);
APT_CMD_API int fnAPT_DLL_MOT_Set_MoveABSParams(int hDevice, double absolute_distance, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_MoveABSParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_MoveABSParams(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	double &absolute_distance);
APT_CMD_API int fnAPT_DLL_MOT_Set_DCPIDParams(int hDevice, long  proportional, long integral, long differential,
	long integral_limit, unsigned short filtercontrol, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_DCPIDParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_DCPIDParams(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	unsigned long &proportional, unsigned long &integral, unsigned long &differential,
	unsigned long &integral_limit, unsigned short &filtercontrol);
APT_CMD_API int fnAPT_DLL_MOT_Ack_DCStatusUpdate(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_StatusBits(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_StatusBits(int hdl, char *Msg_Data, unsigned char& source, unsigned short &scid,
	unsigned long &statusbits, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Suspend_EndofMoveMSGS(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Resume_EndofMoveMSGS(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_TstActuatorType(int hDevice, unsigned char ActuatorType, unsigned char* msg);
///
/// this function actually send the MGMSG_MOT_REQ_STATUSUPDATE message,for some historic reason, now can not change the old function's name.
APT_CMD_API int fnAPT_DLL_MOT_Req_StatusUpdate_2(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_StatusUpdate(int hDevice, char *b, unsigned short &scid, double& Position, long& EncCount,
	unsigned long& StatusBits, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Set_Trigger(int hDevice, unsigned char mode, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_Trigger(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_Trigger(int hDevice, char *b, unsigned char& source, unsigned char &cid, unsigned char &mode);

//applicable only to KST101, KDC101 and KBD101 units
APT_CMD_API int fnAPT_DLL_MOT_Set_KCubeMMIParams(int hDevice, unsigned short JSMode, double JSMaxVel, double JSAccn,
	unsigned short DirSense, double PresetPos1, double PresetPos2, unsigned short DispBrightness,
	unsigned short DispTimeout, unsigned short DispDimLevel, unsigned char * msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_KCubeMMIParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_KCubeMMIParams(int hDevice, char * msg, unsigned char& source, unsigned short &scid,
	unsigned short& JSMode, double& JSMaxVel, double& JSAccn, unsigned short& DirSense, double& PresetPos1,
	double& PresetPos2, unsigned short& DispBrightness, unsigned short& DispTimeout, unsigned short& DispDimLevel);

//applicable only to KST101, KDC101 and KBD101 units
APT_CMD_API int fnAPT_DLL_MOT_Set_KCubeTrigIOConfig(int hDevice, unsigned short Trig1Mode, unsigned short Trig1Polarity,
	unsigned short Trig2Mode, unsigned short Trig2Polarity, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_KCubeTrigIOConfig(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_KCubeTrigIOConfig(int hDevice, char * msg, unsigned char& source, unsigned short &scid,
	unsigned short& Trig1Mode, unsigned short& Trig1Polarity, unsigned short& Trig2Mode, unsigned short& Trig2Polarity);

//applicable only to KST101, KDC101 and KBD101 units
APT_CMD_API int fnAPT_DLL_MOT_Set_KCubePosTrigParams(int hDevice, double StartPosFwd, double IntervalFwd, long NumPulsesFwd,
	double StartPosRev, double IntervalRev,
	long NumPulsesRev, long PulseWidth, long NumCycles, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_KCubePosTrigParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_KCubePosTrigParams(int hDevice, char * msg, unsigned char& source, unsigned short &scid,
	double& StartPosFwd, double& IntervalFwd, long& NumPulsesFwd, double& StartPosRev,
	double& IntervalRev, long& NumPulsesRev, long& PulseWidth, long& NumCycles);

APT_CMD_API int fnAPT_DLL_MOT_Reset_EEPromParams(int hDevice, unsigned char* msg);

//Filter_Flipper_Control_Messages
APT_CMD_API int fnAPT_DLL_MOT_Set_MFFOperParams(int hDevice, long lTransitTime, long lTransitTimeADC,
	unsigned short OperMode1, unsigned short SigMode1,
	long PulseWidth1, unsigned short OperMode2, unsigned short SigMode2, long PulseWidth2, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_MFFOperParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_MFFOperParams(int hDevice, char * msg, unsigned char& source, unsigned short &scid,
	long& lTransitTime, long& lTransitTimeADC, unsigned short& OperMode1,
	unsigned short& SigMode1, long& PulseWidth1, unsigned short& OperMode2, unsigned short& SigMode2, long& PulseWidth2);

APT_CMD_API int fnAPT_DLL_MOT_Set_Sol_OperatingMode(int hDevice, char mode, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_Sol_OperatingMode(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_Sol_OperatingMode(int hDevice, char*b, unsigned char& source, unsigned char &cid, char& mode);

APT_CMD_API int fnAPT_DLL_MOT_Set_Sol_CycleParams(int hDevice, long OnTime, long OffTime, long NumCycles, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_Sol_CycleParams(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_Sol_CycleParams(int hDevice, char*b, unsigned short &scid, long& OnTime,
	long& OffTime, long& NumCycles);

APT_CMD_API int fnAPT_DLL_MOT_Set_Sol_InterlockMode(int hDevice, char mode, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_Sol_InterlockMode(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_Sol_InterlockMode(int hDevice, unsigned char &cid, char*b, char& mode);

APT_CMD_API int fnAPT_DLL_MOT_Set_Sol_State(int hDevice, char mode, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Req_Sol_State(int hDevice, unsigned char* msg);
APT_CMD_API int fnAPT_DLL_MOT_Get_Sol_State(int hDevice, char*b, unsigned char &cid, char&mode);

#endif // Motor_Control_Messages

#endif // !APT_CMD_LIBRARY_MOTOR_H
