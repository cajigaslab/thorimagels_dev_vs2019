namespace FLIMFitting.Utils
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Xml;

    public static class ObjectExtensions
    {
        #region Methods

        public static uint[] Copy(this uint[] pieces)
        {
            return pieces.Select(x =>
            {
                var handle = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(uint)));

                try
                {
                    Marshal.StructureToPtr(x, handle, false);
                    return (uint)Marshal.PtrToStructure(handle, typeof(uint));
                }
                finally
                {
                    Marshal.FreeHGlobal(handle);
                }
            }).ToArray();
        }

        public static ushort[] Copy(this ushort[] pieces)
        {
            return pieces.Select(x =>
            {
                var handle = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(ushort)));

                try
                {
                    Marshal.StructureToPtr(x, handle, false);
                    return (ushort)Marshal.PtrToStructure(handle, typeof(ushort));
                }
                finally
                {
                    Marshal.FreeHGlobal(handle);
                }
            }).ToArray();
        }

        #endregion Methods
    }
}