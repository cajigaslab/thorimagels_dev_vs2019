namespace ScriptManagerDll.View
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Threading;
    using System.Xml;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Win32;

    using ScriptManagerDll.View;
    using ScriptManagerDll.ViewModel;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for MasterView.xaml
    /// </summary>
    public partial class MasterView : UserControl
    {
        #region Fields

        const string CMD_ATTRIB_PANEL = "panel";
        const string CMD_TAG = "/Command";
        const string PANEL_CLASS_NAME = "UserControl1";
        const string PANEL_PROPERTY_NAME = "SettingsDocument";

        private bool _dropPerfomed = false;
        private int _previousSelectedIndex = -1;
        private Type _previousUCType = null;

        #endregion Fields

        #region Constructors

        public MasterView()
        {
            InitializeComponent();

            this.Loaded += new RoutedEventHandler(MasterView_Loaded);
            this.Unloaded += new RoutedEventHandler(MasterView_Unloaded);
        }

        #endregion Constructors

        #region Methods

        void AddPanel()
        {
            XmlDocument doc = new XmlDocument();

            doc.LoadXml(((ScriptManagerViewModel)this.DataContext).CollectionScript[this.lbCommandList2.SelectedIndex].Paramters);

            XmlNode node = doc.SelectSingleNode(CMD_TAG);

            if (null != node)
            {
                //find the name of the panel to load
                node = node.Attributes.GetNamedItem(CMD_ATTRIB_PANEL);

                if (null != node)
                {
                    string fileName = node.Value;
                    try
                    {
                        //dynamically load the panel
                        System.Reflection.Assembly pluginAssembly = System.Reflection.Assembly.LoadFile(Path.GetFullPath(fileName));

                        string moduleName = Path.GetFileNameWithoutExtension(fileName);
                        Type ucType = pluginAssembly.GetType(moduleName + "." + PANEL_CLASS_NAME);
                        if (ucType == null)
                        {
                            ucType = pluginAssembly.GetType(moduleName + "." + moduleName + "Control");
                        }

                        object uc = Activator.CreateInstance(ucType);

                        PropertyInfo propInfo = ucType.GetProperty(PANEL_PROPERTY_NAME);
                        if (propInfo != null)
                        {
                            propInfo.SetValue(uc, doc, null);
                        }

                        //add the panel
                        pluginPanel.Children.Add((UserControl)uc);

                        //store the previous user control type and index
                        _previousUCType = ucType;
                        _previousSelectedIndex = this.lbCommandList2.SelectedIndex;
                    }
                    catch (Exception ex)
                    {
                        string str = ex.Message;
                    }
                }
            }
        }

        private void btnDelete_Click(object sender, RoutedEventArgs e)
        {
            _dropPerfomed = true;
        }

        private void btnPaste_Click(object sender, RoutedEventArgs e)
        {
            _dropPerfomed = true;
            PersistPanel(_previousSelectedIndex);
        }

        private void btnRun_Click(object sender, RoutedEventArgs e)
        {
            if (lbCommandList2.SelectedIndex >= 0)
            {
                PersistPanel(lbCommandList2.SelectedIndex);
            }
        }

        private void btnSave_Click(object sender, RoutedEventArgs e)
        {
            if (lbCommandList2.SelectedIndex >= 0)
            {
                PersistPanel(lbCommandList2.SelectedIndex);
            }
        }

        private void lbCommandList2_LostFocus(object sender, RoutedEventArgs e)
        {
            PersistPanel(lbCommandList2.SelectedIndex);
        }

        private void lbCommandList2_PreviewDrop(object sender, DragEventArgs e)
        {
            PersistPanel(_previousSelectedIndex);
            _dropPerfomed = true;
        }

        private void lbCommandList2_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (this.lbCommandList2.SelectedIndex < 0)
            {
                pluginPanel.Children.Clear();
                return;
            }

            //if a previous selection exists and its valid. persist the item
            if ((false == _dropPerfomed) && (_previousSelectedIndex >= 0) && (_previousSelectedIndex < this.lbCommandList2.Items.Count))
              //  if ( (_previousSelectedIndex >= 0) && (_previousSelectedIndex < this.lbCommandList2.Items.Count))
            {
                PersistPanel(_previousSelectedIndex);
            }

            //remove the existing panel and add in the new panel
                pluginPanel.Children.Clear();
                AddPanel();

                _dropPerfomed = false;
        }

        private void lbCommandList_GotFocus(object sender, RoutedEventArgs e)
        {
            PersistPanel(lbCommandList2.SelectedIndex);
        }

        void MasterView_Loaded(object sender, RoutedEventArgs e)
        {
            if (null == ((MasterView)sender).DataContext)
            {
                return;
            }

            ((ScriptManagerViewModel)this.DataContext).EnableHandlers();

            ((ScriptManagerViewModel)this.DataContext).CommandListPath = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "/CommandList.xml";
        }

        void MasterView_Unloaded(object sender, RoutedEventArgs e)
        {
            if (null == ((MasterView)sender).DataContext)
            {
                return;
            }
            ((ScriptManagerViewModel)this.DataContext).ReleaseHandlers();
        }

        private void PersistPanel(int index)
        {
            //if there is a panel loaded
            if (pluginPanel.Children.Count > 0)
            {
                //load and find the command section for the previously selected item
                XmlDocument doc = new XmlDocument();

                doc.LoadXml(((ScriptManagerViewModel)this.DataContext).CollectionScript[index].Paramters);

                XmlNode node = doc.SelectSingleNode(CMD_TAG);

                if (null != node)
                {
                    node = node.Attributes.GetNamedItem(CMD_ATTRIB_PANEL);

                    if (null != node)
                    {
                        string fileName = node.Value;
                        try
                        {
                            //extract the settings document from the existing control
                            PropertyInfo propInfo = _previousUCType.GetProperty(PANEL_PROPERTY_NAME);

                            XmlDocument propDoc = (XmlDocument)propInfo.GetValue(pluginPanel.Children[0], null);

                            if(null != propDoc)
                            {
                                StringWriter sw = new StringWriter();
                                XmlTextWriter tx = new XmlTextWriter(sw);
                                propDoc.WriteTo(tx);
                                //update the previously selected item with the panel document
                                ((ScriptManagerViewModel)this.DataContext).CollectionScript[index].Paramters = sw.ToString();
                            }
                        }
                        catch (Exception ex)
                        {
                            string str = ex.Message;
                        }
                    }
                }
            }
        }

        private void pluginPanel_LostFocus(object sender, RoutedEventArgs e)
        {
            PersistPanel(lbCommandList2.SelectedIndex);
        }

        #endregion Methods
    }
}