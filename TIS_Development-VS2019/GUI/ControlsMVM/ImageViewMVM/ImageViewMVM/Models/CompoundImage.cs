namespace ImageViewMVM.Models
{
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Timers;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using ThorLogging;

    using ThorSharedTypes;

    public class CompoundImage : VMBase
    {
        #region Fields

        WriteableBitmap _imageXY;
        ObservableCollection<UIElement> _overlayItems = new ObservableCollection<UIElement>();
        ICommand _selectThisImageCommand;

        #endregion Fields

        #region Events

        public event Action<CompoundImage> Selected;

        #endregion Events

        #region Properties

        public WriteableBitmap ImageXY
        {
            get => _imageXY;
            set=> SetProperty(ref _imageXY, value);
        }

        public ObservableCollection<UIElement> OverlayItems
        {
            get => _overlayItems;
            set => SetProperty(ref _overlayItems, value);
        }

        public ICommand SelectThisImageCommand
        {
            get => _selectThisImageCommand ?? (_selectThisImageCommand = new RelayCommand(() => cmmd()));
        }

        #endregion Properties

        #region Methods

        void cmmd()
        {
            Selected?.Invoke(this);
            OnPropertyChanged("OverlayItems");
        }

        #endregion Methods
    }
}