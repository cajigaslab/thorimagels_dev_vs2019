using ImageReviewModule.Model;
using System;
using System.Runtime.InteropServices;

namespace ImageReviewDll.OME
{
    public enum PixelTypes
    {
        PixelType_INT8 = 0,
        PixelType_UINT8 = 1,
        PixelType_UINT16 = 2,
        PixelType_INT16 = 3,
        PixelType_FLOAT = 4,
    }

    public enum ImageTypes
    {
        GRAY = 0,
        RGB = 1,
    }

    public enum CompressionMode
    {
        COMPRESSION_NONE = 0,
        //COMPRESSION_LZ4 = 1,
        COMPRESSION_LZW = 2,
        COMPRESSION_JPEG = 3,
        //COMPRESSION_ZIP = 4,
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct ImageInfo
    {
        public uint Width;
        public uint Height;
        public uint LineBytes;
        public PixelTypes PixelType;
        public ushort validBits;
        public ImageTypes ImageType;
        public CompressionMode CompressionMode;
    }

    public static class ClassicTiffConverterWrapper
    {
        private const string dllName = "ClassicTiffConverter.dll";

        [DllImport(dllName, EntryPoint = "TC_GetImageCount", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int GetImageCount(string fileName, ref int imageHandle, ref uint imageCount);

        [DllImport(dllName, EntryPoint = "TC_GetImageInfo", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetImageInfo(int imageHandle, uint imageNumber, ref ImageInfo info);

        [DllImport(dllName, EntryPoint = "TC_GetImageData", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetImageData(int imageHandle, uint imageNumber, IntPtr imageData);

        [DllImport(dllName, EntryPoint = "TC_CloseImage", CallingConvention = CallingConvention.Cdecl)]
        public static extern int CloseImage(int imageHandle);


        [DllImport(dllName, EntryPoint = "TC_LoadRawDataFile", CallingConvention = CallingConvention.Cdecl)]
        public static extern int LoadRawDataFile(string fileName, ref int imageHandle);
        [DllImport(dllName, EntryPoint = "TC_GetRawData", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetRawData(int imageHandle, int offset, int size, IntPtr imageData);
        [DllImport(dllName, EntryPoint = "TC_CloseRawImage", CallingConvention = CallingConvention.Cdecl)]
        public static extern int CloseRawImage(int imageHandle);


        [DllImport(dllName, EntryPoint = "TC_CreateOMETiff", CallingConvention = CallingConvention.Cdecl)]
        public static extern int CreateOMETiff(string fileName);

        [DllImport(dllName, EntryPoint = "TC_ConfigOMEHeader", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ConfigOMEHeader(long handle, SampleInfo sample, int regionPixelX, int regionPixelY, float regionW, float regionH, ushort zCount, ushort tCount,
            int regionPositionPixelX, int regionPositionPixelY, int bitsPerPixel, float regionPixelSizeUM, double zStepSizeUM, double intervalSec, int channelNumber, string channels);

        [DllImport(dllName, EntryPoint = "TC_SaveOMEData", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SaveOMEData(long handle, int channelID, int zIndex, int tIndex, IntPtr data);

        [DllImport(dllName, EntryPoint = "TC_SaveAdditionalData", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SaveAdditionalData(long handle, string data, int size);

        [DllImport(dllName, EntryPoint = "TC_CloseOMETiff", CallingConvention = CallingConvention.Cdecl)]
        public static extern int CloseOMETiff(long omeHandle);
        // For Raw to TIFF converter
        [DllImport(dllName, EntryPoint = "TC_ConvertRawToTIFF", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern int ConvertRawToTIFF(string rawFileName, string tiffFolderName, int cCount, int tCount, int zCount, double intervalSec, string channelNameArray, long width, long height, double umPerPixel, double zStepSizeUM);
    }
}
