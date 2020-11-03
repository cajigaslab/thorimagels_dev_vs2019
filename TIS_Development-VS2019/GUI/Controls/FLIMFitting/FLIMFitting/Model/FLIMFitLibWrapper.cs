namespace FLIMFitting.Model
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;

    #region Enumerations

    [Flags]
    public enum FlagDef : byte
    {
        AMP1 = 0x01,
        TAU1 = 0x02,
        AMP2 = 0x04,
        TAU2 = 0x08,
        TZERO = 0x10,
        GAUSSW = 0x20,
        SCATTR = 0x40
    }

    #endregion Enumerations

    [StructLayout(LayoutKind.Sequential)]
    public struct FLIMFitParam
    {
        public int CountOfBin;
        public IntPtr BinData;
        public IntPtr NsPerPoint;
        public byte FixFlag;
        public IntPtr ParamVec;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ParamVec
    {
        public double amp1;
        public double tau1;
        public double amp2;
        public double tau2;
        public double tZero;
        public double gaussW;
        public double scattr;
    }

    public class FLIMFitLibWrapper
    {
        #region Fields

        private const string FlimLib = "FLIMFitLibrary.dll";

        #endregion Fields

        #region Methods

        public static bool Fit(List<FLIMData> dataList, byte fixFlag, byte shareFlag, bool isDoubleExp)
        {
            var flimData = new FLIMFitParam();
            if (!GenerateFlimPrama(dataList, fixFlag, shareFlag, ref flimData)) return false;
            bool ret = true;
            if (isDoubleExp)
            {
                if (NlinfitGY2DoubleExponential(ref flimData) > 0)
                    ParseResult(dataList, flimData);
                else
                    ret = false;
            }
            else
            {
                if (NlinfitGY2SingleExponential(ref flimData) > 0)
                    ParseResult(dataList, flimData);
                else
                    ret = false;
            }
            FreeFlimPrama(ref flimData);
            return ret;
        }

        public static bool FitMulti(List<FLIMData> dataList, byte fixFlag, byte shareFlag, bool isDoubleExp)
        {
            var flimData = new FLIMFitParam();
            if (!GenerateFlimPrama(dataList, fixFlag, shareFlag, ref flimData)) return false;
            bool ret = true;
            if (isDoubleExp)
            {
                if (NlinfitGYmultiDoubleExponential(ref flimData, shareFlag) > 0)
                    ParseResult(dataList, flimData);
                else ret = false;
            }
            else
            {
                if (NlinfitGYmultiSingleExponential(ref flimData, shareFlag) > 0)
                    ParseResult(dataList, flimData);
                else ret = false;
            }
            FreeFlimPrama(ref flimData);
            return ret;
        }

        public static bool GetResult(List<FLIMData> dataList, byte shareFlag, bool isDoubleExp)
        {
            var flimData = new FLIMFitParam();
            if (!GenerateFlimPrama(dataList, 0, 0, ref flimData)) return false;
            bool ret = true;
            var binCount = dataList.Count;
            IntPtr resultArray = Marshal.AllocHGlobal(binCount * FLIMData.DATA_SIZE * sizeof(double));
            IntPtr chi2Array = Marshal.AllocHGlobal(binCount * sizeof(double));
            IntPtr tau8Array = Marshal.AllocHGlobal(binCount  * sizeof(double));

            int status = 0;
            if (isDoubleExp)
            {
                status = GetResultDoubleExponential(ref flimData, resultArray, chi2Array, tau8Array);
            }
            else
            {
                status = GetResultSingleExponential(ref flimData, shareFlag, resultArray, chi2Array, tau8Array);
            }

            if (status > 0)
            {
                double[] result = new double[binCount * FLIMData.DATA_SIZE];
                Marshal.Copy(resultArray, result, 0, binCount * FLIMData.DATA_SIZE);
                double[] chi2 = new double[binCount];
                Marshal.Copy(chi2Array, chi2, 0, binCount);
                double[] tau8 = new double[binCount];
                Marshal.Copy(tau8Array, tau8, 0, binCount);
                for (int i = 0; i < binCount; i++)
                {
                    var data = dataList[i];
                    Array.Copy(result, FLIMData.DATA_SIZE * i, data.FitResultArray, 0, FLIMData.DATA_SIZE);
                    data.Chi2 = chi2[i];
                    data.Tau8 = tau8[i];
                }
            }
            else
            {
                ret = false;
            }
            FreeFlimPrama(ref flimData);
            Marshal.FreeHGlobal(resultArray);
            Marshal.FreeHGlobal(chi2Array);
            Marshal.FreeHGlobal(tau8Array);
            return ret;
        }

        [DllImport(FlimLib, EntryPoint = "GetResultDoubleExponential", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetResultDoubleExponential(ref FLIMFitParam flimParam, IntPtr result, IntPtr chi2, IntPtr tau8);

        [DllImport(FlimLib, EntryPoint = "GetResultSingleExponential", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetResultSingleExponential(ref FLIMFitParam flimParam, byte shareFlag, IntPtr result, IntPtr chi2, IntPtr tau8);

        [DllImport(FlimLib, EntryPoint = "NlinfitGY2DoubleExponential", CallingConvention = CallingConvention.Cdecl)]
        public static extern int NlinfitGY2DoubleExponential(ref FLIMFitParam flimParam);

        [DllImport(FlimLib, EntryPoint = "NlinfitGY2SingleExponential", CallingConvention = CallingConvention.Cdecl)]
        public static extern int NlinfitGY2SingleExponential(ref FLIMFitParam flimParam);

        [DllImport(FlimLib, EntryPoint = "NlinfitGYmultiDoubleExponential", CallingConvention = CallingConvention.Cdecl)]
        public static extern int NlinfitGYmultiDoubleExponential(ref FLIMFitParam flimParam, byte shareFlag);

        [DllImport(FlimLib, EntryPoint = "NlinfitGYmultiSingleExponential", CallingConvention = CallingConvention.Cdecl)]
        public static extern int NlinfitGYmultiSingleExponential(ref FLIMFitParam flimParam, byte shareFlag);

        public static bool PreFit(List<FLIMData> dataList, byte fixFlag, byte shareFlag, bool isDoubleExp)
        {
            var flimData = new FLIMFitParam();
            if (!GenerateFlimPrama(dataList, fixFlag, shareFlag, ref flimData)) return false;
            bool ret = true;
            if (isDoubleExp)
            {
                if (PreFitDoubleExponential(ref flimData) > 0)
                {
                    ParseResult(dataList, flimData);
                }
                else ret = false;
            }
            else
            {
                if (PreFitSingleExponential(ref flimData) > 0)
                {
                    ParseResult(dataList, flimData);
                }
                else ret = false;
            }

            FreeFlimPrama(ref flimData);
            return ret;
        }

        [DllImport(FlimLib, EntryPoint = "PreFitDoubleExponential", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PreFitDoubleExponential(ref FLIMFitParam flimParam);

        [DllImport(FlimLib, EntryPoint = "PreFitSingleExponential", CallingConvention = CallingConvention.Cdecl)]
        public static extern int PreFitSingleExponential(ref FLIMFitParam flimParam);

        private static void FreeFlimPrama(ref FLIMFitParam flimPara)
        {
            Marshal.FreeHGlobal(flimPara.NsPerPoint);
            Marshal.FreeHGlobal(flimPara.BinData);
            Marshal.FreeHGlobal(flimPara.ParamVec);
        }

        private static bool GenerateFlimPrama(List<FLIMData> dataList, byte fixFlag, byte shareFlag, ref FLIMFitParam flimPara)
        {
            if (dataList.Count == 0) return false;
            int binCount = dataList.Count;
            int dataSize = FLIMData.DATA_SIZE;
            int dataCount = dataSize * binCount;
            double[] nsArray = new double[binCount];
            double[] dataArray = new double[dataCount];
            double[] paraArray = new double[FLIMData.PARAM_SIZE * binCount];
            for (int i = 0; i < binCount; i++)
            {
                var data = dataList[i];
                nsArray[i] = data.NsPerPoint;
                Array.Copy(data.DataArray, 0, dataArray, i * dataSize, dataSize);
                int index = i * FLIMData.PARAM_SIZE;
                paraArray[index++] = data.Amp1;
                paraArray[index++] = data.Tau1;
                paraArray[index++] = data.Amp2;
                paraArray[index++] = data.Tau2;
                paraArray[index++] = data.TZero;
                paraArray[index++] = data.GaussW;
                paraArray[index] = data.Scattr;
            }
            IntPtr nsPerPoint = Marshal.AllocHGlobal(binCount * sizeof(double));
            Marshal.Copy(nsArray, 0, nsPerPoint, binCount);
            IntPtr binData = Marshal.AllocHGlobal(dataCount * sizeof(double));
            Marshal.Copy(dataArray, 0, binData, dataCount);
            //IntPtr paramVec = Marshal.AllocHGlobal(binCount * Marshal.SizeOf<ParamVec>());
            const int NUM_PARAMVEC_DOUBLES = 7;

            IntPtr paramVec = Marshal.AllocHGlobal(binCount * sizeof(double) * NUM_PARAMVEC_DOUBLES);
            Marshal.Copy(paraArray, 0, paramVec, FLIMData.PARAM_SIZE * binCount);

            flimPara.CountOfBin = binCount;
            flimPara.FixFlag = fixFlag;
            flimPara.NsPerPoint = nsPerPoint;
            flimPara.BinData = binData;
            flimPara.ParamVec = paramVec;
            return true;
        }

        private static void ParseResult(List<FLIMData> dataList, FLIMFitParam flimFitParam)
        {
            int binCount = dataList.Count;
            double[] resultArray = new double[binCount* FLIMData.PARAM_SIZE];
            Marshal.Copy(flimFitParam.ParamVec, resultArray, 0, FLIMData.PARAM_SIZE * binCount);
            for (int i = 0; i < binCount; i++)
            {
                var flimData = dataList[i];
                int index = i * FLIMData.PARAM_SIZE;
                flimData.Amp1 = resultArray[index++];
                flimData.Tau1 = resultArray[index++];
                flimData.Amp2 = resultArray[index++];
                flimData.Tau2 = resultArray[index++];
                flimData.TZero = resultArray[index++];
                flimData.GaussW = resultArray[index++];
                flimData.Scattr = resultArray[index];
            }
        }

        #endregion Methods
    }
}