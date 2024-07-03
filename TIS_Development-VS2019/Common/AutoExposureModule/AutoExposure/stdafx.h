#pragma once

#include <math.h>
#include <vector>
#include <mutex>
#include <atomic>
#include <string>
#include <memory>

#include "..\..\..\Common\Camera.h"
#include "..\..\..\Common\Command.h"
#include "..\..\..\Common\Device.h"
#include "..\..\..\Common\Experiment.h"
#include "..\..\..\Common\Log.h"
#include "..\..\..\Common\ImageManager\ImageManager\Image.h"
#include "..\..\..\Common\ImageManager\ImageManager\ImageManager.h"
#include "..\..\..\Hardware\Devices\DeviceManager\DeviceManager\DeviceManager.h"
#include "..\..\..\Common\ExperimentManager\ExperimentManager\ExperimentManager.h"
#include "..\..\..\Common\HardwareCom\HardwareCom\HardwareComFunctions.h"
#include "..\..\..\Common\ResourceManager\ResourceManager\ResourceManager.h"

const long MSG_LENGTH = 256;
static wchar_t msg[MSG_LENGTH];
extern std::unique_ptr<LogDll> logDll;
