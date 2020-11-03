using System;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;
using System.Diagnostics;
using System.Reflection;
using System.Windows.Controls;
using System.Runtime.InteropServices;
using ThorLogging;
using System.Globalization;
using System.Xml;
using System.IO;
using System.Text;
using System.ComponentModel;
using HardwareSetupDll.ViewModel;
using ThorImageInfastructure;

namespace HardwareSetupDll.View
{
    /// <summary>
    /// Interaction logic for HardwareSetupView.xaml
    /// </summary>
    public partial class HardwareSetupView : UserControl
    {
        private XmlDocument _doc;
        private string _hwSettingsFile;
        private double _centerPosition;
        private double _startPosition;
        private double _endPosition;
        private int _iterations;

        BackgroundWorker _backgroundWorker = new BackgroundWorker();

        public HardwareSetupView()
        {
            InitializeComponent();

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");

            //locate the hardware settings file in the Application Settings folder
            _hwSettingsFile = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\ThorImage\\Application Settings\\HardwareSettings.xml";

            if (!File.Exists(_hwSettingsFile))
            {
                return;
            }

            //get the data provider from the xaml and load the document
            XmlDataProvider dataProvider = (XmlDataProvider)this.FindResource("HardwareSettings");

            _doc = new XmlDocument();

            _doc.Load(_hwSettingsFile);

            dataProvider.Document = _doc;

            scanRange.Text = "1";
            iterations.Text = "5";

            zScan.IsEnabled = true;
            stopZScan.IsEnabled = false;
            centerPos.Text = "-6.0";

            // Set up the Background Worker Events
            _backgroundWorker.DoWork += backgroundWorker_DoWork;
            _backgroundWorker.RunWorkerCompleted += backgroundWorker_RunWorkerCompleted;
            _backgroundWorker.ProgressChanged += new ProgressChangedEventHandler(_backgroundWorker_ProgressChanged);
            _backgroundWorker.WorkerSupportsCancellation = true;
        }

        void _backgroundWorker_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void Button_Click_Add(object sender, RoutedEventArgs e)
        {
            string name;
            string wavelength;
            int fluorescence;

            WavelengthInputDialog dlg = new WavelengthInputDialog();

            bool? ret = dlg.ShowDialog();

            if (ret == false)
            {
                return;
            }

            XmlNodeList nodeList = _doc.GetElementsByTagName("Wavelength");

            foreach (XmlNode n in nodeList)
            {
                if (n.Attributes["name"].Value.Equals(dlg.WavelengthName))
                {
                    StringBuilder sb = new StringBuilder();

                    sb.AppendFormat("Wavelength name {0} already exists!", dlg.WavelengthName);
                    MessageBox.Show(sb.ToString());
                    //error name already exists
                    return;
                }
            }

            name = dlg.WavelengthName;
            wavelength = dlg.WavelengthValue;

            if (dlg.Fluorescence == WavelengthInputDialog.FluorEnum.Pos1)
            {
                fluorescence = 1;
            }
            else
            {
                fluorescence = 0;
            }

            //create a new XML tag for the wavelength settings
            XmlElement newElement = _doc.CreateElement("Wavelength");

            XmlAttribute nameAttribute = _doc.CreateAttribute("name");
            XmlAttribute wavelengthAttribute = _doc.CreateAttribute("wavelength");
            XmlAttribute exAttribute = _doc.CreateAttribute("ex");
            XmlAttribute emAttribute = _doc.CreateAttribute("em");
            XmlAttribute dicAttribute = _doc.CreateAttribute("dic");
            XmlAttribute fluorAttribute = _doc.CreateAttribute("fluor");
            XmlAttribute colorAttribute = _doc.CreateAttribute("color");
            XmlAttribute blackPoint = _doc.CreateAttribute("bp");
            XmlAttribute whitePoint = _doc.CreateAttribute("wp");

            nameAttribute.Value = name;
            wavelengthAttribute.Value = wavelength;
            exAttribute.Value = ((HardwareSetupViewModel)DataContext).Excitation.ToString().Remove(0,3) ;
            emAttribute.Value = ((HardwareSetupViewModel)DataContext).Emission.ToString().Remove(0, 3); 
            dicAttribute.Value = ((HardwareSetupViewModel)DataContext).Dichroic.ToString().Remove(0, 3);
            fluorAttribute.Value = fluorescence.ToString();
            colorAttribute.Value = dlg.FillColor.ToString();
            int bp = 0;
            int wp = 255;
            blackPoint.Value = bp.ToString();
            whitePoint.Value = wp.ToString();
            newElement.Attributes.Append(nameAttribute);
            newElement.Attributes.Append(wavelengthAttribute);
            newElement.Attributes.Append(exAttribute);
            newElement.Attributes.Append(emAttribute);
            newElement.Attributes.Append(dicAttribute);
            newElement.Attributes.Append(fluorAttribute);
            newElement.Attributes.Append(colorAttribute);
            newElement.Attributes.Append(blackPoint);
            newElement.Attributes.Append(whitePoint);

            _doc.DocumentElement.AppendChild(newElement);

            _doc.Save(_hwSettingsFile);
        }

        private void Button_Click_Edit(object sender, RoutedEventArgs e)
        {
            if (wavelengthListView.SelectedIndex == -1)
            {
                return;
            }

            XmlNodeList ndList = _doc.GetElementsByTagName("Wavelength");

            XmlAttributeCollection xmlAtt = ndList[wavelengthListView.SelectedIndex].Attributes;
            string name = xmlAtt[0].Value;
            string wavelength = xmlAtt[1].Value;
            int fluorescence = Convert.ToInt32(xmlAtt[5].Value);
            string hexColor = xmlAtt[6].Value;

            int red = int.Parse(hexColor.Substring(3, 2), NumberStyles.AllowHexSpecifier);
            int green = int.Parse(hexColor.Substring(5, 2), NumberStyles.AllowHexSpecifier);
            int blue = int.Parse(hexColor.Substring(7, 2), NumberStyles.AllowHexSpecifier);

            Color fillColor = Color.FromRgb((byte)red, (byte)green, (byte)blue);
            
            WavelengthInputDialog dlg = new WavelengthInputDialog();

            dlg.WavelengthName = name;
            dlg.WavelengthValue = wavelength;
            dlg.FillColor = fillColor;              

            if (fluorescence == 1)
            {
                dlg.Fluorescence = WavelengthInputDialog.FluorEnum.Pos1;
            }
            else
            {
                dlg.Fluorescence = WavelengthInputDialog.FluorEnum.Pos2;
            }

            bool? ret = dlg.ShowDialog();

            if (ret == false)
            {
                return;
            }

            //updating the edited values to the xml file
            xmlAtt[0].Value = dlg.WavelengthName;
            xmlAtt[1].Value = dlg.WavelengthValue;

            if (dlg.Fluorescence == WavelengthInputDialog.FluorEnum.Pos1)
            {
                fluorescence = 1;
            }
            else
            {
                fluorescence = 0;
            }
            xmlAtt[5].Value = fluorescence.ToString();
            xmlAtt[6].Value = dlg.FillColor.ToString();            

            _doc.Save(_hwSettingsFile);

        }  

        private void Button_Click_Delete(object sender, RoutedEventArgs e)
        {
            if (wavelengthListView.SelectedIndex == -1)
            {
                return;
            }

            XmlNodeList ndList = _doc.GetElementsByTagName("Wavelength");

            ndList[wavelengthListView.SelectedIndex].ParentNode.RemoveChild(ndList[wavelengthListView.SelectedIndex]);

            _doc.Save(_hwSettingsFile);
        }

        private void CommandBinding_Executed(object sender, System.Windows.Input.ExecutedRoutedEventArgs e)
        {

        }

        private void zScan_Click(object sender, RoutedEventArgs e)
        {
            if (_backgroundWorker.IsBusy)
            {
                return;
            }
            zScan.IsEnabled = false;
            stopZScan.IsEnabled = true;

            _centerPosition = Convert.ToDouble(centerPos.Text);
            _startPosition = _centerPosition - Convert.ToDouble(scanRange.Text) / 2.0;
            _endPosition = _centerPosition + Convert.ToDouble(scanRange.Text) / 2.0;
            _iterations = Convert.ToInt32(iterations.Text);
            _backgroundWorker.RunWorkerAsync();
        }

        // Worker Method
        private void backgroundWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            for (int i = 0; i < _iterations; i++)
            {
                //must call set position directly because code is in a background worker which prevents the access to the viewmodel properties.
                SetZPosition(_startPosition);

                System.Threading.Thread.Sleep(1000);

                SetZPosition(_endPosition);

                System.Threading.Thread.Sleep(1000);

                if (_backgroundWorker.CancellationPending)
                {
                    return;
                }
            }
        }

        // Completed Method
        private void backgroundWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            ((HardwareSetupViewModel)DataContext).ZPosition = _centerPosition;
            zScan.IsEnabled = true;
            stopZScan.IsEnabled = false;  
        }

        private void stopZScan_Click(object sender, RoutedEventArgs e)
        {
            _backgroundWorker.CancelAsync();
        }


       [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetZPosition")]
        private extern static bool SetZPosition(double pos);
         
    }
    public class EnumBooleanConverter : IValueConverter
    {

        #region IValueConverter Members



        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {

            var ParameterString = parameter as string;

            if (ParameterString == null)

                return DependencyProperty.UnsetValue;



            if (Enum.IsDefined(value.GetType(), value) == false)

                return DependencyProperty.UnsetValue;



            object paramvalue = Enum.Parse(value.GetType(), ParameterString);

            if (paramvalue.Equals(value))
            {
                return true;
            }
            else
            {
                return false;
            }
        }



        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {

            var ParameterString = parameter as string;

            if (ParameterString == null)

                return DependencyProperty.UnsetValue;



            return Enum.Parse(targetType, ParameterString);

        }



        #endregion

    }

    
}
