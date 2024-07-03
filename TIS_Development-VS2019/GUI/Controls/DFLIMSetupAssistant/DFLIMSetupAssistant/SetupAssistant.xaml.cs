namespace DFLIMSetupAssistant
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Windows.Threading;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class SetupAssistant : Window
    {
        #region Constructors

        public SetupAssistant()
        {
            InitializeComponent();
            Owner = System.Windows.Application.Current.MainWindow;
        }

        #endregion Constructors

        #region Properties

        public bool ShowDiagnostics
        {
            set
            {
                if (value)
                {
                    DiagnosticsColumn.Width = new GridLength(1, GridUnitType.Star);
                }
                else
                {
                    DiagnosticsColumn.Width = new GridLength(0, GridUnitType.Star);
                }
            }
        }

        public bool ShowHistrogram
        {
            set
            {
                if (value)
                {
                    HistogramColumn.Width = new GridLength(1, GridUnitType.Star);
                }
                else
                {
                    HistogramColumn.Width = new GridLength(0, GridUnitType.Star);
                }
            }
        }

        #endregion Properties

        #region Methods

        public void PlotDiagnostics(Dictionary<KeyValuePair<int, SolidColorBrush>, ushort[]> diagnostics)
        {
            foreach (var keyVal in diagnostics)
            {
                switch (keyVal.Key.Key)
                {
                    case 0:
                        if ((bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable0"])
                        {
                            DFLIMDiagnostics1.Plot(keyVal.Key, keyVal.Value);
                        }
                        break;
                    case 1:
                        if ((bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable1"])
                        {
                            DFLIMDiagnostics2.Plot(keyVal.Key, keyVal.Value);
                        }
                        break;
                    case 2:
                        if ((bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable2"])
                        {
                            DFLIMDiagnostics3.Plot(keyVal.Key, keyVal.Value);
                        }
                        break;
                    case 3:
                        if ((bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable3"])
                        {
                            DFLIMDiagnostics4.Plot(keyVal.Key, keyVal.Value);
                        }
                        break;
                }
            }

            if (!(bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable0"])
            {
                DFLIMDiagnostics1.Clear();
            }

            if (!(bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable1"])
            {
                DFLIMDiagnostics2.Clear();
            }

            if (!(bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable2"])
            {
                DFLIMDiagnostics3.Clear();
            }

            if (!(bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable3"])
            {
                DFLIMDiagnostics4.Clear();
            }
        }

        public void PlotHistogram(Dictionary<KeyValuePair<int, SolidColorBrush>, uint[]> histrogram)
        {
            foreach (var keyVal in histrogram)
            {
                switch (keyVal.Key.Key)
                {
                    case 0:
                        DFLIMHistogram1.Plot(keyVal.Key, keyVal.Value);
                        break;
                    case 1:
                        DFLIMHistogram2.Plot(keyVal.Key, keyVal.Value);
                        break;
                    case 2:
                        DFLIMHistogram3.Plot(keyVal.Key, keyVal.Value);
                        break;
                    case 3:
                        DFLIMHistogram4.Plot(keyVal.Key, keyVal.Value);
                        break;
                }
            }

            if (!(bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable0"])
            {
                DFLIMHistogram1.Clear();
            }

            if (!(bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable1"])
            {
                DFLIMHistogram2.Clear();
            }

            if (!(bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable2"])
            {
                DFLIMHistogram3.Clear();
            }

            if (!(bool)MVMManager.Instance["ScanControlViewModel", "LSMChannelEnable3"])
            {
                DFLIMHistogram4.Clear();
            }
        }

        #endregion Methods

        #region Nested Types

        public static class ObjectExtensions
        {
            #region Methods

            public static uint[] Copy(uint[] pieces)
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

            #endregion Methods
        }

        #endregion Nested Types
    }
}