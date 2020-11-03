/**************************************************************************//**

	\file          
	\ingroup       Thorlabs BC1 VISA VXIpnp Driver
	\brief         Caluclation Cluster for the BC1 Beam Analysis

	Thorlabs GmbH - Thorlabs Beam - BC1 - Camera Beam Profiler

	\date          19-Jul-2011
	\copyright     copyright(c) 2006-2012, Thorlabs GmbH

******************************************************************************/
#ifndef __TLBC1_CALCULATIONS_H__
#define __TLBC1_CALCULATIONS_H__


/*=============================================================================
  Macros
=============================================================================*/
#define TLBC1_MAX_ROWS		(1024)	///<	Maximum number of vertical pixel.
#define TLBC1_MAX_COLUMNS	(1360)	///<	Maximum number of horizontal pixel.


/*========================================================================*//**
\struct	TLBC1_Calculations
\brief	Beam analysis results for one scan.
\details The positions and widths are calculated in pixel. The origin of the 
			coordinate system is the upper left corner of the image. To get the
			widths in µm please multiply the widths in pixel with the factor
			'pixelPitch' from the device settings TLBC1_Device.
*//*=========================================================================*/
typedef struct
{
	ViBoolean	isValid;				///< \a VI_TRUE if scan analysis results are valid; \a VI_FALSE if an error occured during analysis and the calculations data is invalid.
	
	ViReal64		baseLevel;			///< Mean noise of the sensor in [digits] (typically 47 digits for 12 bit images and 3 digits for 8 bit images)
	ViReal64		minIntensity;		///< Minimum intensity of the measurement
	ViReal64		maxIntensity;		///< Pixel intensity value for maximum intensity / resoltution of the pixel intensities.
	ViReal64 	saturation;			///< Ratio of the highest intensity in the scan to the dynamic range of the sensor (value range 0.0 ... 1.0).
	ViReal64		saturatedPixel;	///< Ratio of the amount of saturated pixels to amount of pixels inside the calculation area (value range 0.0 ... 1.0).
	
	ViUInt16		imageWidth;			///< Pixel per horizontal line.
	ViUInt16		imageHeight;		///< Pixel per vertical column.

	// calculated on the whole image
	ViUInt16		peakPositionX;			///< Peak x pixel position.
	ViUInt16		peakPositionY;			///< Peak y pixel position.
	ViReal64		peakIntensity;			///< Highest pixel intensity value inside the calculation area.
	ViReal32		centroidPositionX;	///< Centroid x pixel position.
	ViReal32		centroidPositionY;	///< Centroid y pixel position.
	ViReal32		fourSigmaX;				///< Horizontal standard deviation in pixel.
	ViReal32		fourSigmaY;				///< Vertical standard deviation in pixel.
	ViReal32		fourSigmaR;				///< Radial standard deviation in pixel. ("generalized beam diameter" from ISO 11146-2)
	
	ViReal64		beamWidthIsoX;			///< beam with x measured by the ISO 11146-2 along one axis of the beam profile
	ViReal64		beamWidthIsoY;			///< beam with y measured by the ISO 11146-2 along one axis of the beam profile prependicular to the beam width x
	ViReal64		azimuthAngle;			///< azimuth angle measured clockwise by the ISO 11146-2. The angle i between the x axis of the laboratory system and that of the principal axis which is cloder to the x-axis

	ViReal32		ellipseDiaMin;			///< Ellipse minor axis diameter in [pixel]
	ViReal32		ellipseDiaMax;			///< Ellipse major axis diameter in [pixel]
	ViReal32		ellipseDiaMean;		///< Ellipse diameter arithmetic mean value in [pixel]
	ViReal32		ellipseOrientation;  ///< Ellipse orientation angle in degree.
	ViReal32		ellipseEllipticity;  ///< The ellipse's ratio of minor axis diameter to major axis diameter.
	ViReal32		ellipseEccentricity; ///< The ellipse's eccentricity.
	ViReal32		ellipseCenterX;		///< Ellipse center x pixel position.
	ViReal32		ellipseCenterY;		///< Ellipse center y pixel position.
	ViReal32		ellipseFitAmplitude; ///< Ellipse amplitude in Fourier fit (in pixel).

	ViReal32		totalPower;				///< Total power in dBm
	ViReal32		peakPowerDensity;		///< Peak power density in mW/µm²

	// calculated on the selected x-y-profile
	ViReal32		beamWidthClipX;		///< Horizontal beam width at clip level in pixel.
	ViReal32		beamWidthClipY;		///< Vertical beam width at clip level in pixel.

	ViReal32		gaussianFitCentroidPositionX;	///< Centroid x pixel position for the gaussian profile.
	ViReal32		gaussianFitCentroidPositionY;	///< Centroid y pixel position for the gaussian profile.
	ViReal32		gaussianFitRatingX;				///< Ratio of actual data to the gaussian fit of the x profile.
	ViReal32		gaussianFitRatingY;				///< Ratio of actual data to the gaussian fit of the y profile.
	
	ViReal32		calcAreaCenterX;		///< Calculation area left border. The specified pixel colum is included to the calculation area.
	ViReal32		calcAreaCenterY;		///< Calculation area right border. The specified pixel colum is included to the calculation area.
	ViReal32		calcAreaWidth;			///< Calculation area width.
	ViReal32		calcAreaHeight;		///< Calculation area height.
	ViReal64		calcAreaAngle;			///< Calculation area rotation angle in degree. The rectangle/ellipse defined by \ref calcAreaLeft, \ref calcAreaTop, \ref calcAreaRight, and \ref calcAreaBottom will be rotated by this angle. Positive values rotate against the clock.

	ViReal32 	profileValuesX[TLBC1_MAX_COLUMNS];	///< Intensity profile intensity values along the x axis of the whole image.
	ViReal32 	profileValuesY[TLBC1_MAX_ROWS]; 		///< Intensity profile intensity values along the y axis of the whole image.
	ViReal32		profilePeakValueX;						///< Peak intensity value in the x profile inside the calculation area.
	ViReal32		profilePeakValueY;						///< Peak intensity value in the y profile inside the calculation area.
	ViUInt16		profilePeakPosX;							///< Intensity profile peak intensity x pixel position inside the calculation area.
	ViUInt16		profilePeakPosY;							///< Intensity profile peak intensity y pixel position inside the calculation area.
	
	ViReal64		effectiveArea;	///< Area of an ideal flat top beam with same peak intensity in µm².
	
//	ViReal32 	gaussianValuesX[TLBC1_MAX_COLUMNS];
//	ViReal32 	gaussianValuesY[TLBC1_MAX_ROWS]; 
}TLBC1_Calculations;

#endif /* ndef __TLBC1_CALCULATIONS_H__ */  
