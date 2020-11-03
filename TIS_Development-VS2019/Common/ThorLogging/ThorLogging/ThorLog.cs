using System;
using System.Diagnostics;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace ThorLogging
{
    public sealed class ThorLog
    {
        private static volatile ThorLog instance;
        private static object syncRoot = new Object();

        private ThorLog()
        {
        }

        public static ThorLog Instance
        {
            get
            {
                if (instance == null)
                {
                    lock (syncRoot)
                    {
                        if (instance == null)
                            instance = new ThorLog();
                    }
                }
            
                return instance;
            }
        }


        public void TraceEvent(TraceEventType type, int id, string message)
        {
            TLTraceEvent((int)type, id,message);
        }

        [DllImport(".\\Modules_Native\\ThorLoggingUnmanaged.dll", EntryPoint = "TLTraceEvent")]
        private extern static void TLTraceEvent(int type, int id, [MarshalAs(UnmanagedType.LPWStr)] string message);

    }
}
