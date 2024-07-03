using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ThorSharedTypes
{
    /// <summary>
    /// This struct is used to convey important notifications that occur in the native capture library
    /// </summary>
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct CaptureNotification
    {
        public Int32 isAsyncParamUpdate; // true iff parameters are suspected to be out of sync between UI and device
    }
}
