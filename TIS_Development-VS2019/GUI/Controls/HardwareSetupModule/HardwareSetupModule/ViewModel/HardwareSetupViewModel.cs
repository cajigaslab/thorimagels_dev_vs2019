using System;
using System.Windows;
using System.ComponentModel;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;
using System.Runtime.InteropServices;
using System.Diagnostics;
using ThorImageInfastructure;
using Microsoft.Practices.Composite.Events;
using Microsoft.Practices.Composite.Wpf.Events;
using Microsoft.Practices.Composite.Regions;
using Microsoft.Practices.Unity;
using ThorLogging;
using System.Reflection;
using System.Resources;

namespace HardwareSetupDll.ViewModel
{
    /// <summary>
    /// ViewModel class for the HardwareSetup model object
    /// </summary>
    public class HardwareSetupViewModel : ViewModelBase
    {
        #region fields
        // wrapped HardwareSetup object
        private readonly HardwareSetup _HardwareSetup;
        private IEventAggregator _eventAggregator;
        private IRegionManager _regionManager;
        private IUnityContainer _container;
        private ICommand _addWavelengthCommand;
        private ICommand _deleteWavelengthCommand;
        private ICommand _modifyWavelengthCommand;
        private RelayCommand _excitationCommand;
        private RelayCommand _dichroicCommand;
        private RelayCommand _emissionCommand;
        private RelayCommand _shutterCommand;
        private PositionEnum _dichroic = 0;
        private PositionEnum _excitation = 0;
        private PositionEnum _emission = 0;
        private double _zposition=0;

        public static readonly RoutedCommand CommandOne = new RoutedCommand();
        public static readonly RoutedCommand CommandTwo = new RoutedCommand();
        public static readonly RoutedCommand CommandThree = new RoutedCommand();

        #endregion fields

        /// <summary>
        /// Create a new instance of the HardwareSetupViewModel class
        /// </summary>
        /// <param name="HardwareSetup">Wrapped HardwareSetup object</param>
        public HardwareSetupViewModel(IEventAggregator eventAggregator, IRegionManager regionManager, IUnityContainer container, HardwareSetup HardwareSetup)
        {           
            this._eventAggregator = eventAggregator;
            this._regionManager = regionManager;
            this._container = container;
           
            if (HardwareSetup != null)
            {
                this._HardwareSetup = HardwareSetup;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, this.GetType().Name + " HardwareSetup is null. Creating a new HardwareSetup object.");
                HardwareSetup = new HardwareSetup();

                if (HardwareSetup == null)
                {
                    ResourceManager rm = new ResourceManager("HardwareSetupModule.Properties.Resources", Assembly.GetExecutingAssembly());
                    ThorLog.Instance.TraceEvent(TraceEventType.Critical, 1, this.GetType().Name + " " + rm.GetString("CreateHardwareSetupModelFailed"));
                    throw new NullReferenceException("HardwareSetup");
                }

                this._HardwareSetup = HardwareSetup;
            }


                      
            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

     
        private void AddWavelength()
        {

        }
        
        private void DeleteWavelength()
        {
        }

        private void ModifyWavelength()
        {
        }

        private void ExcitationMove(object var)
        {
            string name = (string)var;
            string start = "Pos";
            int val = Convert.ToInt32(name.TrimStart(start.ToCharArray()));
            SetFilterPositionEx(val);
        }

        private void DichroicMove(object var)
        {
            string name = (string)var;
            string start = "Pos";
            int val = Convert.ToInt32(name.TrimStart(start.ToCharArray()));
            SetFilterPositionDic(val);

        }

        private void EmissionMove(object var)
        {
            string name = (string)var;
            string start = "Pos";
            int val = Convert.ToInt32(name.TrimStart(start.ToCharArray()));
            SetFilterPositionEm(val);

        }

        private void ShutterMove(object var)
        {
            bool value = (bool)var;

            if (value)
            {
                int shutterPos = 1;
                SetShutterPosition(shutterPos);
            }
            else
            {
                int shutterPos = 0;
                SetShutterPosition(shutterPos);
            }
        }

        public void PublishChangeEvent()
        {
            ChangeEvent changeEvent = new ChangeEvent();
            changeEvent.IsChanged = true;

            //command published to change the hardware settings in the Exp Setup Control
            _eventAggregator.GetEvent<HardwareSettingsChangeEvent>().Publish(changeEvent);

        }

        #region commands

        public ICommand AddWavelengthCommand
        {
            get
            {
                if (this._addWavelengthCommand == null)
                    this._addWavelengthCommand = new RelayCommand((x) => AddWavelength());

                return this._addWavelengthCommand;
            }
        }

        public ICommand DeleteWavelengthCommand
        {
            get
            {
                if (this._deleteWavelengthCommand == null)
                    this._deleteWavelengthCommand = new RelayCommand((x) => DeleteWavelength());

                return this._deleteWavelengthCommand;
            }
        }

        public ICommand ModifyWavelengthCommand
        {
            get
            {
                if (this._modifyWavelengthCommand == null)
                    this._modifyWavelengthCommand = new RelayCommand((x) => ModifyWavelength());

                return this._modifyWavelengthCommand;
            }
        }
        
        public ICommand ExcitationCommand
        {
            get
            {
                if (this._excitationCommand == null)
                    this._excitationCommand = new RelayCommand((x) => ExcitationMove(x));

                return this._excitationCommand;
            }
        }

        public ICommand DichroicCommand
        {
            get
            {
                if (this._dichroicCommand == null)
                    this._dichroicCommand = new RelayCommand((x) => DichroicMove(x));

                return this._dichroicCommand;
            }
        }

        public ICommand EmissionCommand
        {
            get
            {
                if (this._emissionCommand == null)
                    this._emissionCommand = new RelayCommand((x) => EmissionMove(x));

                return this._emissionCommand;
            }
        }

        public ICommand ShutterCommand
        {
            get
            {
                if (this._shutterCommand == null)
                    this._shutterCommand = new RelayCommand((x) => ShutterMove(x));

                return this._shutterCommand;
            }
        }

        #endregion commands

        #region properties

        /// <summary>
        /// Gets the wrapped HardwareSetup object
        /// </summary>
        public HardwareSetup HardwareSetup
        {
            get 
            {
                return this._HardwareSetup;
            }
        }


            public enum PositionEnum{Pos1,Pos2,Pos3,Pos4,Pos5,Pos6,Pos7,Pos8};


        public PositionEnum Dichroic
        {
            get
            {
                return _dichroic;
            }

            set
            {
                _dichroic = value;
            }
        }

        public PositionEnum Excitation
        {
            get
            {
                return _excitation;
            }

            set
            {
                _excitation = value;
            }
        }

        public PositionEnum Emission
        {
            get
            {
                return _emission;
            }

            set
            {
                _emission = value;
            }
        }

        public double ZPosition
        {
            get
            {
                double val=0;
                GetZPosition(ref val);
                return val;
            }

            set
            {                
                SetZPosition(value);
            }
        }
     
        #endregion properties


        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetFilterPositionEx")]
        private extern static bool SetFilterPositionEx(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetFilterPositionEm")]
        private extern static bool SetFilterPositionEm(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetFilterPositionDic")]
        private extern static bool SetFilterPositionDic(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetShutterPosition")]
        private extern static bool SetShutterPosition(int pos);

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetZPosition")]
        private extern static bool SetZPosition(double pos);
    
        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "GetZPosition")]
        private extern static bool GetZPosition(ref double pos);
    }
}
