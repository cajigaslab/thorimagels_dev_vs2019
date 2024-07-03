namespace ThorSharedTypes
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct FrameInfoStruct
    {
        public Int32 imageWidth;
        public Int32 imageHeight;
        public Int32 channels;
        public Int32 fullFrame;
        public Int32 isMROI;
        public Int32 scanAreaID;
        public Int32 scanAreaIndex;
        public Int32 totalScanAreas;
        public Int32 isNewMROIFrame;
        public Int32 fullImageWidth;
        public Int32 fullImageHeight;
        public Int32 topInFullImage;
        public Int32 leftInFullImage;
        public Int32 mROIStripeFieldSize;
        public Int32 bufferType;
        public UInt64 copySize;
        public Int32 numberOfPlanes;
        public Int32 polarImageType;
        public Int32 totalSequences;
        public Int32 sequenceIndex;
        public Int32 sequenceSelectedChannels;
        public Int32 pixelAspectRatioYScale;
    }

    public class FrameData
    {
        #region Fields

        public readonly object dataLock = new object();

        public int averageFrameCount;
        public int averageMode;
        public int bitsPerPixel;
        public int channelSelection;
        public bool contiguousChannels;
        public uint[] dFLIMArrivalTimeSumData;
        public double[] dFLIMBinDurations;
        public ushort[] dFLIMSinglePhotonData;
        public FrameInfoStruct frameInfo;
        public bool isFastZPreviewImage;
        public ushort[] pixelData;
        public PixelSizeUM pixelSizeUM;

        #endregion Fields
    }
}