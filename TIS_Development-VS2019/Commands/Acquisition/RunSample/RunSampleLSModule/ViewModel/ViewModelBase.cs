#region Header

// The following code is inspired by the work of Josh Smith
// http://joshsmithonwpf.wordpress.com/

#endregion Header

namespace RunSampleLSDll.ViewModel
{
    using System;
    using System.Collections;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Data;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.ServiceModel;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Timers;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Threading;
    using System.Xml;

    using CustomMessageBox;

    using DatabaseInterface;

    using GeometryUtilities;

    using RunSampleLSDll.View;

    using ThorImage;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Base class for all ViewModel classes in the application. Provides support for 
    /// property changes notification.
    /// </summary>
    public abstract class ViewModelBase : INotifyPropertyChanged
    {
        #region Fields

        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();

        #endregion Fields

        #region Events

        /// <summary>
        /// Raised when a property on this object has a new value.
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Indexers

        public object this[string propertyName, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                return (null != myPropInfo) ? myPropInfo.GetValue(this) : defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    myPropInfo.SetValue(this, value);
                }
            }
        }

        public object this[string propertyName, int index, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        return collection.GetType().GetProperty("Item").GetValue(collection, new object[] { index });
                    }
                    else
                    {
                        return myPropInfo.GetValue(this, null);
                    }
                }
                return defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        collection.GetType().GetProperty("Item").SetValue(collection, value, new object[] { index });
                    }
                    else
                    {
                        myPropInfo.SetValue(this, value, null);
                    }
                }
            }
        }

        #endregion Indexers

        #region Methods

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(RunSampleLSViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

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
            if ((TypeDescriptor.GetProperties(this)[propertyName] == null) && propertyName != "")
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