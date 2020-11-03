namespace DFLIMControl.ViewModel
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

    using DFLIMControl.Model;

    using DFLIMSetupAssistant;

    using ThorLogging;

    using ThorSharedTypes;

    public class DFLIMControlCaptureViewModel : DFLIMControlViewModelBase
    {
        #region Fields

        private Thread _histogramThread;
        private bool _runHistogramThread = true;

        #endregion Fields

        #region Constructors

        public DFLIMControlCaptureViewModel()
            : base()
        {
            _mainViewModelName = "RunSampleLSViewModel";
            DFLIMAcquisitionMode = 0;
            System.Windows.Application.Current.Exit += Current_Exit;

            _histogramThread = new Thread(DFLIMDataUpdate);
            _histogramThread.Start();
        }

        #endregion Constructors

        #region Properties

        public int DFLIMAcquisitionMode
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        void Current_Exit(object sender, ExitEventArgs e)
        {
            _runHistogramThread = false;
        }

        void DFLIMDataUpdate()
        {
            do
            {
                try
                {
                    if (0 == DFLIMAcquisitionMode)
                    {
                        CopyHistogram();
                    }
                    else if (1 == DFLIMAcquisitionMode)
                    {
                        CopyDiagnostics();
                    }
                }
                catch (Exception ex)
                {
                    ex.ToString();
                }
                Thread.Sleep(100);
            } while (_runHistogramThread);
        }

        #endregion Methods
    }
}