#include "DAQBoard.h"
#include "CameraConfig.h"

int StartPockelsCalibration(DAQBoard* _hDAQController, CameraConfig* _pCameraConfig,bool& runningFlag);
int StopPockelsCalibration();
