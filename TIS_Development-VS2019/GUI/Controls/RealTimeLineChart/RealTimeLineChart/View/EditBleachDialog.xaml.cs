namespace RealTimeLineChart.View
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;
    using System.Xml;

    using RealTimeLineChart.ViewModel;

    using ThorLogging;

    /// <summary>
    /// Interaction logic for EditBleachDialog.xaml
    /// </summary>
    public partial class EditBleachDialog : Window
    {
        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="EditBleachDialog"/> class.
        /// </summary>
        public EditBleachDialog()
        {
            InitializeComponent();
            this.Loaded += new RoutedEventHandler(EditBleachDialog_Loaded);
            this.Unloaded += new RoutedEventHandler(EditBleachDialog_Unloaded);
        }

        #endregion Constructors

        #region Events

        /// <summary>
        /// Occurs when [bleach dialog closed].
        /// </summary>
        public event Action<int, int> BleachDialogClosed;

        #endregion Events

        #region Properties

        /// <summary>
        /// Gets or sets the real time vm.
        /// </summary>
        /// <value>
        /// The real time vm.
        /// </value>
        public RealTimeLineChartViewModel RealTimeVM
        {
            get; set;
        }

        /// <summary>
        /// Gets or sets the settings.
        /// </summary>
        /// <value>
        /// The settings.
        /// </value>
        public XmlDocument Settings
        {
            get; set;
        }

        /// <summary>
        /// Gets or sets the window corner.
        /// </summary>
        /// <value>
        /// The window corner.
        /// </value>
        public Point windowCorner
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Handles the Click event of the btnCancel control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }

        /// <summary>
        /// Handles the Click event of the btnOK control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = true;
            this.Close();
        }

        /// <summary>
        /// Handles the Loaded event of the EditBleachDialog control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void EditBleachDialog_Loaded(object sender, RoutedEventArgs e)
        {
            try
            {
                //Set window top-left corner:
                var workingArea = System.Windows.SystemParameters.WorkArea;
                int left = Convert.ToInt32(windowCorner.X.ToString());
                int top = Convert.ToInt32(windowCorner.Y.ToString());
                this.Left = left;
                this.Top = top;

                XmlNodeList nodeList = Settings.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]/Bleach");

                if (0 < nodeList.Count)
                {
                    foreach (XmlNode node in nodeList)
                    {
                        tbBlchCycle.Text = Convert.ToInt32(node.Attributes["cycle"].Value.ToString(), CultureInfo.InvariantCulture).ToString();
                        tbBlchCycleInterval.Text = Convert.ToDouble(node.Attributes["interval"].Value.ToString(), CultureInfo.InvariantCulture).ToString();
                        ShtClsTxBox.Text = Convert.ToDouble(node.Attributes["pmtCloseTime"].Value.ToString(), CultureInfo.InvariantCulture).ToString();
                        BlchTmTxBox.Text = Convert.ToDouble(node.Attributes["bleachTime"].Value.ToString(), CultureInfo.InvariantCulture).ToString();
                        tbBlchIdle.Text = Convert.ToDouble(node.Attributes["bleachIdleTime"].Value.ToString(), CultureInfo.InvariantCulture).ToString();
                        tbBlchIteration.Text = Convert.ToInt32(node.Attributes["bleachIteration"].Value.ToString(), CultureInfo.InvariantCulture).ToString();
                        ShtOpTxBox.Text = Convert.ToDouble(node.Attributes["outDelayTime"].Value.ToString(), CultureInfo.InvariantCulture).ToString();
                        cbAsyncTrigMode.SelectedIndex = Convert.ToInt32(node.Attributes["bleachTrigMode"].Value.ToString(), CultureInfo.InvariantCulture);
                    }
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "RealTimeLineChart LoadDataFromFile Error: " + ex.Message);
            }
        }

        /// <summary>
        /// Handles the Unloaded event of the EditBleachDialog control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void EditBleachDialog_Unloaded(object sender, RoutedEventArgs e)
        {
            BleachDialogClosed((int)this.Left, (int)this.Top);
        }

        #endregion Methods
    }
}