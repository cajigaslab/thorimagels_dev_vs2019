namespace LogFileWindow
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Timers;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Xml;

    using LogFileWindow.ViewModel;

    using ThorLogging;

    public struct LogFileData
    {
        #region Fields

        public long channelEnable;
        public double[] profileDataX;
        public double[][] profileDataY;

        #endregion Fields

        #region Other

        //public long numChannel;

        #endregion Other
    }

    /// <summary>
    /// This class contains a few useful extenders for the ListBox
    /// </summary>
    public class ListBoxExtenders : DependencyObject
    {
        #region Fields

        public static readonly DependencyProperty AutoScrollToCurrentItemProperty = DependencyProperty.RegisterAttached("AutoScrollToCurrentItem", typeof(bool), typeof(ListBoxExtenders), new UIPropertyMetadata(default(bool), OnAutoScrollToCurrentItemChanged));

        #endregion Fields

        #region Methods

        /// <summary>
        /// Returns the value of the AutoScrollToCurrentItemProperty
        /// </summary>
        /// <param name="obj">The dependency-object whichs value should be returned</param>
        /// <returns>The value of the given property</returns>
        public static bool GetAutoScrollToCurrentItem(DependencyObject obj)
        {
            return (bool)obj.GetValue(AutoScrollToCurrentItemProperty);
        }

        /// <summary>
        /// This method will be called when the ListBox should
        /// be scrolled to the given index
        /// </summary>
        /// <param name="listBox">The ListBox which should be scrolled</param>
        /// <param name="index">The index of the item to which it should be scrolled</param>
        public static void OnAutoScrollToCurrentItem(ListBox listBox, int index)
        {
            if (listBox != null && listBox.Items != null && listBox.Items.Count > index && index >= 0)
                listBox.ScrollIntoView(listBox.Items[index]);
        }

        /// <summary>
        /// This method will be called when the AutoScrollToCurrentItem
        /// property was changed
        /// </summary>
        /// <param name="s">The sender (the ListBox)</param>
        /// <param name="e">Some additional information</param>
        public static void OnAutoScrollToCurrentItemChanged(DependencyObject s, DependencyPropertyChangedEventArgs e)
        {
            var listBox = s as ListBox;
            if (listBox != null)
            {
                var listBoxItems = listBox.Items;
                if (listBoxItems != null)
                {
                    var newValue = (bool)e.NewValue;

                    var autoScrollToCurrentItemWorker = new EventHandler((s1, e2) => OnAutoScrollToCurrentItem(listBox, listBox.Items.CurrentPosition));

                    if (newValue)
                        listBoxItems.CurrentChanged += autoScrollToCurrentItemWorker;
                    else
                        listBoxItems.CurrentChanged -= autoScrollToCurrentItemWorker;
                }
            }
        }

        /// <summary>
        /// Sets the value of the AutoScrollToCurrentItemProperty
        /// </summary>
        /// <param name="obj">The dependency-object whichs value should be set</param>
        /// <param name="value">The value which should be assigned to the AutoScrollToCurrentItemProperty</param>
        public static void SetAutoScrollToCurrentItem(DependencyObject obj, bool value)
        {
            obj.SetValue(AutoScrollToCurrentItemProperty, value);
        }

        #endregion Methods
    }

    /// <summary>
    /// Interaction logic for LogFile.xaml
    /// </summary>
    public partial class LogFile : Window
    {
        #region Fields

        private LogFileViewModel _vm;

        #endregion Fields

        #region Constructors

        public LogFile()
        {
            InitializeComponent();

            try
            {
                // Make this window the topmost within the app
                this.Owner = Application.Current.MainWindow;
            }
            catch (Exception e)
            {
                e.ToString();
            }
            this.Closed += LogFile_Closed;
            this.Loaded += LogFile_Loaded;
            this.SizeChanged += new SizeChangedEventHandler(LogFile_SizeChanged);
        }

        #endregion Constructors

        #region Methods

        void LogFile_Closed(object sender, EventArgs e)
        {
            _vm.Stop();

            try
            {
                Application.Current.MainWindow.Activate();
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        void LogFile_Loaded(object sender, RoutedEventArgs e)
        {
            //Adjust View Size for scroll bar
            this.lbLog.Height = logFileWindow.ActualHeight - 50;

            _vm = new LogFileViewModel();

            this.DataContext = _vm;

            _vm.Start();
        }

        void LogFile_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            this.lbLog.Height = logFileWindow.ActualHeight - 50;
        }

        #endregion Methods
    }
}