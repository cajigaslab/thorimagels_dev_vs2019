namespace RealTimeLineChart.ViewModel
{
    using System;
    using System.Collections.Generic;
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

    /// <summary>
    /// Interaction logic for EditPipeDialog.xaml
    /// </summary>
    public partial class EditPipeDialog : Window
    {
        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="EditPipeDialog"/> class.
        /// </summary>
        public EditPipeDialog()
        {
            InitializeComponent();
            this.Loaded += EditPipeDialog_Loaded;
        }

        #endregion Constructors

        #region Methods

        public static void CreateXmlNode(XmlDocument doc, string nodeName, XmlNode source = null)
        {
            XmlNode node = doc.CreateNode(XmlNodeType.Element, nodeName, null);
             if (null == source)
             {
             doc.DocumentElement.AppendChild(node);
             }
             else
             {
             source.AppendChild(node);
             }
        }

        /// <summary>
        /// Handles the Click event of the btnOK control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                this.Close();
            }
                XmlDocument thorsyncSettings = new XmlDocument();
                thorsyncSettings.Load(".\\ThorRealTimeDataSettings.xml");

                if (null != thorsyncSettings)
                {
                    string valueString = vm.RemotePCHostName;

                    XmlNode node = thorsyncSettings.SelectSingleNode("/RealTimeDataSettings/UserSettings/IPCRemoteHostPCName");

                    if (node != null)
                    {
                        SetAttribute(node, thorsyncSettings, "name", valueString);

                        thorsyncSettings.Save(".\\ThorRealTimeDataSettings.xml");
                    }
                    else
                    {
                        CreateXmlNode(thorsyncSettings, "IPCRemoteHostPCName");
                        node = thorsyncSettings.SelectSingleNode("/RealTimeDataSettings/UserSettings/IPCRemoteHostPCName");
                        if (node != null)
                        {
                            SetAttribute(node, thorsyncSettings, "name", valueString);
                            thorsyncSettings.Save(".\\ThorRealTimeDataSettings.xml");

                        }
                    }
                }

            this.Close();
        }

        /// <summary>
        /// Displays the name of the saved host.
        /// </summary>
        private void DisplaySavedHostName()
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;

            if (vm == null)
            {
                return;
            }
            for (int i = 0; i < lbPCName.Items.Count; i++)
            {
                (lbPCName.Items[i] as ListBoxItem).Content = vm._selectRemotePCName[i];
            }
        }

        /// <summary>
        /// Displays the saved ip addr.
        /// </summary>
        private void DisplaySavedIPAddr()
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;

            if (vm == null)
            {
                return;
            }
            for (int i = 0; i < lbPCName.Items.Count; i++)
            {
                (lbPCName.Items[i] as ListBoxItem).Content = vm._selectRemodePCIPAddr[i];
            }
        }

        /// <summary>
        /// Handles the Loaded event of the EditPipeDialog control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        void EditPipeDialog_Loaded(object sender, RoutedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;

            if (vm == null)
            {
                return;
            }

            if (vm.IDMode == 0)
            {
                DisplaySavedHostName();
            }
            else
            {
                DisplaySavedIPAddr();
            }
        }

        /// <summary>
        /// Handles the SelectionChanged event of the IDMode control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="SelectionChangedEventArgs"/> instance containing the event data.</param>
        private void IDMode_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.SaveRemotePCHostNameToXML();
            if (vm.IDMode == 0)
            {
                DisplaySavedHostName();
            }
            else
            {
                DisplaySavedIPAddr();
            }
        }

        /// <summary>
        /// Handles the Click event of the MenuItem control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void MenuItem_Click(object sender, RoutedEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;

            if (vm == null)
            {
                return;
            }
            if (vm.IDMode == 0)
            {
                vm._selectRemotePCName[vm.SelectedRemotePCNameIndex] = vm.RemotePCHostName;
                DisplaySavedHostName();
            }
            else
            {
                vm._selectRemodePCIPAddr[vm.SelectedRemotePCNameIndex] = vm.RemotePCHostName;
                DisplaySavedIPAddr();
            }
        }

        /// <summary>
        /// Handles the PreviewMouseDown event of the RemotePCHostName control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void RemotePCHostName_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            RealTimeLineChartViewModel vm = (RealTimeLineChartViewModel)this.DataContext;
            if (vm == null)
            {
                return;
            }
            vm.SelectedRemotePCNameIndex = lbPCName.Items.IndexOf(sender as ListBoxItem);
            vm.SaveRemotePCHostNameToXML();
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                if (vm.IDMode == 0)
                {
                    if (vm.SelectedRemotePCNameIndex < lbPCName.Items.Count)
                    {
                        vm.RemotePCHostName = vm._selectRemotePCName[vm.SelectedRemotePCNameIndex];
                    }
                }
                else
                {
                    if (vm.SelectedRemotePCNameIndex < lbPCName.Items.Count)
                    {
                        vm.RemotePCHostName = vm._selectRemodePCIPAddr[vm.SelectedRemotePCNameIndex];
                    }
                }
            }
        }

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