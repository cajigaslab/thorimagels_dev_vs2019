namespace RealTimeLineChart.View
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
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

    /// <summary>
    /// Interaction logic for EditLinesDialog.xaml
    /// </summary>
    public partial class EditLinesDialog : Window
    {
        #region Fields

        //ObservableCollection<RadioButton> _aiTrigRBtns;
        ObservableCollection<Label> _lineLbls;
        ObservableCollection<RadioButton> _lineRBtns;
        ObservableCollection<RadioButton> _stimRBtns;
        string[] _toolTips;

        #endregion Fields

        #region Constructors

        public EditLinesDialog()
        {
            InitializeComponent();
            this.Loaded += new RoutedEventHandler(EditLinesDialog_Loaded);
            this.Unloaded += new RoutedEventHandler(EditLinesDialog_Unloaded);
            _lineLbls = new ObservableCollection<Label>();
            _lineRBtns = new ObservableCollection<RadioButton>();
            _stimRBtns = new ObservableCollection<RadioButton>();
            //_aiTrigRBtns = new ObservableCollection<RadioButton>();
            _toolTips = new string[] { "Select single stimulus line. ", "Click above stimulus check box to enable selection. ", "Change line color setting. ", "Select single analog trigger line. ", "Select above combo box to be analog trigger. " };
        }

        #endregion Constructors

        #region Events

        public event Action<int, int> LinesDialogClosed;

        #endregion Events

        #region Properties

        public XmlDocument Settings
        {
            get;
            set;
        }

        public string SettingsFileName
        {
            get;
            set;
        }

        public Point windowCorner
        {
            get;
            set;
        }

        public RealTimeLineChartViewModel _realVM
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            bool ret = true;
            string strTemp = string.Empty;
            ObservableCollection<string> aliasNames = new ObservableCollection<string>();

            //Save to XML & update ViewModel:
            XmlNodeList nodeList = Settings.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]");

            if (nodeList.Count <= 0)
                return;

            XmlNodeList channelList = nodeList[0].SelectNodes("DataChannel");

            if (channelList.Count <= 0)
                return;

            //verify line names:
            for (int i = 0; i < channelList.Count; i++)
            {
                string aliasName = ((TextBox)((StackPanel)lbLines.Items[i]).Children[2]).Text;
                string error = string.Empty;
                if (0 == aliasName.Length)
                {
                    ret = false;
                    MessageBox.Show("Line Name cannot be empty at # " + (i + 1).ToString("00"), "Note", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                    return;
                }
                if (RealTimeLineChartViewModel.ContainsInvalidCharacters(aliasName, ref error))
                {
                    ret = false;
                    MessageBox.Show("Line names cannot contain special characters: " + error, "Note", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                    return;
                }
                if (aliasNames.Contains(aliasName))
                {
                    ret = false;
                    MessageBox.Show("Line names must be unique.", "Note", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                    return;
                }
                for (int j = 0; j < aliasNames.Count; j++)
                {
                    if (aliasNames[j].Contains(aliasName) || aliasName.Contains(aliasNames[j]))
                    {
                        ret = false;
                        MessageBox.Show("One line name cannot be a subset of another: " + aliasName, "Note", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                        return;
                    }
                }
                aliasNames.Add(aliasName);
            }

            //local params & default values:
            Color lColor = new Color();
            RealTimeLineChartViewModel.SetAttribute(nodeList[0], Settings, "hwTrigType", "0");

            //go through all line attributes:
            _realVM.IsCounterLinePlotEnabled.Clear();
            for (int i = 0; i < channelList.Count; i++)
            {
                RealTimeLineChartViewModel.SetAttribute(channelList[i], Settings, "alias", aliasNames[i].ToString());
                RealTimeLineChartViewModel.SetAttribute(channelList[i], Settings, "enable", ((true == ((CheckBox)((StackPanel)lbLines.Items[i]).Children[0]).IsChecked) ? "1" : "0"));

                switch (channelList[i].Attributes["group"].Value)
                {
                    case "/AI":
                    case "/DI":
                    default:
                        //if (i < channelList.Count - 1)   //(((StackPanel)lbLines.Items[i]).Children.Count > 2)
                        {
                            //No need to configure clock, leave it for future:
                            //channelList[i].Attributes["sample"].Value = ((true == (RadioButton)((StackPanel)lbLines.Items[i]).Children[2]).IsChecked) ? "1" : "0";
                            //---    Analog trigger line has to be the first on the scan list    ---//
                            //if (i < _aiTrigRBtns.Count) //for all AI channels:
                            //{
                            //    channelList[i].Attributes["aiTrigger"].Value = ((true == _aiTrigRBtns.ElementAt<RadioButton>(i).IsEnabled) && (true == _aiTrigRBtns.ElementAt<RadioButton>(i).IsChecked)) ? "1" : "0";
                            //    if (0 == channelList[i].Attributes["aiTrigger"].Value.CompareTo("1"))
                            //    {
                            //        if (0 == channelList[i].Attributes["enable"].Value.CompareTo("0"))
                            //        {
                            //            ret = false;
                            //            MessageBox.Show("analog trigger line must be selected for acquire.", "Note", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                            //            break;
                            //        }
                            //        nodeList[0].Attributes.GetNamedItem("hwTrigType").Value = "1";
                            //    }
                            //}
                            //channelList[i].Attributes["Stimulus"].Value = ((true == ((RadioButton)((StackPanel)lbLines.Items[i]).Children[4]).IsChecked) && (true == ((RadioButton)((StackPanel)lbLines.Items[i]).Children[4]).IsEnabled)) ? "1" : "0";
                            RealTimeLineChartViewModel.SetAttribute(channelList[i], Settings, "Stimulus", (((true == _stimRBtns.ElementAt<RadioButton>(i).IsEnabled) && (true == _stimRBtns.ElementAt<RadioButton>(i).IsChecked)) ? "1" : "0"));
                        }
                        break;
                    case "/CI":
                        {   //Counter line:
                            _realVM.IsCounterLinePlotEnabled.Add((true == ((CheckBox)((StackPanel)lbLines.Items[i]).Children[4]).IsChecked) ? true : false);
                        }
                        break;
                }

                //Save color settings:
                lColor = ((System.Windows.Media.SolidColorBrush)_lineLbls.ElementAt<Label>(i).Background).Color;
                RealTimeLineChartViewModel.SetAttribute(channelList[i], Settings, "red", lColor.R.ToString());
                RealTimeLineChartViewModel.SetAttribute(channelList[i], Settings, "green", lColor.G.ToString());
                RealTimeLineChartViewModel.SetAttribute(channelList[i], Settings, "blue", lColor.B.ToString());
            }
            //First line has to be enabled for analog trigger:
            if (1 == cbAiTrigMode.SelectedIndex)
            {
                if (RealTimeLineChartViewModel.GetAttribute(channelList[0], Settings, "enable", ref strTemp) && (0 == strTemp.CompareTo("0")))
                {
                    ret = false;
                    MessageBox.Show("Analog 01 must be selected for analog trigger.", "Note", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                    return;
                }
                RealTimeLineChartViewModel.SetAttribute(nodeList[0], Settings, "hwTrigType", "1");
            }

            //update VM properties:
            if (ret)
            {
                _realVM.IsStimulusEnabled = (true == chkStim.IsChecked) ? true : false;
                _realVM.IsAiTriggerEnabled = (1 == cbAiTrigMode.SelectedIndex) ? true : false;
            }

            //Validate user input:
            XmlNodeList totalList = nodeList[0].SelectNodes("DataChannel[@group='/DI' and @enable='1']");

            RealTimeLineChartViewModel.SetAttribute(nodeList[0], Settings, "totalDI", totalList.Count.ToString());

            totalList = nodeList[0].SelectNodes("DataChannel[@group='/AI' and @enable='1']");

            RealTimeLineChartViewModel.SetAttribute(nodeList[0], Settings, "totalAI", totalList.Count.ToString());

            XmlNodeList samplerateList;
            if (0 == _realVM.BoardType.CompareTo("NI6363"))
            {
                samplerateList = nodeList[0].SelectNodes("SampleRate[@name='High 1MHz' and @enable='1']");
                if ((samplerateList.Count > 0) && (totalList.Count > 2))
                {
                    ret = false;
                    MessageBox.Show("Too many analog channels for high sample rate.", "Note", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                    return;
                }
            }
            else if (0 == _realVM.BoardType.CompareTo("NI6321"))
            {
                samplerateList = nodeList[0].SelectNodes("SampleRate[@name='High 250kHz' and @enable='1']");
                if ((samplerateList.Count > 0) && (totalList.Count > 1))
                {
                    ret = false;
                    MessageBox.Show("Too many analog channels for high sample rate.", "Note", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                    return;
                }
                samplerateList = nodeList[0].SelectNodes("SampleRate[@name='Medium 100kHz' and @enable='1']");
                if ((samplerateList.Count > 0) && (totalList.Count > 2))
                {
                    ret = false;
                    MessageBox.Show("Too many analog channels for medium sample rate.", "Note", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                    return;
                }
            }

            totalList = nodeList[0].SelectNodes("DataChannel[@group='/CI' and @enable='1']");
            if (totalList.Count == 0)
            {

            }
            this.DialogResult = (true == ret) ? true : false;
            this.Close();
        }

        //private void cbAiTrigMode_SelectionChanged(object sender, SelectionChangedEventArgs e)
        //{
        //    if (1 == cbAiTrigMode.SelectedIndex)
        //    {
        //        for (int i = 0; i < _aiTrigRBtns.Count; i++)
        //        {
        //            _aiTrigRBtns.ElementAt<RadioButton>(i).IsEnabled = true;
        //            _aiTrigRBtns.ElementAt<RadioButton>(i).Visibility = Visibility.Visible;
        //            ToolTipService.SetToolTip(_aiTrigRBtns.ElementAt<RadioButton>(i), _toolTips[3]);
        //        }
        //    }
        //    else
        //    {
        //        for (int i = 0; i < _aiTrigRBtns.Count; i++)
        //        {
        //            _aiTrigRBtns.ElementAt<RadioButton>(i).IsChecked = false;
        //            _aiTrigRBtns.ElementAt<RadioButton>(i).IsEnabled = false;
        //            _aiTrigRBtns.ElementAt<RadioButton>(i).Visibility = Visibility.Hidden;
        //            ToolTipService.SetToolTip(_aiTrigRBtns.ElementAt<RadioButton>(i), _toolTips[4]);
        //        }
        //    }
        //}
        private void chkStim_Click(object sender, RoutedEventArgs e)
        {
            if (true == chkStim.IsChecked)
            {
                for (int i = 0; i < _stimRBtns.Count; i++)
                {
                    _stimRBtns.ElementAt<RadioButton>(i).IsEnabled = true;
                    ToolTipService.SetToolTip(_stimRBtns.ElementAt<RadioButton>(i), _toolTips[0]);
                }
            }
            else
            {
                for (int i = 0; i < _stimRBtns.Count; i++)
                {
                    _stimRBtns.ElementAt<RadioButton>(i).IsChecked = false;
                    _stimRBtns.ElementAt<RadioButton>(i).IsEnabled = false;
                    ToolTipService.SetToolTip(_stimRBtns.ElementAt<RadioButton>(i), _toolTips[1]);
                }
            }
        }

        private void ColorDelta_Changed(object sender, MouseWheelEventArgs e)
        {
            if (0 == ((Slider)sender).Name.CompareTo("sliderR"))
            {
                sliderR.Value += (int)(e.Delta / 120);
            }
            if (0 == ((Slider)sender).Name.CompareTo("sliderG"))
            {
                sliderG.Value += (int)(e.Delta / 120);
            }
            if (0 == ((Slider)sender).Name.CompareTo("sliderB"))
            {
                sliderB.Value += (int)(e.Delta / 120);
            }

            for (int i = 0; i < _lineRBtns.Count; i++)
            {
                if (true == _lineRBtns.ElementAt<RadioButton>(i).IsChecked)
                {
                    int red = (int)sliderR.Value;
                    int green = (int)sliderG.Value;
                    int blue = (int)sliderB.Value;
                    Color nColor = Color.FromRgb(Convert.ToByte(red), Convert.ToByte(green), Convert.ToByte(blue));
                    _lineLbls.ElementAt<Label>(i).Background = new SolidColorBrush(nColor);
                }
            }
        }

        private void Color_Changed(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            for (int i = 0; i < _lineRBtns.Count; i++)
            {
                if (true == _lineRBtns.ElementAt<RadioButton>(i).IsChecked)
                {
                    int red = (int)sliderR.Value;
                    int green = (int)sliderG.Value;
                    int blue = (int)sliderB.Value;
                    Color nColor = Color.FromRgb(Convert.ToByte(red), Convert.ToByte(green), Convert.ToByte(blue));
                    _lineLbls.ElementAt<Label>(i).Background = new SolidColorBrush(nColor);
                }
            }
        }

        void EditLinesDialog_Loaded(object sender, RoutedEventArgs e)
        {
            //Set window top-left corner:
            var workingArea = System.Windows.SystemParameters.WorkArea;
            int left = Convert.ToInt32(windowCorner.X.ToString());
            int top = Convert.ToInt32(windowCorner.Y.ToString());
            this.Left = left;
            this.Top = top;

            //Trigger type visibility:
            cbAiTrigMode.Visibility = (0 == _realVM.BoardType.CompareTo("NI6321")) ? Visibility.Hidden : Visibility.Visible;

            //Load from XML or ViewModel:
            XmlNodeList nodeList = Settings.SelectNodes("/RealTimeDataSettings/DaqDevices/AcquireBoard[@active=1]");

            if (nodeList.Count <= 0)
                return;

            nodeList = nodeList[0].SelectNodes("DataChannel");

            if (nodeList.Count <= 0)
                return;

            int n_ai = 1, n_di = 1, n_ci = 1;
            foreach (XmlNode node in nodeList)
            {
                StackPanel spanel = new StackPanel();
                spanel.Orientation = System.Windows.Controls.Orientation.Horizontal;
                spanel.HorizontalAlignment = System.Windows.HorizontalAlignment.Left;

                //overall width: (account for all lateral items' width)
                spanel.Width = 370;
                //int col2Width = 100;
                CheckBox box = new CheckBox();
                box.IsChecked = (1 == Convert.ToInt32(node.Attributes["enable"].Value.ToString())) ? true : false;
                box.Width = 20;
                box.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Left;
                box.VerticalAlignment = System.Windows.VerticalAlignment.Center;

                Label lbl = new Label();
                lbl.Width = 70;
                lbl.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Left;
                lbl.VerticalAlignment = System.Windows.VerticalAlignment.Center;
                if (true == node.Attributes.GetNamedItem("group").Value.Equals("/AI"))
                {
                    string content = "Analog " + n_ai.ToString("D2");
                    lbl.Content = content;
                    n_ai++;
                }
                if (true == node.Attributes.GetNamedItem("group").Value.Equals("/DI"))
                {
                    string content = "Digital " + n_di.ToString("D2");
                    lbl.Content = content;
                    n_di++;
                }
                if (true == node.Attributes.GetNamedItem("group").Value.Equals("/CI"))
                {
                    string content = "Counter " + n_ci.ToString("D2");
                    lbl.Content = content;
                    n_ci++;
                }
                TextBox tbox = new TextBox();
                tbox.Text = node.Attributes["alias"].Value.ToString();
                tbox.Width = 100;
                tbox.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Left;
                tbox.Margin = new Thickness() { Left = 3, Right = 3, Top = 3, Bottom = 3 };

                Label lblEmty = new Label();
                lblEmty.Width = 20;

                spanel.Children.Add(box);
                spanel.Children.Add(lbl);
                spanel.Children.Add(tbox);
                spanel.Children.Add(lblEmty);

                //Not for counters:
                if (false == node.Attributes.GetNamedItem("group").Value.Equals("/CI"))
                {
                    /*Leave out sample clock configuration for future:
                    RadioButton rb2 = new RadioButton();
                    rb2.Width = 50;
                    rb2.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Center;
                    rb2.VerticalAlignment = System.Windows.VerticalAlignment.Center;
                    rb2.Margin = new Thickness() { Left = 3, Right = 3, Top = 3, Bottom = 3 };
                    rb2.GroupName = "sample";
                    rb2.IsChecked = (1 == Convert.ToInt32(node.Attributes["sample"].Value.ToString())) ? true : false;*/

                    /*if (true == node.Attributes.GetNamedItem("group").Value.Equals("/AI"))
                    {
                        //---   Analog trigger line has to be the first on the scan list    ---//
                        RadioButton rb2 = new RadioButton();
                        rb2.Width = col2Width;
                        rb2.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Center;
                        rb2.HorizontalAlignment = System.Windows.HorizontalAlignment.Center;
                        rb2.VerticalAlignment = System.Windows.VerticalAlignment.Center;
                        rb2.GroupName = "aiTrigger";
                        rb2.IsChecked = (1 == Convert.ToInt32(node.Attributes["aiTrigger"].Value.ToString())) ? true : false;
                        rb2.IsEnabled = _realVM.IsAiTriggerEnabled;
                        string str2 = (true == rb2.IsEnabled) ? _toolTips[3] : _toolTips[4];
                        ToolTipService.SetToolTip(rb2, str2);
                        ToolTipService.SetShowOnDisabled(rb2, true);
                        _aiTrigRBtns.Add(rb2);
                        spanel.Children.Add(_aiTrigRBtns.Last<RadioButton>());
                    }
                    else if (true == node.Attributes.GetNamedItem("group").Value.Equals("/DI"))
                    {
                        Label lb2 = new Label();
                        lb2.Width = col2Width;
                        lb2.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Center;
                        lb2.HorizontalAlignment = System.Windows.HorizontalAlignment.Center;
                        lb2.VerticalAlignment = System.Windows.VerticalAlignment.Center;
                        spanel.Children.Add(lb2);
                    }*/
                    RadioButton rb3 = new RadioButton();
                    rb3.Width = 70;
                    rb3.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Center;
                    rb3.HorizontalAlignment = System.Windows.HorizontalAlignment.Center;
                    rb3.VerticalAlignment = System.Windows.VerticalAlignment.Center;
                    //rb3.Margin = new Thickness() { Left = 3, Right = 3, Top = 3, Bottom = 3 };
                    rb3.GroupName = "Stimulus";
                    rb3.IsChecked = (1 == Convert.ToInt32(node.Attributes["Stimulus"].Value.ToString())) ? true : false;
                    rb3.IsEnabled = _realVM.IsStimulusEnabled;
                    string str3 = (true == rb3.IsEnabled) ? _toolTips[0] : _toolTips[1];
                    ToolTipService.SetToolTip(rb3, str3);
                    ToolTipService.SetShowOnDisabled(rb3, true);
                    _stimRBtns.Add(rb3);
                    spanel.Children.Add(_stimRBtns.Last<RadioButton>());
                }
                else
                {
                    //Label lb2 = new Label();
                    //lb2.Width = col2Width;
                    //lb2.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Center;
                    //lb2.HorizontalAlignment = System.Windows.HorizontalAlignment.Center;
                    //lb2.VerticalAlignment = System.Windows.VerticalAlignment.Center;
                    //spanel.Children.Add(lb2);

                    CheckBox box2 = new CheckBox();
                    box2.IsChecked = ((n_ci - 1) <= _realVM.IsCounterLinePlotEnabled.Count) ? _realVM.IsCounterLinePlotEnabled[(n_ci - 2)] : false;    //n_ci: 1-based and be incremented before
                    box2.Width = 20;
                    box2.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Center;
                    box2.HorizontalAlignment = System.Windows.HorizontalAlignment.Center;
                    box2.VerticalAlignment = System.Windows.VerticalAlignment.Center;
                    //box2.Margin = new Thickness() { Left = 3, Right = 3, Top = 3, Bottom = 3 };
                    spanel.Children.Add(box2);
                    Label lbl3 = new Label();
                    lbl3.Width = 50;
                    lbl3.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Left;
                    lbl3.VerticalAlignment = System.Windows.VerticalAlignment.Center;
                    //lbl3.Margin = new Thickness() { Left = 3, Right = 3, Top = 3, Bottom = 3 };
                    lbl3.Content = "Plot";
                    spanel.Children.Add(lbl3);
                }

                //Color Configuration:
                RadioButton rb = new RadioButton();
                rb.Width = 20;
                rb.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Left;
                rb.VerticalAlignment = System.Windows.VerticalAlignment.Center;
                rb.Margin = new Thickness() { Left = 3, Right = 3, Top = 3, Bottom = 3 };
                rb.GroupName = "Lines";
                ToolTipService.SetToolTip(rb, _toolTips[2]);
                rb.Checked += new RoutedEventHandler(UpdateActiveLine);
                _lineRBtns.Add(rb);
                spanel.Children.Add(_lineRBtns.Last<RadioButton>());

                Label lbox4 = new Label();
                int red = Convert.ToInt32(node.Attributes["red"].Value.ToString());
                int green = Convert.ToInt32(node.Attributes["green"].Value.ToString());
                int blue = Convert.ToInt32(node.Attributes["blue"].Value.ToString());
                Color tmpColor = Color.FromRgb(Convert.ToByte(red), Convert.ToByte(green), Convert.ToByte(blue));
                lbox4.Background = new SolidColorBrush(tmpColor);
                lbox4.Width = 50;
                lbox4.HorizontalContentAlignment = System.Windows.HorizontalAlignment.Left;
                lbox4.Margin = new Thickness() { Left = 3, Right = 3, Top = 3, Bottom = 3 };
                _lineLbls.Add(lbox4);
                spanel.Children.Add(_lineLbls.Last<Label>());
                lbLines.Items.Add(spanel);
            }
            //parent's GUI:
            chkStim.IsChecked = _realVM.IsStimulusEnabled;
            cbAiTrigMode.SelectedIndex = (true == _realVM.IsAiTriggerEnabled) ? 1 : 0;
        }

        void EditLinesDialog_Unloaded(object sender, RoutedEventArgs e)
        {
            LinesDialogClosed((int)this.Left, (int)this.Top);
        }

        void UpdateActiveLine(object sender, RoutedEventArgs e)
        {
            if (false == epdRGB.IsExpanded)
            {
                epdRGB.IsExpanded = true;
            }
            for (int i = 0; i < _lineRBtns.Count; i++)
            {
                if (true == _lineRBtns.ElementAt<RadioButton>(i).IsChecked)
                {
                    Color bColor = ((System.Windows.Media.SolidColorBrush)_lineLbls.ElementAt<Label>(i).Background).Color;
                    sliderR.Value = bColor.R;
                    sliderG.Value = bColor.G;
                    sliderB.Value = bColor.B;
                }
            }
        }

        #endregion Methods
    }
}