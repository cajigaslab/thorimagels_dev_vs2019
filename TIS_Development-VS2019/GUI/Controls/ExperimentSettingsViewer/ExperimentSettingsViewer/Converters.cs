namespace ExperimentSettingsViewer
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Globalization;
    using System.IO;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Navigation;
    using System.Xml;

    using ThorSharedTypes;

    public class BandwidthConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            string str = string.Empty;
            string strValue = value.ToString();
            try
            {
                if (typeof(XmlAttribute) == value.GetType())
                {

                    XmlAttribute valueAttribute = (XmlAttribute)value;
                    strValue = valueAttribute.Value.ToString();

                    str = Math.Round(Double.Parse(strValue.ToString()) / 1000.0, 4).ToString();
                }
            }
            catch (Exception ex)
            {
                string msg = ex.Message;
            }

            return str;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class BooleanToVisibilityCustomConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(Visibility))
                throw new InvalidOperationException("The target must be a Visibility");

            Visibility ret = Visibility.Collapsed;

            bool convertedValue = false;
            int tmp1 = 0;
            if (true == Int32.TryParse(value.ToString(), out tmp1))
            {
                convertedValue = (1 == tmp1) ? true : false;
            }
            else
            {
                bool tmp2 = false;
                if (true == bool.TryParse(value.ToString(), out tmp2))
                {
                    convertedValue = tmp2;
                }
            }

            switch (convertedValue)
            {
                case false:
                    {
                        ret = (0 == System.Convert.ToInt32(parameter)) ? Visibility.Collapsed : Visibility.Visible;
                    }
                    break;
                case true:
                    {
                        ret = (1 == System.Convert.ToInt32(parameter)) ? Visibility.Collapsed : Visibility.Visible;
                    }
                    break;
                default:
                    {
                    }
                    break;
            }

            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class ChannelConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(Visibility))
                throw new InvalidOperationException("The target must be a Visibility");

            Visibility ret = Visibility.Visible;

            switch (System.Convert.ToInt32(parameter))
            {
                case 1:
                    ret = (System.Convert.ToBoolean(System.Convert.ToInt32(value) & 0x1)) ? Visibility.Visible : Visibility.Collapsed;
                    break;
                case 2:
                    ret = (System.Convert.ToBoolean(System.Convert.ToInt32(value) & 0x2)) ? Visibility.Visible : Visibility.Collapsed;
                    break;
                case 3:
                    ret = (System.Convert.ToBoolean(System.Convert.ToInt32(value) & 0x4)) ? Visibility.Visible : Visibility.Collapsed;
                    break;
                case 4:
                    ret = (System.Convert.ToBoolean(System.Convert.ToInt32(value) & 0x8)) ? Visibility.Visible : Visibility.Collapsed;
                    break;
            }

            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class DetectorVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(Visibility))
                throw new InvalidOperationException("The target must be a Visibility");

            Visibility ret = Visibility.Collapsed;

            switch (System.Convert.ToInt32(value))
            {
                case 0:
                    {
                        ret = (0 == System.Convert.ToInt32(parameter)) ? Visibility.Visible : Visibility.Collapsed;
                    }
                    break;
                case 1:
                    {
                        ret = (1 == System.Convert.ToInt32(parameter)) ? Visibility.Visible : Visibility.Collapsed;
                    }
                    break;
                default:
                    {
                        ret = Visibility.Visible;
                    }
                    break;
            }

            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class DoubleCultureConverter : IValueConverter
    {
        #region Methods

        object IValueConverter.Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                System.Globalization.CultureInfo originalCultureInfo = (System.Globalization.CultureInfo)System.Threading.Thread.CurrentThread.CurrentCulture.Clone();
                if (0 == originalCultureInfo.NumberFormat.NumberDecimalSeparator.CompareTo(","))
                {
                    if (targetType == typeof(string))
                    {
                        return (Double.Parse(value.ToString().Replace(".", ",")).ToString());
                    }
                    else if (targetType == typeof(double))
                    {
                        return (Double.Parse(value.ToString().Replace(".", ",")));
                    }
                    else if (targetType == typeof(object))
                    {
                        if (value.GetType() == typeof(XmlAttribute))
                        {
                            XmlAttribute valueAttribute = (XmlAttribute)value;
                            return (object)Math.Round(Double.Parse(valueAttribute.Value.ToString().Replace(".", ",")), 3).ToString();
                        }
                        return (object)value;
                    }
                    else
                    {
                        throw new InvalidOperationException("The target must be a string or double");
                    }
                }
                else
                {
                    if (targetType == typeof(string))
                    {
                        return (Double.Parse(value.ToString()).ToString());
                    }
                    else if (targetType == typeof(double))
                    {
                        return (Double.Parse(value.ToString()));
                    }
                    else if (targetType == typeof(object))
                    {
                        if (value.GetType() == typeof(XmlAttribute))
                        {

                            XmlAttribute valueAttribute = (XmlAttribute)value;
                            return (object)Math.Round(Double.Parse(valueAttribute.Value.ToString()), 3).ToString();
                        }
                        return (object)value;
                    }
                    else
                    {
                        throw new InvalidOperationException("The target must be a string or double");
                    }
                }

            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        object IValueConverter.ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            System.Globalization.CultureInfo originalCultureInfo = (System.Globalization.CultureInfo)System.Threading.Thread.CurrentThread.CurrentCulture.Clone();
            try
            {
                System.Globalization.CultureInfo switchedCultureInfo = (System.Globalization.CultureInfo)System.Threading.Thread.CurrentThread.CurrentCulture.Clone();
                object returnValue;
                if (0 == switchedCultureInfo.NumberFormat.NumberDecimalSeparator.CompareTo(","))
                {
                    switchedCultureInfo.NumberFormat.NumberDecimalSeparator = ".";
                    System.Threading.Thread.CurrentThread.CurrentCulture = switchedCultureInfo;

                    if (targetType == typeof(string))
                    {
                        returnValue = (Double.Parse(Double.Parse(value.ToString().Replace(",", ".")).ToString(), CultureInfo.InvariantCulture).ToString());
                    }
                    else if (targetType == typeof(double))
                    {
                        returnValue = (Double.Parse(Double.Parse(value.ToString().Replace(",", ".")).ToString(), CultureInfo.InvariantCulture));
                    }
                    else if (targetType == typeof(object))
                    {
                        returnValue = (object)value.ToString();
                    }
                    else
                    {
                        System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;

                        throw new InvalidOperationException("The target must be a string or double");
                    }
                    System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
                }
                else
                {
                    if (targetType == typeof(string))
                    {
                        returnValue = (Double.Parse(value.ToString(), CultureInfo.InvariantCulture)).ToString();
                    }
                    else if (targetType == typeof(double))
                    {
                        returnValue = (Double.Parse(value.ToString(), CultureInfo.InvariantCulture));
                    }
                    else if (targetType == typeof(object))
                    {
                        returnValue = (object)value.ToString();
                    }
                    else
                    {
                        throw new InvalidOperationException("The target must be a string or double");
                    }
                }
                return returnValue;
            }
            catch (FormatException ex)
            {
                string str = ex.Message;
                System.Threading.Thread.CurrentThread.CurrentCulture = originalCultureInfo;
                return null;
            }
        }

        #endregion Methods
    }

    public class InputRangeConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            string str = string.Empty;
            string strValue = value.ToString();
            try
            {

                if (typeof(XmlAttribute) == value.GetType())
                {

                    XmlAttribute valueAttribute = (XmlAttribute)value;
                    strValue = valueAttribute.Value.ToString();

                    switch (Int32.Parse(strValue.ToString()))
                    {
                        case 5: str = "100mV"; break;
                        case 6: str = "200mV"; break;
                        case 7: str = "400mV"; break;
                        case 10: str = "1V"; break;
                        case 11: str = "2V"; break;
                        case 12: str = "4V"; break;
                        default: str = "100mV"; break;
                    }
                }

            }
            catch (Exception ex)
            {
                string msg = ex.Message;
            }

            return str;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class IntToBoolConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(bool?))
                throw new InvalidOperationException("The target must be a bool");

            if (Int32.Parse(value.ToString()) == 1)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(string))
                throw new InvalidOperationException("The target must be a string");
            if ((bool)value == true)
            {
                return "1";
            }
            else
            {
                return "0";
            }
        }

        #endregion Methods
    }

    public class IntToTriggerConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            try
            {
                if (typeof(XmlAttribute) == value.GetType())
                {

                    XmlAttribute valueAttribute = (XmlAttribute)value;
                    int val = Int32.Parse(valueAttribute.Value.ToString());
                    if (0 == val)
                    {
                        return "SW Trigger";
                    }
                    else if (1 == val)
                    {
                        return "HW TrigFirst";
                    }
                    else if (2 == val)
                    {
                        return "HW TrigEach";
                    }
                }
            }
            catch (Exception ex)
            {
                string msg = ex.Message;
            }
            return string.Empty;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        #endregion Methods
    }

    public class IntToVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(Visibility))
                throw new InvalidOperationException("The target must be a Visibility");

            if (Int32.Parse(value.ToString()) == 1)
            {
                return Visibility.Visible;
            }
            else
            {
                return Visibility.Collapsed;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(int))
                throw new InvalidOperationException("The target must be a int");
            if ((Visibility)value == Visibility.Visible)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }

        #endregion Methods
    }

    public class lsmTypeToVisibilityCustomConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(Visibility))
                throw new InvalidOperationException("The target must be a Visibility");

            Visibility ret = Visibility.Collapsed;

            switch ((string)value)
            {
                case "GalvoGalvo":
                case "GalvoGalvoNI":
                    {
                        ret = (0 == System.Convert.ToInt32(parameter)) ? Visibility.Visible : Visibility.Collapsed;
                    }
                    break;
                default:
                    break;
            }

            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class ModeToBoolNegateConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(bool))
                throw new InvalidOperationException("The target must be a bool");

            bool ret = false;

            switch (System.Convert.ToInt32(value))
            {
                case 0:
                    {
                        ret = (0 == System.Convert.ToInt32(parameter)) ? false : true;
                    }
                    break;
                case 1:
                    {
                        ret = (1 == System.Convert.ToInt32(parameter)) ? false : true;
                    }
                    break;
                case 2:
                    {
                        ret = (2 == System.Convert.ToInt32(parameter)) ? false : true;
                    }
                    break;
                case 3:
                    {
                        ret = (3 == System.Convert.ToInt32(parameter)) ? false : true;
                    }
                    break;
                default:
                    {
                    }
                    break;
            }

            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class ModeVisibilityConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            if (targetType != typeof(Visibility))
                throw new InvalidOperationException("The target must be a Visibility");

            Visibility ret = Visibility.Collapsed;

            switch (System.Convert.ToInt32(value))
            {
                case 0:
                    {
                        ret = (0 == System.Convert.ToInt32(parameter)) ? Visibility.Visible : Visibility.Collapsed;
                    }
                    break;
                case 1:
                    {
                        ret = (1 == System.Convert.ToInt32(parameter)) ? Visibility.Visible : Visibility.Collapsed;
                    }
                    break;
                case 2:
                    {
                        ret = (2 == System.Convert.ToInt32(parameter)) ? Visibility.Visible : Visibility.Collapsed;
                    }
                    break;
                case 3:
                    {
                        ret = (3 == System.Convert.ToInt32(parameter)) ? Visibility.Visible : Visibility.Collapsed;
                    }
                    break;
                default:
                    {
                    }
                    break;
            }

            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter,
            System.Globalization.CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class NegateConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is bool)
            {
                return !(bool)value;
            }
            return value;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is bool)
            {
                return !(bool)value;
            }
            return value;
        }

        #endregion Methods
    }

    public class ObjectiveConverter : IMultiValueConverter
    {
        #region Methods

        public object Convert(object[] values, Type targetType, object parameter,
            CultureInfo culture)
        {
            try
            {
                if (values.Length < 1) { return null; }

                var data = values[0] as IEnumerable;
                if (data == null) { return null; }

                string objName = ((XmlAttribute)values[0]).Value.ToString();

                XmlDataProvider xdp = (XmlDataProvider)parameter;
                XmlDocument doc = new XmlDocument();

                if (null == xdp)
                {
                    return null;
                }
                else if (null != xdp.Document)
                {
                    doc.LoadXml(xdp.Document.InnerXml.ToString());
                }
                else if (null == xdp.Document && null != xdp.Source)
                {
                    doc.Load(Uri.UnescapeDataString(xdp.Source.AbsolutePath));
                }
                else
                {
                    return null;
                }
                
                XmlNodeList nodes = doc.SelectNodes("/HardwareSettings/Objectives/Objective");

                foreach (XmlNode node in nodes)
                {
                    string attrType = (string)values[1];
                    string str = string.Empty;

                    if (node.Attributes.GetNamedItem("name").Value.ToString().Equals(objName))
                    {
                        if (attrType.Equals("NA:"))
                        {
                            XmlManager.GetAttribute(node, doc, "na", ref str);
                            return str;
                        }
                        else if (attrType.Equals("Beam Exp 1:"))
                        {
                            if (XmlManager.GetAttribute(node, doc, "beamExp", ref str))
                            {
                                double dval = System.Convert.ToDouble(str);
                                return string.Format("{0}", dval / 100.0);
                            }
                        }
                        else if (attrType.Equals("Beam Exp 2:"))
                        {
                            if (XmlManager.GetAttribute(node, doc, "beamExp2", ref str))
                            {
                                double dval = System.Convert.ToDouble(str);
                                return string.Format("{0}", dval / 100.0);
                            }
                        }
                        else if (attrType.Equals("Obj Turret:"))
                        {
                            XmlManager.GetAttribute(node, doc, "turretPosition", ref str);
                            return str;
                        }
                    }
                }
                return string.Empty;

            }
            catch (Exception ex)
            {
                string str = ex.Message;
                return null;
            }
        }

        public object[] ConvertBack(object value, Type[] targetTypes,
            object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        #endregion Methods
    }

    public class StreamTriggerConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (targetType != typeof(int))
                throw new InvalidOperationException("The target must be an int");
            int ret = 0;

            try
            {
                switch (Int32.Parse(value.ToString()))
                {
                    case 1:
                        ret = 0;
                        break;
                    case 4:
                        ret = 1;
                        break;
                    default:
                        ret = 0;
                        break;
                }
            }
            catch { }

            return ret;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (targetType != typeof(string))
                throw new InvalidOperationException("The target must be a string");
            string ret = "1";

            try
            {
                switch (Int32.Parse(value.ToString()))
                {
                    case 0:
                        ret = "1";
                        break;
                    case 1:
                        ret = "4";
                        break;
                    default:
                        ret = "1";
                        break;
                }
            }

            catch { }

            return ret;
        }

        #endregion Methods
    }

    public class StringToRoundedDoubleConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            double temp = 0;
            string str;
            if (value is XmlAttribute)
            {
                str = (value as XmlAttribute).Value;
            }
            else
            {
                str = (string)value;
            }
            double.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out temp);

            if (null != parameter)
            {
                int roundDecimals = 0;
                if (int.TryParse(parameter.ToString(), out roundDecimals))
                {
                    temp = Math.Round(temp, roundDecimals);
                }
            }

            return temp;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }

    public class UnixDateTimeConverter : IValueConverter
    {
        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            string temp = string.Empty;
            int val = 0;

            if (value is XmlAttribute)
            {
                val = System.Convert.ToInt32((value as XmlAttribute).Value);
            }
            else
            {
                val = System.Convert.ToInt32(value);
            }

            temp = String.Format("{0:u}", new System.DateTime(1970, 1, 1, 0, 0, 0, 0).AddSeconds(val).ToLocalTime());

            return temp;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotSupportedException();
        }

        #endregion Methods
    }
}