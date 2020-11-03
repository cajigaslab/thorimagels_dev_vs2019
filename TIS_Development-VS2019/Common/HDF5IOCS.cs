namespace HDF5CS
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Text;

    public class H5CSWrapper
    {
        #region Fields

        IntPtr _dataPtr;
        private string _fileName;

        #endregion Fields

        #region Constructors

        public H5CSWrapper(string fileName)
        {
            _fileName = fileName;
        }

        public H5CSWrapper()
        {
            _fileName = null;
        }

        #endregion Constructors

        #region Methods

        public bool CheckH5GrpDataset(string groupnm, string datasetnm, ref UInt64 size)
        {
            if ((true == OpenH5()) && 1 == CheckH5GrpData(groupnm, datasetnm, ref size))
            { return true; }
            else
            { return false; }
        }

        public bool CloseH5()
        {
            if (1 == CloseH5File())
            { return true; }
            else
            { return false; }
        }

        public bool CreateGroupDatasetNames<T>(string grp, string[] datasets, int dsetnum)
        {
            int dtype = GetH5Type(typeof(T));
            if (1 == CreateGroupDatasets(grp, datasets, dsetnum, dtype))
            { return true; }
            else
            { return false; }
        }

        public bool CreateH5()
        {
            _fileName = GetSavePathAndFileName();
            try
            {
                if (1 == CreateH5File(_fileName, 0, 0))     //openType: 0 Create, 1 ReadOnly, 2 ReadWrite (different from DotNet).
                { return true; }
                else
                { return false; }
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show(ex.Message);
                return false;
            }
        }

        public bool DestroyH5()
        {
            if (1 == DestroyH5File())
            { return true; }
            else
            { return false; }
        }

        public bool ExtendDataset<T>(string groupnm, string datasetnm, T[] buf, bool extend, UInt32 writesize)
        {
            bool ret = true;
            int dtype = GetH5Type(typeof(T));
            GCHandle handle = GCHandle.Alloc(buf, GCHandleType.Pinned);
            IntPtr add = handle.AddrOfPinnedObject();
            if (1 != ExtendData(groupnm, datasetnm, add, dtype, extend, writesize))
            {
                ret = false;
            }
            handle.Free();
            return ret;
        }

        public bool GetGroupDatasetName(string path, ref H5NodeInfo nInfo)
        {
            int gnum = 0, dnum = 0;
            IntPtr groups = IntPtr.Zero;
            IntPtr datasets = IntPtr.Zero;
            if (1 == GetGroupDatasetNames(path, out groups, out gnum, out datasets, out dnum))
            {
                MarshalStrArray(groups, gnum, out nInfo.groups);
                MarshalStrArray(datasets, dnum, out nInfo.datasets);
                return true;
            }
            else
            {
                return false;
            }
        }

        public string GetSavePathAndFileName()
        {
            StringBuilder str = new StringBuilder(260, 260);
            if (1 == GetPathandFilename(str, 260))
            {
                return str.ToString();
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// From Unmanaged to managed string array.
        /// </summary>
        /// <param name="pUnStrArray"></param>
        /// <param name="AryCnt"></param>
        /// <param name="StrArray"></param>
        public void MarshalStrArray(IntPtr pUnStrArray, int AryCnt, out string[] StrArray)
        {
            if (AryCnt > 0)
            {
                IntPtr[] pIntPtrArray = new IntPtr[AryCnt];
                StrArray = new string[AryCnt];

                Marshal.Copy(pUnStrArray, pIntPtrArray, 0, AryCnt);

                for (int i = 0; i < AryCnt; i++)
                {
                    StrArray[i] = Marshal.PtrToStringAnsi(pIntPtrArray[i]);
                    Marshal.FreeCoTaskMem(pIntPtrArray[i]);
                }

                Marshal.FreeCoTaskMem(pUnStrArray);
            }
            else
            {
                StrArray = null;
            }
        }

        public bool OpenH5()
        {
            try
            {
                if (1 == OpenH5File(_fileName, 1))     //openType: 1 ReadOnly, 2 ReadWrite (different from DotNet).
                { return true; }
                else
                { return false; }
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show(ex.Message);
                return false;
            }
        }

        public bool OpenRWH5()
        {
            try
            {
                if (1 == OpenH5File(_fileName, 2))     //openType: 1 ReadOnly, 2 ReadWrite (different from DotNet).
                { return true; }
                else
                { return false; }
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show(ex.Message);
                return false;
            }
        }

        public double[] ReadDoubleData(string group, string dataset, UInt64 start, UInt64 size)
        {
            OpenH5();
            double[] ret = ReadH5Dataset<double>(group, dataset, start, size);
            CloseH5();
            return ret;
        }

        /// <summary>
        /// Returned value will be uInt32 / uInt64 instead of Int32 / Int64
        /// </summary>
        public T[] ReadH5Dataset<T>(string groupnm, string datasetnm, UInt64 start, UInt64 readsize)
        {
            try
            {
                _dataPtr = Marshal.AllocHGlobal((int)readsize * Marshal.SizeOf(typeof(T)));
                T[] readData = new T[readsize];

                if (typeof(T) == typeof(Double))
                {
                    if (1 == ReadH5Data(groupnm, datasetnm, _dataPtr, 1, start, readsize))
                    {
                        Marshal.Copy(_dataPtr, (double[])(object)readData, 0, (int)readsize);
                    }
                }
                else if (typeof(T) == typeof(Int32))
                {
                    if (1 == ReadH5Data(groupnm, datasetnm, _dataPtr, 2, start, readsize))
                    {
                        Marshal.Copy(_dataPtr, (Int32[])(object)readData, 0, (int)readsize);
                    }
                }
                else if (typeof(T) == typeof(Int64))
                {
                    if (1 == ReadH5Data(groupnm, datasetnm, _dataPtr, 3, start, readsize))
                    {
                        Marshal.Copy(_dataPtr, (Int64[])(object)readData, 0, (int)readsize);
                    }
                }

                Marshal.FreeHGlobal(_dataPtr);
                return readData;
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show(ex.Message);
                return null;
            }
        }

        public Int32[] ReadIntegerData(string group, string dataset, UInt64 start, UInt64 size)
        {
            OpenH5();
            Int32[] ret = ReadH5Dataset<Int32>(group, dataset, start, size);
            CloseH5();
            return ret;
        }

        public bool SetSavePathAndFileName(string pathandname)
        {
            try
            {
                if (1 == SetPathandFilename(pathandname))
                { return true; }
                else
                { return false; }
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show(ex.Message);
                return false;
            }
        }

        public bool WriteDataset<T>(string groupnm, string datasetnm, T[] buf, UInt64 start, UInt32 writesize)
        {
            bool ret = true;
            int dtype = GetH5Type(typeof(T));
            GCHandle handle = GCHandle.Alloc(buf, GCHandleType.Pinned);
            IntPtr add = handle.AddrOfPinnedObject();
            if (1 != WriteData(groupnm, datasetnm, add, dtype, start, writesize))
            {
                ret = false;
            }
            handle.Free();
            return ret;
        }

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "CheckGroupDataset")]
        private static extern int CheckH5GrpData(string groupnm, string datasetnm, ref UInt64 size);

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "CloseFileIO")]
        private static extern int CloseH5File();

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "CreateGroupDatasets", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern int CreateGroupDatasets([In][MarshalAs(UnmanagedType.LPStr)] string grp, [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr)] string[] datasets, int dsetnum, int dtype);

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "CreateFileIO", CharSet = CharSet.Unicode)]
        private static extern int CreateH5File(string filenm, int openType, UInt64 bufsize);

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "DestroyFileIO")]
        private static extern int DestroyH5File();

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "ExtendData")]
        private static extern int ExtendData(string groupnm, string datasetnm, IntPtr buf, int dataType, bool extend, UInt32 writesize);

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "GetGroupDatasetNames", CharSet = CharSet.Unicode)]
        private static extern int GetGroupDatasetNames(string path, out IntPtr groups, out int gnum, out IntPtr datasets, out int dnum);

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "GetPathandFilename", CharSet = CharSet.Unicode)]
        private static extern int GetPathandFilename(StringBuilder path, int length);

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "OpenFileIO", CharSet = CharSet.Unicode)]
        private static extern int OpenH5File(string filenm, int openType);

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "ReadData")]
        private static extern int ReadH5Data(string groupnm, string datasetnm, IntPtr buf, int dataType, UInt64 start, UInt64 readsize);

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "SetPathandFilename", CharSet = CharSet.Unicode)]
        private static extern int SetPathandFilename(string path);

        [DllImport(".\\Modules_Native\\HDF5IO.dll", EntryPoint = "WriteData")]
        private static extern int WriteData(string groupnm, string datasetnm, IntPtr buf, int dataType, UInt64 start, UInt32 writesize);

        private int GetH5Type(Type Tobj)
        {
            int dtype = 0;
            switch (Type.GetTypeCode(Tobj))
            {
                case TypeCode.Double:
                    dtype = 1;
                    break;
                case TypeCode.UInt32:
                    dtype = 2;
                    break;
                case TypeCode.UInt64:
                    dtype = 3;
                    break;
                case TypeCode.Single:
                    dtype = 4;
                    break;
                case TypeCode.Byte:
                    dtype = 5;
                    break;
            }
            return dtype;
        }

        #endregion Methods
    }

    public class H5NodeInfo
    {
        #region Fields

        public string[] datasets;
        public string[] groups;

        #endregion Fields
    }
}