#region Header

// The following code is inspired by the work of Josh Smith
// http://joshsmithonwpf.wordpress.com/

#endregion Header

namespace ThorSharedTypes
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Linq.Expressions;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Input;
    using System.Xml;

    using Microsoft.Practices.Composite.Events;
    using Microsoft.Practices.Composite.Modularity;
    using Microsoft.Practices.Composite.Regions;
    using Microsoft.Practices.Composite.Wpf.Events;
    using Microsoft.Practices.Unity;

    using ThorLogging;

    public interface IMVM
    {
        #region Properties

        object this[string propertyName, object defaultObject = null]
        {
            get;
            set;
        }

        object this[string propertyName, int index, object defaultObject = null]
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        PropertyInfo GetPropertyInfo(string propertyName);

        void LoadXMLSettings();

        void OnPropertyChange(string propertyName);

        void UpdateExpXMLSettings(ref XmlDocument xmlDoc);

        #endregion Methods
    }

    public interface IViewModelActions
    {
        #region Methods

        void HandleViewLoaded();

        void HandleViewUnloaded();

        #endregion Methods
    }

    /// <summary>
    /// Singleton instance collections of all dynamic loading IMVM
    /// </summary>
    public class MVMManager
    {
        #region Fields

        private static string[] _dependentMVMs = { "MesoScanMVM" };
        private static MVMManager _instance = null;
        private static HashSet<string> _loadedAssemblies = new HashSet<string>();
        private static Dictionary<string, Object> _mvmCollection;
        private static XmlDocument[] _xmlDoc = new XmlDocument[(int)SettingsFileType.SETTINGS_FILE_LAST];

        #endregion Fields

        #region Constructors

        private MVMManager()
        {
        }

        #endregion Constructors

        #region Properties

        public static MVMManager Instance
        {
            get
            {
                if (null == _instance)
                {
                    _instance = new MVMManager();
                }
                return _instance;
            }
        }

        public Dictionary<string, Object> MVMCollection
        {
            get { return _mvmCollection; }
        }

        public XmlDocument[] SettingsDoc
        {
            get { return _xmlDoc; }
        }

        #endregion Properties

        #region Indexers

        public object this[string destination, object defaultObject = null]
        {
            get
            {
                try
                {
                    if (_mvmCollection.ContainsKey(destination))
                    {
                        return (ThorSharedTypes.IMVM)(_mvmCollection[destination]);
                    }
                    else
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "IMVM Get failed, destination: " + destination + " is not found.\n");
                        return defaultObject;
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Warning, 1, "IMVM Get failed, destination: " + destination + " error: " + ex.InnerException.Message);
                    return defaultObject;
                }

            }
            set
            {
                try
                {
                    if (_mvmCollection.ContainsKey(destination))
                    {
                        _mvmCollection[destination] = value;
                    }
                    else
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "IMVM Set failed, destination: " + destination + " is not found.\n");
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Warning, 1, "IMVM Set failed, destination: " + destination + " error: " + ex.InnerException.Message);
                }
            }
        }

        public object this[string destination, string propertyName, object defaultObject = null]
        {
            get
            {
                try
                {
                    if (_mvmCollection.ContainsKey(destination))
                    {
                        return ((ThorSharedTypes.IMVM)(_mvmCollection[destination]))[propertyName, defaultObject];
                    }
                    else
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "IMVM Get failed, destination: " + destination + " is not found.\n");
                        return defaultObject;
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Warning, 1, "IMVM Get failed, destination: " + destination + " property: " + propertyName + " error: " + ex.InnerException.Message);
                    return defaultObject;
                }
            }
            set
            {
                try
                {
                    if (_mvmCollection.ContainsKey(destination))
                    {
                        ((ThorSharedTypes.IMVM)(_mvmCollection[destination]))[propertyName] = value;
                    }
                    else
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "IMVM Set failed, destination: " + destination + " is not found.\n");
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Warning, 1, "IMVM Set failed, destination: " + destination + " property: " + propertyName + " error: " + ex.InnerException.Message);
                }
            }
        }

        public object this[string destination, string propertyName, int index, object defaultObject = null]
        {
            get
            {
                try
                {
                    if (_mvmCollection.ContainsKey(destination))
                    {
                        return ((ThorSharedTypes.IMVM)(_mvmCollection[destination]))[propertyName, index, defaultObject];
                    }
                    else
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "IMVM Get failed, destination: " + destination + " is not found.\n");
                        return defaultObject;
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Warning, 1, "IMVM Get failed, destination: " + destination + " property: " + propertyName + " index: " + index + " error: " + ex.InnerException.Message);
                    return defaultObject;
                }

            }
            set
            {
                try
                {
                    if (_mvmCollection.ContainsKey(destination))
                    {
                        ((ThorSharedTypes.IMVM)(_mvmCollection[destination]))[propertyName, index] = value;
                    }
                    else
                    {
                        ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "IMVM Set failed, destination: " + destination + " is not found.\n");
                    }
                }
                catch (Exception ex)
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Warning, 1, "IMVM Set failed, destination: " + destination + " property: " + propertyName + " index: " + index + " error: " + ex.InnerException.Message);
                }

            }
        }

        #endregion Indexers

        #region Methods

        public static void CollectMVM()
        {
            string ModulesPath = System.IO.Path.GetDirectoryName(System.Diagnostics.Process.GetCurrentProcess().MainModule.FileName) + "\\Modules\\MVM";

            var files = System.IO.Directory.GetFiles(ModulesPath, "*MVM.dll", System.IO.SearchOption.AllDirectories);

            List<string> foundFiles = new List<string>();

            if (null == _mvmCollection)
            {
                _mvmCollection = new Dictionary<string, Object>();
            }

            foreach (var file in files)
            {
                if (!_loadedAssemblies.Contains(file))
                {
                    if (!_dependentMVMs.Contains(Path.GetFileNameWithoutExtension(file)))
                    {
                        LoadInstance(file);
                    }
                    else
                    {
                        foundFiles.Add(file);
                    }
                    _loadedAssemblies.Add(file);
                }
            }
            //load found last since they are dependent on other mvms
            foreach (string found in foundFiles)
            {
                LoadInstance(found);
            }
        }

        public static bool InterfaceClassFilter(Type typeObj, Object criteriaObj)
        {
            Type baseClassType = (Type)criteriaObj;
            return (typeObj == baseClassType) ? true : false;
        }

        public void AddMVM(string name, object obj)
        {
            if (null == _mvmCollection)
            {
                _mvmCollection = new Dictionary<string, Object>();
            }
            if (!_mvmCollection.ContainsKey(name))
            {
                _mvmCollection.Add(name, obj);
            }
        }

        /// <summary>
        /// request all (or specified) MVMs to load settings
        /// </summary>
        /// <param name="mvmNames"></param>
        public void LoadMVMSettings(string[] mvmNames = null)
        {
            if (null == mvmNames)
            {
                foreach (var item in _mvmCollection.Values)
                {
                    if (item is ThorSharedTypes.IMVM)
                    {
                        (item as ThorSharedTypes.IMVM).LoadXMLSettings();
                    }
                }
            }
            else
            {
                foreach (var item in mvmNames)
                {
                    if (_mvmCollection.ContainsKey(item))
                    {
                        if (_mvmCollection[item] is ThorSharedTypes.IMVM)
                        {
                            ((ThorSharedTypes.IMVM)_mvmCollection[item]).LoadXMLSettings();
                        }
                    }
                }
            }
        }

        public void LoadSettings(bool lockAccess = false)
        {
            for (SettingsFileType i = SettingsFileType.SETTINGS_FILE_FIRST; i < SettingsFileType.SETTINGS_FILE_LAST; i++)
            {
                LoadSettings(i, lockAccess);
            }
        }

        public bool LoadSettings(SettingsFileType fType, bool lockAccess = false)
        {
            XmlDocument xmlLocal = new XmlDocument();
            string settingsFile = string.Empty;
            switch (fType)
            {
                case SettingsFileType.APPLICATION_SETTINGS:
                    settingsFile = ResourceManagerCS.GetApplicationSettingsFileString();
                    break;
                case SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS:
                    settingsFile = ResourceManagerCS.GetCaptureTemplatePathString() + "Active.xml";
                    break;
                case SettingsFileType.HARDWARE_SETTINGS:
                    settingsFile = ResourceManagerCS.GetHardwareSettingsFileString();
                    break;
                case SettingsFileType.SETTINGS_FILE_LAST:
                default:
                    return false;
            }

            //return false if not found without replacing current
            if (!File.Exists(settingsFile))
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Information, 1, " Unable to load " + settingsFile);
                return false;
            }
            if (lockAccess)
                ResourceManagerCS.BorrowDocMutexCS(fType);

            xmlLocal.Load(settingsFile);

            if (lockAccess)
                ResourceManagerCS.ReturnDocMutexCS(fType);

            _xmlDoc[(int)fType] = xmlLocal;
            return true;
        }

        public bool ReloadSettings(SettingsFileType fType, bool lockAccess = false)
        {
            SaveSettings(fType, lockAccess);
            return LoadSettings(fType, lockAccess);
        }

        public void SaveSettings(bool lockAccess = false)
        {
            for (SettingsFileType i = SettingsFileType.SETTINGS_FILE_FIRST; i < SettingsFileType.SETTINGS_FILE_LAST; i++)
            {
                SaveSettings(i, lockAccess);
            }
        }

        public void SaveSettings(SettingsFileType fType, bool lockAccess = false)
        {
            string settingsFile = string.Empty;

            switch (fType)
            {
                case SettingsFileType.APPLICATION_SETTINGS:
                    settingsFile = ResourceManagerCS.GetApplicationSettingsFileString();
                    break;
                case SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS:
                    settingsFile = ResourceManagerCS.GetCaptureTemplatePathString() + "Active.xml";
                    break;
                case SettingsFileType.HARDWARE_SETTINGS:
                    settingsFile = ResourceManagerCS.GetHardwareSettingsFileString();
                    break;
                case SettingsFileType.SETTINGS_FILE_LAST:
                default:
                    return;
            }
            if (null != _xmlDoc[(int)fType])
            {
                if (lockAccess)
                    ResourceManagerCS.BorrowDocMutexCS(fType);

                _xmlDoc[(int)fType].Save(settingsFile);

                if (lockAccess)
                    ResourceManagerCS.ReturnDocMutexCS(fType);
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, "Could not save settings file" + settingsFile + "\n");
            }
        }

        /// <summary>
        /// persist all settings from all or specified MVMs to experiment document reference
        /// </summary>
        /// <param name="experimentDoc"></param>
        /// <param name="mvmNames"></param>
        public void UpdateMVMXMLSettings(ref XmlDocument experimentDoc, string[] mvmNames = null)
        {
            if (null == mvmNames)
            {
                foreach (var item in _mvmCollection.Values)
                {
                    (item as ThorSharedTypes.IMVM).UpdateExpXMLSettings(ref experimentDoc);
                }
            }
            else
            {
                foreach (var item in mvmNames)
                {
                    if (_mvmCollection.ContainsKey(item))
                        ((ThorSharedTypes.IMVM)_mvmCollection[item]).UpdateExpXMLSettings(ref experimentDoc);
                }
            }
        }

        private static void LoadInstance(string file)
        {
            Type[] types;
            try
            {
                types = System.Reflection.Assembly.LoadFrom(file).GetTypes();

                foreach (Type t in types)
                {
                    // Specify the TypeFilter delegate that compares the interfaces against filter criteria.
                    System.Reflection.TypeFilter theFilter = new System.Reflection.TypeFilter(InterfaceClassFilter);
                    if (0 < t.FindInterfaces(theFilter, typeof(IMVM)).Length)
                    {
                        Object obj = Activator.CreateInstance(t);
                        _mvmCollection.Add(t.Name, obj);
                    }
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
                // Can't load as .NET assembly, so ignore
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, " Unable to load " + file);
            }
        }

        #endregion Methods
    }

    public class PC<T> : INotifyPropertyChanged
    {
        #region Fields

        private T _value = default(T);

        #endregion Fields

        #region Constructors

        public PC(T val)
        {
            this.Value = val;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string[] DependentProperties
        {
            get;
            set;
        }

        public T Value
        {
            get { return _value; }
            set { _value = value; OnPropertyChanged("Value"); }
        }

        #endregion Properties

        #region Methods

        void OnPropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Methods
    }

    /// <summary>
    /// Base class for all ViewModel classes in the application. Provides support for 
    /// property changes notification.
    /// </summary>
    public abstract class VMBase : INotifyPropertyChanged
    {
        #region Events

        /// <summary>
        /// Raised when a property on this object has a new value.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Methods

        /// <summary>
        /// Warns the developer if this object does not have a public property with
        /// the specified name. This method does not exist in a Release build.
        /// </summary>
        [Conditional("DEBUG")]
        [DebuggerStepThrough]
        public void VerifyPropertyName(string propertyName)
        {
            // verify that the property name matches a real,
            // public, instance property on this object.
            // or is empty, indicating a call to refresh all
            // bindings
            if (TypeDescriptor.GetProperties(this)[propertyName] == null && propertyName != "")
            {
                Debug.Fail("Invalid property name: " + propertyName);
            }
        }

        /// <summary>
        /// Raises this object's PropertyChanged event.
        /// </summary>
        /// <param name="propertyName">The name of the property that has a new value.</param>
        protected virtual void OnPropertyChanged(string propertyName)
        {
            this.VerifyPropertyName(propertyName);

            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Methods
    }
}