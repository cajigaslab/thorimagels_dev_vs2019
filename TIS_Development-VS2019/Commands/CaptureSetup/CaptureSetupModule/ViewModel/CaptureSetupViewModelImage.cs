namespace CaptureSetupDll.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Drawing.Design;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Xml;

    using CaptureSetupDll.Model;

    using OverlayManager;

    using ThorImageInfastructure;

    using ThorLogging;

    using ThorSharedTypes;

    public partial class CaptureSetupViewModel : ViewModelBase
    {
        #region Fields

        ScrollBarVisibility _ivScrollbarVisibility = ScrollBarVisibility.Hidden;

        #endregion Fields

        #region Properties

        public int DFLIMDiagnosticsBufferLength
        {
            get
            {
                return _captureSetup.DFLIMDiagnosticsBufferLength;
            }
            set
            {
                _captureSetup.DFLIMDiagnosticsBufferLength = value;
            }
        }

        public ushort[][] DFLIMDiagnosticsData
        {
            get
            {
                return _captureSetup.DFLIMDiagnosticsData;
            }
        }

        public object DFLIMDiagnosticsDataLock
        {
            get
            {
                return this._captureSetup.DFLIMDiagnosticsDataLock;
            }
        }

        public uint[][] DFLIMHistogramData
        {
            get
            {
                return this._captureSetup.DFLIMHistogramData;
            }
        }

        public object DFLIMHistogramDataLock
        {
            get
            {
                return this._captureSetup.DFLIMHistogramDataLock;
            }
        }

        public bool DFLIMNewHistogramData
        {
            get
            {
                return this._captureSetup.DFLIMNewHistogramData;
            }
            set
            {
                this._captureSetup.DFLIMNewHistogramData = value;
            }
        }

        public ScrollBarVisibility IVScrollbarVisibility
        {
            get => _ivScrollbarVisibility;
            set
            {
                _ivScrollbarVisibility = value;
                OnPropertyChanged("IVScrollbarVisibility");
            }
        }

        public int LSMChannel
        {
            get => (int)MVMManager.Instance["ScanControlViewModel", "LSMChannel"];
        }

        public bool[] LSMChannelEnable
        {
            get => (bool[])MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable"];
        }

        public bool NewDFLIMDiagnosticsData
        {
            get
            {
                return this._captureSetup.NewDFLIMDiagnosticsData;
            }
            set
            {
                this._captureSetup.NewDFLIMDiagnosticsData = value;
            }
        }

        #endregion Properties
    }
}