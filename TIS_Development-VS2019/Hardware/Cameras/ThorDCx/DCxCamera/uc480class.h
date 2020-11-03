// usb2cameraclass.h: interface for the Cuc480 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UC480CLASS_H__INCLUDED_)
#define AFX_UC480CLASS_H__INCLUDED_

#if _MSC_VER > 1000
 #pragma once
#endif // _MSC_VER > 1000

#include ".\include\uc480.h"
#include ".\include\uc480_deprecated.h"

class Cuc480
{
  public:
    Cuc480 ();
    virtual ~Cuc480 ();

    HCAM  GetCameraHandle ()          { return m_hcam; }
    HWND  GetWindowHandle ()          { return m_hwnd; }
    HWND  GetNotifyWindowHandle ()    { return m_hwndNotify; }
    BOOL  IsInit()                    { return m_hcam != NULL; }
    int   EnableDeviceStateNotification(BOOL boEnable, HWND hwnd = NULL);
    
    // init/exit functions
    int   InitCamera            (int nId = 0, HWND hwnd = NULL);
    int   ExitCamera            ();
    int   SetErrorReport        (int lMode);
    int   GetErrorText          (int lErr, char* pcErrText);

    // inquire functions
    int   GetNumberOfCameras    (int* plNumCameras);
    int   GetDllVersion         (int* plDllVers);
    int   GetPciSlot            (int* plSlot);
    int   GetIRQ                (int* plIRQ);
    int   GetColorDepth         (int* plBpp, int* plColFormat);
    int   CameraStatus          (int lInfo, long lValue);
    int   GetCameraType         ();
    int   GetOsVersion          (void);

	int   GetAOIRect			(IS_RECT *rect);
	int   SetAOI                (IS_RECT rect);

	int	  GetScalerInfo			(SENSORSCALERINFO *info);	
	int   GetScaler				(double* scaler);	

	int   GetBinningSupport     ();
	int   GetSubSamplingSupport ();
	int   GetBinning            (int hv);	//0: horizontal, 1: vertical
	int   GetSubSampling		(int hv);	//0: horizontal, 1: vertical
	int   SetBinning			(int hv, int binning);
	int   SetSubSampling		(int hv, int subSamplig);

	int   GetFlipStatus			();	//return mixed, must use | or & to determin
	int   SetFlipStatus			(int hv, int flip);	//flip : 1: enable, 0: disable

	int   GetFrameRate			(double* frameRate);
	int   GetPixelClock			(int* pixelClock);
	int   GetFrameTimeRange     (double* min, double* max, double* interval);
	int   GetColorMode          ();

	int   GetBlacklvlOffsetRange(IS_RANGE_S32* blRange);
	int   GetBlacklvlOffset     (int* blCurrent);
	int   SetBlacklvlOffset     (int blacklvl);

	int   GetGamma				(int* gamma);
	double   GetGain				();
	int   SetGain               (double gain);

	int   SetHotPixelCorrection (int enable);	//0: disable, 1: enable

    // image parameter functions
    int   SetBrightness               (int lBright);
    int   SetContrast                 (int lCont);
    int   SetGamma                    (int lGamma);
    int   SetSaturation               (int lSatU, int lSatV);
    int   SetHue                      (int lHue);
    int   SetColorMode                (int lMode);
    int   SetWhiteBalance             (int nMode );
	int   SetAutoParameter            (int param);
    int   SetWhiteBalanceMultipliers  (double dblRed, double dblGreen, double dblBlue );

    // callback functions
    int   InitEvent             (HANDLE hEv, int nWhich);
    int   EnableEvent           (int nWhich);
    int   ExitEvent             (int nWhich);
    int   DisableEvent          (int nWhich);
    int   EnableAutoExit        (int nMode);
    int   EnableMessage         (int nWhich, HWND hwnd);

    // display functions
    int   SetImageSize          (int lWidth, int lHeight);
    int   SetImagePos           (int lPosX, int lPosY);
    int   SetScrollPos          (int lPosX, int lPosY);
    int   SetDisplayOffset      (int lOffsetX, int lOffsetY);
    int   SetDisplayMode        (int Mode);
    int   SetDisplayWindow      (HWND hWnd);
    int   SetDisplayKeyColor    (int lKeyCol);
    int   SetMainDisplayWindow  (HWND hWnd);
    int   UpdateDisplay         ();
    int   SetDisplayHook        (int lMode);
	int   SetScaler             (double scaler);
	int	  SetAutoParameter		(int param, double* pval1, double* pval2);

    // video capture functions
    int   CaptureVideo   (int lWait);
    int   StopLiveVideo  (int lWait);
    int   FreezeVideo    (int lWait);
    int   CaptureFrame   (BITMAPINFOHEADER* pbmih, char* pcImage);
    int   SetCaptureMode (int lMode);
	int   SetIO          (int command, int mode);
	int   ForceTrigger   ();

    // memory handling functions
    int   AllocImageMem     (int lWidth, int lHeight, int lBpp, char** ppcImgMem, int* plID);
    int   FreeImageMem      (char* pcMem, int lID);
    int   SetImageMem       (char* pcMem, int lID);
    int   GetActiveImageMem (char** ppcImgMem, int* plID);
    int   InquireImageMem   (char* pcMem, int lID, int* plX, int* plY, int* plBpp, int* plPitch);
	int   SetAllocatedImageMem(int lWidth, int lHeight, int lBpp, char* pcImgMem, int* plID);
	int	  AddSequence       (char* pcImgMem, int nID);
	int   InitSequence      ();
	int   CopyImageMem		(char* source, int nID, char* dest);

   // digital I/O and trigger input functions
    int   ReadTriggerPort    (int* plIn);
    int   ReadDigitalPort    (int* plIn);
    int   WriteDigitalPort   (int lOut);
    int   SetExternalTrigger (int lMode);

    // EEPROM access functions
    int   GetCameraInfo (CAMINFO* pInfo);
    int   ReadEEPROM    (int lAdr, char* pcBuf, int lCount);
    int   WriteEEPROM   (int lAdr, char* pcBuf, int lCount);

    // bitmap load and save functions
    int   SaveImage    (char* pcFile);
    int   LoadImageEx    (char* pcFile);
    int   SaveImageMem (char* pcFile, char* pcMem, int lID);

    // bitmap render functions
    int   RenderBitmap (int nMemID, HWND hwnd, int nMode);

    // timing functions
	int   GetPixelClockRange	( int *nMin, int *nMax );
    int   SetPixelClock(int nClock);
    int   SetFrameRate(double dFr, double* pdFrNew);
    int   SetExposureTime(double EXP, double* newEXP );

	int   GetExposureRange(double *expMin, double *expMax, double *expInterval);
	int	  GetExposure(double *exposure);

    int   Renumerate( int reserved );
    int   GetSensorInfo( PSENSORINFO pInfo );
    int   LoadParameters( char* pFilename);

	int   GetCameraList(PUC480_CAMERA_LIST pucl);
	int   SetFlashStrobe(INT nMode, INT nLine);
	int   SetFlashDelay(ULONG ulDelay, ULONG ulDuration);

    int   DirectRenderer(UINT nMode, void* pParam, UINT nSizeOfParam);

    int   GetMaxImageSize(INT *pnSizeX, INT *pnSizeY);

	int   GetBinningCommand(int value, int hv);
	int   GetSubSamplingCommand(int value, int hv);

  private:
    HCAM  m_hcam;
    HWND  m_hwnd;
    HWND  m_hwndNotify;
};

#endif // !defined(AFX_UC480CLASS_H__INCLUDED_)
