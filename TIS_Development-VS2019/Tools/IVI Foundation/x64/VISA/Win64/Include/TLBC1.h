/**************************************************************************//**

	\file          
	\ingroup       TLBC1_INSTR_DRIVER_x Thorlabs BC1 VISA VXIpnp Driver
	\brief         VXIpnp driver for the Thorlabs BC1 camera beam profiler.

	Thorlabs GmbH - Thorlabs Beam - BC1 - Camera Beam Profiler
	
	\note For detailed description of the instrument driver functions refer to
			the TLBC1.fp file (for NI-CVI users) or the HTML documentation that
			was installed with the driver into 
			[IVI Foundation]\\VISA\\WinXX\\TLBC1\\Manual
			
	\note Basic sample programs for different programming environments/languages
			were installed with the driver into 
			[IVI Foundation]\\VISA\\WinXX\\TLBC1\\Examples

	\date          19-Jul-2011
	\copyright     copyright(c) 2006-2013, Thorlabs GmbH

******************************************************************************/

#ifndef __TLBC1_H__
#define __TLBC1_H__

/*=============================================================================
  Include files
=============================================================================*/  
#include <vpptype.h>

// calculation results used for the function TLBC1_get_scan_data() 
#include "TLBC1_Calculations.h"
#include "TLBC1_structs.h"

/*=============================================================================
  Defines
=============================================================================*/
#ifdef __cplusplus
	 extern "C" {
#endif

#if defined(_WIN64)
#define _BC_FUNC            __fastcall
#elif (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)) && !defined(_NI_mswin16_)
#define _BC_FUNC            __stdcall
#elif defined(_CVI_) && defined(_NI_i386_)
#define _BC_FUNC            _pascal
#elif (defined(_WINDOWS) || defined(_Windows)) && !defined(_NI_mswin16_)
#define _BC_FUNC            _far _pascal _export
#elif (defined(hpux) || defined(__hpux)) && (defined(__cplusplus) || defined(__cplusplus__))
#define _BC_FUNC
#else
#define _BC_FUNC
#endif

/*========================================================================*//**
\defgroup   TLBC1_INSTR_DRIVER_x Thorlabs BC1 VISA VXIpnp Driver
\brief   VXIpnp driver for the Thorlabs BC1 camera beam profiler.
@{
*//*=========================================================================*/

/*=============================================================================
  Constants
=============================================================================*/
/*========================================================================*//**
\defgroup   TLBC1_USEFUL_x Some limits and other useful macros.
@{
*//*=========================================================================*/
#define INV_DEVICE_HANDLE MAX_CAMERA_DEVICES    ///< The index of a camera can never reach the max device count
		 
#define TLBC1_ERR_DESCR_BUFFER_SIZE     256     ///< Minimum error description buffer size  
		 
#define BC1_RESOURCE_NAME "USB::0x1313::0x8012::"  ///< Pseudo Resource Name to code the serial number and device index 

#define MIN_AVERAGE_COUNT 1   ///< minimum of frames that can be averaged (1 = no averaging)
#define MAX_AVERAGE_COUNT 100 ///< maximum of frames that can be averaged

/**@}*/  // defgroup TLBC1_USEFUL_x


/*========================================================================*//**
\defgroup VI_INSTR_x Instrument Error Codes

\details VPP Status codes:
	
	Reserved value ranges for...\n\n
	
	VISA library status codes (see visa.h):\n
		Warnings:   0x3FFF0000 ... 0x3FFFFFFF\n
		Errors:     0xBFFF0000 ... 0xBFFFFFFF\n\n
		
	VPP Instrument specific (see vpptype.h):\n
		Warnings:   0x3FFC0000 ,,, 0x3FFC07FF\n\n
		
	Driver specific:\n
		Warnings:   0x3FFC0800 ... 0x3FFC083F\n
		Errors:     0xBFFC0800 ... 0xBFFC083F\n\n

	Camera specific:\n
		Warnings:   0x3FFC0840 ... 0x3FFC0FFF\n
		Errors:     0xBFFC0840 ... 0xBFFC0FFF\n\n

*//*=========================================================================*/
/**@{*/ 
#define VI_SPECIFIC_STATUS_MIN   (0x3FFC0800L)                             ///< Minimum value of instrument specific ViStatus codes.
#define VI_SPECIFIC_STATUS_MAX   (0x3FFC0FFFL)                             ///< Maximum value of instrument specific ViStatus codes.
#define VI_SPECIFIC_ERROR_MIN    (_VI_ERROR + VI_SPECIFIC_STATUS_MIN)      ///< Minimum value of instrument specific ViStatus error codes.
#define VI_SPECIFIC_ERROR_MAX    (_VI_ERROR + VI_SPECIFIC_STATUS_MAX)      ///< Maximum value of instrument specific ViStatus error codes.
		 
#define VI_DRIVER_STATUS_OFFSET  (VI_SPECIFIC_STATUS_MIN)                  ///< Offset for driver specific ViStatus codes. (Warnings)
#define VI_DRIVER_ERROR_OFFSET   (VI_SPECIFIC_ERROR_MIN)                   ///< Offset for driver specific ViStatus codes. (Errors)
#define VI_INSTR_STATUS_OFFSET   (VI_DRIVER_STATUS_OFFSET + 0x00000040L)   ///< Offset for instrument specific ViStatus codes. (Warnings) (0x3FFC0840)
#define VI_INSTR_ERROR_OFFSET    (VI_DRIVER_ERROR_OFFSET + 0x00000040L)    ///< Offset for instrument specific ViStatus codes. (Errors) (0xBFFC0840)
		 
#define VI_ERROR_INVALID_IMAGE_SIZE    (VI_DRIVER_ERROR_OFFSET)            ///< The image dimensions are not as expected. 
#define VI_ERROR_NOT_SUPPORTED         (VI_DRIVER_ERROR_OFFSET + 1)        ///< this function is not supported in this mode 
#define VI_ERROR_INVALID_HANDLE        (VI_DRIVER_ERROR_OFFSET + 2)        ///< this camera handle is invalid 
#define VI_ERROR_NO_VALID_IMAGE        (VI_DRIVER_ERROR_OFFSET + 3)        ///< this camera image is invalid  

#define VI_ERROR_INV_CENTROID       (VI_DRIVER_ERROR_OFFSET + 4) ///< the calculated centroid is outside the image
#define VI_ERROR_INV_SIGMA          (VI_DRIVER_ERROR_OFFSET + 5) ///< the calculated sigma value is below 0 
#define VI_ERROR_INV_BEAM_WIDTH     (VI_DRIVER_ERROR_OFFSET + 6) ///< the calculated beam width is below 0
#define VI_ERROR_IMAGE_BRIGHT       (VI_DRIVER_ERROR_OFFSET + 7) ///< The image is too bright for this operation
		 
#define VI_WARN_CALC_AREA_CLIPPED   (VI_DRIVER_STATUS_OFFSET) ///< this calculation area has been clipped 
/**@}*/  

/*========================================================================*//**
\defgroup TLBC1_Trigger_Mode_x The trigger modes of the camera
*//*=========================================================================*/
/**@{*/
#define TLBC1_Trigger_Mode_No_Trigger              0  ///< Camera runs in continuous mode.
#define TLBC1_Trigger_Mode_HW_Trigger_Pulse        2  ///< Camera triggers on a TTL impulse and starts exposure after user defined delay.
#define TLBC1_Trigger_Mode_HW_Trigger_Repetition   3  ///< Camera triggers exposure with user defined frequency and phase locks on a TTL impulse.
/**@}*/ 

/*========================================================================*//**
\defgroup TLBC1_Trigger_Edge_x The edge for the trigger mode
\details If hardware trigger is used, the edge has to be defined where the trigger should appear
*//*=========================================================================*/
/**@{*/
#define TLBC1_Trigger_Edge_Rising   0  ///< Trigger on the rising edge
#define TLBC1_Trigger_Edge_Falling  1  ///< Trigger on the falling edge 
/**@}*/ 
		 
/*========================================================================*//**
\defgroup   TLBC1_CalcAreaForm_x Calculation Area Forms
\brief   Geometric form of the calculation area.
@{
*//*=========================================================================*/
#define TLBC1_CalcAreaForm_Rectangle   (0)   ///<  Nonrotated rectangular calculation area. Fast calculation in auto calc area mode.
#define TLBC1_CalcAreaForm_Ellipse     (1)   ///<  Rotated elliptical calculation area. Fast calculation in auto calc area mode.
#define TLBC1_CalcAreaForm_AutoIso     (2)   ///<  Automatically determined rotated rectangular calculation area. Iterative calculation, slower but ISO compliant. Only available in auto calc area mode.
/**@}*/  // defgroup TLBC1_CalcAreaForm_x

/*========================================================================*//**
\defgroup TLBC1_Profile_Position_x Predefined profile cut positions
*//*=========================================================================*/
/**@{*/
#define TLBC1_Profile_Position_ROI_Center          0  ///< Center of the ROI is the position of the profile cut. 
#define TLBC1_Profile_Position_Peak_Position       1  ///< The position of the highest intensity inside the calculation area.
#define TLBC1_Profile_Position_Centroid_Position   2  ///< The position of the centroid inside the calculation area.
#define TLBC1_Profile_Position_User_Position       3  ///< A user defined position inside the ROI.
/**@}*/ 

/*========================================================================*//**
\defgroup TLBC1_Ellipse_Mode_x Ellipse calculation modes
*//*=========================================================================*/
/**@{*/
#define TLBC1_Ellipse_Mode_ClipLevel_Contour 0  ///< find the countour where the intensities are around the clip level 
#define TLBC1_Ellipse_Mode_Approximated      1  ///< approximate the ellipse with the contour data
/**@}*/ 

/*========================================================================*//**
\defgroup TLBC1_MeasurementMethod_x Profile measurement methods
*//*=========================================================================*/
/**@{*/
#define TLBC1_MeasurementMethod_FullImage 0  ///< Use all pixel information in the calculation area.
#define TLBC1_MeasurementMethod_Slit      1  ///< Emulate a slit beam profiler.
/**@}*/ 
	
/*========================================================================*//**
\defgroup   TLBC1_AmbientLightCorrection_Mode_x Ambient Light Correction Mode
\brief   Ambient light correction modes.
@{
*//*=========================================================================*/
#define TLBC1_AmbientLightCorrection_Mode_Off   0  ///< Ambient light correction disabled. A device specific base level is used.
#define TLBC1_AmbientLightCorrection_Mode_On    1  ///< Ambient light correction enabled. Measurement configuration specific base level measured by a special procedure will be used.
/**@}*/  // defgroup TLBC1_AmbientLightCorrection_Mode_x

/*========================================================================*//**
\defgroup   TLBC1_AmbientLightCorrection_Status_x Ambient Light Correction Status
\brief   Ambient light correction status.
@{
*//*=========================================================================*/
#define TLBC1_AmbientLightCorrection_Status_Ready     0  ///< Ambient light correction data availabel. ALC can be enabled.
#define TLBC1_AmbientLightCorrection_Status_Never     1  ///< Ambient light correction measurement procedure was never run since TLBC1_init(). ALC can't be enabled.
#define TLBC1_AmbientLightCorrection_Status_Fail      2  ///< The latest ambient light correction measurement procedure failed. ALC can't be enabled.
/**@}*/  // defgroup TLBC1_AmbientLightCorrection_Status_x
		 
/*========================================================================*//**
\defgroup   TLBC1_AveragingMode_x Averaging Mode
\brief   Averaging mode.
@{
*//*=========================================================================*/
/*------------------------------------------------------------------------*//**
\brief   Floating average.
\details The latest measurement data is added to the stored measurement data
			and weightend.\n\n

			E.g. if the 100th measurement data arrived, it is added to the 99 
			other measurement data with a weightend of 1/100.
*//*-------------------------------------------------------------------------*/
#define TLBC1_AveragingMode_Floating      0  

/*------------------------------------------------------------------------*//**
\brief   Moving average.
\details The latest N measurement datas are summed up and averaged.
*//*-------------------------------------------------------------------------*/
#define TLBC1_AveragingMode_Moving        1
/**@}*/  // defgroup TLBC1_AveragingMode_x
		 
/*=============================================================================
 Global functions
=============================================================================*/

/*------------------------------------------------------------------------*//**
\brief   Initialize an instrument driver session
\details This function initializes the instrument driver session and performs
			the following initialization actions:
			- (1) Opens a session to the Default Resource Manager resource and a
					session to the selected device using the Resource Name.
			- (2) Performs an identification query on the Instrument.
			- (3) Resets the instrument to a known state.
			- (4) Sends initialization commands to the instrument.
			- (5) Returns an instrument handle which is used to differentiate
					between different sessions of this instrument driver.

\note    (1) Each time this function is invoked an unique session is opened.
\param[in] rsrcName  This parameter specifies the device (resource) with which
							to establish a communication session. It is recommended to
							use TLBC1_get_device_information() to get the valid
							resource string of a device available in the system.\n
							The syntax for the resource string is shown below.
							Optional segments are shown in square brackets ([]).
							Required segments that must be filled in are denoted by
							angle brackets (<>).

							\code{.txt}
							USB::0x1313::<product id>::<serial number>
							\endcode

							The product id codes for supported instruments are shown
							below.
							\code{.txt}
							Product ID   Instrument Type
							-------------------------------------------------
							0x8012       BC106 Camera Beam Profiler
							\endcode

							Example Resource Strings:
							USB::0x1313::0x8012::M12345678\n
							BC106 with the serial number M12345678.
\param[in] id_query  This parameter specifies whether an identification query
							is performed during the initialization process.
							- VI_OFF (0): Skip query.
							- VI_ON  (1): Do query (default).
\param[in] reset_instr  This parameter specifies whether the instrument is
							reset during the initialization process.
							- VI_OFF (0) - no reset
							- VI_ON  (1) - instrument is reset (default)
\param[out] vi       outputparameter
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_init (ViRsrc rsrcName, ViBoolean id_query, 
									 ViBoolean reset_instr, ViPSession vi);


/*------------------------------------------------------------------------*//**
\brief   Closes the instrument driver session.
\note    The instrument must be reinitialized to use it again.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_close (ViSession vi);     


/*========================================================================*//**
\defgroup   TLBC1_RESSOURCE_FUNC_x Ressource Functions
\brief   Use these functions to find and select devices in your system that can
			be controlled with this driver.
@{
*//*=========================================================================*/

/*------------------------------------------------------------------------*//**
\brief   Get the number of devices available in your system that can be 
			controlled with this driver.
\param[in] vi              This parameter is only needed for IVI compliant. Set
									to \a VI_NULL.
\param[out] device_count   Receives the number of connected devices.
\return                    Status code. For error codes and descriptions see 
									TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_device_count (ViSession vi,
														ViPUInt32 device_count); 

/*------------------------------------------------------------------------*//**
\brief   Get identification information of a connected device.
\details You don't have to open a session with the device with TLBC1_init() 
			before you can use this function.
\param[in] vi              This parameter is only needed for IVI compliance.
									Set to \a VI_NULL.
\param[in] device_index    The device's index. Valid values range from 0 to 
									(number of connected devices - 1) 
									(see TLBC1_get_device_count()).
\param[out] manufacturer   A 64 byte string buffer to receive the manufacturer 
									name. You may pass \a VI_NULL if you don't need this
									value.
\param[out] model_name     A 64 byte string buffer to receive the 
									instrument/model name. You may pass \a VI_NULL if
									you don't need this value.
\param[out] serial_number  A 64 byte string buffer to receive the serial 
									number. You may pass \a VI_NULL if you don't need 
									this value.
\param[out] device_available  \a VI_TRUE if the device is available (not used 
									by another program). You may pass \a VI_NULL if you 
									don't need this value.
\param[out] resource_name  A 256 byte string buffer to receive the ressource 
									identification string. Use this string in function
									TLBC1_init(). You may pass \a VI_NULL if you don't 
									need this value.
\return                    Status code. For error codes and descriptions see 
									TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_device_information (ViSession vi,
																ViUInt32 device_index,
																ViChar manufacturer[],
																ViChar model_name[],
																ViChar serial_number[],
																ViPBoolean device_available,
																ViChar resource_name[]); 
/**@}*/  // defgroup TLBC1_RESSOURCE_FUNC_x


/*========================================================================*//**
\defgroup   TLBC1_UTILITY_FUNC_x Utility Functions
\brief   This class of functions provides utility and lower level functions to
			communicate with the instrument.
@{
*//*=========================================================================*/

/*------------------------------------------------------------------------*//**
\brief   Run the device self-test routine.
\details detailedDescription
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] test_result This parameter contains the value returned from the
							device self test routine. A retured zero value indicates a
							successful run, A value other than zero indicates failure.
\param[out] test_message   This parameter returns the interpreted code as an
							user readable message string. The array must contain at
							least 256 elements ViChar[256].
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_self_test (ViSession vi, ViPInt16 test_result, ViChar test_message[]);

/*------------------------------------------------------------------------*//**
\brief   Reset the device.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_reset (ViSession vi);

/*------------------------------------------------------------------------*//**
\brief   This function returns the revision numbers of the instrument driver
			and the device firmware.
\param[in] vi        This parameter accepts the instrument handle returned by
							<Initialize> to select the desired instrument driver
							session. You may pass \a VI_NULL.\n\n

							Note: If you pass an invalid session handle the function
							will not abort with an error but an empty string will be
							returned for the firmware revision parameter \a instr_rev.
\param[out] driver_rev  This parameter returns the Instrument Driver
							revision.\n\n
							Notes:
							- (1) The array must contain at least 256 elements
									ViChar[256].
							- (2) You may pass \a VI_NULL if you don't need this
									value.
\param[out] instr_rev   This parameter returns the device firmware
							revision.\n\n
							Notes:
							- (1) The array must contain at least 256 elements
									ViChar[256].
							- (2) You may pass \a VI_NULL if you don't need this
									value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_revision_query (ViSession vi,ViChar driver_rev[],ViChar instr_rev[]);

/*------------------------------------------------------------------------*//**
\brief   Query the instrument's latest error code.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] error_code  This parameter returns the instrument error number. 
							You may pass \a VI_NULL if you don't need this value. 
\param[out] error_message  This parameter returns the instrument error 
							message.\n\n
							
							Notes:
							- (1) The array must contain at least 256 elements 
									ViChar[256].
							- (2) You may pass \a VI_NULL if you don't need this 
									value. 
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_error_query (ViSession vi, ViPInt32 error_code, ViChar error_message[]);

/*------------------------------------------------------------------------*//**
\brief   This function translates the error return value from a VXIplug&play
			instrument driver function to a user-readable string.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] status_code  The instrument error number.
\param[out] message  This parameter returns the interpreted code as an user
							readable message string.\n\n

							Notes:
							- (1) The array must contain at least 256 elements
									ViChar[256].
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_error_message (ViSession vi, ViStatus status_code, ViChar message[]);

/*------------------------------------------------------------------------*//**
\brief   This function returns the device identification information.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] instr_name  The instrument name (e.g. "BC106-VIS").\n\n

							Notes:
							- (1) The array must contain at least 256 elements
									ViChar[256].
							- (2) You may pass \a VI_NULL if you don't need this
									value.
\param[in] serial_number   The instrument's serial number 
							(e.g. "M1234567").\n\n

							Notes:
							- (1) The array must contain at least 256 elements
									ViChar[256].
							- (2) You may pass \a VI_NULL if you don't need this
									value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_identification_query (ViSession vi,
															 ViChar instr_name[],
															 ViChar serial_number[]);
/**@}*/  // defgroup TLBC1_UTILITY_FUNC_x


/*========================================================================*//**
\defgroup   TLBC1_CONFIGURATION_FUNC_x Data Acqisition Configuration Functions
\brief   Functions grouped into the data acquisition configuration class can be
			used to configure the acquisition parameters for a camera beam image.
@{
*//*=========================================================================*/

/*------------------------------------------------------------------------*//**
\brief   Get the gain value range supported by the instrument.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] min_gain Minimum gain factor.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] max_gain Maximum gain factor.\n
							You may pass \a VI_NULL if you don't need this value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_gain_range (ViSession vi, ViPUInt16 min_gain, ViPUInt16 max_gain);

/*------------------------------------------------------------------------*//**
\brief   Get the gain value.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] gain     The gain value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_gain (ViSession vi, ViPUInt16 gain);

/*------------------------------------------------------------------------*//**
\brief   Set the gain value.
\details Setting the gain value will disable auto exposure mode.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] gain      The gain value. For the value range supported by the 
							instrument see TLPC_get_gain_range().
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_gain (ViSession vi, ViUInt16 gain);


/*------------------------------------------------------------------------*//**
\brief   Get the auto exposure mode.
\details If auto exposure mode is enabled the data of the latest image is
			used to calculate improved exposure settings for the next scan. On
			fluctuating beam intensities it is still possible to scan dark or
			overexposed images as the new exposure parameters are only used for
			the next scan.\n
			If auto exposure mode is disabled the exposure parameters set with 
			TLBC1_set_exposure_time() are used.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] auto_exposure  The auto exposure mode
									- VI_ON  = Auto exposure is active
									- VI_OFF = Auto exposure is inactive
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_auto_exposure (ViSession vi,
														 ViPBoolean auto_exposure);

/*------------------------------------------------------------------------*//**
\brief   Set the auto exposure mode.
\details If auto exposure mode is enabled the data of the latest image is
			used to calculate improved exposure settings for the next scan. On
			fluctuating beam intensities it is still possible to scan dark or
			overexposed images as the new exposure parameters are only used for
			the next scan.\n
			If auto exposure mode is disabled the exposure parameters set with
			TLBC1_set_exposure_time() are used.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] auto_exposure   The auto exposure mode
									- VI_ON  = Auto exposure is active
									- VI_OFF = Auto exposure is inactive
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_auto_exposure (ViSession vi,
														 ViBoolean auto_exposure);

/*------------------------------------------------------------------------*//**
\brief   Get the exposure time value range supported by the instrument.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] t_min    Minimum exposure time in µs.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] t_max    Maximum exposure time in µs.\n
							You may pass \a VI_NULL if you don't need this value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_exposure_time_range (ViSession vi,
															  ViPReal64 t_min,
															  ViPReal64 t_max);


/*------------------------------------------------------------------------*//**
\brief   Get the exposure time.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] exposure_time  The exposure time in µs.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_exposure_time (ViSession vi,ViPReal64 exposure_time);


/*------------------------------------------------------------------------*//**
\brief   Set the exposure time.
\details Calling this function will disable auto exposure mode.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] exposure_time   The exposure time in µs. For the supported value
							range see TLBC1_get_exposure_time_range().
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_exposure_time (ViSession vi, ViReal64 exposure_time);

/*------------------------------------------------------------------------*//**
\brief   Get the precision mode.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] precision_mode The image precision:
							- 0 = Fast (with reduced intensity resolution)
							- 1 = Precise (with full intensity resolution)

\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_precision_mode (ViSession vi,
														  ViPUInt8 precision_mode);

/*------------------------------------------------------------------------*//**
\brief   Set the precision mode.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] precision_mode  The image precision:
							- 0 = Fast (with reduced intensity resolution)
							- 1 = Precise (with full intensity resolution)

\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_precision_mode (ViSession vi,
														  ViUInt8 precision_mode);

/*------------------------------------------------------------------------*//**
\brief   Get information about the sensor's dimensions.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] pixel_count_x  Count of horizontal pixel.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] pixel_count_y  Count of vertical pixel.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] pixel_pitch_horizontal  Distance in µm between the left edge of
							each pixel to the left edge of the horizontally next
							pixel.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] pixel_pitch_vertical Distance in µm between the top edge of each
							pixel to the top edge of the vertically next pixel.\n
							You may pass \a VI_NULL if you don't need this value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_sensor_information (ViSession vi,
																ViPUInt16 pixel_count_x,
																ViPUInt16 pixel_count_y,
																ViPReal64 pixel_pitch_horizontal,
																ViPReal64 pixel_pitch_vertical);

/*------------------------------------------------------------------------*//**
\brief   Get the rectangle of the ROI.
\details The ROI (Region Of Interest) defines a rectangular subarea of the
			sensor area whereas the maximum ROI is the full sensor size (see
			TLBC1_get_sensor_information()) and the smallest 32 x 32 pixel. Only
			image data of the selected ROI are transmitted from the device so that
			narrower ROI size reduces bandwidth and therefore increases
			measurement speed (frames per second).
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] left     Left border pixel index of the ROI rectangle.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] top      Top border pixel index of the ROI rectangle.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] width    Width of the ROI rectangle in pixel.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] height   Height of the ROI rectangle in pixel.\n
							You may pass \a VI_NULL if you don't need this value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_roi (ViSession vi, 
											ViPUInt16 left, ViPUInt16 top, 
											ViPUInt16 width, ViPUInt16 height);

/*------------------------------------------------------------------------*//**
\brief   Set the rectangle of the ROI.
\details The ROI (Region Of Interest) defines a rectangular subarea of the
			sensor area whereas the maximum ROI is the full sensor size (see
			TLBC1_get_sensor_information()) and the smallest 4 x 4 pixel. Only
			image data of the selected ROI are transmitted from the device so that
			narrower ROI size reduces bandwidth and therefore increases
			measurement speed (frames per second).\n
			Notes:
			- (1) This function will coerce the passed set values to fitting
					values. Use TLBC1_get_roi() to read back the actual ROI
					rectangle after setting them.
			- (2) This function tries to retain the currently set calculation
					area. In case that the current calculation area doesn't fit into
					the new ROI the calculation area will be reset to full ROI.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] left     Left border pixel index of the ROI rectangle.
\param[out] top      Top border pixel index of the ROI rectangle.
\param[out] width    Width of the ROI rectangle in pixel.
\param[out] height   Height of the ROI rectangle in pixel.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_roi (ViSession vi, 
											ViUInt16 left, ViUInt16 top, 
											ViUInt16 width, ViUInt16 height);

/*------------------------------------------------------------------------*//**
\brief   Get the currently set trigger parameters.
\details The electrical TTL level trigger input can be used to synchronize
			laser pulses to the camera exposure time. By default 'No Trigger' is
			chosen for continuous caption of CW light sources.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] trigger_mode   The trigger mode (see \ref TLBC1_Trigger_Mode_x):
							- 0 = No Trigger: The beam profiler continuously scans
									for images as fast as possible.
							- 2 = Hardware Trigger Pulse: The beam profiler triggers
									on a selected edge of a TTL signal at the device's
									BNC connector and starts exposure after a user
									defined delay time.
							- 3 = Hardware Trigger Repetition: The beam profiler
									starts exposure with a user defined frequency and
									phase locks on a selected edge of a TTL signal at
									the device's BNC connector.\n\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] trigger_value  The meaning of this value depends on the
							\a trigger_mode:
							- No Trigger: The value is not used.
							- Hardware Trigger Pulse: Delay in µs from trigger to
							  start of exposure.
							- Hardware Trigger Repetition: Repetition frequency in
							  kHz. \n\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] trigger_edge   The trigger edge (see \ref TLBC1_Trigger_Edge_x):
							- 0 = Rising Edge
							- 1 = Falling Edge\n\n
							The value is not used in No Trigger mode. You may pass
							\a VI_NULL if you don't need this value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_trigger (ViSession vi,
												 ViPUInt16 trigger_mode,
												 ViPUInt32 trigger_value,
												 ViPUInt16 trigger_edge);

/*------------------------------------------------------------------------*//**
\brief   Set the trigger parameters.
\details The electrical TTL level trigger input can be used to synchronize
			laser pulses to the camera exposure time. By default 'No Trigger' is
			chosen for continuous caption of CW light sources.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] trigger_mode The trigger mode (see \ref TLBC1_Trigger_Mode_x):
							- 0 = No Trigger: The beam profiler continuously scans
									for images as fast as possible.
							- 2 = Hardware Trigger Pulse: The beam profiler triggers
									on a selected edge of a TTL signal at the device's
									BNC connector and starts exposure after a user
									defined delay time.
							- 3 = Hardware Trigger Repetition: The beam profiler
									starts exposure with a user defined frequency and
									phase locks on a selected edge of a TTL signal at
									the device's BNC connector.
\param[in] trigger_value   The meaning of this value depends on the
							\a trigger_mode:
							- No Trigger: The value is ignored.
							- Hardware Trigger Pulse: Delay in µs from trigger to
							  start of exposure.
							- Hardware Trigger Repetition: Repetition frequency in
							  kHz.
\param[in] trigger_edge The trigger edge (see \ref TLBC1_Trigger_Edge_x):
							- 0 = Rising Edge
							- 1 = Falling Edge\n\n
							The value is ignored in No Trigger mode.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_trigger (ViSession vi,
												 ViUInt16 trigger_mode,
												 ViUInt32 trigger_value,
												 ViUInt16 trigger_edge);

/*------------------------------------------------------------------------*//**
\brief   Get the ambient light correction mode
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] mode     The ambient light correction mode (see
							\ref TLBC1_AmbientLightCorrection_Mode_x).
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_ambient_light_correction_mode (ViSession vi,
																			  ViUInt8 *mode);

/*------------------------------------------------------------------------*//**
\brief   Set the ambient light correction mode
\details The ambient light correction procedure must have been run successfully
			before ambient light correction can be enabled. See also
			TLBC1_run_ambient_light_correction() and
			TLBC1_get_ambient_light_correction_status()
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] mode      The ambient light correction mode (see
							\ref TLBC1_AmbientLightCorrection_Mode_x).
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_ambient_light_correction_mode (ViSession vi,
																			  ViUInt8 mode);

/*------------------------------------------------------------------------*//**
\brief   Get the ambient light correction data status
\details This status indicates the availability of valid ambient light
			correction data. The data is volatile and needs to be measured at least
			once after TLBC1_init(). Ambient light correction can't be enabled if
			there is no valid ALC data.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] status   The ambient light correction status (see
							\ref TLBC1_AmbientLightCorrection_Status_x).
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_ambient_light_correction_status (ViSession vi,
																				 ViUInt8 *status);

/*------------------------------------------------------------------------*//**
\brief   Run the ambient light correction measurement
\details Block the laser beam while running this measurement. The function will
			block until the measurement is done. This can take up to 30 seconds.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_run_ambient_light_correction (ViSession vi);

/*------------------------------------------------------------------------*//**
\brief   Get the bad pixel correction support information.
\details Depending on the device's firmware the bad pixel correction feature
			is supported or not.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] supported   \a VI_TRUE if the device supports the bad pixel
							correction feature.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_bad_pixel_correction_support (ViSession vi,
																			 ViBoolean *supported);

/*------------------------------------------------------------------------*//**
\brief   Get the bad pixel correction mode.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] enabled  \a VI_ON if bad pixel correction is enabled. If the bad
							pixel correction feature is not supported by the device
							this parameter will always receive \a false (see
							TLBC1_get_bad_pixel_correction_support()).
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_bad_pixel_correction_mode (ViSession vi,
																		 ViBoolean *enabled);

/*------------------------------------------------------------------------*//**
\brief   Set the bad pixel correction mode.
\details This function will return an error if bad pixel correction is not
			supported by the device (see TLBC1_get_bad_pixel_correction_support()).
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] enable   Set \a VI_ON to enable bad pixel correction.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_bad_pixel_correction_mode (ViSession vi,
																		 ViBoolean enable);

/*------------------------------------------------------------------------*//**
\brief   Run the bad pixel correction measurement
\details Switch off the laser and cover the camera aperture while running this
			measurement. The function will block until the measurement is done.
			This can take up to 30 seconds.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_run_bad_pixel_correction (ViSession vi);

/**@}*/  // defgroup TLBC1_CONFIGURATION_FUNC_x


/*========================================================================*//**
\defgroup   TLBC1_CALCULATION_FUNC_x Calculation Setup Functions
\brief   This class of functions groups functions that access parameters for
			the analysis of a camera beam image.
@{
*//*=========================================================================*/

/*------------------------------------------------------------------------*//**
\brief   Get the wavelength value range supported by the instrument.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] min_wavelength Minimum wavelength in nm.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] max_wavelength Maximum wavelength in nm.\n
							You may pass \a VI_NULL if you don't need this value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_wavelength_range (ViSession vi, 
														  ViPReal64 min_wavelength, 
														  ViPReal64 max_wavelength);

/*------------------------------------------------------------------------*//**
\brief   Get the wavelength value.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] wavelength  The wavelength in nm.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_wavelength (ViSession vi, ViPReal64 wavelength);

/*------------------------------------------------------------------------*//**
\brief   Set the wavelength value.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] wavelength   The wavelength in nm.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_wavelength (ViSession vi, ViReal64 wavelength);

/*------------------------------------------------------------------------*//**
\brief   Get the user power offset used in power calculation.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] power_offset   Gets the power offset in dBm that is set by the 
									user. The default value is 0.0 dBm.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_user_power_offset (ViSession vi,
															  ViPReal64 power_offset);

/*------------------------------------------------------------------------*//**
\brief   Set the user power offset used in power calculation.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] power_offset   The power offset in dBm. The default value is 
							0.0 dBm.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_user_power_offset (ViSession vi,
															  ViReal64 power_offset);

/*------------------------------------------------------------------------*//**
\brief   Get the clip level where the clipped beam width is calulated.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] clip_level  Clip level as fraction from the peak to the base line
							(value range 0.05 to 0.95).
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_clip_level (ViSession vi,
													 ViPReal64 clip_level);

/*------------------------------------------------------------------------*//**
\brief   Set the clip level where the clipped beam width is calulated.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] clip_level   Clip level as fraction from the peak to the base line
							(value range 0.05 to 0.95).
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_clip_level (ViSession vi,
													 ViReal64 clip_level);

/*------------------------------------------------------------------------*//**
\brief   Get the attenuation of a filter placed in front of the beam profiler
			camera.
\details This attenuation value is used to calculate a correct power
			measurement value.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] attenuation The filter attenuation in dB.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_attenuation (ViSession vi,
													  ViPReal64 attenuation);

/*------------------------------------------------------------------------*//**
\brief   Set the attenuation of a filter placed in front of the beam profiler
			camera.
\details This attenuation value is used to calculate a correct power
			measurement value.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] attenuation  The filter attenuation in dB.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_attenuation (ViSession vi,
													  ViReal64 attenuation);

/*------------------------------------------------------------------------*//**
\brief   Get the averaging parameters.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] mode     The averaging mode. See \ref TLBC1_AveragingMode_x.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] value    The number of measurements to build the average.\n
							You may pass \a VI_NULL if you don't need this value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_averaging (ViSession vi,
													ViPUInt8 mode,
													ViPUInt16 value);

/*------------------------------------------------------------------------*//**
\brief   Set the averaging parameters.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] mode      The averaging mode. See \ref TLBC1_AveragingMode_x.
\param[in] value     The number of measurements to build the average (value 
							range \ref MIN_AVERAGE_COUNT to \ref MAX_AVERAGE_COUNT).
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_averging (ViSession vi,
												  ViUInt8 mode,
												  ViUInt16 value);

/*------------------------------------------------------------------------*//**
\brief   Get the position where x and y plots are taken.
\details The position defines the intersection point of two right angled cut
			lines in the image rectangle. The gaussian fit x and y are calculated
			on the resulting cut profiles along those lines.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] preset   Profile position preset
							(see \ref TLBC1_Profile_Position_x).\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] x_position  The x pixel position of the intersection point. This
							parameter is only used with preset
							\ref TLBC1_Profile_Position_User_Position.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] y_position  The y pixel position of the intersection point. This
							parameter is only used with preset
							\ref TLBC1_Profile_Position_User_Position.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] rotation_angle The rotation angle of the cut lines in degree.
							Postive values rotate against the clock.\n
							You may pass \a VI_NULL if you don't need this value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_profile_cut_position (ViSession vi,
																  ViPUInt8 preset,
																  ViPUInt16 x_position,
																  ViPUInt16 y_position,
																  ViPReal64 rotation_angle);

/*------------------------------------------------------------------------*//**
\brief   Set the position where x and y plots are taken.
\details The position defines the intersection point of two right angled cut
			lines in the image rectangle. The gaussian fit x and y are calculated
			on the resulting cut profiles along those lines.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] preset    Profile position preset
							(see \ref TLBC1_Profile_Position_x).
\param[in] x_position   The x pixel position of the intersection point. This
							parameter is only used with preset
							\ref TLBC1_Profile_Position_User_Position.
\param[in] y_position   The y pixel position of the intersection point. This
							parameter is only used with preset
							\ref TLBC1_Profile_Position_User_Position.
\param[in] rotation_angle  The rotation angle of the cut lines in degree.
							Postive values rotate against the clock.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_profile_cut_position (ViSession vi,
																  ViUInt8 preset,
																  ViUInt16 x_position,
																  ViUInt16 y_position,
																  ViReal64 rotation_angle);

/*------------------------------------------------------------------------*//**
\brief   Get the method for determining the calculation area.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] automatic If \a VI_ON the driver tries to automatically find a good
							calculation area for each image. It uses the \a form
							parameter and the clip level set in
							TLBC1_set_auto_calculation_area_clip_level() as
							parameters.\n
							If \a VI_OFF the user defined rectangle/ellipse
							(TLBC1_set_user_calculation_area()) according to \a form
							is used as calculation area.\n
							You may pass \a VI_NULL if you don't need this value.
\param[in] form      The form of the calculation area. See
							\ref TLBC1_CalcAreaForm_x.\n
							You may pass \a VI_NULL if you don't need this value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_calculation_area_mode (ViSession vi,
																	ViBoolean *automatic,
																	ViUInt8 *form);

/*------------------------------------------------------------------------*//**
\brief   Select the method for determining the calculation area.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] automatic If \a VI_ON the driver tries to automatically find a good
							calculation area for each image. It uses the \a form
							parameter and the clip level set in
							TLBC1_set_auto_calculation_area_clip_level() as
							parameters.\n
							If \a VI_OFF the user defined rectangle/ellipse
							(TLBC1_set_user_calculation_area()) according to \a form
							is used as calculation area.
\param[in] form      The form of the calculation area. See
							\ref TLBC1_CalcAreaForm_x.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_calculation_area_mode (ViSession vi,
																	ViBoolean automatic,
																	ViUInt8 form);

/*------------------------------------------------------------------------*//**
\brief   Get the geometry of the user defined calculation area.
\details These parameters only have effect on the calculation results if the
			calculation area mode is set to user defined.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] centerX  Horizontal calculation area center pixel position.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] centerY  Vertical calculation area center pixel position.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] width    Width of the calculation area in pixels before rotating by 
							\a angle.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] height   Height of the calculation area in pixels before rotating 
							by \a angle.\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] angle    The calculation area rotation angle in degree. The
							rectangle/ellipse defined by \a centerX, \a centerY, 
							\a width, and \a height will be rotated by this angle. 
							Positive values rotate against the clock.\n
							You may pass \a VI_NULL if you don't need this value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_user_calculation_area (ViSession vi,
																	ViReal64 *centerX, 
																	ViReal64 *centerY,
																	ViReal64 *width,
																	ViReal64 *height,
																	ViReal64 *angle);

/*------------------------------------------------------------------------*//**
\brief   Set the geometry of the user defined calculation area.
\details These parameters only have effect on the calculation results if the
			calculation area mode is set to user defined.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] centerX   Horizontal calculation area center pixel position.
\param[in] centerY   Vertical calculation area center pixel position.
\param[in] width     Width of the calculation area in pixels before rotating by 
							\a angle.
\param[in] height    Height of the calculation area in pixels before rotating 
							by \a angle.
\param[in] angle     The calculation area rotation angle in degree. The
							rectangle/ellipse defined by \a centerX, \a centerY, 
							\a width, and \a height will be rotated by this angle. 
							Positive values rotate against the clock.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_user_calculation_area (ViSession vi,
																	ViReal64 centerX, 
																	ViReal64 centerY,
																	ViReal64 width,
																	ViReal64 height,
																	ViReal64 angle);

/*------------------------------------------------------------------------*//**
\brief   Get the automatic calculation area clip level.
\details This clip level is used when automatically determining a calculation
			area. The parameter only has an effect on the calculation results if
			the calculation area mode is set to automatic.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] clipLevel The clip level as fraction of the current image's peak
							intensity.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_auto_calculation_area_clip_level (ViSession vi,
																				  ViReal64 *clipLevel);

/*------------------------------------------------------------------------*//**
\brief   Set the automatic calculation area clip level.
\details This clip level is used when automatically determining a calculation
			area. The parameter only has an effect on the calculation results if
			the calculation area mode is set to automatic.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[in] clipLevel The clip level as fraction of the current image's peak
							intensity. (value range 0.0 to 1.0)
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_auto_calculation_area_clip_level (ViSession vi,
																				  ViReal64 clipLevel);

ViStatus _VI_FUNC TLBC1_get_max_hold (ViSession vi,
												  ViPBoolean max_hold);

ViStatus _VI_FUNC TLBC1_set_max_hold (ViSession vi,
												  ViBoolean max_hold);

ViStatus _VI_FUNC TLBC1_get_ellipse_mode (ViSession vi,
														ViPUInt8 mode);

ViStatus _VI_FUNC TLBC1_set_ellipse_mode (ViSession vi,
														ViUInt8 mode);

ViStatus _VI_FUNC TLBC1_get_rotation_angle (ViSession vi,
														  ViPReal64 rotation_angle);

ViStatus _VI_FUNC TLBC1_set_rotation_angle (ViSession vi,
														  ViReal64 rotation_angle);

/*------------------------------------------------------------------------*//**
\brief   Get the measurement method.
\details The camera beam profiler can emulate a slit beam profiler. This can be
			useful to compare slit beam profiler measurements with the camera beam
			profiler's measurements.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] method   The measurement method (see \ref TLBC1_MeasurementMethod_x):
							- 0 = Full Image: The full camera image is used to
									calculate beam width and profile fits. (default)
							- 1 = Slit Emulation: The driver emulates slit beam
									profiler measurement data from the image and uses
									that data for beam width and profile fit
									calculation.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_measurement_method (ViSession vi,
																ViUInt8 *method);

/*------------------------------------------------------------------------*//**
\brief   Set the measurement method.
\details The camera beam profiler can emulate a slit beam profiler. This can be
			useful to compare slit beam profiler measurements with the camera beam
			profiler's measurements.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] method   The measurement method (see \ref TLBC1_MeasurementMethod_x):
							- 0 = Full Image: The full camera image is used to
									calculate beam width and profile fits. (default)
							- 1 = Slit Emulation: The driver emulates slit beam
									profiler measurement data from the image and uses
									that data for beam width and profile fit
									calculation.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_set_measurement_method (ViSession vi,
																ViUInt8 method);

/**@}*/  // defgroup TLBC1_CALCULATION_FUNC_x


/*========================================================================*//**
\defgroup   TLBC1_DATA_FUNC_x Data Functions
\brief   This class of functions transfers measurement data from the 
			instrument. 
@{
*//*=========================================================================*/

/*------------------------------------------------------------------------*//**
\brief   Get all in one scan data.
\details This function reads out one image from the beam profiler camera
			according to the current acquisition parameters (see 
			\ref TLBC1_CONFIGURATION_FUNC_x).\n
			That image is immediately analysed according to the current image 
			analye parameters (see \ref TLBC1_CALCULATION_FUNC_x).\n
			Finally the image and all analyze results are saved into the 
			\a scan_data.
\param[in] vi           This parameter accepts the instrument handle returned
								by TLBC1_init() to select the desired instrument driver
								session.
\param[out] scan_data   Receives the all in one scan data including analysis 
								results.
\return  Status code. For error codes and descriptions see 
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_scan_data (ViSession vi, TLBC1_Calculations * const scan_data);


/*------------------------------------------------------------------------*//**
\brief   Analyzes a image of the Thorlabs Beam Camera.
\details This function analyzes the image as it is taken from the beam profiler
			camera according to the current acquisition parameters (see 
			\ref TLBC1_CONFIGURATION_FUNC_x).\n
			Finally the image and all analyze results are saved into the 
			\a scan_data.
\param[in] vi           This parameter accepts the instrument handle returned
								by TLBC1_init() to select the desired instrument driver
								session. You need an open session to a beam profiler to
								use this function.
\param[in] fileName     The filename of the image with an absolute path.
\param[out] scan_data   Receives the all in one scan data including analysis 
								results.
\param[in] calcArea		Holds the parameter that are used to define the calculation area.
\param[in] profileCut	Presets to define the profile cut position.
\param[in] clipLevel		Clip level for beam width at clip level calculation.
\param[in] ellipseMode	Ellipse calculation modes. See \ref TLBC1_Ellipse_Mode_x.
\param[in] coordinateSystemRotationAngle	For future purposes. You must pass 0.0!
\param[in] measureMethod	Profile measurement methods. See \ref TLBC1_MeasurementMethod_x.
\return  Status code. For error codes and descriptions see TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_analyze_image( ViSession vi, 
													ViChar const fileName[],
													TLBC1_Calculations * const scan_data, 
													calc_area_t calcArea,
													profile_cut_t profileCut,
													ViReal64 clipLevel,
													ViUInt8 ellipseMode,
													ViReal64 coordinateSystemRotationAngle,
													ViUInt8 measureMethod);

/**@}*/  // defgroup TLBC1_DATA_FUNC_x


/*========================================================================*//**
\defgroup   TLBC1_LABVIEWDATA_x LabVIEW Functions
\brief   LabView convenience functions
\details This class of functions provides a convenient way for LabView users to
			transfers measurement data from the instrument and access the beam
			analyse results without using LabVIEW clusters. 
@{
*//*=========================================================================*/

ViStatus _VI_FUNC TLBC1_request_new_measurement (ViSession vi);

ViStatus _VI_FUNC TLBC1_get_image (ViSession vi,
											  ViUInt8 pixel_data[], ViPUInt16 image_width,
											  ViPUInt16 image_height, ViPUInt8 bytes_per_pixel);

ViStatus _VI_FUNC TLBC1_get_profiles (ViSession vi, ViReal64 profile_x[], ViReal64 profile_y[]);

ViStatus _VI_FUNC TLBC1_get_peak (ViSession vi,
											 ViPUInt16 peak_intensity,
											 ViPUInt16 peak_position_x,
											 ViPUInt16 peak_position_y);

/*------------------------------------------------------------------------*//**
\brief   Get the saturation.
\details Saturation level of the instrument's AD converter for the current
			image. For a good SNR (signal-to-noise ratio), the saturation level
			should be not below 40% and not beyond 95%.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] saturation  Ratio of the highest intensity in the scan to the
							dynamic range of the sensor (Value range 0.0 ... 1.0).\n
							You may pass \a VI_NULL if you don't need this value.
\param[out] saturatedPixel Ratio of the amount of saturated pixels to amount of
							pixels inside the calculation area (Value range
							0.0 ... 1.0).\n
							You may pass \a VI_NULL if you don't need this value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_saturation (ViSession vi,
													 ViReal64 *saturation,
													 ViReal64 *saturatedPixel);

ViStatus _VI_FUNC TLBC1_get_centroid (ViSession vi,
												  ViPUInt16 centroid_position_x,
												  ViPUInt16 centroid_position_y);

ViStatus _VI_FUNC TLBC1_get_power (ViSession vi,
											  ViPReal64 total_power);

/*------------------------------------------------------------------------*//**
\brief   Get the measured peak power density.
\details Peak power density is the power on the peak pixel divided by its area.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] peakDensity The peak power density in mW/µm².
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_peak_power_density (ViSession vi,
																ViPReal64 peakDensity);

ViStatus _VI_FUNC TLBC1_get_beam_width (ViSession vi,
													 ViPReal64 beam_width_clip_x,
													 ViPReal64 beam_width_clip_y,
													 ViPReal64 sigma_x, ViPReal64 sigma_y);

/*------------------------------------------------------------------------*//**
\brief   Returns the geometry of the actual calculation area.
\details If calculation area is user defined this function will return the same
			data as was set in TLBC1_set_calculation_area().\n
			If calculation area is determined automatically this function will
			return the automatically found geometry. For the form see
			TLBC1_set_calculation_area().
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] left     Calculation area left border pixel index. The specified
							pixel colum is included to the calculation area. You may 
							pass \a VI_NULL if you don't need this value.
\param[out] top      Calculation area top border pixel index. The specified
							pixel row is included to the calculation area. You may 
							pass \a VI_NULL if you don't need this value.
\param[out] width    Gets the width of the calculation area in pixel. You may 
							pass \a VI_NULL if you don't need this value.
\param[out] height   Gets the height of the calculation area in pixel. You may 
							pass \a VI_NULL if you don't need this value.
\param[out] angle    The rotation angle in degree by which the
							rectangle/ellipse will be rotated. Positive values rotate
							against the clock. You may pass \a VI_NULL if you don't 
							need this value.
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_actual_calculation_area (ViSession vi,
																	  ViReal64 *centerX,
																	  ViReal64 *centerY,
																	  ViReal64 *width,
																	  ViReal64 *height,
																	  ViReal64 *angle);

ViStatus _VI_FUNC TLBC1_get_ellipse_diameters (ViSession vi,
															  ViPReal64 minorAxisDiameter,
															  ViPReal64 majorAxisDiameter,
															  ViPReal64 meanDiameter);

ViStatus _VI_FUNC TLBC1_get_ellipse_geometry (ViSession vi,
															 ViPReal64 orientation,
															 ViPReal64 ellipticity,
															 ViPReal64 eccentricity,
															 ViPReal64 centerXPosition,
															 ViPReal64 centerYPosition);

/*------------------------------------------------------------------------*//**
\brief   Get the effective area.
\param[in] vi        This parameter accepts the instrument handle returned by
							TLBC1_init() to select the desired instrument driver
							session.
\param[out] effectiveArea  Area of an ideal flat top beam with same peak 
							intensity as the measured beam in µm².
\return  Status code. For error codes and descriptions see
			TLBC1_error_message().
*//*-------------------------------------------------------------------------*/
ViStatus _VI_FUNC TLBC1_get_effective_area (ViSession vi,
														  ViPReal64 effectiveArea);

/**@}*/  // defgroup TLBC1_LABVIEWDATA_x
/**@}*/  // defgroup TLBC1_INSTR_DRIVER_x


#ifdef __cplusplus
	 }
#endif

#endif  /* ndef __TLBC1_H__ */
