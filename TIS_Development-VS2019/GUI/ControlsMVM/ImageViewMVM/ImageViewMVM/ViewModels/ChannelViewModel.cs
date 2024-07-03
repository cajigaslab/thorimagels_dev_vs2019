namespace ImageViewMVM.ViewModels
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Diagnostics;
    using System.IO;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Media;

    using ImageViewMVM.Models;

    using ThorLogging;

    using ThorSharedTypes;

    public class ChannelGroupViewModel : VMBase
    {
        #region Fields

        const int BUTTON_SIZE = 20;

        Thickness _channelGroupMargin = new Thickness(5, 0, 5, 0);
        ICommand _channelGroupMouseOnTopCommand;
        string _channelGroupName = string.Empty;
        ObservableCollection<ChannelViewModel> _channels = new ObservableCollection<ChannelViewModel>();
        string _channelsStackOrientation = string.Empty;

        #endregion Fields

        #region Properties

        public Thickness ChannelGroupMargin
        {
            get
            {
                return _channelGroupMargin;
            }
            set
            {
                _channelGroupMargin = value;
                OnPropertyChanged("ChannelGroupMargin");
            }
        }

        public ICommand ChannelGroupMouseOnTopCommand
        {
            get => _channelGroupMouseOnTopCommand ?? (_channelGroupMouseOnTopCommand = new RelayCommandWithParam((x) => MoveChannelIcons(x.Equals(bool.TrueString))));
        }

        public string ChannelGroupName
        {
            get => _channelGroupName;
            set => _channelGroupName = value;
        }

        public Visibility ChannelGroupNameVisibility
        {
            get => _channelGroupName == string.Empty ? Visibility.Collapsed : Visibility.Visible;
        }

        public ObservableCollection<ChannelViewModel> Channels
        {
            get => _channels;
        }

        public string ChannelsStackOrientation
        {
            get
            {
                return _channelsStackOrientation;
            }
            set
            {
                _channelsStackOrientation = value;
                OnPropertyChanged("ChannelsStackOrientation");
            }
        }

        #endregion Properties

        #region Methods

        public void MoveChannelIcons(bool isMouseOnTop)
        {
            /*if (Channels.Count > 1)
            {
                //Set the ZIndex of the first channel to be the largest so it can show up in front of the others
                Channels[0].ChannelZIndex = Channels.Count - 1;
                ChannelGroupMargin = isMouseOnTop ? new Thickness(0, 0, 0, -5 * Channels.Count) : new Thickness(0, 0, 0, BUTTON_SIZE * (1 - Channels.Count));

                for (int i = 1; i < Channels.Count; i++)
                {
                    //If the mouse is on top of this group of channels rotate from 300 to 360 degrees, and translate from a stack position to an expanded position
                    Channels[i].StartRotationPos = isMouseOnTop ? 300 : 360;
                    Channels[i].StopRotationPos = isMouseOnTop ? 360 : 300;
                    Channels[i].StartTranslationPos = isMouseOnTop ? BUTTON_SIZE * -i : -5 * i;
                    Channels[i].StopTranslationPos = isMouseOnTop ? -5 * i : BUTTON_SIZE * -i;
                    Channels[i].ChannelZIndex = Channels.Count - i - 1;
                }
            }*/
        }

        #endregion Methods
    }

    public class ChannelViewModel : VMBase
    {
        #region Fields

        const int PALETTE_SIZE = 256;

        int _chanNameFontSize = 30;
        Brush _channelColor = Brushes.Transparent;
        bool _channelDisplayEnable = true;
        string _channelName;
        Visibility _channelVisibility = Visibility.Collapsed;
        private ObservableCollection<ImageViewControl.LUTComboBoxView> _colorPalettesUI = new ObservableCollection<ImageViewControl.LUTComboBoxView>();
        ICommand _comboBoxOpenedCommand;
        GetPixelInfo _getPixelInfoDelegate;
        int _selectedColorUI = 0;
        int _startRotationPos = 360;
        int _startTranslationPos = 0;
        int _stopRotationPos = 360;
        int _stopTranslationPos = 0;
        int _zIndex = 0;

        #endregion Fields

        #region Events

        public event Action<int, int> ChannelDisplayEnabledChanged;

        #endregion Events

        #region Properties

        public int ChanNameFontSize
        {
            get
            {
                return _chanNameFontSize;
            }
            set
            {
                _chanNameFontSize = value;
                OnPropertyChanged("ChanNameFontSize");
            }
        }

        public Brush ChannelColor
        {
            get => _channelColor;
            set => SetProperty(ref _channelColor, value);
        }

        public string ChannelColorString
        {
            get;
            set;
        }

        public bool ChannelDisplayEnable
        {
            get => _channelDisplayEnable;
            set
            {
                if (SetProperty(ref _channelDisplayEnable, value))
                {
                    ChannelDisplayEnabledChanged?.Invoke(GroupIndex, Index);
                }

            }
        }

        public string ChannelName
        {
            get => _channelName;
            set
            {
                SetProperty(ref _channelName, value);
                OnPropertyChanged("ChannelNameAbbreviated");
                OnPropertyChanged("Tooltip");
            }
        }

        public string ChannelNameAbbreviated
        {
            get
            {
                if (_channelName.ToLower().Contains("chan"))
                {
                    var x = _channelName.Substring(_channelName.ToLower().IndexOf("chan") + 4);
                    switch (x.Length)
                    {
                        case 1: ChanNameFontSize = 30; break;
                        case 2: ChanNameFontSize = 25; break;
                        default: ChanNameFontSize = 23; break;
                    }
                    return x;
                }
                else
                {
                    switch (_channelName.Length)
                    {
                        case 1: ChanNameFontSize = 30; break;
                        case 2: ChanNameFontSize = 25; break;
                        default: ChanNameFontSize = 23; break;
                    }
                    return _channelName;
                }
            }
        }

        public Visibility ChannelVisibility
        {
            get => _channelVisibility;
            set => SetProperty(ref _channelVisibility, value);
        }

        public int ChannelZIndex
        {
            get
            {
                return _zIndex;
            }
            set
            {
                _zIndex = value;
                OnPropertyChanged("ChannelZIndex");
            }
        }

        public ObservableCollection<ImageViewControl.LUTComboBoxView> ColorPalettesUI
        {
            get
            {
                return _colorPalettesUI;
            }
            set
            {
                _colorPalettesUI = value;
                OnPropertyChanged("ColorPalettesUI");
            }
        }

        public ICommand ComboBoxOpenedCommand
        {
            get => _comboBoxOpenedCommand ?? (_comboBoxOpenedCommand = new RelayCommand(() => ReloadComboBoxItemSource()));
        }

        public GetPixelInfo GetPixelInfoDelegate
        {
            set => _getPixelInfoDelegate = value;
        }

        public int GroupIndex
        {
            get;
            set;
        }

        public int Index
        {
            get; set;
        }

        public List<Color>[] LUTColors
        {
            get;
            set;
        }

        public List<string> LutNames
        {
            get;
            set;
        }

        public int? RollOverPointIntensity
        {
            get => _getPixelInfoDelegate?.Invoke(Index, GroupIndex);
        }

        public bool RollOverPositionChanged
        {
            set
            {
                OnPropertyChanged("RollOverPointIntensity");
            }
        }

        public int SelectedColorUI
        {
            get
            {
                return _selectedColorUI;
            }
            set
            {
                if (-1 != value)
                {
                    _selectedColorUI = value;
                    ChannelColorString = LutNames[value];
                }
                OnPropertyChanged("SelectedColorUI");
            }
        }

        public int StartRotationPos
        {
            get
            {
                return _startRotationPos;
            }
            set
            {
                _startRotationPos = value;
                OnPropertyChanged("StartRotationPos");
            }
        }

        public int StartTranslationPos
        {
            get
            {
                return _startTranslationPos;
            }
            set
            {
                _startTranslationPos = value;
                OnPropertyChanged("StartTranslationPos");
            }
        }

        public int StopRotationPos
        {
            get
            {
                return _stopRotationPos;
            }
            set
            {
                _stopRotationPos = value;
                OnPropertyChanged("StopRotationPos");
            }
        }

        public int StopTranslationPos
        {
            get
            {
                return _stopTranslationPos;
            }
            set
            {
                _stopTranslationPos = value;
                OnPropertyChanged("StopTranslationPos");
            }
        }

        public string Tooltip
        {
            get
            {
                return "Enables/disables display of channel " + _channelName;
            }
        }

        #endregion Properties

        #region Methods

        public Color GetChannelColor()
        {
            Brush brush;

            // NOTE: LUTColors can have null entries if their corresponding LUT files are corrupt or otherwise unable to be loaded
            int lutIdx = LutNames.IndexOf(ChannelColorString);
            if(LUTColors[lutIdx] == null)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Critical, 1, $"The Lookup Table for channel \"{ChannelColorString}\" is null, please check that the LUT file at index {lutIdx} is valid.");
            }

            double luminance = (0.2126 * LUTColors[lutIdx][PALETTE_SIZE - 1].R + 0.7152 * LUTColors[lutIdx][PALETTE_SIZE - 1].G + 0.0722 * LUTColors[lutIdx][PALETTE_SIZE - 1].B);
            if (luminance > 240)
            {
                //if the color is too bright it will not
                //display on a white background
                //substitute gray if the color is too bright
                brush = new SolidColorBrush(Colors.Gray);
            }
            else
            {
                brush = new SolidColorBrush(LUTColors[LutNames.IndexOf(ChannelColorString)][PALETTE_SIZE - 1]);
            }
            return ((SolidColorBrush)brush).Color;
        }

        public void ReloadComboBoxItemSource()
        {
            ObservableCollection<ImageViewControl.LUTComboBoxView> colorPalettesUI = new ObservableCollection<ImageViewControl.LUTComboBoxView>();
            for (int i = 0; i < LutNames.Count; i++)
            {
                if (null == LUTColors[i])
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " cannot load LUT file #:" + i + " named:" + LutNames[i]);
                }
                else
                {
                    colorPalettesUI.Add(new ImageViewControl.LUTComboBoxView(LUTColors[i], LutNames[i]));
                }
            }
            ColorPalettesUI = colorPalettesUI;
            int test = SelectedColorUI;
            _selectedColorUI = -1;
            OnPropertyChanged("SelectedColorUI");
            SelectedColorUI = test;
        }

        #endregion Methods
    }
}