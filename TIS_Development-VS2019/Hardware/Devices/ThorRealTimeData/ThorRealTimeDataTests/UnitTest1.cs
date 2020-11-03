namespace ThorRealTimeDataTests
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Runtime.InteropServices;
    using System.Text;

    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class UnitTest1
    {
        #region Methods

        [TestMethod]
        [DeploymentItem("..\\..\\..\\x64\\Debug\\ThorRealTimeData.dll")]
        [DeploymentItem("..\\..\\..\\ThorRealTimeDataSettings.xml")]
        [DeploymentItem("..\\..\\..\\ThorRealTimeData\\bin")]
        public void FreeRun()
        {
            // Use stopwatch to control FreeRun timing:
            Stopwatch timer = new Stopwatch();

            string curDir = Directory.GetCurrentDirectory();
            string[] fileList = Directory.GetFiles("..\\..\\..\\TestSettings");
            int fid = 0;
                Assert.IsTrue(fileList.Length > 0);

                for (int i = 0; i < fileList.Length; i++)
                {
                    timer.Start();
                    fid = Convert.ToInt32(fileList[i].Substring(fileList[i].LastIndexOf('\\')+1,4));
                    File.Copy(fileList[i], "ThorRealTimeDataSettings.xml", true);

                    SetFileSaving(1);

                    string testFileName = curDir + "\\" + string.Format("test{0}.h5", fid);
                    Assert.IsTrue(1 == SetH5PathAndFilename(testFileName));

                    Assert.IsTrue(1 == StartAcquireData());

                    while (1 == IsInAcquire())
                    {
                        if (15000 <= timer.ElapsedMilliseconds)
                        {
                            break;
                        }
                    }

                    StopAcquireData();
                    timer.Stop();
                    timer.Reset();

                    Assert.IsTrue(true == File.Exists(testFileName));

                    //Do Bleaching:
                    StartAsyncAcquireData();

                    while (1 == IsInAsyncAcquire())
                    {
                    }

                    StopAsyncAcquireData();

                }
        }

        [DllImport("ThorRealTimeData.dll", EntryPoint = "CheckH5GrpData")]
        private static extern int CheckH5GrpData(string groupnm, string datasetnm, ref UInt64 size);

        [DllImport("ThorRealTimeData.dll", EntryPoint = "CloseH5File")]
        private static extern int CloseH5File();

        [DllImport("ThorRealTimeData.dll", EntryPoint = "GetH5PathAndFilename", CharSet = CharSet.Unicode)]
        private static extern int GetH5PathAndFilename(StringBuilder path, int length);

        [DllImport("ThorRealTimeData.dll", EntryPoint = "GetStructData")]
        private static extern int GetStructData(ref CompoundDataStruct CompDataStruct);

        [DllImport("ThorRealTimeData.dll", EntryPoint = "IsFileSaving")]
        private static extern int IsFileSaving();

        [DllImport("ThorRealTimeData.dll", EntryPoint = "IsInAcquire")]
        private static extern int IsInAcquire();

        [DllImport("ThorRealTimeData.dll", EntryPoint = "IsInAsyncAcquire")]
        private static extern int IsInAsyncAcquire();

        [DllImport("ThorRealTimeData.dll", EntryPoint = "OpenH5File", CharSet = CharSet.Unicode)]
        private static extern int OpenH5File(string filenm, int openType);

        [DllImport("ThorRealTimeData.dll", EntryPoint = "ReadH5Data")]
        private static extern int ReadH5Data(string groupnm, string datasetnm, IntPtr buf, int dataType, UInt64 start, UInt64 readsize);

        [DllImport("ThorRealTimeData.dll", EntryPoint = "SetFileSaving")]
        private static extern int SetFileSaving(Int32 save);

        [DllImport("ThorRealTimeData.dll", EntryPoint = "SetH5PathAndFilename", CharSet = CharSet.Unicode)]
        private static extern int SetH5PathAndFilename(string pathAndName);

        [DllImport("ThorRealTimeData.dll", EntryPoint = "StartAcquireData")]
        private static extern int StartAcquireData();

        [DllImport("ThorRealTimeData.dll", EntryPoint = "StartAsyncAcquireData")]
        private static extern int StartAsyncAcquireData();

        [DllImport("ThorRealTimeData.dll", EntryPoint = "StopAcquireData")]
        private static extern int StopAcquireData();

        [DllImport("ThorRealTimeData.dll", EntryPoint = "StopAsyncAcquireData")]
        private static extern int StopAsyncAcquireData();

        #endregion Methods

        #region Nested Types

        /// <summary>
        /// Identical data structure defined in ThorRealTimeData.dll.
        /// global counter (gCtr64) with size (gcLengthCom) to serve as timing (x-axis), 
        /// multiple analog input channels (aiData) with size of aiLength (length of each channel x channel numbers),
        /// multiple digital input channels (diData) with size of diLength (length of each channel x channel numbers),
        /// edge counting channel (ciData64) with size (ciLengthCom).
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct CompoundDataStruct
        {
            public UInt64 gcLength;
            public UInt64 aiLength;
            public UInt64 diLength;
            public UInt64 ciLength;
            public IntPtr aiData;
            public IntPtr diData;
            public IntPtr ciData;
            public IntPtr gCtr64;
        }

        #endregion Nested Types
    }
}