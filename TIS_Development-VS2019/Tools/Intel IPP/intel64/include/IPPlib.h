
//IPPlib.h
#include "..\..\..\..\Common\PDLL\pdll.h"
//dll wrapper class using the virtual class

#if defined( _WIN32 ) || defined ( _WIN64 )
#define __STDCALL  __stdcall
#define __CDECL    __cdecl
#define __INT64    __int64
#define __UINT64    unsigned __int64
#else
#define __STDCALL
#define __CDECL
#define __INT64    long long
#define __UINT64    unsigned long long
#endif


/* /////////////////////////////////////////////////////////////////////////////
//        The following enumerator defines a status of IPP operations
//                     negative value means error
*/
typedef enum {
	/* errors */
	ippStsNotSupportedModeErr    = -9999,/* The requested mode is currently not supported  */
	ippStsCpuNotSupportedErr     = -9998,/* The target cpu is not supported */

	ippStsDitherTypeErr          = -224, /* Unsupported dithering type */
	ippStsH264BufferFullErr      = -223, /* Buffer for the output bitstream is full */
	ippStsWrongAffinitySettingErr= -222, /* An affinity setting not correspond to an affinity setting that was set by f.ippSetAffinity() */
	ippStsLoadDynErr             = -221, /* Fatal error during loading of dynamic library */

	ippStsPointAtInfinity        = -220, /* Point at infinity is detected  */

	ippStsI18nUnsupportedErr     = -219, /* Internationalization (i18n) is not supported                                                                  */
	ippStsI18nMsgCatalogOpenErr  = -218, /* Message Catalog cannot be opened, for extended information use errno in Linux and GetLastError in Windows* */
	ippStsI18nMsgCatalogCloseErr = -217, /* Message Catalog cannot be closed, for extended information use errno in Linux and GetLastError in Windows* */

	ippStsUnknownStatusCodeErr   = -216, /* Unknown Status Code */

	ippStsOFBSizeErr             = -215, /* Wrong value for crypto OFB block size */
	ippStsLzoBrokenStreamErr     = -214, /* LZO safe decompression function cannot decode LZO stream */

	ippStsRoundModeNotSupportedErr  = -213, /* Unsupported round mode*/
	ippStsDecimateFractionErr    = -212, /* Unsupported fraction in Decimate */
	ippStsWeightErr              = -211, /* Wrong value of weight */

	ippStsQualityIndexErr        = -210, /* Quality Index can't be calculated for image filled with constant */
	ippStsIIRPassbandRippleErr   = -209, /* Ripple in passband for Chebyshev1 design is less than zero, equal to zero or greater than 29 */
	ippStsFilterFrequencyErr     = -208, /* Cut of frequency of filter is less than zero, equal to zero or greater than 0.5 */
	ippStsFIRGenOrderErr         = -207, /* Order of an FIR filter for design them is less than one                    */
	ippStsIIRGenOrderErr         = -206, /* Order of an IIR filter for design them is less than one or greater than 12 */

	ippStsConvergeErr            = -205, /* The algorithm does not converge*/
	ippStsSizeMatchMatrixErr     = -204, /* Unsuitable sizes of the source matrices*/
	ippStsCountMatrixErr         = -203, /* Count value is negative or equal to 0*/
	ippStsRoiShiftMatrixErr      = -202, /* RoiShift value is negative or not dividend to size of data type*/

	ippStsResizeNoOperationErr   = -201, /* One of the output image dimensions is less than 1 pixel */
	ippStsSrcDataErr             = -200, /* The source buffer contains unsupported data */
	ippStsMaxLenHuffCodeErr      = -199, /* Huff: Max length of Huffman code is more than expected one */
	ippStsCodeLenTableErr        = -198, /* Huff: Invalid codeLenTable */
	ippStsFreqTableErr           = -197, /* Huff: Invalid freqTable */

	ippStsIncompleteContextErr   = -196, /* Crypto: set up of context is not complete */

	ippStsSingularErr            = -195, /* Matrix is singular */
	ippStsSparseErr              = -194, /* Tap positions are not in ascending order, negative or repeated*/
	ippStsBitOffsetErr           = -193, /* Incorrect bit offset value */
	ippStsQPErr                  = -192, /* Incorrect quantization parameter */
	ippStsVLCErr                 = -191, /* Illegal VLC or FLC during stream decoding */
	ippStsRegExpOptionsErr       = -190, /* RegExp: Options for pattern are incorrect */
	ippStsRegExpErr              = -189, /* RegExp: The structure pRegExpState contains wrong data */
	ippStsRegExpMatchLimitErr    = -188, /* RegExp: The match limit has been exhausted */
	ippStsRegExpQuantifierErr    = -187, /* RegExp: wrong quantifier */
	ippStsRegExpGroupingErr      = -186, /* RegExp: wrong grouping */
	ippStsRegExpBackRefErr       = -185, /* RegExp: wrong back reference */
	ippStsRegExpChClassErr       = -184, /* RegExp: wrong character class */
	ippStsRegExpMetaChErr        = -183, /* RegExp: wrong metacharacter */
	ippStsStrideMatrixErr        = -182,  /* Stride value is not positive or not dividend to size of data type */
	ippStsCTRSizeErr             = -181,  /* Wrong value for crypto CTR block size */
	ippStsJPEG2KCodeBlockIsNotAttached =-180, /* codeblock parameters are not attached to the state structure */
	ippStsNotPosDefErr           = -179,      /* Not positive-definite matrix */

	ippStsEphemeralKeyErr        = -178, /* ECC: Bad ephemeral key   */
	ippStsMessageErr             = -177, /* ECC: Bad message digest  */
	ippStsShareKeyErr            = -176, /* ECC: Invalid share key   */
	ippStsIvalidPublicKey        = -175, /* ECC: Invalid public key  */
	ippStsIvalidPrivateKey       = -174, /* ECC: Invalid private key */
	ippStsOutOfECErr             = -173, /* ECC: Point out of EC     */
	ippStsECCInvalidFlagErr      = -172, /* ECC: Invalid Flag        */

	ippStsMP3FrameHeaderErr      = -171,  /* Error in fields IppMP3FrameHeader structure */
	ippStsMP3SideInfoErr         = -170,  /* Error in fields IppMP3SideInfo structure */

	ippStsBlockStepErr           = -169,  /* Step for Block less than 8 */
	ippStsMBStepErr              = -168,  /* Step for MB less than 16 */

	ippStsAacPrgNumErr           = -167,  /* AAC: Invalid number of elements for one program   */
	ippStsAacSectCbErr           = -166,  /* AAC: Invalid section codebook                     */
	ippStsAacSfValErr            = -164,  /* AAC: Invalid scalefactor value                    */
	ippStsAacCoefValErr          = -163,  /* AAC: Invalid quantized coefficient value          */
	ippStsAacMaxSfbErr           = -162,  /* AAC: Invalid coefficient index  */
	ippStsAacPredSfbErr          = -161,  /* AAC: Invalid predicted coefficient index  */
	ippStsAacPlsDataErr          = -160,  /* AAC: Invalid pulse data attributes  */
	ippStsAacGainCtrErr          = -159,  /* AAC: Gain control not supported  */
	ippStsAacSectErr             = -158,  /* AAC: Invalid number of sections  */
	ippStsAacTnsNumFiltErr       = -157,  /* AAC: Invalid number of TNS filters  */
	ippStsAacTnsLenErr           = -156,  /* AAC: Invalid TNS region length  */
	ippStsAacTnsOrderErr         = -155,  /* AAC: Invalid order of TNS filter  */
	ippStsAacTnsCoefResErr       = -154,  /* AAC: Invalid bit-resolution for TNS filter coefficients  */
	ippStsAacTnsCoefErr          = -153,  /* AAC: Invalid TNS filter coefficients  */
	ippStsAacTnsDirectErr        = -152,  /* AAC: Invalid TNS filter direction  */
	ippStsAacTnsProfileErr       = -151,  /* AAC: Invalid TNS profile  */
	ippStsAacErr                 = -150,  /* AAC: Internal error  */
	ippStsAacBitOffsetErr        = -149,  /* AAC: Invalid current bit offset in bitstream  */
	ippStsAacAdtsSyncWordErr     = -148,  /* AAC: Invalid ADTS syncword  */
	ippStsAacSmplRateIdxErr      = -147,  /* AAC: Invalid sample rate index  */
	ippStsAacWinLenErr           = -146,  /* AAC: Invalid window length (not short or long)  */
	ippStsAacWinGrpErr           = -145,  /* AAC: Invalid number of groups for current window length  */
	ippStsAacWinSeqErr           = -144,  /* AAC: Invalid window sequence range  */
	ippStsAacComWinErr           = -143,  /* AAC: Invalid common window flag  */
	ippStsAacStereoMaskErr       = -142,  /* AAC: Invalid stereo mask  */
	ippStsAacChanErr             = -141,  /* AAC: Invalid channel number  */
	ippStsAacMonoStereoErr       = -140,  /* AAC: Invalid mono-stereo flag  */
	ippStsAacStereoLayerErr      = -139,  /* AAC: Invalid this Stereo Layer flag  */
	ippStsAacMonoLayerErr        = -138,  /* AAC: Invalid this Mono Layer flag  */
	ippStsAacScalableErr         = -137,  /* AAC: Invalid scalable object flag  */
	ippStsAacObjTypeErr          = -136,  /* AAC: Invalid audio object type  */
	ippStsAacWinShapeErr         = -135,  /* AAC: Invalid window shape  */
	ippStsAacPcmModeErr          = -134,  /* AAC: Invalid PCM output interleaving indicator  */
	ippStsVLCUsrTblHeaderErr          = -133,  /* VLC: Invalid header inside table */
	ippStsVLCUsrTblUnsupportedFmtErr  = -132,  /* VLC: Unsupported table format */
	ippStsVLCUsrTblEscAlgTypeErr      = -131,  /* VLC: Unsupported Ecs-algorithm */
	ippStsVLCUsrTblEscCodeLengthErr   = -130,  /* VLC: Incorrect Esc-code length inside table header */
	ippStsVLCUsrTblCodeLengthErr      = -129,  /* VLC: Unsupported code length inside table */
	ippStsVLCInternalTblErr           = -128,  /* VLC: Invalid internal table */
	ippStsVLCInputDataErr             = -127,  /* VLC: Invalid input data */
	ippStsVLCAACEscCodeLengthErr      = -126,  /* VLC: Invalid AAC-Esc code length */
	ippStsNoiseRangeErr         = -125,  /* Noise value for Wiener Filter is out range. */
	ippStsUnderRunErr           = -124,  /* Data under run error */
	ippStsPaddingErr            = -123,  /* Detected padding error shows the possible data corruption */
	ippStsCFBSizeErr            = -122,  /* Wrong value for crypto CFB block size */
	ippStsPaddingSchemeErr      = -121,  /* Invalid padding scheme  */
	ippStsInvalidCryptoKeyErr   = -120,  /* A compromised key causes suspansion of requested cryptographic operation  */
	ippStsLengthErr             = -119,  /* Wrong value of string length */
	ippStsBadModulusErr         = -118,  /* Bad modulus caused a module inversion failure */
	ippStsLPCCalcErr            = -117,  /* Linear prediction could not be evaluated */
	ippStsRCCalcErr             = -116,  /* Reflection coefficients could not be computed */
	ippStsIncorrectLSPErr       = -115,  /* Incorrect Linear Spectral Pair values */
	ippStsNoRootFoundErr        = -114,  /* No roots are found for equation */
	ippStsJPEG2KBadPassNumber   = -113,  /* Pass number exceeds allowed limits [0,nOfPasses-1] */
	ippStsJPEG2KDamagedCodeBlock= -112,  /* Codeblock for decoding is damaged */
	ippStsH263CBPYCodeErr       = -111,  /* Illegal Huffman code during CBPY stream processing */
	ippStsH263MCBPCInterCodeErr = -110,  /* Illegal Huffman code during MCBPC Inter stream processing */
	ippStsH263MCBPCIntraCodeErr = -109,  /* Illegal Huffman code during MCBPC Intra stream processing */
	ippStsNotEvenStepErr        = -108,  /* Step value is not pixel multiple */
	ippStsHistoNofLevelsErr     = -107,  /* Number of levels for histogram is less than 2 */
	ippStsLUTNofLevelsErr       = -106,  /* Number of levels for LUT is less than 2 */
	ippStsMP4BitOffsetErr       = -105,  /* Incorrect bit offset value */
	ippStsMP4QPErr              = -104,  /* Incorrect quantization parameter */
	ippStsMP4BlockIdxErr        = -103,  /* Incorrect block index */
	ippStsMP4BlockTypeErr       = -102,  /* Incorrect block type */
	ippStsMP4MVCodeErr          = -101,  /* Illegal Huffman code during MV stream processing */
	ippStsMP4VLCCodeErr         = -100,  /* Illegal Huffman code during VLC stream processing */
	ippStsMP4DCCodeErr          = -99,   /* Illegal code during DC stream processing */
	ippStsMP4FcodeErr           = -98,   /* Incorrect fcode value */
	ippStsMP4AlignErr           = -97,   /* Incorrect buffer alignment            */
	ippStsMP4TempDiffErr        = -96,   /* Incorrect temporal difference         */
	ippStsMP4BlockSizeErr       = -95,   /* Incorrect size of block or macroblock */
	ippStsMP4ZeroBABErr         = -94,   /* All BAB values are zero             */
	ippStsMP4PredDirErr         = -93,   /* Incorrect prediction direction        */
	ippStsMP4BitsPerPixelErr    = -92,   /* Incorrect number of bits per pixel    */
	ippStsMP4VideoCompModeErr   = -91,   /* Incorrect video component mode        */
	ippStsMP4LinearModeErr      = -90,   /* Incorrect DC linear mode */
	ippStsH263PredModeErr       = -83,   /* Prediction Mode value error                                       */
	ippStsH263BlockStepErr      = -82,   /* Step value is less than 8                                         */
	ippStsH263MBStepErr         = -81,   /* Step value is less than 16                                        */
	ippStsH263FrameWidthErr     = -80,   /* Frame width is less than 8                                        */
	ippStsH263FrameHeightErr    = -79,   /* Frame height is less than or equal to zero                        */
	ippStsH263ExpandPelsErr     = -78,   /* Expand pixels number is less than 8                               */
	ippStsH263PlaneStepErr      = -77,   /* Step value is less than the plane width                           */
	ippStsH263QuantErr          = -76,   /* Quantizer value is less than or equal to zero, or greater than 31 */
	ippStsH263MVCodeErr         = -75,   /* Illegal Huffman code during MV stream processing                  */
	ippStsH263VLCCodeErr        = -74,   /* Illegal Huffman code during VLC stream processing                 */
	ippStsH263DCCodeErr         = -73,   /* Illegal code during DC stream processing                          */
	ippStsH263ZigzagLenErr      = -72,   /* Zigzag compact length is more than 64                             */
	ippStsFBankFreqErr          = -71,   /* Incorrect value of the filter bank frequency parameter */
	ippStsFBankFlagErr          = -70,   /* Incorrect value of the filter bank parameter           */
	ippStsFBankErr              = -69,   /* Filter bank is not correctly initialized              */
	ippStsNegOccErr             = -67,   /* Negative occupation count                      */
	ippStsCdbkFlagErr           = -66,   /* Incorrect value of the codebook flag parameter */
	ippStsSVDCnvgErr            = -65,   /* No convergence of SVD algorithm               */
	ippStsJPEGHuffTableErr      = -64,   /* JPEG Huffman table is destroyed        */
	ippStsJPEGDCTRangeErr       = -63,   /* JPEG DCT coefficient is out of the range */
	ippStsJPEGOutOfBufErr       = -62,   /* Attempt to access out of the buffer    */
	ippStsDrawTextErr           = -61,   /* System error in the draw text operation */
	ippStsChannelOrderErr       = -60,   /* Wrong order of the destination channels */
	ippStsZeroMaskValuesErr     = -59,   /* All values of the mask are zero */
	ippStsQuadErr               = -58,   /* The quadrangle is nonconvex or degenerates into triangle, line or point */
	ippStsRectErr               = -57,   /* Size of the rectangle region is less than or equal to 1 */
	ippStsCoeffErr              = -56,   /* Unallowable values of the transformation coefficients   */
	ippStsNoiseValErr           = -55,   /* Bad value of noise amplitude for dithering             */
	ippStsDitherLevelsErr       = -54,   /* Number of dithering levels is out of range             */
	ippStsNumChannelsErr        = -53,   /* Bad or unsupported number of channels                   */
	ippStsCOIErr                = -52,   /* COI is out of range */
	ippStsDivisorErr            = -51,   /* Divisor is equal to zero, function is aborted */
	ippStsAlphaTypeErr          = -50,   /* Illegal type of image compositing operation                           */
	ippStsGammaRangeErr         = -49,   /* Gamma range bounds is less than or equal to zero                      */
	ippStsGrayCoefSumErr        = -48,   /* Sum of the conversion coefficients must be less than or equal to 1    */
	ippStsChannelErr            = -47,   /* Illegal channel number                                                */
	ippStsToneMagnErr           = -46,   /* Tone magnitude is less than or equal to zero                          */
	ippStsToneFreqErr           = -45,   /* Tone frequency is negative, or greater than or equal to 0.5           */
	ippStsTonePhaseErr          = -44,   /* Tone phase is negative, or greater than or equal to 2*PI              */
	ippStsTrnglMagnErr          = -43,   /* Triangle magnitude is less than or equal to zero                      */
	ippStsTrnglFreqErr          = -42,   /* Triangle frequency is negative, or greater than or equal to 0.5       */
	ippStsTrnglPhaseErr         = -41,   /* Triangle phase is negative, or greater than or equal to 2*PI          */
	ippStsTrnglAsymErr          = -40,   /* Triangle asymmetry is less than -PI, or greater than or equal to PI   */
	ippStsHugeWinErr            = -39,   /* Kaiser window is too huge                                             */
	ippStsJaehneErr             = -38,   /* Magnitude value is negative                                           */
	ippStsStrideErr             = -37,   /* Stride value is less than the row length */
	ippStsEpsValErr             = -36,   /* Negative epsilon value error             */
	ippStsWtOffsetErr           = -35,   /* Invalid offset value of wavelet filter                                       */
	ippStsAnchorErr             = -34,   /* Anchor point is outside the mask                                             */
	ippStsMaskSizeErr           = -33,   /* Invalid mask size                                                           */
	ippStsShiftErr              = -32,   /* Shift value is less than zero                                                */
	ippStsSampleFactorErr       = -31,   /* Sampling factor is less than or equal to zero                                */
	ippStsSamplePhaseErr        = -30,   /* Phase value is out of range: 0 <= phase < factor                             */
	ippStsFIRMRFactorErr        = -29,   /* MR FIR sampling factor is less than or equal to zero                         */
	ippStsFIRMRPhaseErr         = -28,   /* MR FIR sampling phase is negative, or greater than or equal to the sampling factor */
	ippStsRelFreqErr            = -27,   /* Relative frequency value is out of range                                     */
	ippStsFIRLenErr             = -26,   /* Length of a FIR filter is less than or equal to zero                         */
	ippStsIIROrderErr           = -25,   /* Order of an IIR filter is not valid */
	ippStsDlyLineIndexErr       = -24,   /* Invalid value of the delay line sample index */
	ippStsResizeFactorErr       = -23,   /* Resize factor(s) is less than or equal to zero */
	ippStsInterpolationErr      = -22,   /* Invalid interpolation mode */
	ippStsMirrorFlipErr         = -21,   /* Invalid flip mode                                         */
	ippStsMoment00ZeroErr       = -20,   /* Moment value M(0,0) is too small to continue calculations */
	ippStsThreshNegLevelErr     = -19,   /* Negative value of the level in the threshold operation    */
	ippStsThresholdErr          = -18,   /* Invalid threshold bounds */
	ippStsContextMatchErr       = -17,   /* Context parameter doesn't match the operation */
	ippStsFftFlagErr            = -16,   /* Invalid value of the FFT flag parameter */
	ippStsFftOrderErr           = -15,   /* Invalid value of the FFT order parameter */
	ippStsStepErr               = -14,   /* Step value is not valid */
	ippStsScaleRangeErr         = -13,   /* Scale bounds are out of the range */
	ippStsDataTypeErr           = -12,   /* Bad or unsupported data type */
	ippStsOutOfRangeErr         = -11,   /* Argument is out of range or point is outside the image */
	ippStsDivByZeroErr          = -10,   /* An attempt to divide by zero */
	ippStsMemAllocErr           = -9,    /* Not enough memory allocated for the operation */
	ippStsNullPtrErr            = -8,    /* Null pointer error */
	ippStsRangeErr              = -7,    /* Bad values of bounds: the lower bound is greater than the upper bound */
	ippStsSizeErr               = -6,    /* Wrong value of data size */
	ippStsBadArgErr             = -5,    /* Function arg/param is bad */
	ippStsNoMemErr              = -4,    /* Not enough memory for the operation */
	ippStsSAReservedErr3        = -3,    /* Unknown/unspecified error, -3 */
	ippStsErr                   = -2,    /* Unknown/unspecified error, -2 */
	ippStsSAReservedErr1        = -1,    /* Unknown/unspecified error, -1 */

	/* no errors */
	ippStsNoErr                 =   0,   /* No error, it's OK */

	/* warnings  */
	ippStsNoOperation       =   1,       /* No operation has been executed */
	ippStsMisalignedBuf     =   2,       /* Misaligned pointer in operation in which it must be aligned */
	ippStsSqrtNegArg        =   3,       /* Negative value(s) of the argument in the function Sqrt */
	ippStsInvZero           =   4,       /* INF result. Zero value was met by InvThresh with zero level */
	ippStsEvenMedianMaskSize=   5,       /* Even size of the Median Filter mask was replaced by the odd one */
	ippStsDivByZero         =   6,       /* Zero value(s) of the divisor in the function Div */
	ippStsLnZeroArg         =   7,       /* Zero value(s) of the argument in the function Ln     */
	ippStsLnNegArg          =   8,       /* Negative value(s) of the argument in the function Ln */
	ippStsNanArg            =   9,       /* Not a Number argument value warning                  */
	ippStsJPEGMarker        =   10,      /* JPEG marker was met in the bitstream                 */
	ippStsResFloor          =   11,      /* All result values are floored                        */
	ippStsOverflow          =   12,      /* Overflow occurred in the operation                   */
	ippStsLSFLow            =   13,      /* Quantized LP syntethis filter stability check is applied at the low boundary of [0,pi] */
	ippStsLSFHigh           =   14,      /* Quantized LP syntethis filter stability check is applied at the high boundary of [0,pi] */
	ippStsLSFLowAndHigh     =   15,      /* Quantized LP syntethis filter stability check is applied at both boundaries of [0,pi] */
	ippStsZeroOcc           =   16,      /* Zero occupation count */
	ippStsUnderflow         =   17,      /* Underflow occurred in the operation */
	ippStsSingularity       =   18,      /* Singularity occurred in the operation                                       */
	ippStsDomain            =   19,      /* Argument is out of the function domain                                      */
	ippStsNonIntelCpu       =   20,      /* The target cpu is not Genuine Intel                                         */
	ippStsCpuMismatch       =   21,      /* The library for given cpu cannot be set                                     */
	ippStsNoIppFunctionFound =  22,      /* Application does not contain IPP functions calls                            */
	ippStsDllNotFoundBestUsed = 23,      /* The newest version of IPP dll was not found by dispatcher                   */
	ippStsNoOperationInDll  =   24,      /* The function does nothing in the dynamic version of the library             */
	ippStsInsufficientEntropy=  25,      /* Insufficient entropy in the random seed and stimulus bit string caused the prime/key generation to fail */
	ippStsOvermuchStrings   =   26,      /* Number of destination strings is more than expected                         */
	ippStsOverlongString    =   27,      /* Length of one of the destination strings is more than expected              */
	ippStsAffineQuadChanged =   28,      /* 4th vertex of destination quad is not equal to customer's one               */
	ippStsWrongIntersectROI =   29,      /* Wrong ROI that has no intersection with the source or destination ROI. No operation */
	ippStsWrongIntersectQuad =  30,      /* Wrong quadrangle that has no intersection with the source or destination ROI. No operation */
	ippStsSmallerCodebook   =   31,      /* Size of created codebook is less than cdbkSize argument */
	ippStsSrcSizeLessExpected = 32,      /* DC: The size of source buffer is less than expected one */
	ippStsDstSizeLessExpected = 33,      /* DC: The size of destination buffer is less than expected one */
	ippStsStreamEnd           = 34,      /* DC: The end of stream processed */
	ippStsDoubleSize        =   35,      /* Sizes of image are not multiples of 2 */
	ippStsNotSupportedCpu   =   36,      /* The cpu is not supported */
	ippStsUnknownCacheSize  =   37,      /* The cpu is supported, but the size of the cache is unknown */
	ippStsSymKernelExpected =   38,      /* The Kernel is not symmetric*/
	ippStsEvenMedianWeight  =   39,      /* Even weight of the Weighted Median Filter was replaced by the odd one */
	ippStsWrongIntersectVOI =   40,      /* VOI has no intersection with the source or destination volume. No operation                             */
	ippStsI18nMsgCatalogInvalid=41,      /* Message Catalog is invalid, English message returned                                                    */
	ippStsI18nGetMessageFail  = 42,      /* Failed to fetch a localized message, English message returned. For extended information use errno in Linux and GetLastError in Windows* */
	ippStsWaterfall           = 43,      /* Cannot load required library, waterfall is used */
	ippStsPrevLibraryUsed     = 44,      /* Cannot load required library, previous dynamic library is used */
	ippStsLLADisabled         = 45,      /* OpenMP Low Level Affinity is disabled */
	ippStsNoAntialiasing      = 46,      /* The mode does not support antialiasing */
	ippStsRepetitiveSrcData   = 47       /* DC: The source data is too repetitive */
} IppStatus;



typedef unsigned char   Ipp8u;
typedef unsigned short  Ipp16u;
typedef unsigned int    Ipp32u;

typedef signed char    Ipp8s;
typedef signed short   Ipp16s;
typedef signed int     Ipp32s;
typedef float   Ipp32f;
typedef double  Ipp64f;
typedef struct {
	Ipp32f  re;
	Ipp32f  im;
} Ipp32fc;
typedef struct {
	Ipp32s  re;
	Ipp32s  im;
} Ipp32sc;

typedef struct {
	Ipp64f  re;
	Ipp64f  im;
} Ipp64fc;
typedef struct {
	Ipp16s  re;
	Ipp16s  im;
} Ipp16sc;



typedef __INT64 Ipp64s;
typedef __UINT64 Ipp64u;
typedef double  Ipp64f;


typedef struct {
	int width;
	int height;
} IppiSize;

typedef struct {
	int x;
	int y;
} IppiPoint;

typedef struct FFTSpec_R_16s    IppsFFTSpec_R_16s;
typedef struct FFTSpec_R_32s    IppsFFTSpec_R_32s;
typedef struct FFTSpec_R_32f    IppsFFTSpec_R_32f;
typedef struct FFTSpec_R_64f    IppsFFTSpec_R_64f;


typedef struct DFTSpec_R_16s    IppsDFTSpec_R_16s;
typedef struct DFTSpec_R_32f    IppsDFTSpec_R_32f;
typedef struct DFTSpec_R_64f    IppsDFTSpec_R_64f;




typedef enum _IppiNorm {
	ippiNormInf = 0,
	ippiNormL1 = 1,
	ippiNormL2 = 2,
	ippiNormFM = 3
} IppiNorm;

typedef struct {
	int x;
	int y;
	int width;
	int height;
} IppiRect;

typedef enum {
	ippUndef = -1,
	ipp1u    =  0,
	ipp8u    =  1,
	ipp8uc   =  2,
	ipp8s    =  3,
	ipp8sc   =  4,
	ipp16u   =  5,
	ipp16uc  =  6,
	ipp16s   =  7,
	ipp16sc  =  8,
	ipp32u   =  9,
	ipp32uc  = 10,
	ipp32s   = 11,
	ipp32sc  = 12,
	ipp32f   = 13,
	ipp32fc  = 14,
	ipp64u   = 15,
	ipp64uc  = 16,
	ipp64s   = 17,
	ipp64sc  = 18,
	ipp64f   = 19,
	ipp64fc  = 20
} IppDataType;

typedef enum _IppiBorderType {
	ippBorderConst     =  0,
	ippBorderRepl      =  1,
	ippBorderWrap      =  2,
	ippBorderMirror    =  3,
	ippBorderMirrorR   =  4,
	ippBorderMirror2   =  5,
	ippBorderInMem     =  6,
	ippBorderInMemTop     =  0x0010,
	ippBorderInMemBottom  =  0x0020,
	ippBorderInMemLeft    =  0x0040,
	ippBorderInMemRight   =  0x0080
} IppiBorderType;

typedef enum {
	ippAlgHintNone,
	ippAlgHintFast,
	ippAlgHintAccurate
} IppHintAlgorithm;

typedef enum {
	ippAxsHorizontal,
	ippAxsVertical,
	ippAxsBoth
} IppiAxis;

typedef enum  _IppiMaskSize {
	ippMskSize1x3 = 13,
	ippMskSize1x5 = 15,
	ippMskSize3x1 = 31,
	ippMskSize3x3 = 33,
	ippMskSize5x1 = 51,
	ippMskSize5x5 = 55
} IppiMaskSize;

enum {
	IPPI_INTER_NN     = 1,
	IPPI_INTER_LINEAR = 2,
	IPPI_INTER_CUBIC  = 4,
	IPPI_INTER_CUBIC2P_BSPLINE,     /* two-parameter cubic filter (B=1, C=0) */
	IPPI_INTER_CUBIC2P_CATMULLROM,  /* two-parameter cubic filter (B=0, C=1/2) */
	IPPI_INTER_CUBIC2P_B05C03,      /* two-parameter cubic filter (B=1/2, C=3/10) */
	IPPI_INTER_SUPER  = 8,
	IPPI_INTER_LANCZOS = 16,
	IPPI_ANTIALIASING  = (1 << 29),
	IPPI_SUBPIXEL_EDGE = (1 << 30),
	IPPI_SMOOTH_EDGE   = (1 << 31)
};

enum {
	IPP_FFT_DIV_FWD_BY_N = 1,
	IPP_FFT_DIV_INV_BY_N = 2,
	IPP_FFT_DIV_BY_SQRTN = 4,
	IPP_FFT_NODIV_BY_ANY = 8
};

struct MomentState64f;
typedef struct MomentState64f IppiMomentState_64f;
struct DFT2DSpec_C_32fc;
typedef struct DFT2DSpec_C_32fc IppiDFTSpec_C_32fc;
typedef struct DFTSpec_C_64f    IppsDFTSpec_C_64f;
typedef struct DFTSpec_C_64fc   IppsDFTSpec_C_64fc;
typedef struct RandUniState_32f IppsRandUniState_32f;

class IPPCVlib
{
public:
	virtual IppStatus ippiMean_16u_C1MR( const Ipp16u*, int, const Ipp8u*, int, IppiSize, Ipp64f* ) = 0;	
	virtual IppStatus ippiMean_StdDev_16u_C1MR( const Ipp16u*, int, const Ipp8u*, int, IppiSize, Ipp64f*, Ipp64f* ) = 0;	
	virtual IppStatus ippiMinMaxIndx_16u_C1MR(const Ipp16u*, int, const Ipp8u*, int, IppiSize, Ipp32f*, Ipp32f*, IppiPoint*, IppiPoint*) = 0;
	virtual IppStatus ippiLabelMarkersGetBufferSize_16u_C1R(IppiSize,int*) = 0;
	virtual IppStatus ippiLabelMarkers_16u_C1IR(Ipp16u*, int, IppiSize, int, int, IppiNorm, int *, Ipp8u* ) =  0;
};

class IPPCVDll : public PDLL, IPPCVlib
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(IPPCVDll)
#pragma warning(pop)

	// calculate mean of roi with mask option for an usigned short image
	DECLARE_FUNCTION6( IppStatus, ippiMean_16u_C1MR, const Ipp16u*, int, const Ipp8u*, int, IppiSize, Ipp64f* );
	DECLARE_FUNCTION7( IppStatus, ippiMean_StdDev_16u_C1MR, const Ipp16u*, int, const Ipp8u*, int, IppiSize, Ipp64f*, Ipp64f* );
	DECLARE_FUNCTION9( IppStatus, ippiMinMaxIndx_16u_C1MR, const Ipp16u*, int, const Ipp8u*, int, IppiSize, Ipp32f*, Ipp32f*, IppiPoint*, IppiPoint* );	
	DECLARE_FUNCTION2( IppStatus, ippiLabelMarkersGetBufferSize_16u_C1R,IppiSize,int*);
	DECLARE_FUNCTION8(IppStatus, ippiLabelMarkers_16u_C1IR, Ipp16u*, int, IppiSize, int, int, IppiNorm, int *, Ipp8u*);
};

class IPPIlib
{
public:
	//virtual IppStatus ippiSet_8u_C1R(Ipp8u, const Ipp8u*, int, IppiSize);
	virtual IppStatus ippiCopy_16u_C1R(const Ipp16u*, int, Ipp16u*, int, IppiSize)= 0;	
	virtual IppStatus ippiComputeThreshold_Otsu_8u_C1R(const Ipp8u*, int, IppiSize, Ipp8u*)= 0;
	virtual IppStatus ippiThreshold_LTValGTVal_16u_C1R(const Ipp16u* , int , Ipp16u* , int , IppiSize , Ipp16u , Ipp16u , Ipp16u , Ipp16u)= 0;
	virtual IppStatus ippiThreshold_LTValGTVal_16u_C1IR( Ipp16u* , int, IppiSize, Ipp16u, Ipp16u , Ipp16u, Ipp16u)= 0;
	virtual IppStatus ippiThreshold_LTVal_16u_C1IR( Ipp16u* , int, IppiSize, Ipp16u, Ipp16u)= 0;
	virtual IppStatus ippiThreshold_LT_16u_C1R( Ipp16u* , int, Ipp16u*, int, IppiSize, Ipp16u)= 0;
	virtual IppStatus ippiSet_16u_C1R(Ipp16u, Ipp16u* , int, IppiSize)= 0;
	virtual IppStatus ippiDilate3x3_16u_C1IR( Ipp16u*, int, IppiSize)= 0;
	virtual IppStatus ippiErode3x3_16u_C1IR( Ipp16u*, int, IppiSize)= 0;
	virtual IppStatus ippiMomentInitAlloc_64f( IppiMomentState_64f**, IppHintAlgorithm)= 0;
	virtual IppStatus ippiMomentFree_64f(IppiMomentState_64f*)= 0;
	virtual IppStatus ippiMoments64f_16u_C1R( const Ipp16u*, int, IppiSize, IppiMomentState_64f*)= 0;
	virtual IppStatus ippiCopyConstBorder_16u_C1R(const Ipp16u*,int ,IppiSize ,Ipp16u*,int, IppiSize, int, int, Ipp16u )= 0;
	virtual IppStatus ippiSub_16u_C1IRSfs(const Ipp16u*, int, Ipp16u*, int, IppiSize, int) = 0;
	virtual IppStatus ippiRotate_16u_C1R(const Ipp16u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcROI, Ipp16u* pDst, int dstStep, IppiRect dstROI,
		double angle, double xShift, double yShift, int interpolation) = 0;
	virtual IppStatus ippiMirror_16u_C1R(const Ipp16u* pSrc, int srcStep, Ipp16u* pDst, int dstStep, IppiSize roiSize, IppiAxis flip) = 0;
	virtual IppStatus ippiMirror_32f_C1R(const Ipp32f* pSrc, int srcStep, Ipp32f* pDst, int dstStep, IppiSize roiSize, IppiAxis flip) = 0;

	virtual Ipp32sc* ippiMalloc_32sc_C1(int, int, int*) = 0;
	virtual Ipp32fc* ippiMalloc_32fc_C1(int, int, int*) = 0;
	virtual Ipp32f* ippiMalloc_32f_C1(int, int, int*) = 0;
	virtual void ippiFree(void*) = 0;
	virtual IppStatus ippiDFTInitAlloc_C_32fc(IppiDFTSpec_C_32fc**, IppiSize, int, IppHintAlgorithm) = 0;
	virtual IppStatus ippiDFTFree_C_32fc( IppiDFTSpec_C_32fc*) = 0;
	virtual IppStatus ippiDFTFwd_CToC_32fc_C1R(const Ipp32fc*, int, Ipp32fc*, int, const IppiDFTSpec_C_32fc*, Ipp8u*) = 0;
	virtual IppStatus ippiDFTInv_CToC_32fc_C1R(const Ipp32fc*, int, Ipp32fc*, int, const IppiDFTSpec_C_32fc*, Ipp8u*) = 0;
	virtual IppStatus ippiFilterGauss_32f_C1R(const Ipp32f*, int, Ipp32f*, int, IppiSize, IppiMaskSize) = 0;

};

class IPPIDll : public PDLL, IPPIlib
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(IPPIDll)
#pragma warning(pop)

	//DECLARE_FUNCTION4( IppStatus, ippiSet_8u_C1R, Ipp8u, const Ipp8u*, int, IppiSize );	
	DECLARE_FUNCTION4(IppStatus, ippiComputeThreshold_Otsu_8u_C1R, const Ipp8u*, int, IppiSize, Ipp8u*);
	DECLARE_FUNCTION5(IppStatus, ippiCopy_16u_C1R, const Ipp16u*, int, Ipp16u*, int, IppiSize);
	DECLARE_FUNCTION9(IppStatus, ippiThreshold_LTValGTVal_16u_C1R, const Ipp16u*, int, Ipp16u*, int, IppiSize, Ipp16u, Ipp16u, Ipp16u, Ipp16u);
	DECLARE_FUNCTION7(IppStatus, ippiThreshold_LTValGTVal_16u_C1IR, Ipp16u*, int, IppiSize, Ipp16u, Ipp16u, Ipp16u, Ipp16u);
	DECLARE_FUNCTION5(IppStatus, ippiThreshold_LTVal_16u_C1IR, Ipp16u*, int, IppiSize, Ipp16u, Ipp16u);
	DECLARE_FUNCTION6(IppStatus, ippiThreshold_LT_16u_C1R, Ipp16u*, int, Ipp16u*, int, IppiSize, Ipp16u);
	DECLARE_FUNCTION4(IppStatus, ippiSet_16u_C1R, Ipp16u, Ipp16u* , int, IppiSize);
	DECLARE_FUNCTION3(IppStatus, ippiDilate3x3_16u_C1IR, Ipp16u*, int, IppiSize);
	DECLARE_FUNCTION3(IppStatus, ippiErode3x3_16u_C1IR, Ipp16u*, int, IppiSize);
	DECLARE_FUNCTION4(IppStatus, ippiMoments64f_16u_C1R, const Ipp16u*, int, IppiSize, IppiMomentState_64f*);
	DECLARE_FUNCTION9(IppStatus, ippiCopyConstBorder_16u_C1R, const Ipp16u*, int, IppiSize, Ipp16u*, int, IppiSize, int, int, Ipp16u);
	DECLARE_FUNCTION6(IppStatus, ippiSub_16u_C1IRSfs, const Ipp16u*, int, Ipp16u*, int, IppiSize, int);	
	DECLARE_FUNCTION4(IppStatus, ippiMarkSpecklesGetBufferSize, IppiSize, IppDataType, int, int*);
	DECLARE_FUNCTION6(IppStatus, ippiGetSpatialMoment_64f, const IppiMomentState_64f*, int, int, int, IppiPoint, Ipp64f*);
	DECLARE_FUNCTION2(IppStatus, ippiMomentInitAlloc_64f, IppiMomentState_64f**, IppHintAlgorithm);
	DECLARE_FUNCTION1(IppStatus, ippiMomentFree_64f, IppiMomentState_64f*);
	DECLARE_FUNCTION11(IppStatus, ippiRotate_16u_C1R, const Ipp16u*, IppiSize, int, IppiRect, Ipp16u*, int, IppiRect, double, double, double, int);
	DECLARE_FUNCTION6(IppStatus, ippiMirror_16u_C1R, const Ipp16u*, int, Ipp16u*, int, IppiSize, IppiAxis);
	DECLARE_FUNCTION6(IppStatus, ippiMirror_32f_C1R, const Ipp32f*, int, Ipp32f*, int, IppiSize, IppiAxis);

	DECLARE_FUNCTION3(Ipp32sc*, ippiMalloc_32sc_C1, int, int, int*);
	DECLARE_FUNCTION3(Ipp32fc*, ippiMalloc_32fc_C1, int, int, int*);
	DECLARE_FUNCTION3(Ipp32f*, ippiMalloc_32f_C1, int, int, int*);	
	DECLARE_FUNCTION1(void, ippiFree, void*);
	DECLARE_FUNCTION4(IppStatus, ippiDFTInitAlloc_C_32fc, IppiDFTSpec_C_32fc**, IppiSize, int, IppHintAlgorithm);
	DECLARE_FUNCTION1(IppStatus, ippiDFTFree_C_32fc, IppiDFTSpec_C_32fc*);
	DECLARE_FUNCTION6(IppStatus, ippiDFTFwd_CToC_32fc_C1R, const Ipp32fc*, int, Ipp32fc*, int, const IppiDFTSpec_C_32fc*, Ipp8u*);
	DECLARE_FUNCTION6(IppStatus, ippiDFTInv_CToC_32fc_C1R, const Ipp32fc*, int, Ipp32fc*, int, const IppiDFTSpec_C_32fc*, Ipp8u*);
	DECLARE_FUNCTION6(IppStatus, ippiFilterGauss_32f_C1R, const Ipp32f*, int, Ipp32f*, int, IppiSize, IppiMaskSize);

};

class IPPMlib
{
public:
	virtual IppStatus ippmQRDecomp_m_32f(const Ipp32f*, int, int, Ipp32f*, Ipp32f*, int, int, int, int)= 0;	
	virtual IppStatus ippmQRBackSubst_mva_32f(const Ipp32f*, int, int, Ipp32f*, const Ipp32f*, int, int, Ipp32f*, int, int, int, int, int)= 0;	
};

class IPPMDll : public PDLL, IPPMlib
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(IPPMDll)
#pragma warning(pop)

	DECLARE_FUNCTION9(IppStatus, ippmQRDecomp_m_32f, const Ipp32f*, int, int, Ipp32f*, Ipp32f*, int, int, int, int);
	DECLARE_FUNCTION13(IppStatus, ippmQRBackSubst_mva_32f, const Ipp32f*, int, int, Ipp32f*, const Ipp32f*, int, int, Ipp32f*, int, int, int, int, int);
};

class IPPSlib
{
public:
	virtual Ipp32f* ippsMalloc_32f(int)= 0;	
	virtual void ippsFree(void*) = 0;
	virtual IppStatus ippsSet_32f(Ipp32f, Ipp32f*, int) = 0;
	virtual IppStatus ippsSet_64f(Ipp64f, Ipp64f*, int) = 0;
	virtual IppStatus ippsZero_32f(Ipp32f*, int)= 0;
	virtual IppStatus ippsZero_64f(Ipp64f*, int) = 0;
	virtual IppStatus ippsZero_64fc(Ipp64fc*, int) = 0;
	virtual IppStatus ippsCopy_32f(const Ipp32f*, Ipp32f*, int)= 0;
	virtual IppStatus ippsCopy_64fc(const Ipp64fc*, Ipp64fc*, int) = 0;
	virtual IppStatus ippsPolarToCart_32fc(const Ipp32f*, const Ipp32f*, Ipp32fc*, int) = 0;
	virtual IppStatus ippsCartToPolar_32fc(const Ipp32fc*, Ipp32f*, Ipp32f*, int)= 0;
	virtual IppStatus ippsAbs_64f_I(Ipp64f*,int) = 0;
	virtual IppStatus ippsAbs_64f(const Ipp64f*, Ipp64f*,int) = 0;
	virtual IppStatus ippsMin_32f(const Ipp32f*, int, Ipp32f*)= 0;
	virtual IppStatus ippsMax_32f(const Ipp32f*, int, Ipp32f*)= 0;
	virtual IppStatus ippsAddC_32f_I(Ipp32f, Ipp32f*, int)= 0;
	virtual IppStatus ippsAddProduct_64fc( const Ipp64fc*, const Ipp64fc*, Ipp64fc*, int) = 0;	// pSrcDst[n] = pSrcDst[n] + pSrc1[n] * pSrc2[n], n=0,1,2,..len-1.
	virtual IppStatus ippsMulC_32f_I(Ipp32f, Ipp32f*, int)= 0;
	virtual IppStatus ippsMul_64f_I(const Ipp64f*, Ipp64f*, int) = 0;
	virtual IppStatus ippsMul_64fc_I(const Ipp64fc*, Ipp64fc*, int) = 0;
	virtual IppStatus ippsMul_64f(const Ipp64f*, const Ipp64f*, Ipp64f*, int) = 0;
	virtual IppStatus ippsMul_64fc(const Ipp64fc*, const Ipp64fc*, Ipp64fc*, int) = 0;
	virtual IppStatus ippsSqr_64f_I(Ipp64f*, int) = 0;
	virtual IppStatus ippsSqr_64fc_I(Ipp64fc*, int) = 0;
	virtual IppStatus ippsSqrt_64f_I(Ipp64f*,int) = 0;
	virtual IppStatus ippsSqrt_64fc_I(Ipp64fc*,int) = 0;
	virtual IppStatus ippsDiv_64f_I(const Ipp64f*, Ipp64f*, int) = 0;		// Div(X,Y) : Y[n] = Y[n] / X[n]
	virtual IppStatus ippsDiv_64fc_I(const Ipp64fc*, Ipp64fc*, int) = 0;	// Div(X,Y) : Y[n] = Y[n] / X[n]
	virtual IppStatus ippsSortAscend_64f_I(Ipp64f*, int) = 0;
	virtual IppStatus ippsSortDescend_64f_I(Ipp64f*, int) = 0;
	virtual IppStatus ippsRandUniformInitAlloc_32f(IppsRandUniState_32f**, Ipp32f, Ipp32f, unsigned int)= 0;
	virtual IppStatus ippsRandUniform_32f(Ipp32f*, int, IppsRandUniState_32f*)= 0;
	virtual IppStatus ippsRandUniformFree_32f(IppsRandUniState_32f*)= 0;
	virtual IppStatus ippsFFTInitAlloc_R_64f(IppsFFTSpec_R_64f**,int, int, IppHintAlgorithm)=0;
	virtual IppStatus ippsFFTFwd_RToCCS_64f(const Ipp64f*, Ipp64f*,const IppsFFTSpec_R_64f*, Ipp8u*)=0;
	virtual IppStatus ippsFFTFree_R_64f(IppsFFTSpec_R_64f*)=0;
	virtual Ipp64f* ippsMalloc_64f(int) = 0;	
	virtual Ipp64fc* ippsMalloc_64fc(int) = 0;
	virtual IppStatus ippsConjCcs_64fc(const Ipp64f*, Ipp64fc*, int)=0;
	//virtual IppStatus ippsFFTSpec_R_64f(IppsFFTSpec_R_64f** ,int, int, IppHintAlgorithm);
	virtual IppStatus ippsFFTInitAlloc_R_32f(IppsFFTSpec_R_32f** , int, int, IppHintAlgorithm ) = 0;
	virtual IppStatus ippsFFTInitAlloc_R_16s(IppsFFTSpec_R_16s** ,int, int, IppHintAlgorithm)=0;
	virtual IppStatus ippsFFTInitAlloc_R_32s(IppsFFTSpec_R_32s** ,int, int, IppHintAlgorithm)=0;
	virtual IppStatus ippsFFTFwd_RToCCS_16s_Sfs( const Ipp16s*, Ipp16s*,const IppsFFTSpec_R_16s*,int, Ipp8u*)=0;
	virtual IppStatus ippsFFTFwd_RToCCS_32s_Sfs( const Ipp32s*, Ipp32s*,const IppsFFTSpec_R_32s*,int, Ipp8u*)=0;
	virtual IppStatus ippsMagnitude_16sc_Sfs(const Ipp16sc*,Ipp16s*,int,int)=0;
	virtual IppStatus ippsMagnitude_32sc_Sfs(const Ipp32sc* pSrc,Ipp32s* pDst,int len,int scaleFactor)=0;
	virtual IppStatus ippsMagnitude_64fc(const Ipp64fc*, Ipp64f*, int) = 0;
	virtual IppStatus ippsMagnitude_64f(const Ipp64f*, const Ipp64f*, Ipp64f*, int) = 0;
	virtual IppStatus ippsReal_64fc(const Ipp64fc*, Ipp64f*, int) = 0;
	virtual IppStatus ippsImag_64fc(const Ipp64fc*, Ipp64f*, int) = 0;
	virtual IppStatus ippsRealToCplx_64f(const Ipp64f*, const Ipp64f*, Ipp64fc*, int) = 0;
	virtual IppStatus ippsFFTFree_R_32s(IppsFFTSpec_R_32s*)=0;
	virtual Ipp32s* ippsMalloc_32s(int)=0; 
	virtual IppStatus ippsDFTInitAlloc_R_64f(IppsDFTSpec_R_64f** , int, int, IppHintAlgorithm ) = 0;
	virtual IppStatus ippsDFTInitAlloc_R_32f(IppsDFTSpec_R_32f** , int, int, IppHintAlgorithm ) = 0;
	virtual IppStatus ippsDFTInitAlloc_R_16s(IppsDFTSpec_R_16s** ,int, int, IppHintAlgorithm) = 0;
	virtual IppStatus ippsDFTInitAlloc_C_64f(IppsDFTSpec_C_64f**, int, int, IppHintAlgorithm) = 0;
	virtual IppStatus ippsDFTInitAlloc_C_64fc(IppsDFTSpec_C_64fc**, int, int, IppHintAlgorithm) = 0;
	virtual IppStatus ippsDFTFree_R_64f(IppsDFTSpec_R_64f*)=0;
	virtual IppStatus ippsDFTFree_R_32f(IppsDFTSpec_R_32f*)=0;
	virtual IppStatus ippsDFTFree_R_16s(IppsDFTSpec_R_16s*)=0;
	virtual IppStatus ippsDFTFree_C_64f(IppsDFTSpec_C_64f*)=0;
	virtual IppStatus ippsDFTFree_C_64fc(IppsDFTSpec_C_64fc*)=0;
	virtual IppStatus ippsDFTFwd_RToCCS_64f(const Ipp64f*,Ipp64f*,const IppsDFTSpec_R_64f*,Ipp8u*)=0;
	virtual IppStatus ippsDFTFwd_RToCCS_16s_Sfs(const Ipp16s*,Ipp16s*,const IppsDFTSpec_R_16s*,int,Ipp8u*)=0;
	virtual IppStatus ippsDFTFwd_CToC_64f(const Ipp64f*, const Ipp64f*, Ipp64f*, Ipp64f*, const IppsDFTSpec_C_64f*, Ipp8u*) = 0;
	virtual IppStatus ippsDFTFwd_CToC_64fc(const Ipp64fc*, Ipp64fc*, const IppsDFTSpec_C_64fc*, Ipp8u*) = 0;
	virtual Ipp16s* ippsMalloc_16s(int)=0;
	virtual IppStatus ippsMagnitude_32fc(const Ipp32fc*,Ipp32f*,int)=0;
	virtual IppStatus ippsAdd_32f_I(const Ipp32f*,Ipp32f*, int)=0;
};

class IPPSDll : public PDLL, IPPSlib
{

#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(IPPSDll)
#pragma warning(pop)

	//call the macro and pass your functions
	DECLARE_FUNCTION1(Ipp32f*, ippsMalloc_32f, int);
	DECLARE_FUNCTION1(void, ippsFree, void*);
	DECLARE_FUNCTION4(IppStatus, ippsAdd_32fc, const Ipp32fc*, const Ipp32fc*, Ipp32fc*, int);
	DECLARE_FUNCTION3(IppStatus, ippsAdd_32fc_I, const Ipp32fc*, Ipp32fc*, int);
	DECLARE_FUNCTION3(IppStatus, ippsDivC_32fc_I, Ipp32fc, Ipp32fc*, int);
	DECLARE_FUNCTION3(IppStatus, ippsSet_32f, Ipp32f, Ipp32f*, int);
	DECLARE_FUNCTION3(IppStatus, ippsSet_64f, Ipp64f, Ipp64f*, int);
	DECLARE_FUNCTION2(IppStatus, ippsZero_32f, Ipp32f*, int);
	DECLARE_FUNCTION2(IppStatus, ippsZero_64f, Ipp64f*, int);
	DECLARE_FUNCTION2(IppStatus, ippsZero_64fc, Ipp64fc*, int);
	DECLARE_FUNCTION3(IppStatus, ippsCopy_32f, const Ipp32f*, Ipp32f*, int);
	DECLARE_FUNCTION3(IppStatus, ippsCopy_64fc, const Ipp64fc*, Ipp64fc*, int);
	DECLARE_FUNCTION4(IppStatus, ippsPolarToCart_32fc, const Ipp32f*, const Ipp32f*, Ipp32fc*, int);
	DECLARE_FUNCTION4(IppStatus, ippsCartToPolar_32fc, const Ipp32fc*, Ipp32f*, Ipp32f*, int);	
	DECLARE_FUNCTION2(IppStatus, ippsAbs_64f_I, Ipp64f*, int);
	DECLARE_FUNCTION3(IppStatus, ippsAbs_64f, const Ipp64f*, Ipp64f*, int);
	DECLARE_FUNCTION3(IppStatus, ippsMin_32f, const Ipp32f*, int, Ipp32f*);
	DECLARE_FUNCTION3(IppStatus, ippsMax_32f, const Ipp32f*, int, Ipp32f*);
	DECLARE_FUNCTION3(IppStatus, ippsAddC_32f_I, Ipp32f, Ipp32f*, int);
	DECLARE_FUNCTION4(IppStatus, ippsAddProduct_64fc, const Ipp64fc*, const Ipp64fc*, Ipp64fc*, int);	// pSrcDst[n] = pSrcDst[n] + pSrc1[n] * pSrc2[n], n=0,1,2,..len-1.
	DECLARE_FUNCTION3(IppStatus, ippsMulC_32f_I, Ipp32f, Ipp32f*, int);
	DECLARE_FUNCTION3(IppStatus, ippsMul_64f_I, const Ipp64f*, Ipp64f*, int);
	DECLARE_FUNCTION3(IppStatus, ippsMul_64fc_I, const Ipp64fc*, Ipp64fc*, int);
	DECLARE_FUNCTION4(IppStatus, ippsMul_64f, const Ipp64f*, const Ipp64f*, Ipp64f*, int);
	DECLARE_FUNCTION4(IppStatus, ippsMul_64fc, const Ipp64fc*, const Ipp64fc*, Ipp64fc*, int);
	DECLARE_FUNCTION2(IppStatus, ippsSqr_64f_I, Ipp64f*, int);
	DECLARE_FUNCTION2(IppStatus, ippsSqr_64fc_I, Ipp64fc*, int);
	DECLARE_FUNCTION2(IppStatus, ippsSqrt_64f_I, Ipp64f*, int);
	DECLARE_FUNCTION2(IppStatus, ippsSqrt_64fc_I, Ipp64fc*, int);
	DECLARE_FUNCTION3(IppStatus, ippsDiv_64f_I, const Ipp64f*, Ipp64f*, int);		// Div(X,Y) : Y[n] = Y[n] / X[n]
	DECLARE_FUNCTION3(IppStatus, ippsDiv_64fc_I, const Ipp64fc*, Ipp64fc*, int);	// Div(X,Y) : Y[n] = Y[n] / X[n]
	DECLARE_FUNCTION2(IppStatus, ippsSortAscend_64f_I, Ipp64f*, int);
	DECLARE_FUNCTION2(IppStatus, ippsSortDescend_64f_I, Ipp64f*, int);
	DECLARE_FUNCTION4(IppStatus, ippsRandUniformInitAlloc_32f, IppsRandUniState_32f**, Ipp32f, Ipp32f, unsigned int);
	DECLARE_FUNCTION3(IppStatus, ippsRandUniform_32f, Ipp32f*, int, IppsRandUniState_32f*);
	DECLARE_FUNCTION1(IppStatus, ippsRandUniformFree_32f, IppsRandUniState_32f*);
	DECLARE_FUNCTION4(IppStatus, ippsFFTInitAlloc_R_64f,IppsFFTSpec_R_64f**,int , int , IppHintAlgorithm );
	DECLARE_FUNCTION4(IppStatus, ippsFFTFwd_RToCCS_64f,const Ipp64f*, Ipp64f*,const IppsFFTSpec_R_64f*, Ipp8u*);
	DECLARE_FUNCTION1(IppStatus, ippsFFTFree_R_64f,IppsFFTSpec_R_64f*);
	DECLARE_FUNCTION1(Ipp64f*, ippsMalloc_64f, int);
	DECLARE_FUNCTION1(Ipp64fc*, ippsMalloc_64fc, int);
	DECLARE_FUNCTION3(IppStatus, ippsConjCcs_64fc,const Ipp64f*, Ipp64fc*, int);
	//DECLARE_FUNCTION4(IppStatus, IppsFFTSPec_R_64f,IppsFFTSpec_R_64f**,int , int , IppHintAlgorithm );
	DECLARE_FUNCTION4(IppStatus, ippsFFTInitAlloc_R_32f, IppsFFTSpec_R_32f** , int, int, IppHintAlgorithm);
	DECLARE_FUNCTION4(IppStatus, ippsFFTInitAlloc_R_16s,IppsFFTSpec_R_16s**,int, int, IppHintAlgorithm);
	DECLARE_FUNCTION4(IppStatus, ippsFFTInitAlloc_R_32s,IppsFFTSpec_R_32s**,int, int, IppHintAlgorithm);
	DECLARE_FUNCTION5(IppStatus, ippsFFTFwd_RToCCS_16s_Sfs,const Ipp16s*, Ipp16s*,const IppsFFTSpec_R_16s*,int, Ipp8u*);
	DECLARE_FUNCTION5(IppStatus, ippsFFTFwd_RToCCS_32s_Sfs,const Ipp32s*, Ipp32s*,const IppsFFTSpec_R_32s*,int, Ipp8u*);
	DECLARE_FUNCTION4(IppStatus, ippsMagnitude_16sc_Sfs,const Ipp16sc*,Ipp16s*,int,int);
	DECLARE_FUNCTION4(IppStatus, ippsMagnitude_32sc_Sfs,const Ipp32sc*,Ipp32s*,int,int);
	DECLARE_FUNCTION3(IppStatus, ippsMagnitude_64fc, const Ipp64fc*, Ipp64f*, int);
	DECLARE_FUNCTION4(IppStatus, ippsMagnitude_64f, const Ipp64f*, const Ipp64f*, Ipp64f*, int);
	DECLARE_FUNCTION3(IppStatus, ippsReal_64fc, const Ipp64fc*, Ipp64f*, int);
	DECLARE_FUNCTION3(IppStatus, ippsImag_64fc, const Ipp64fc*, Ipp64f*, int);
	DECLARE_FUNCTION4(IppStatus, ippsRealToCplx_64f, const Ipp64f*, const Ipp64f*, Ipp64fc*, int);
	DECLARE_FUNCTION1(IppStatus, ippsFFTFree_R_32s,IppsFFTSpec_R_32s*);
	DECLARE_FUNCTION1(Ipp32s*,  ippsMalloc_32s, int );
	DECLARE_FUNCTION4(IppStatus, ippsDFTInitAlloc_R_64f,IppsDFTSpec_R_64f**,int, int, IppHintAlgorithm);
	DECLARE_FUNCTION4(IppStatus, ippsDFTInitAlloc_R_32f,IppsDFTSpec_R_32f**,int, int, IppHintAlgorithm);
	DECLARE_FUNCTION4(IppStatus, ippsDFTInitAlloc_R_16s,IppsDFTSpec_R_16s**,int, int, IppHintAlgorithm);
	DECLARE_FUNCTION4(IppStatus, ippsDFTInitAlloc_C_64f, IppsDFTSpec_C_64f**, int, int, IppHintAlgorithm);
	DECLARE_FUNCTION4(IppStatus, ippsDFTInitAlloc_C_64fc, IppsDFTSpec_C_64fc**, int, int, IppHintAlgorithm);
	DECLARE_FUNCTION1(IppStatus, ippsDFTFree_R_64f, IppsDFTSpec_R_64f*);
	DECLARE_FUNCTION1(IppStatus, ippsDFTFree_R_32f, IppsDFTSpec_R_32f*);
	DECLARE_FUNCTION1(IppStatus, ippsDFTFree_R_16s, IppsDFTSpec_R_16s*);
	DECLARE_FUNCTION1(IppStatus, ippsDFTFree_C_64f, IppsDFTSpec_C_64f*);
	DECLARE_FUNCTION1(IppStatus, ippsDFTFree_C_64fc, IppsDFTSpec_C_64fc*);
	DECLARE_FUNCTION4(IppStatus, ippsDFTFwd_RToCCS_64f,const Ipp64f*,Ipp64f*,const IppsDFTSpec_R_64f*,Ipp8u*);
	DECLARE_FUNCTION5(IppStatus, ippsDFTFwd_RToCCS_16s_Sfs,const Ipp16s*,Ipp16s*,const IppsDFTSpec_R_16s*,int, Ipp8u*);
	DECLARE_FUNCTION6(IppStatus, ippsDFTFwd_CToC_64f, const Ipp64f*, const Ipp64f*, Ipp64f*, Ipp64f*, const IppsDFTSpec_C_64f*, Ipp8u*);
	DECLARE_FUNCTION4(IppStatus, ippsDFTFwd_CToC_64fc, const Ipp64fc*, Ipp64fc*, const IppsDFTSpec_C_64fc*, Ipp8u*);
	DECLARE_FUNCTION1(Ipp16s*, ippsMalloc_16s, int );
	DECLARE_FUNCTION3(IppStatus, ippsMagnitude_32fc, const Ipp32fc*, Ipp32f*, int);
	DECLARE_FUNCTION3(IppStatus, ippsAdd_32f_I, const Ipp32f*, Ipp32f*, int);
};

class IPPCORElib
{
public:
	virtual IppStatus ippSetNumThreads(int)= 0;	
};

class IPPCOREDll : public PDLL, IPPCORElib
{
#pragma warning(push)
#pragma warning(disable:26495)
#pragma warning(disable:26444)
	//call the macro and pass your class name
	DECLARE_CLASS(IPPCOREDll)
#pragma warning(pop)

	DECLARE_FUNCTION1( IppStatus, ippSetNumThreads, int );	
};