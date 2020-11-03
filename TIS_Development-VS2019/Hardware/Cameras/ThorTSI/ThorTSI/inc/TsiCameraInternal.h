/******************************************************************************/
/* TsiCameraInternal.h                                                        */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/******************************************************************************/
#ifndef __THORLABS_SCIENTIFIC_IMAGING_CAMERA_INTERNAL_H__
#define __THORLABS_SCIENTIFIC_IMAGING_CAMERA_INTERNAL_H__

#include "TsiColorCamera.h"

enum TSI_DATA_TABLE_SELECT 
{
   TSI_DATA_TABLE_SELECT_20MHZ = 0,
   TSI_DATA_TABLE_SELECT_40MHZ,
   TSI_DATA_TABLE_SELECT_MAX
};

enum TSI_CONFIG_SELECT 
{
   TSI_CONFIG_SELECT_AUTO_LOAD_TAP_BALANCE = 0,
   TSI_CONFIG_SELECT_MAX
};

enum TSI_CAMERA_HARDWARE_INTERFACE_TYPE
{
   TSI_GIG_E_INTERFACE,
   TSI_CAMERA_LINK_INTERFACE,
   TSI_USB_INTERFACE
};

class ICameraInterface;
class TsiImageInternal;

typedef void (__cdecl *TSI_FUNCTION_SCRATCHPAD_COLOR_PIPELINE_CALLBACK) (TsiColorImage *tsi_image, void *context);

//==============================================================================
// TsiCameraInternal C++ Class
//------------------------------------------------------------------------------
//==============================================================================
class TsiCameraInternal : public TsiColorCamera
{
   //---------------------------------------------------------------------------
   // PUBLIC
   //---------------------------------------------------------------------------
   public:
               TsiCameraInternal (void);
      virtual ~TsiCameraInternal (void);

      virtual bool            Open                 (void            );
      virtual bool            Close                (void            );

      virtual bool            Status               (TSI_CAMERA_STATUS *status);

      virtual char           *GetCameraName        (void            );
      virtual bool            SetCameraName        (char *name      );

      virtual int             GetDataTypeSize      (TSI_DATA_TYPE    data_type);

      virtual int             GetParameter         (TSI_PARAM_ID param_id );
      virtual bool            GetParameter         (TSI_PARAM_ID param_id, size_t length, void *data);

      virtual bool            SetParameter         (TSI_PARAM_ID param_id, void  *data  );
      virtual bool            SetParameter         (TSI_PARAM_ID param_id, int    value );

      virtual bool            ResetCamera          (void            );

      virtual TsiImage       *GetPendingImage      (void            );
      virtual TsiImage       *GetLastPendingImage  (void            );
      virtual bool            FreeAllPendingImages (void            ); 
      virtual bool            FreeImage            (TsiImage       *);       

      virtual bool            StartAndWait         (int time_out_ms );
      virtual bool            Start                (void            );
      virtual bool            Stop                 (void            );

      virtual int             GetAcquisitionStatus (void            );
      virtual int             GetExposeCount       (void            );
      virtual int             GetFrameCount        (void            );
      virtual bool            WaitForImage         (int timeout_ms  );
      virtual bool            ResetExposure        (void            );

      virtual char           *GetLastErrorStr      (void                                       );
      virtual TSI_ERROR_CODE  GetErrorCode         (void                                       );
      virtual bool            ClearError           (void                                       );
      virtual bool            GetErrorStr          (TSI_ERROR_CODE code, char *str, int &str_len);

      virtual bool            SetTextCommand       (char *str                                );
      virtual bool            SetTextCallback      (TSI_TEXT_CALLBACK_FUNCTION, void *context);

      virtual bool            SetCameraControlCallback     (TSI_FUNCTION_CAMERA_CONTROL_CALLBACK     func, void *context);
      virtual bool            SetCameraControlCallbackEx   (TSI_FUNCTION_CAMERA_CONTROL_CALLBACK_EX  func, void *context);
      virtual bool            SetImageNotificationCallback (TSI_FUNCTION_IMAGE_NOTIFICATION_CALLBACK func, void *context);
      virtual bool            SetImageCallback             (TSI_FUNCTION_IMAGE_CALLBACK              func, void *context);


      //------------------------------------------------------------------------
      // Tap Balance Methods
      //------------------------------------------------------------------------
      virtual bool TapBalanceConfig         (int select, bool enable);
     
      virtual bool TapBalanceSelectTable    (int select);

      virtual bool TapBalanceGetTableConfig (int *rows, int *num_taps);
      virtual bool TapBalanceGetTableData   (int row, float *mV, float *t1_gain, float *t2_gain);
      virtual bool TapBalanceGetTableData   (int row, float *mV, float *t1_gain, float *t2_gain, float *t3_gain, float *t4_gain);

      virtual bool TapBalanceSetTableConfig (int rows, int num_taps);
      virtual bool TapBalanceSetTableData   (int row, float mV, float t1_gain, float t2_gain);
      virtual bool TapBalanceSetTableData   (int row, float mV, float t1_gain, float t2_gain, float t3_gain, float t4_gain);

      virtual bool TapBalanceCommitData     (void);

      virtual bool TapBalanceLoadDataFromFile   (char *filename);
      virtual bool TapBalanceWriteDataToCamera  (void);
      virtual bool TapBalanceLoadDataFromCamera (void);

      //------------------------------------------------------------------------
      // TsiColorCamera Methods
      //------------------------------------------------------------------------
      virtual TsiColorImage* GetPendingColorImage (TSI_COLOR_PROCESSING_MODE postProcess);
      virtual TsiColorImage* GetLastPendingColorImage (TSI_COLOR_PROCESSING_MODE postProcess);

      //------------------------------------------------------------------------
      // Misc functions
      //------------------------------------------------------------------------
      virtual double GetFrameRate    (void);
      virtual double GetFrameRateAvg (void);
      virtual double GetBandwidth    (void);
      virtual double GetBandwidthAvg (void);

      virtual TSI_CAMERA_HARDWARE_INTERFACE_TYPE GetCameraInterfaceType();

      //------------------------------------------------------------------------
      // Serial IO functions
      //------------------------------------------------------------------------
      virtual int   SendData (void *data, int num_tx_bytes);
      virtual int   RecvData (void *data, int num_rx_bytes, unsigned int timeout_ms);

      //------------------------------------------------------------------------
      // Trigger based image acquisition.
      //------------------------------------------------------------------------
      virtual bool StartTriggerAcquisition  (void);
      virtual bool StopTriggerAcquisition   (bool rearm);
      virtual bool SWTrigger                (void);

      //------------------------------------------------------------------------
      // Image averaging.
      //------------------------------------------------------------------------
      virtual int  GetImageAvgNumFrames(void          );
      virtual bool SetImageAvgNumFrames(int num_frames);

      //------------------------------------------------------------------------
      // Color methods.
      //------------------------------------------------------------------------
      virtual bool IsColorCamera() const;

      virtual bool SetScratchpadColorPipelineCallBack (TSI_FUNCTION_SCRATCHPAD_COLOR_PIPELINE_CALLBACK func, void* context);
      virtual bool ScratchPadPostProcessImage (TsiImageInternal* image);
      virtual bool ScratchPadConcatenateColorTransform (double* p3x3Matrix);
      virtual bool ScratchPadConcatenateColorTransform (TSI_TRANSFORM t, unsigned int outputBitDepth);
      virtual bool ScratchPadSetInputTransform (int* pNx2pBmatrix, int N, int B);
      virtual bool ScratchPadSetOutputTransform (int* pMx2pBmatrix, int M, int B);
      virtual bool ScratchPadClearColorPipeline();
      virtual bool ScratchPadFinalizeColorPipeline();

      virtual ICameraInterface *GetICameraInterface(void);

   //---------------------------------------------------------------------------
   // PROTECTED
   //---------------------------------------------------------------------------
   protected:
};

#endif
