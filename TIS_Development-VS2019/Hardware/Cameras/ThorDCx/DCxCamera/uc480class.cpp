#include "stdafx.h"
#include "uc480class.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cuc480::Cuc480 ()
{
	m_hcam = NULL;
}


Cuc480::~Cuc480 ()
{

}


int Cuc480::InitCamera (int nId, HWND hwnd)
{
	int nRet = IS_CANT_OPEN_DEVICE;
	HCAM hu = (HCAM) nId;

	if((m_hcam == NULL))
	{
		nRet = is_InitCamera (&hu, hwnd);

		if (nRet == IS_SUCCESS)
		{
			m_hcam = hu;
			m_hwnd = hwnd;

			// Disable Autoexit
			is_EnableAutoExit( m_hcam, IS_DISABLE_AUTO_EXIT );
		}
	}

	return nRet;
}


int Cuc480::ExitCamera ()
{
	int nRet = IS_CANT_CLOSE_DEVICE;

	if(m_hcam != NULL)
	{
		nRet = is_ExitCamera (m_hcam);
		m_hcam = NULL;
	}

	return nRet;
}


int Cuc480::SetErrorReport (int lMode)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetErrorReport (m_hcam, lMode);
}


int Cuc480::GetNumberOfCameras (int* plNumCameras)
{
	return is_GetNumberOfCameras (plNumCameras);
}


int Cuc480::GetDllVersion (int* plDllVers)
{
	*plDllVers = is_GetDLLVersion();
	return IS_SUCCESS;
}


int Cuc480::GetColorDepth (int* plBpp, int* plColFormat)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_GetColorDepth (m_hcam, plBpp, plColFormat);
}


int Cuc480::CameraStatus (int lInfo, long lValue)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_CameraStatus (m_hcam, lInfo, (ULONG)lValue);
}


int Cuc480::GetCameraType ()
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_GetCameraType (m_hcam);
}

int Cuc480::GetOsVersion ()
{
	return is_GetOsVersion ();
}

int Cuc480::GetAOIRect (IS_RECT *rect)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_AOI(m_hcam, IS_AOI_IMAGE_GET_AOI, (void*)rect, sizeof(*rect));
}

int Cuc480::SetAOI (IS_RECT rect)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_AOI(m_hcam, IS_AOI_IMAGE_SET_AOI, (void*)&rect, sizeof(rect));
}

int Cuc480::GetScalerInfo (SENSORSCALERINFO *info)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_GetSensorScalerInfo(m_hcam, info, sizeof(*info));
}

int Cuc480::GetScaler (double* scaler)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetSensorScaler(m_hcam, IS_ENABLE_SENSOR_SCALER, *scaler);
}

int Cuc480::GetBinningSupport ()
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetBinning(m_hcam, IS_GET_SUPPORTED_BINNING);
}
int Cuc480::GetSubSamplingSupport ()
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetSubSampling(m_hcam, IS_GET_SUPPORTED_SUBSAMPLING);
}

int Cuc480::GetBinning (int hv)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetBinning(m_hcam, 0 == hv ? IS_GET_BINNING_FACTOR_HORIZONTAL : IS_GET_BINNING_FACTOR_VERTICAL);
}

int Cuc480::GetSubSampling (int hv)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetSubSampling(m_hcam, 0 == hv ? IS_GET_SUBSAMPLING_FACTOR_HORIZONTAL : IS_GET_SUBSAMPLING_FACTOR_VERTICAL);
}

int Cuc480::SetBinning (int hv, int binning)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetBinning(m_hcam, GetBinningCommand(binning, hv));
}
int Cuc480::SetSubSampling (int hv, int subSamplig)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetSubSampling(m_hcam, GetSubSamplingCommand(subSamplig, hv));
}

int Cuc480::GetFlipStatus ()
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetRopEffect(m_hcam, IS_GET_ROP_EFFECT, 1, 0);
}

int Cuc480::SetFlipStatus (int hv, int flip)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetRopEffect(m_hcam, 0 == hv ? IS_SET_ROP_MIRROR_LEFTRIGHT : IS_SET_ROP_MIRROR_UPDOWN, flip, 0);
}

int Cuc480::GetFrameRate (double* frameRate)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_GetFramesPerSecond(m_hcam, frameRate);
}

int Cuc480::GetPixelClock (int* pixelClock)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_PixelClock(m_hcam, IS_PIXELCLOCK_CMD_GET, (void*)pixelClock, sizeof(*pixelClock));
}

int Cuc480::GetFrameTimeRange (double* min, double* max, double* interval)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_GetFrameTimeRange(m_hcam, min, max, interval);
}

int Cuc480::GetColorMode ()
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetColorMode(m_hcam, IS_GET_COLOR_MODE);
}

int Cuc480::GetBlacklvlOffsetRange (IS_RANGE_S32* blRange)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_Blacklevel(m_hcam, IS_BLACKLEVEL_CMD_GET_OFFSET_RANGE, (void*)blRange, sizeof(*blRange));
}

int Cuc480::GetBlacklvlOffset (int* blCurrent)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_Blacklevel(m_hcam, IS_BLACKLEVEL_CMD_GET_OFFSET, (void*)blCurrent, sizeof(*blCurrent));
}

int Cuc480::SetBlacklvlOffset (int blacklvl)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_Blacklevel(m_hcam, IS_BLACKLEVEL_CMD_SET_OFFSET, (void*)&blacklvl, sizeof(blacklvl));
}

int Cuc480::GetGamma (int* gamma)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_Gamma(m_hcam, IS_GAMMA_CMD_GET, (void*)gamma, sizeof(*gamma));
}

double Cuc480::GetGain	()
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetHWGainFactor(m_hcam, IS_GET_MASTER_GAIN_FACTOR, 100) / (double)100.0;
}

int Cuc480::SetGain (double gain)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetHWGainFactor(m_hcam, IS_SET_MASTER_GAIN_FACTOR, (int)(gain * 100));
}

int Cuc480::SetHotPixelCorrection (int enable)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	if (0 == enable)
		return is_HotPixel(m_hcam, IS_HOTPIXEL_DISABLE_CORRECTION, NULL, NULL);	
	return is_HotPixel(m_hcam, IS_HOTPIXEL_ENABLE_SOFTWARE_USER_CORRECTION, NULL, NULL);
}

int Cuc480::SetBrightness (int lBright)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetBrightness (m_hcam, lBright);
}

int Cuc480::SetContrast (int lCont)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetContrast (m_hcam, lCont);
}


int Cuc480::SetGamma (int lGamma)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetGamma (m_hcam, lGamma);
}


int Cuc480::SetWhiteBalance( INT nMode )
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetWhiteBalance(m_hcam, nMode);
}

int Cuc480::SetAutoParameter (int param)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	double val1 = 1;
	double val2 = 0;
	return is_SetAutoParameter(m_hcam, param, &val1, &val2);
}

int Cuc480::SetWhiteBalanceMultipliers(double dblRed, double dblGreen, double dblBlue )
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetWhiteBalanceMultipliers(m_hcam, dblRed, dblGreen, dblBlue);
}


int Cuc480::SetColorMode (int lMode)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetColorMode (m_hcam, lMode);
}


int Cuc480::InitEvent (HANDLE hEv, int nWhich)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_InitEvent (m_hcam, hEv, nWhich);
}


int Cuc480::EnableEvent (int nWhich)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_EnableEvent (m_hcam, nWhich);
}


int Cuc480::DisableEvent (int nWhich)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_DisableEvent (m_hcam, nWhich);
}


int Cuc480::ExitEvent (int nWhich)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_ExitEvent (m_hcam, nWhich);
}


int Cuc480::EnableMessage (int nWhich, HWND hwnd)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_EnableMessage(m_hcam, nWhich, hwnd);
}


int Cuc480::EnableAutoExit (int nMode)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_EnableAutoExit(m_hcam, nMode);
}


int Cuc480::SetImageSize (int lWidth, int lHeight)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetImageSize (m_hcam, lWidth, lHeight);
}


int Cuc480::SetImagePos (int lPosX, int lPosY)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetDisplayPos (m_hcam, lPosX, lPosY);
}


int Cuc480::SetDisplayMode (int Mode)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetDisplayMode (m_hcam, Mode);
}


int Cuc480::SetDisplayWindow (HWND hWnd)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetHwnd (m_hcam, hWnd);
}


int Cuc480::SetDisplayKeyColor (int lKeyCol)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetKeyColor (m_hcam, (lKeyCol & 0x00FF0000) >> 16, (lKeyCol & 0x0000FF00) >> 8, lKeyCol & 0x000000FF);
}


int Cuc480::UpdateDisplay ()
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_UpdateDisplay (m_hcam);
}

int Cuc480::SetScaler(double scaler)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetSensorScaler (m_hcam, IS_ENABLE_SENSOR_SCALER | IS_ENABLE_ANTI_ALIASING, scaler);
}

int	Cuc480::SetAutoParameter (int param, double* pval1, double* pval2)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetAutoParameter(m_hcam, param, pval1, pval2);
}

int Cuc480::CaptureVideo (int lWait)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_CaptureVideo (m_hcam, lWait);
}


int Cuc480::StopLiveVideo (int lWait)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_StopLiveVideo (m_hcam, lWait);
}


int Cuc480::FreezeVideo (int lWait)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_FreezeVideo (m_hcam, lWait);
}

int Cuc480::SetIO (int command, int mode)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_IO (m_hcam, command, (void*)&mode, sizeof(mode));
}

int Cuc480::ForceTrigger ()
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_ForceTrigger(m_hcam);
}

int Cuc480::AllocImageMem (int lWidth, int lHeight, int lBpp, char** ppcImgMem, int* plID)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_AllocImageMem (m_hcam, lWidth, lHeight, lBpp, ppcImgMem, plID);
}


int Cuc480::FreeImageMem (char* pcMem, int lID)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_FreeImageMem (m_hcam, pcMem, lID);
}


int Cuc480::SetImageMem (char* pcMem, int lID)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetImageMem (m_hcam, pcMem, lID);
}


int Cuc480::GetActiveImageMem (char** ppcImgMem, int* plID)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_GetActiveImageMem (m_hcam, ppcImgMem, plID);
}


int Cuc480::InquireImageMem (char* pcMem, int lID, int* plX, int* plY, int* plBpp, int* plPitch)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_InquireImageMem (m_hcam, pcMem, lID, plX, plY, plBpp, plPitch);
}

int Cuc480::SetAllocatedImageMem(int lWidth, int lHeight, int lBpp, char* pcImgMem, int* plID)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetAllocatedImageMem (m_hcam, lWidth, lHeight, lBpp, pcImgMem, plID);
}

int	Cuc480::AddSequence (char* pcImgMem, int nID)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_AddToSequence(m_hcam, pcImgMem, nID);
}

int Cuc480::InitSequence ()
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_InitImageQueue(m_hcam, 0);
}

int Cuc480::CopyImageMem (char* source, int nID, char* dest)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_CopyImageMem(m_hcam, source, nID, dest);
}

int Cuc480::ReadTriggerPort (int* plIn)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	int nRet = IS_SUCCESS;

	if(plIn != NULL)
		*plIn = is_SetExternalTrigger (m_hcam, IS_GET_TRIGGER_INPUTS);
	else
		nRet = IS_NULL_POINTER;

	return nRet;
}


int Cuc480::ReadDigitalPort (int* plIn)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	int nRet = IS_SUCCESS;

	if(plIn != NULL)
		*plIn = is_SetIO (m_hcam, IS_GET_IO);
	else
		nRet = IS_NULL_POINTER;

	return nRet;
}


int Cuc480::WriteDigitalPort (int lOut)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetIO (m_hcam, lOut);
}


int Cuc480::SetExternalTrigger (int lMode)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetExternalTrigger (m_hcam, lMode);
}


int Cuc480::GetCameraInfo (CAMINFO* pInfo)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	int nRet = IS_SUCCESS;

	if(pInfo != NULL)
		nRet = is_GetCameraInfo (m_hcam, pInfo);
	else
		nRet = IS_NULL_POINTER;

	return nRet;
}


int Cuc480::ReadEEPROM (int lAdr, char* pcBuf, int lCount)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	int nRet = IS_SUCCESS;

	if(pcBuf != NULL)
		nRet = is_ReadEEPROM (m_hcam, lAdr, pcBuf, lCount);
	else
		nRet = IS_INVALID_PARAMETER;

	return nRet;
}


int Cuc480::WriteEEPROM (int lAdr, char* pcBuf, int lCount)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	int nRet = IS_SUCCESS;

	if(pcBuf != NULL)
		nRet = is_WriteEEPROM (m_hcam, lAdr, pcBuf, lCount);
	else
		nRet = IS_INVALID_PARAMETER;

	return nRet;
}


int Cuc480::SaveImage (char* pcFile)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SaveImage (m_hcam, pcFile);
}


int Cuc480::LoadImageEx (char* pcFile)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_LoadImage (m_hcam, pcFile);
}


int Cuc480::SaveImageMem (char* pcFile, char* pcMem, int lID)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SaveImageMem (m_hcam, pcFile, pcMem, lID);
}


int Cuc480::RenderBitmap (int nMemID, HWND hwnd, int nMode)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_RenderBitmap(m_hcam, nMemID, hwnd, nMode);
}


int	Cuc480::GetPixelClockRange	( int *nMin, int *nMax )
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_GetPixelClockRange( m_hcam, nMin, nMax );
}

int Cuc480::SetPixelClock(int nClock)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetPixelClock(m_hcam, nClock);
}


int Cuc480::SetFrameRate(double dFr, double* pdFrNew)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetFrameRate(m_hcam, dFr, pdFrNew);
}


int Cuc480::SetExposureTime(double EXP, double* newEXP )
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetExposureTime(m_hcam, EXP, newEXP );
}

int Cuc480::GetExposureRange(double *expMin, double *expMax, double *expInterval)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_GetExposureRange(m_hcam, expMin, expMax, expInterval);
}

int	Cuc480::GetExposure(double *exposure)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_Exposure(m_hcam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void*)exposure, sizeof(*exposure));
}

int Cuc480::EnableDeviceStateNotification(BOOL boEnable, HWND hwnd)
{
	int nRet;

	if(hwnd == NULL)
	{
		if(m_hwndNotify != NULL)
			hwnd = m_hwndNotify;
	}

	if(!boEnable)
		hwnd = NULL;

	nRet = is_EnableMessage( NULL, IS_NEW_DEVICE, hwnd );
	if(nRet == IS_SUCCESS)
	{
		nRet = is_EnableMessage( NULL, IS_DEVICE_REMOVAL, hwnd );
		if(nRet == IS_SUCCESS)
			m_hwndNotify = hwnd;
	}

	return nRet;
}


int Cuc480::GetSensorInfo( PSENSORINFO pInfo )
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_GetSensorInfo( m_hcam, pInfo );
}


int Cuc480::LoadParameters( char* pFilename)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_LoadParameters( m_hcam, pFilename );
}

int Cuc480::GetCameraList(PUC480_CAMERA_LIST pucl)
{
	return is_GetCameraList(pucl);
}

int Cuc480::SetFlashStrobe(INT nMode, INT nLine)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetFlashStrobe(m_hcam, nMode, nLine);
}

int Cuc480::SetFlashDelay(ULONG ulDelay, ULONG ulDuration)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_SetFlashDelay(m_hcam,ulDelay,ulDuration);
}

int Cuc480::DirectRenderer(UINT nMode, void* pParam, UINT nSizeOfParam) 
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	return is_DirectRenderer(m_hcam, nMode, pParam, nSizeOfParam);
}


int Cuc480::GetMaxImageSize(INT *pnSizeX, INT *pnSizeY)
{
	if( m_hcam == NULL )	return IS_NO_SUCCESS;
	// Check if the camera supports an arbitrary AOI
	INT nAOISupported = 0;
	BOOL bAOISupported = TRUE;
	if (is_ImageFormat(m_hcam,
		IMGFRMT_CMD_GET_ARBITRARY_AOI_SUPPORTED, 
		(void*)&nAOISupported, 
		sizeof(nAOISupported)) == IS_SUCCESS)
	{
		bAOISupported = (nAOISupported != 0);
	}

	if (bAOISupported)
	{
		// Get maximum image size
		SENSORINFO sInfo;
		is_GetSensorInfo (m_hcam, &sInfo);
		*pnSizeX = sInfo.nMaxWidth;
		*pnSizeY = sInfo.nMaxHeight;
	}
	else
	{
		// Get image size of the current format
		*pnSizeX = is_SetImageSize(m_hcam, IS_GET_IMAGE_SIZE_X, 0);
		*pnSizeY = is_SetImageSize(m_hcam, IS_GET_IMAGE_SIZE_Y, 0);
	}

	return IS_SUCCESS;
}

int Cuc480::GetBinningCommand (int value, int hv)
{
	INT command = IS_BINNING_DISABLE;	//make invalid value larger than largest.
	switch (value)
	{
	case 2:
		command = hv == 0 ? IS_BINNING_2X_HORIZONTAL : IS_BINNING_2X_VERTICAL;
		break;
	case 3:
		command = hv == 0 ? IS_BINNING_3X_HORIZONTAL : IS_BINNING_3X_VERTICAL;
		break;										   			
	case 4:											   			
		command = hv == 0 ? IS_BINNING_4X_HORIZONTAL : IS_BINNING_4X_VERTICAL;
		break;										   			
	case 5:											   			
		command = hv == 0 ? IS_BINNING_5X_HORIZONTAL : IS_BINNING_5X_VERTICAL;
		break;										   			
	case 6:	
		command = hv == 0 ? IS_BINNING_6X_HORIZONTAL : IS_BINNING_6X_VERTICAL;
		break;										   			
	case 8:
		command = hv == 0 ? IS_BINNING_8X_HORIZONTAL : IS_BINNING_8X_VERTICAL;
		break;										   			
	case 16:										   			
		command = hv == 0 ? IS_BINNING_16X_HORIZONTAL : IS_BINNING_16X_VERTICAL;
		break;
	default:
		break;
	}
	return command;	
}
int Cuc480::GetSubSamplingCommand (int value, int hv)
{
	INT command = IS_SUBSAMPLING_DISABLE;	//make invalid value larger than largest.
	switch (value)
	{
	case 2:
		command = hv == 0 ? IS_SUBSAMPLING_2X_HORIZONTAL : IS_SUBSAMPLING_2X_VERTICAL;
		break;				   
	case 3:					   
		command = hv == 0 ? IS_SUBSAMPLING_3X_HORIZONTAL : IS_SUBSAMPLING_3X_VERTICAL;
		break;					
	case 4:						
		command = hv == 0 ? IS_SUBSAMPLING_4X_HORIZONTAL : IS_SUBSAMPLING_4X_VERTICAL;
		break;					
	case 5:						
		command = hv == 0 ? IS_SUBSAMPLING_5X_HORIZONTAL : IS_SUBSAMPLING_5X_VERTICAL;
		break;					
	case 6:						
		command = hv == 0 ? IS_SUBSAMPLING_6X_HORIZONTAL : IS_SUBSAMPLING_6X_VERTICAL;
		break;					
	case 8:						
		command = hv == 0 ? IS_SUBSAMPLING_8X_HORIZONTAL : IS_SUBSAMPLING_8X_VERTICAL;
		break;					
	case 16:					
		command = hv == 0 ? IS_SUBSAMPLING_16X_HORIZONTAL : IS_SUBSAMPLING_16X_VERTICAL;
		break;
	default:
		break;
	}
	return command;	
}