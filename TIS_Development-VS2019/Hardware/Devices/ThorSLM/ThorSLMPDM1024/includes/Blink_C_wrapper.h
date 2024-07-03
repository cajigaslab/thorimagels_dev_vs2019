//
//:  Blink_SDK_C_wrapper for programming languages that can interface with DLLs
//
//   (c) Copyright Meadowlark Optics 2017, All Rights Reserved.


#ifndef BLINK_C_WRAPPER_H_
#define BLINK_C_WRAPPER_

#ifdef BLINK_C_WRAPPER_EXPORTS
#define BLINK_C_WRAPPER_API __declspec(dllexport)
#else
#define BLINK_C_WRAPPER_API __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C" { /* using a C++ compiler */
#endif

  BLINK_C_WRAPPER_API void Create_SDK(unsigned int SLM_bit_depth,
                                      unsigned int* n_boards_found,
                                      int *constructed_ok,
                                      int is_nematic_type,
                                      int RAM_write_enable,
                                      int use_GPU_if_available,
                                      int max_transient_frames,
                                      char* static_regional_lut_file);

  BLINK_C_WRAPPER_API void Delete_SDK();

  BLINK_C_WRAPPER_API
  int Is_slm_transient_constructed();

  BLINK_C_WRAPPER_API
  int Write_overdrive_image(int board,
                            unsigned char* target_phase,
                            int wait_for_trigger,
                            int flip_immediate,
                            int external_pulse,
                            unsigned int trigger_timeout_ms);

  BLINK_C_WRAPPER_API
  int Calculate_transient_frames(unsigned char* target_phase,
                                 unsigned int* byte_count);

  BLINK_C_WRAPPER_API
  int Retrieve_transient_frames(unsigned char* frame_buffer);

  BLINK_C_WRAPPER_API
  int Write_transient_frames(int board,
                             unsigned char* frame_buffer,
                             int wait_for_trigger,
                             int flip_immediate,
                             int external_puls,
                             unsigned int trigger_timeout_ms);

  BLINK_C_WRAPPER_API
  int Read_transient_buffer_size(char *filename,
                                 unsigned int* byte_count);

  BLINK_C_WRAPPER_API
  int Read_transient_buffer(char *filename,
                            unsigned int byte_count,
                            unsigned char *frame_buffer);

  BLINK_C_WRAPPER_API
  int Save_transient_frames(char *filename,
                            unsigned char *frame_buffer);

  BLINK_C_WRAPPER_API
  const char* Get_last_error_message();

  BLINK_C_WRAPPER_API
  int Load_overdrive_LUT_file(char* static_regional_lut_file);

  BLINK_C_WRAPPER_API
  int Load_linear_LUT(int board);

  BLINK_C_WRAPPER_API
  const char* Get_version_info();

  BLINK_C_WRAPPER_API
  void SLM_power(int power_state);

  // ----------------------------------------------------------------------------
  //  Write_image
  // ----------------------------------------------------------------------------
  BLINK_C_WRAPPER_API
  int Write_image(int board,
                  unsigned char* image,
                  unsigned int image_size,
                  int wait_for_trigger,
                  int flip_immediate,
                  int output_pulse_image_flip,
                  int output_pulse_image_refresh,
                  unsigned int trigger_timeout_ms);

  BLINK_C_WRAPPER_API int ImageWriteComplete(int board, unsigned int trigger_timeout_ms);

  //These functions are specific to the 1k

  BLINK_C_WRAPPER_API
  int Load_sequence(int board, 
      unsigned char* image_array,
      unsigned int image_size,
      int ListLength,
      int wait_for_trigger,
      int flip_immediate,
      int output_pulse_image_flip,
      int output_pulse_image_refresh,
      unsigned int trigger_timeout_ms);

  BLINK_C_WRAPPER_API 
  int Select_image(int board,
    int frame,
    int wait_for_trigger,
    int flip_immediate,
    int output_pulse_image_flip,
    int output_pulse_image_refresh,
    unsigned int flip_timeout_ms);

  // ----------------------------------------------------------------------------
  //  Load_LUT_file
  // ----------------------------------------------------------------------------
  BLINK_C_WRAPPER_API int Load_LUT_file(int board, char* LUT_file);

  BLINK_C_WRAPPER_API int Load_LUT_Arrays(int board, const unsigned short* RampUp, const unsigned short* RampDown, const unsigned short* PreRampUp, const unsigned short* PreRampDown, const unsigned short* PostRampUp, const unsigned short* PostRampDown);

  BLINK_C_WRAPPER_API int SetRampDelay(int board, unsigned int ramp_delay);

  BLINK_C_WRAPPER_API int SetPreRampSlope(int board, unsigned int preRampSlope);

  BLINK_C_WRAPPER_API int SetPostRampSlope(int board, unsigned int postRampSlope);

  BLINK_C_WRAPPER_API int Synchronize();

  BLINK_C_WRAPPER_API int SetDACTestMode(int board, int enableTestMode);

  BLINK_C_WRAPPER_API int SetDACTestValue(int board, int DACValue);

  BLINK_C_WRAPPER_API int GetDACVals(int board, int* DACValues);


  // ----------------------------------------------------------------------------
  //  Compute_TF
  // ----------------------------------------------------------------------------
  BLINK_C_WRAPPER_API int Compute_TF(float frame_rate);

  // ----------------------------------------------------------------------------
  //  Set_true_frames
  // ----------------------------------------------------------------------------
  BLINK_C_WRAPPER_API void Set_true_frames(int true_frames);

  BLINK_C_WRAPPER_API void Stop_sequence();

  BLINK_C_WRAPPER_API double Read_SLM_temperature(int board);

  BLINK_C_WRAPPER_API int Get_image_width(int board);

  BLINK_C_WRAPPER_API int Get_image_height(int board);

  BLINK_C_WRAPPER_API int Get_image_depth(int board);

  BLINK_C_WRAPPER_API int Read_Serial_Number(int board);

  BLINK_C_WRAPPER_API double Get_pixel_pitch(int board);

  BLINK_C_WRAPPER_API double Get_cover_voltage(int board);

  BLINK_C_WRAPPER_API int Set_cover_voltage(int board, double Voltage);


#ifdef __cplusplus
}
#endif

#endif //BLINK_C_WRAPPER_