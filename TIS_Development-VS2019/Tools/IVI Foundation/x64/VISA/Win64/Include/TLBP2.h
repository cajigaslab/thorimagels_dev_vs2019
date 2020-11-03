/**************************************************************************//**

   \file          TLBP2.h

   \brief         VXIpnp driver for the Thorlabs BP2 multi slit beam profiler


   Thorlabs GmbH - Thorlabs Beam - BP209 - Multi Slit Beam Profiler

   \date          09-Jul-2011
   \copyright     2011-2013, Thorlabs GmbH

******************************************************************************/

#ifndef __TLBP2_H__
#define __TLBP2_H__

#include <vpptype.h>  

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files
//==============================================================================
// Constants

/*========================================================================*//**
\defgroup  BP2_x Instrument Parameter
\brief     These parameter are useful for the initialization of an instrument
@{
*//*=========================================================================*/

#define BP2_VI_FIND_RSC_PATTERN	("USB?*?{VI_ATTR_MANF_ID==0x1313 && VI_ATTR_MODEL_CODE==0x8019}") ///< resource pattern for the Visa resource manager function 'viFindRsrc()' 
#define BP2_MAX_DEVICE_COUNT 		(10)			 	///< maximal count of BP2 instrument that can be connected 
#define BP2_MAX_AD_VALUE 			(0x7AFF)			///< highest intensity value of a sample

/**@}*/   // End of defgroup BP2_STATUS_x

/*========================================================================*//**
\defgroup  BP2_STATUS_x Status codes
\brief     These status flags can be returned in the GetStatus function
@{
*//*=========================================================================*/

// status codes
#define BP2_STATUS_SCAN_AVAILABLE      (0x0001)   ///< A scan is available to read over USB.
#define BP2_STATUS_DRUM_STABILIZED    	(0x0002)   ///< The drum has been stabilized and the measurement can be used.
#define BP2_STATUS_DRUM_DESTABILIZED   (0x0004)   ///< The drum has been destabilized and the measurement shoukd be rejected. 
/**@}*/   // End of defgroup BP2_STATUS_x
		 
/*========================================================================*//**
\defgroup  BP2_Err_x Error codes
\brief     These error codes flags can be returned in a driver function
@{
*//*=========================================================================*/

// error and warning codes
#define VI_INSTR_WARNING_OFFSET                    (0x3FFC0900L)
#define VI_INSTR_ERROR_OFFSET          (_VI_ERROR + 0x3FFC0900L)  //0xBFFC0900
		 
#define BP2_ERR_NO_NEW_DATA      		(VI_INSTR_ERROR_OFFSET + 0x00)   ///< No new scan is available
#define BP2_ERR_INV_INSTR_DATA     		(VI_INSTR_ERROR_OFFSET + 0x01)   ///< The internal data is not initialized 
#define BP2_ERR_INV_OBJECT	      		(VI_INSTR_ERROR_OFFSET + 0x02)   ///< One parameter is not initialized
#define BP2_ERR_PARAMETER_OUT_OF_RANGE (VI_INSTR_ERROR_OFFSET + 0x03)   ///< One parameter is out of range
#define BP2_ERR_INV_CONSTR_COUNTER  	(VI_INSTR_ERROR_OFFSET + 0x04)   ///< The construction counter is not initialized
#define BP2_ERR_INV_ELAPSED_COUNTER  	(VI_INSTR_ERROR_OFFSET + 0x05)   ///< The elapsed time could not be measured because no scan has been finished.
#define BP2_ERR_INV_DATA_SIZE  			(VI_INSTR_ERROR_OFFSET + 0x06)   ///< The requested data does not match with the received data 
		 
#define BP2_ERR_NSUP_STATE  				(VI_INSTR_ERROR_OFFSET + 0x07)   ///< The device is not in the correct mode to support this operation
#define BP2_ERR_SLIT_UNUSED  				(VI_INSTR_ERROR_OFFSET + 0x08)   ///< The slit is not used 
#define BP2_ERR_NO_PEAK_FOUND				(VI_INSTR_ERROR_OFFSET + 0x09)   ///< No valid peak was found
#define BP2_ERR_NO_VALID_WIDTH			(VI_INSTR_ERROR_OFFSET + 0xA0)   ///< No valid beam width was found 
#define BP2_ERR_NO_VALID_CENTROID		(VI_INSTR_ERROR_OFFSET + 0xA1)   ///< No valid centroid was found  
		 
#define BP2_ERR_NO_VALID_CALIBRATION	(VI_INSTR_ERROR_OFFSET + 0xA2)   ///< No valid calibration found  
		 
#define BP2_WARN_NO_BEAM_WIDTH_CLIPX		(VI_INSTR_WARNING_OFFSET + 0x01)   ///< Invalid beam width x
#define BP2_WARN_NO_BEAM_WIDTH_CLIPY		(VI_INSTR_WARNING_OFFSET + 0x02)   ///< Invalid beam width y
		 
#define BP2_WARN_UNKNOWN_ERROR				(VI_INSTR_WARNING_OFFSET + 0x03)   ///< The error code is unknown 
#define BP2_WARN_PARAMETER_OUT_OF_RANGE	(VI_INSTR_WARNING_OFFSET + 0x04)   ///< The used wavelength/FWBias is out of the response range
/**@}*/   // End of defgroup BP2_Err_x
		 
/*========================================================================*//**
\defgroup  BP2_Baseline_Mode_x Base Line Calclation Modes
\brief     The mode for the base line sets how the base line is calculated
@{
*//*=========================================================================*/

// base line mode
#define BP2_BASELINE_MODE_DARK_WINDOW     (0)   ///< The base line is the mean intensity of the dark window
#define BP2_BASELINE_MODE_FIRST_SAMPLES  	(1)   ///< The base line is the mean intensity of the first 10 samples of the slit window
#define BP2_BASELINE_MODE_USER_VALUE	  	(2)	///< The base line value is given by the user	 

/**@}*/   // End of defgroup BP2_Baseline_Mode_x
		 
/*========================================================================*//**
\defgroup  BP2_REFERENCE_POSITION_PRESET_x Predefined reference position for the calculation results which are positions
\brief     The preset for the reference position which can depend on a calculation result and can change from scan to scan
@{
*//*=========================================================================*/

// base line mode
#define BP2_REFERENCE_POSITION_PRESET_SENSOR_CENTER		(0)   ///< The reference position is the center of the sensor
#define BP2_REFERENCE_POSITION_PRESET_ROI_CENTER 			(1)   ///< The reference position is the center of the defined roi
#define BP2_REFERENCE_POSITION_PRESET_PEAK_POSITION		(2)	///< The reference position is the calculated peak position. The peak position is the origin of the position coordinate system (0,0)	 
#define BP2_REFERENCE_POSITION_PRESET_CENTROID_POSITION	(3)	///< The reference position is the calculated centroid position. The centroid position is the origin of the position coordinate system (0,0)	 
#define BP2_REFERENCE_POSITION_PRESET_USER_POSITION		(4)	///< The reference position is a user defined position within the sensor dimensions 

/**@}*/   // End of defgroup BP2_Baseline_Mode_x

//==============================================================================
// Types
		 
/*========================================================================*//**
\struct  BP2_DEVICE
\brief   parameters to identify a device during the initialization
*//*=========================================================================*/
typedef struct{
	ViChar resourceString[256]; ///< unique resource string of the pattern "USB0::0x1313::0x8019::Mxxxxxxxx::RAW"
}BP2_DEVICE;

/*========================================================================*//**
\struct  BP2_SLIT_DATA
\brief   contains the native measurement which is given by the instrument
and the result from the dark level calculation
\details This data are used to calculate the beam profile parameter in the function TLBP2_get_slit_scan_data()
*//*=========================================================================*/
typedef struct{
	ViUInt16 slit_sample_count;					///< count of samples for this slit (maximal 7500)
	ViReal32 slit_dark_level;						///< calculated dark level of the dark window for this slit
	ViReal32 slit_samples_intensities[7500];  ///< array of the sample intensities in digits in the range from -darkLevel to BP2_MAX_AD_VALUE with dark level correction
	ViReal32	slit_samples_positions[7500];  	///< position of the sample in µm to the first sample
	ViUInt16 encoder_increments;					///< count of encoder increments inside the slit window (ca. 130); used to set the correct A/D frequency to cover the slit with the measurement
}BP2_SLIT_DATA;

/*========================================================================*//**
\struct  BP2_CALCULATIONS
\brief   contains the result of the beam profile analyze
\details Results from the function TLBP2_get_slit_scan_data()  
*//*=========================================================================*/
typedef struct{
	
	ViBoolean isValid;					///< Are all parameter calculated correctly?
	
	ViUInt16	peakIndex;					///< Index of the sample in the intensity array that contains the peak
	ViReal32 peakPosition;				///< Position of highest intensity in the profile in µm 
	ViReal32 peakIntensity; 			///< Profile intensity in percent (range measured from the dark level to the upper value of the AD converter) 

	ViUInt16 centroidIndex; 			///< Index of the sample that is nearest to the centroid position
	ViReal32	centroidPosition;			///< Calculated centroid position in µm
	
	ViReal32 beamWidthClip; 			///< Beam width in µm measured from the peak to the clip level left and right

	ViReal32 gaussianFitAmplitude;	///< Highest intensity value of the gaussian fit in digits (with dark level correction)
	ViReal32 gaussianFitCentroid; 	///< Position of the centroid of the gaussian fit in µm
	ViReal32 gaussianFitDiameter; 	///< Diameter of the gaussian fit in µm
	ViReal32 gaussianFitPercentage;  ///< Percentage of the conformity of the measured profile with the gaussian fit curve
	ViReal32 gaussianFitCurve[7500]; ///< Array of calculated intensities of the gaussian fit curve (with dark level correction)      
	
	ViReal32 besselFitPercentage;		///< Percentage of the conformity of the measured profile with the bessel fit curve
	ViReal32 besselFitCurve[7500];	///< Array of calculated intensities of the bessel fit curve (with dark level correction)      

	ViReal32 sigma; 						///< Calculated sigma value of the profile
	
	ViReal32 calcAreaLeftBorder;		///< left border in µm either automatically calculated or set by user
	ViReal32 calcAreaRightBorder; 	///< right border in µm either automatically calculated or set by user 
	
}BP2_CALCULATIONS;

/*------------------------------------------------------------------------*//**
\brief    Mode of samples interpretation
\details  The samples can contain the slit scanning information 
or the integration in a knife edge mode
*//*-------------------------------------------------------------------------*/
typedef enum{
	BP2_Slit_Scanning 		= 0,		///< The beam width is wider than four times the slit width -> the samples are slit scanned
	BP2_Knife_Edge_Scanning = 1		///< The beam width is smaller than the slit width -> the intensities are the integrated profile
}BP2_RECONSTRUCTION_MODE;

//==============================================================================
// The following functions are documentated inside the function panel.
// The documentation from the function panel is extracted to the folder 'TLBP2_files' an can be access with the file 'TLBP2.html'
//==============================================================================
// Global functions
		 
ViStatus _VI_FUNC TLBP2_get_connected_devices (ViSession vi, BP2_DEVICE _VI_FAR device_list[], ViUInt32 *device_count); 

ViStatus _VI_FUNC TLBP2_init (ViRsrc rsrcName, ViBoolean id_query, ViBoolean reset_instr, ViPSession vi);

ViStatus _VI_FUNC TLBP2_close (ViSession vi);  

//==============================================================================
// Utility functions
//==============================================================================  
ViStatus _VI_FUNC TLBP2_reset(ViSession vi);

ViStatus _VI_FUNC TLBP2_self_test(ViSession vi, ViPInt16 test_result, ViChar test_message[]);

ViStatus _VI_FUNC TLBP2_revision_query(ViSession vi, ViChar driver_rev[], ViChar instr_rev[]);

ViStatus _VI_FUNC TLBP2_error_query(ViSession vi, ViPInt32 error_code, ViChar error_message[]);

ViStatus _VI_FUNC TLBP2_error_message(ViSession vi, ViStatus status_code, ViChar message[]);

//==============================================================================
// Service functions
//==============================================================================
ViStatus _VI_FUNC TLBP2_set_service_mode (ViSession vi,ViUInt8 password);


ViStatus _VI_FUNC TLBP2_get_instrument_name (ViSession vi,ViChar instrument_name[]);

ViStatus _VI_FUNC TLBP2_set_instrument_name (ViSession vi, const ViChar instrument_name[]);


ViStatus _VI_FUNC TLBP2_get_serial_number (ViSession vi, ViChar serial_number[]);

ViStatus _VI_FUNC TLBP2_set_serial_number (ViSession vi, const ViChar serial_number[]);

ViStatus _VI_FUNC TLBP2_get_elapsed_time_counter (ViSession vi,ViPUInt16 years, ViPUInt8 months, ViPUInt8 days,
                                                  ViPUInt8 hours, ViPUInt8 minutes);

ViStatus _VI_FUNC TLBP2_update_elapsed_time_counter (ViSession vi);

ViStatus _VI_FUNC TLBP2_get_cpld_version (ViSession vi,
                                          ViChar cpld_version[]);


ViStatus _VI_FUNC TLBP2_get_wavelength_range (ViSession vi,ViPUInt16 min_wavelength,ViPUInt16 max_wavelength);

ViStatus _VI_FUNC TLBP2_set_wavelength_range (ViSession vi,ViUInt16 min_wavelength,ViUInt16 max_wavelength);


ViStatus _VI_FUNC TLBP2_get_start_offset_range (ViSession vi, 
														ViPUInt16 min_offset, 
														ViPUInt16 max_offset);

ViStatus _VI_FUNC TLBP2_get_start_offset (ViSession vi, ViPUInt16 offset);

ViStatus _VI_FUNC TLBP2_set_start_offset (ViSession vi, ViUInt16 offset);

ViStatus _VI_FUNC TLBP2_get_drum_circum (ViSession vi, ViPReal64 drum_circum);

ViStatus _VI_FUNC TLBP2_set_drum_circum (ViSession vi, ViReal64 drum_circum);


ViStatus _VI_FUNC TLBP2_get_fwbias_range (ViSession vi, 
														ViPUInt16 min_fw_bias, 
														ViPUInt16 max_fw_bias);

ViStatus _VI_FUNC TLBP2_get_fwbias (ViSession vi, ViPUInt16 fw_bias);

ViStatus _VI_FUNC TLBP2_set_fwbias (ViSession vi, ViUInt16 fw_bias);


ViStatus _VI_FUNC TLBP2_get_slit_parameter (ViSession vi,
                                            ViUInt8 slit_used[],
                                            ViUInt8 slit_length[],
                                            ViUInt8 slit_width[],
														  ViUInt8 slit_orientation[]);

ViStatus _VI_FUNC TLBP2_set_slit_parameter (ViSession vi,
                                            const ViUInt8 slit_used[],
                                            const ViUInt8 slit_length[],
                                            const ViUInt8 slit_width[],
														  const ViUInt8 slit_orientation[]);

ViStatus _VI_FUNC TLBP2_get_sensor_response (ViSession vi,
                                             ViPUInt8 response_data_count,
                                             ViPUInt16 lowest_wavelength,
                                             ViPUInt16 highest_wavelength,
                                             ViPUInt8 wavelength_step,
                                             ViUInt16 response_data[]);

ViStatus _VI_FUNC TLBP2_set_sensor_response (ViSession vi,
															ViUInt8 response_data_count,
                                             ViUInt16 lowest_wavelength,
                                             ViUInt16 highest_wavelength,
                                             ViUInt8 wavelength_step,
                                             const ViUInt16 response_data[]);

ViStatus _VI_FUNC TLBP2_get_construction_parameter (ViSession vi,
                                                    ViPUInt8 constrution_year,
                                                    ViPUInt8 constrution_month,
                                                    ViPUInt8 constrution_day,
                                                    ViChar assembler_name[]);

ViStatus _VI_FUNC TLBP2_set_construction_parameter (ViSession vi,
                                                    ViUInt8 constrution_year,
                                                    ViUInt8 constrution_month,
                                                    ViUInt8 constrution_day,
                                                    const ViChar assembler_name[]);

ViStatus _VI_FUNC TLBP2_get_calibration_parameter (ViSession vi,
                                                   ViPUInt8 calibration_year,
                                                   ViPUInt8 calibration_month,
                                                   ViPUInt8 calibration_day,
                                                   ViChar calibration_firmware[],
                                                   ViChar calibration_SW_version[],
                                                   ViChar assembler_name[]);

ViStatus _VI_FUNC TLBP2_set_calibration_parameter (ViSession vi,
                                                   ViUInt8 calibration_year,
                                                   ViUInt8 calibration_month,
                                                   ViUInt8 calibration_day,
                                                   const ViChar calibration_firmware[],
                                                   const ViChar calibration_SW_version[],
                                                   const ViChar assembler_name[]);


ViStatus _VI_FUNC TLBP2_get_slit_position_range (ViSession vi,
																 ViPUInt16 min_position, 
																 ViPUInt16 max_position);

ViStatus _VI_FUNC TLBP2_get_slit_positions (ViSession vi, ViUInt16 slit_positions[]);

ViStatus _VI_FUNC TLBP2_set_slit_positions (ViSession vi, const ViUInt16 slit_positions[]);

ViStatus _VI_FUNC TLBP2_save_slit_positions (ViSession vi, const ViUInt16 slit_positions[]);

ViStatus _VI_FUNC TLBP2_get_slit_positions_roi (ViSession vi, ViUInt16 slit_positions_roi[]);

ViStatus _VI_FUNC TLBP2_set_slit_positions_roi (ViSession vi, const ViUInt16 slit_positions_roi[]);

ViStatus _VI_FUNC TLBP2_save_slit_positions_roi (ViSession vi, const ViUInt16 slit_positions_roi[]);


ViStatus _VI_FUNC TLBP2_get_motor_dac_range (ViSession vi,
															ViPUInt16 min_dac_value, 
															ViPUInt16 max_dac_value);

ViStatus _VI_FUNC TLBP2_get_motor_dac (ViSession vi,ViPUInt16 motor_dac_value);

ViStatus _VI_FUNC TLBP2_set_motor_dac (ViSession vi,ViUInt16 motor_dac_value);

ViStatus _VI_FUNC TLBP2_get_motor_dac_5_20hz (ViSession vi, ViPUInt16 dac_value_5hz, ViPUInt16 dac_value_20hz);

ViStatus _VI_FUNC TLBP2_set_motor_dac_5_20hz (ViSession vi, ViUInt16 dac_value_5hz, ViUInt16 dac_value_20hz) ;

ViStatus _VI_FUNC TLBP2_get_construction_operating_hours_counter (ViSession vi,
                                                                  ViPUInt16 days,
                                                                  ViPUInt8 hours,
                                                                  ViPUInt8 minutes);

ViStatus _VI_FUNC TLBP2_get_calibration_operating_hours_counter (ViSession vi,
                                                                 ViPUInt16 days,
                                                                 ViPUInt8 hours,
                                                                 ViPUInt8 minutes);

ViStatus _VI_FUNC TLBP2_reset_calibration_operating_hours_counter (ViSession vi);

ViStatus _VI_FUNC TLBP2_reset_construction_operating_hours_counter (ViSession vi);

ViStatus _VI_FUNC TLBP2_get_power_factor (ViSession vi, ViPReal64 power_factor);

ViStatus _VI_FUNC TLBP2_set_power_factor (ViSession vi, ViReal64 power_factor);
//==============================================================================
// Action/Status functions
//==============================================================================
ViStatus _VI_FUNC TLBP2_get_device_status (ViSession vi, ViPUInt16 device_status);



//==============================================================================
// Configuration functions
//==============================================================================
ViStatus _VI_FUNC TLBP2_get_slit_samples_count_range (ViSession vi,
                                                      ViPUInt16 min_sample_count,
                                                      ViPUInt16 max_sample_count);

ViStatus _VI_FUNC TLBP2_get_slit_samples_counts (ViSession vi,ViUInt16 sample_count_buffer[]);


ViStatus _VI_FUNC TLBP2_set_slit_samples_counts (ViSession vi, const ViUInt16 sample_count_buffer[]);

ViStatus _VI_FUNC TLBP2_get_slit_samples_frequency_range (ViSession vi,
                                                          ViPReal32 min_samples_frequency,
                                                          ViPReal32 max_samples_frequency);

ViStatus _VI_FUNC TLBP2_get_slit_samples_frequencies (ViSession vi, ViReal64 samples_frequencies_buffer[]);

ViStatus _VI_FUNC TLBP2_set_slit_samples_frequencies (ViSession vi, const ViReal64 samples_frequencies_buffer[]);

ViStatus _VI_FUNC TLBP2_get_offset_range (ViSession vi,
                                          ViPUInt16 minOffset,
                                          ViPUInt16 maxOffset);

ViStatus _VI_FUNC TLBP2_get_offsets (ViSession vi,
                                     ViUInt16 offset_slits[],
                                     ViPUInt16 offset_power);

ViStatus _VI_FUNC TLBP2_set_offsets (ViSession vi,
                                     ViUInt16 offset_slits[],
                                     ViUInt16 offset_power);


ViStatus _VI_FUNC TLBP2_get_gain_range (ViSession vi,
													 ViPUInt8 min_gain, 
													 ViPUInt8 max_gain);

ViStatus _VI_FUNC TLBP2_get_gains (ViSession vi,ViUInt8 gain_buffer[], ViPUInt8 gain_power);

ViStatus _VI_FUNC TLBP2_set_gains (ViSession vi,const ViUInt8 gain_buffer[], ViUInt8 gain_power);

ViStatus _VI_FUNC TLBP2_get_auto_gain (ViSession vi,ViPBoolean auto_gain);

ViStatus _VI_FUNC TLBP2_set_auto_gain (ViSession vi,ViBoolean auto_gain);


ViStatus _VI_FUNC TLBP2_get_bandwidth_range (ViSession vi,
                                             ViPReal64 min_bandwidth,
                                             ViPReal64 max_bandwidth);

ViStatus _VI_FUNC TLBP2_get_bandwidths (ViSession vi,ViReal64 bandwidth_buffer[]);

ViStatus _VI_FUNC TLBP2_set_bandwidths (ViSession vi, const ViReal64 bandwidth_buffer[]);


ViStatus _VI_FUNC TLBP2_get_drum_speed_range (ViSession vi,
                                              ViPReal64 min_speed,
                                              ViPReal64 max_speed);

ViStatus _VI_FUNC TLBP2_get_drum_speed (ViSession vi,ViPReal64 drum_speed);

ViStatus _VI_FUNC TLBP2_set_drum_speed (ViSession vi,ViReal64 drum_speed);

ViStatus _VI_FUNC TLBP2_set_drum_speed_ex (ViSession vi,
                                           ViReal64 drum_speed,
                                           ViPUInt16 sample_count,
                                           ViPReal64 sample_resolution);

ViStatus _VI_FUNC TLBP2_add_drum_speed_offset (ViSession vi,
                                               ViReal32 drum_speed_offset);

ViStatus _VI_FUNC TLBP2_clear_drum_speed_offset (ViSession vi);

ViStatus _VI_FUNC TLBP2_get_speed_correction (ViSession vi,
                                              ViPBoolean correction);

ViStatus _VI_FUNC TLBP2_set_speed_correction (ViSession vi,
                                              ViBoolean correction);

ViStatus _VI_FUNC TLBP2_get_averaged_drum_speed (ViSession vi,
                                                 ViPReal64 drum_speed);


ViStatus _VI_FUNC TLBP2_get_user_power_factor_range (ViSession vi,
                                                ViPReal64 min_power_factor,
                                                ViPReal64 max_power_factor);

ViStatus _VI_FUNC TLBP2_get_user_power_factor (ViSession vi,
                                               ViPReal64 power_factor);

ViStatus _VI_FUNC TLBP2_set_user_power_factor (ViSession vi,
                                               ViReal64 power_factor);

ViStatus _VI_FUNC TLBP2_get_wavelength (ViSession vi, ViPReal64 wavelength);

ViStatus _VI_FUNC TLBP2_set_wavelength (ViSession vi, ViReal64 wavelength);

ViStatus _VI_FUNC TLBP2_get_use_roi (ViSession vi,ViPBoolean use_roi);

ViStatus _VI_FUNC TLBP2_set_use_roi (ViSession vi,ViBoolean use_roi);

//==============================================================================
// Calculation functions
//==============================================================================
ViStatus _VI_FUNC TLBP2_get_averaging (ViSession vi,
                                       ViPUInt8 average_count,
                                       ViPUInt8 average_mode);

ViStatus _VI_FUNC TLBP2_set_averaging (ViSession vi,
                                       ViUInt8 average_count,
                                       ViUInt8 average_mode);

ViStatus _VI_FUNC TLBP2_get_scanning_method (ViSession vi,
															ViUInt8 slit_index,
                                             ViPUInt8 scanning_method);

ViStatus _VI_FUNC TLBP2_set_scanning_method (ViSession vi,
															ViUInt8 slit_index,
                                             ViUInt8 scanning_method);

ViStatus _VI_FUNC TLBP2_get_clip_level (ViSession vi,
                                        ViPReal32 clip_level);

ViStatus _VI_FUNC TLBP2_set_clip_level (ViSession vi,
                                        ViReal32 clip_level);

ViStatus _VI_FUNC TLBP2_get_max_hold (ViSession vi,
                                      ViPBoolean max_hold);

ViStatus _VI_FUNC TLBP2_set_max_hold (ViSession vi,
                                      ViBoolean max_hold);

ViStatus _VI_FUNC TLBP2_get_base_line(ViSession vi, ViUInt8 slit_index, ViPUInt8 mode, ViPReal32 base_line);

ViStatus _VI_FUNC TLBP2_set_base_line (ViSession vi, ViUInt8 slit_index, ViUInt8 mode, ViReal32 base_line);   

ViStatus _VI_FUNC TLBP2_get_beam_width_correction (ViSession vi,ViUInt8 slit_index,ViPBoolean beam_width_correction);

ViStatus _VI_FUNC TLBP2_set_beam_width_correction (ViSession vi,ViUInt8 slit_index, ViBoolean beam_width_correction);

ViStatus _VI_FUNC TLBP2_get_calculation_area (ViSession vi,
                                              ViUInt8 slit_index, ViPBoolean mode,
                                              ViPReal32 clip_level,
                                              ViPReal32 left_border,
                                              ViPReal32 right_border);

ViStatus _VI_FUNC TLBP2_set_calculation_area (ViSession vi,
                                              ViUInt8 slit_index, ViBoolean mode,
                                              ViReal32 clip_level,
                                              ViReal32 left_border,
                                              ViReal32 right_border);

ViStatus _VI_FUNC TLBP2_get_position_correction (ViSession vi,
                                                 ViPBoolean position_correction);

ViStatus _VI_FUNC TLBP2_set_position_correction (ViSession vi,
                                                 ViBoolean position_correction);

ViStatus _VI_FUNC TLBP2_get_reference_position (ViSession vi,ViUInt8 slit_index,
                                                ViPUInt8 preset,
                                                ViPReal64 reference_position);

ViStatus _VI_FUNC TLBP2_set_reference_position (ViSession vi,ViUInt8 slit_index,
                                                ViUInt8 preset,
                                                ViReal64 reference_position);

//==============================================================================
// Data functions
//==============================================================================

ViStatus _VI_FUNC TLBP2_get_slit_scan_data (ViSession vi,
                                            BP2_SLIT_DATA slit_data[],
														  BP2_CALCULATIONS calculation_result[],
                                            ViPReal64 power,
														  ViPReal32 powerSaturation,
														  ViReal64 power_intensities[]);

ViStatus _VI_FUNC TLBP2_get_knife_edge_reconstruction (ViSession vi,
                                                       const BP2_SLIT_DATA slit_data[],
                                                       const BP2_CALCULATIONS calculation_result[],
                                                       const ViBoolean slit_indices[],
                                                       BP2_SLIT_DATA slit_data_knife_edge[],
                                                       BP2_CALCULATIONS calculation_results_knife_edge[]);

ViStatus _VI_FUNC TLBP2_get_drum_elapsed_times (ViSession vi, ViUInt16 elapsed_time_buffer[]);  
//==============================================================================
// LabView functions
//============================================================================== 

ViStatus _VI_FUNC TLBP2_request_scan_data (ViSession vi,
                                           ViPReal64 power,
                                           ViPReal32 power_window_saturation,
														 ViReal64 power_intensities[]);

ViStatus _VI_FUNC TLBP2_get_scan_data_information (ViSession vi,
                                                   ViUInt8 slit_index,
                                                   ViPUInt16 sample_count,
                                                   ViPReal32 dark_level);

ViStatus _VI_FUNC TLBP2_get_sample_intensities (ViSession vi,
                                                ViUInt8 slit_index,
                                                ViReal64 sample_intensities[],
                                                ViReal64 sample_positions[]);

ViStatus _VI_FUNC TLBP2_get_slit_peak (ViSession vi,
                                       ViUInt8 slit_index, 
													ViPUInt16 peak_index,
                                       ViPReal32 peak_position,
                                       ViPReal32 peak_intensity);

ViStatus _VI_FUNC TLBP2_get_slit_centroid (ViSession vi,
                                           ViUInt8 slit_index,
                                           ViPUInt16 centroid_index,
                                           ViPReal32 centroid_position);

ViStatus _VI_FUNC TLBP2_get_slit_beam_width (ViSession vi,
                                             ViUInt8 slit_index,
                                             ViPReal32 beam_width_clip,
                                             ViPReal32 beam_width_sigma);

ViStatus _VI_FUNC TLBP2_get_slit_gaussian_fit (ViSession vi,
                                               ViUInt8 slit_index,
                                               ViPReal32 gaussian_fit_amplitude,
                                               ViPReal32 gaussian_fit_diameter,
                                               ViPReal32 gaussian_fit_percentage,
                                               ViReal64 gaussian_fit_intensities[]);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __TLBP2_H__ */
