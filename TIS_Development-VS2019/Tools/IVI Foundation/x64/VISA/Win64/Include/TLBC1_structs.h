#ifndef __TLBC1_structs_H__
#define __TLBC1_structs_H__

/*========================================================================*//**
\struct  param_range_t
\brief   Holds the min and maximum value and the step
*//*=========================================================================*/
typedef struct
{
	ViReal64      minValue;	///< minimum value
	ViReal64      maxValue; ///< maximum value
	ViReal64      step;     ///< modify at count of step
} param_range_t;

/*========================================================================*//**
\struct rect_u16_t
\brief   A rectangle
*//*=========================================================================*/
typedef struct
{
	ViUInt16      left;		///< left border of the rectangle (0...)
	ViUInt16      top;      ///< top border of the rectangle (0...)
	ViUInt16      width;    ///< width of the rectangle
	ViUInt16		  height;	///< height of the rectangle 
} rect_u16_t;

/*========================================================================*//**
\struct alc_map_t
\brief   Ambient light correction data set.
*//*=========================================================================*/
typedef struct
{
	ViReal64 exposureTime;	///< Currently used exposure time in µs.
	ViReal64	darkLevel;		///< Dark level in digits (is always a 12 bit value).
} alc_map_t;

/*========================================================================*//**
\struct  calc_area_t
\brief   Holds the parameter that are used to define the calculation area
*//*=========================================================================*/
typedef struct
{
	ViBoolean   autoMode;   ///< \a VI_TRUE if automatic calculation area has to be used.
	ViUInt8     form;       ///< The form of the calculation area. See \?ref TLBC1_CalcAreaForm_x.
	ViReal64    centerX;    ///< Horizontal calculation area center pixel position.
	ViReal64    centerY;    ///< Vertical calculation area center pixel position.
	ViReal64    width;      ///< Width of the calculation area in pixel before rotating by \ref angle.
	ViReal64    height;     ///< Height of the calculation area in pixel before rotating by \ref angle.
	ViReal64    angle;      ///< Angle how the calculation area is rotated in degree. Positive values rotate against the clock.
	ViReal64    clipLevel;  ///< Clip level for the calculation area relative to the peak level (base line corrected). Value 1.0 = 100%
} calc_area_t;

/*========================================================================*//**
\struct  profile_cut_t
\brief   Parameters to define a profile cut position
*//*=========================================================================*/
typedef struct
{
	ViUInt8     preset;     ///< Predefined profile cut position (\ref TLBC1_Profile_Position_x)
	ViUInt16    positionX;  ///< Profile cut position in x direction used with preset \ref TLBC1_Profile_Position_User_Position
	ViUInt16    positionY;  ///< Profile cut position in y direction used with preset \ref TLBC1_Profile_Position_User_Position
	ViReal64    angle;      ///< Profile cut cross is rotated around the position by angle in degree. Only used with preset \ref TLBC1_Profile_Position_User_Position
} profile_cut_t;

#endif
