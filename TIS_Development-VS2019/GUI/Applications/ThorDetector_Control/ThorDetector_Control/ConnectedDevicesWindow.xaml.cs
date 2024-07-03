/// <summary>
/// The ThorDetector_Control namespace.
/// </summary>
namespace ThorDetector_Control
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
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
    using System.Windows.Shapes;
    using System.Xml;

    /// <summary>
    /// Interaction logic for ConnectedDevicesWindow.xaml
    /// </summary>
    public partial class ConnectedDevicesWindow : Window
    {
        #region Fields

        /// <summary>
        /// The s n_ lenth
        /// </summary>
        private const int SN_LENTH = 6;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="ConnectedDevicesWindow"/> class.
        /// </summary>
        public ConnectedDevicesWindow()
        {
            InitializeComponent();
            this.Loaded += ConnectedDevicesWindow_Loaded;
        }

        #endregion Constructors

        #region Methods

        /// <summary>
        /// Handles the Loaded event of the ConnectedDevicesWindow control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void ConnectedDevicesWindow_Loaded(object sender, RoutedEventArgs e)
        {
            string settingsFile = Environment.CurrentDirectory + "\\ThorDetectorSettings.xml";
            XmlDocument settingsXML = new XmlDocument();
            settingsXML.Load(settingsFile);
            XmlNodeList ndList = settingsXML.SelectNodes("/ThorDetectorSettings/Detector1");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                GetAttribute(ndList[0], settingsXML, "serialNumber", ref str);
                tbSNDetector1.Text = str;
            }

            ndList = settingsXML.SelectNodes("/ThorDetectorSettings/Detector2");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                GetAttribute(ndList[0], settingsXML, "serialNumber", ref str);
                tbSNDetector2.Text = str;
            }

            ndList = settingsXML.SelectNodes("/ThorDetectorSettings/Detector3");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                GetAttribute(ndList[0], settingsXML, "serialNumber", ref str);
                tbSNDetector3.Text = str;
            }

            ndList = settingsXML.SelectNodes("/ThorDetectorSettings/Detector4");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                GetAttribute(ndList[0], settingsXML, "serialNumber", ref str);
                tbSNDetector4.Text = str;
            }

            ndList = settingsXML.SelectNodes("/ThorDetectorSettings/Detector5");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                GetAttribute(ndList[0], settingsXML, "serialNumber", ref str);
                tbSNDetector5.Text = str;
            }

            ndList = settingsXML.SelectNodes("/ThorDetectorSettings/Detector6");
            if (ndList.Count > 0)
            {
                string str = string.Empty;
                GetAttribute(ndList[0], settingsXML, "serialNumber", ref str);
                tbSNDetector6.Text = str;
            }
        }

        /// <summary>
        /// Gets the attribute value from the input node and document.
        /// If the attribute does not exist return false.
        /// </summary>
        /// <param name="node">The node.</param>
        /// <param name="doc">The document.</param>
        /// <param name="attrName">Name of the attribute.</param>
        /// <param name="attrValue">The attribute value.</param>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        private bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
        {
            bool ret;

            if (null == node.Attributes.GetNamedItem(attrName))
            {
                ret = false;
            }
            else
            {
                attrValue = node.Attributes[attrName].Value;
                ret = true;
            }

            return ret;
        }

        /// <summary>
        /// Handles the Click event of the OK control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void OK_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                const int MAX_DETECTOR_NUM = 6;
                string[] deviceName = { "Detector1", "Detector2", "Detector3", "Detector4", "Detector5", "Detector6" };
                string[] serialNumber = { tbSNDetector1.Text, tbSNDetector2.Text, tbSNDetector3.Text, tbSNDetector4.Text, tbSNDetector5.Text, tbSNDetector6.Text };
                string settingsFile = Environment.CurrentDirectory + "\\ThorDetectorSettings.xml";
                XmlDocument settingsXML = new XmlDocument();
                settingsXML.Load(settingsFile);
                XmlNodeList ndList = null;
                for (int i = 0; i < MAX_DETECTOR_NUM; i++)
                {
                    if ("NA" != serialNumber[i] && string.Empty != serialNumber[i])
                    {
                        for (int j = 0; j < MAX_DETECTOR_NUM; j++)
                        {
                            if (i == j) continue;
                            if (serialNumber[i] == serialNumber[j])
                            {
                                MessageBox.Show("Do not repeat serial numbers for different Detectors.");
                                return;
                            }
                        }
                    }
                    ndList = settingsXML.SelectNodes("/ThorDetectorSettings/" + deviceName[i]);
                    if (ndList.Count > 0)
                    {
                        SetAttribute(ndList[0], settingsXML, "serialNumber", serialNumber[i]);
                    }
                }

                settingsXML.Save(settingsFile);
            }
            catch (System.IO.FileNotFoundException ex)
            {
                MessageBox.Show(ex.Message, "ThorDetector settings file does not exist");
                this.Close();
            }
            this.Close();
        }

        /// <summary>
        /// assign the attribute value to the input node and document
        /// if the attribute does not exist add it to the document
        /// </summary>
        /// <param name="node">The node.</param>
        /// <param name="doc">The document.</param>
        /// <param name="attrName">Name of the attribute.</param>
        /// <param name="attValue">The att value.</param>
        private void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attValue)
        {
            XmlNode tempNode = node.Attributes[attrName];

            if (null == tempNode)
            {
                XmlAttribute attr = doc.CreateAttribute(attrName);

                attr.Value = attValue;

                node.Attributes.Append(attr);
            }
            else
            {
                node.Attributes[attrName].Value = attValue;
            }
        }

        #endregion Methods
    }
}