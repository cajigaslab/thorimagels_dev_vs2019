namespace OverlayManager
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Xml;

    using ThorLogging;

    public static class BleachClass
    {
        #region Fields

        public static readonly DependencyProperty ClkRateProperty = DependencyProperty.Register("ClkRate", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty DwellTimeProperty = DependencyProperty.Register("DwellTime", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty EpochCountProperty = DependencyProperty.Register("EpochCount", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty FillProperty = DependencyProperty.Register("Fill", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty IterationsProperty = DependencyProperty.Register("Iterations", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty LongIdleTimeProperty = DependencyProperty.Register("LongIdleTime", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty PixelModeProperty = DependencyProperty.Register("PixelMode", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty PostCycleIdleMSProperty = DependencyProperty.Register("PostCycleIdleMS", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty PostEpochIdleMSProperty = DependencyProperty.Register("PostEpochIdleMS", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty PostIdleTimeProperty = DependencyProperty.Register("PostIdleTime", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty PostPatIdleTimeProperty = DependencyProperty.Register("PostPatIdleTime", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty PowerProperty = DependencyProperty.Register("Power", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty PreCycleIdleMSProperty = DependencyProperty.Register("PreCycleIdleMS", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty PreEpochIdleMSProperty = DependencyProperty.Register("PreEpochIdleMS", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty PreIdleTimeProperty = DependencyProperty.Register("PreIdleTime", typeof(string), typeof(BleachClass), null);
        public static readonly DependencyProperty PrePatIdleTimeProperty = DependencyProperty.Register("PrePatIdleTime", typeof(string), typeof(BleachClass), null);

        public static string[] BleachAttributes = { "BleachClass.PreIdleTime",              //0
                                                      "BleachClass.DwellTime",              //1
                                                      "BleachClass.PostIdleTime",           //2
                                                      "BleachClass.Iterations",             //3
                                                      "BleachClass.Fill",                   //4
                                                      "BleachClass.PixelMode",              //5
                                                      "BleachClass.Power",                  //6
                                                      "BleachClass.ClkRate",                //7
                                                      "BleachClass.LongIdleTime",           //8
                                                      "BleachClass.PrePatIdleTime",         //9
                                                      "BleachClass.PostPatIdleTime",        //10
                                                      "BleachClass.PreCycleIdleMS",         //11
                                                      "BleachClass.PostCycleIdleMS",        //12
                                                      "BleachClass.PreEpochIdleMS",         //13
                                                      "BleachClass.PostEpochIdleMS",        //14
                                                      "BleachClass.EpochCount",             //15
                                                      "BleachClass.ZVlaue"                  //16
                                                  };
        public static string[] BleachNamespace = { "xmlns:bleach", "clr-namespace:OverlayManager;assembly=OverlayManager" };
        public static string BleachPrefix = "bleach";

        #endregion Fields

        #region Methods

        public static bool GetBleachAttribute(XmlNode node, XmlDocument doc, int bAttributeID, ref string value)
        {
            string attrName = BleachAttributes[bAttributeID].ToString();
            //Get bleach attributes with prefix:
            if (null != node.Attributes[attrName, BleachNamespace[1]])
            {
                value = node.Attributes[attrName, BleachNamespace[1]].Value;
                return true;
            }
            else
            {
                return false;
            }
        }

        public static string GetClkRate(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(ClkRateProperty);
        }

        public static string GetDwellTime(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(DwellTimeProperty);
        }

        public static string GetEpochCount(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(EpochCountProperty);
        }

        public static string GetFill(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(FillProperty);
        }

        public static string GetIterations(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(IterationsProperty);
        }

        public static string GetLongIdleTime(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(LongIdleTimeProperty);
        }

        public static string GetPixelMode(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(PixelModeProperty);
        }

        public static string GetPostCycleIdleMS(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(PostCycleIdleMSProperty);
        }

        public static string GetPostEpochIdleMS(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(PostEpochIdleMSProperty);
        }

        public static string GetPostIdleTime(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(PostIdleTimeProperty);
        }

        public static string GetPostPatIdleTime(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(PostPatIdleTimeProperty);
        }

        public static string GetPower(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(PowerProperty);
        }

        public static string GetPreCycleIdleMS(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(PreCycleIdleMSProperty);
        }

        public static string GetPreEpochIdleMS(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(PreEpochIdleMSProperty);
        }

        public static string GetPreIdleTime(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(PreIdleTimeProperty);
        }

        public static string GetPrePatIdleTime(UIElement element)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            return (string)element.GetValue(PrePatIdleTimeProperty);
        }

        public static void SetBleachAttribute(XmlNode node, XmlDocument doc, int bAttributeID, string value)
        {
            string attrName = BleachAttributes[bAttributeID].ToString();

            //Remove inherited attributes without prefix here:
            if (null != node.Attributes[attrName])
            {
                XmlNodeList tmpNodes = doc.DocumentElement.ChildNodes.Item(0).ChildNodes.Item(0).ChildNodes;
                for (int i = 0; i < tmpNodes.Count; i++)
                {
                    if (null != tmpNodes[i].Attributes[attrName])
                    {
                        XmlAttribute rattr = doc.DocumentElement.ChildNodes.Item(0).ChildNodes.Item(0).ChildNodes[i].Attributes[attrName];
                        doc.DocumentElement.ChildNodes.Item(0).ChildNodes.Item(0).ChildNodes[i].Attributes.Remove(rattr);
                    }
                }
            }
            //Persist bleach attributes with prefix:
            if (null == node.Attributes[attrName, BleachNamespace[1]])
            {
                XmlAttribute attr = doc.CreateAttribute(BleachPrefix, attrName, BleachNamespace[1]);

                attr.Value = value;

                node.Attributes.Append(attr);
            }
            else
            {
                node.Attributes[attrName, BleachNamespace[1]].Value = value;
            }
        }

        public static void SetBleachNamespace(XmlDocument doc)
        {
            try
            {
                XmlNode tempNode = doc.DocumentElement.Attributes[BleachNamespace[0]];
                if (null == tempNode)
                {
                    doc.DocumentElement.SetAttribute(BleachNamespace[0], BleachNamespace[1]);
                }
            }
            catch (Exception ex)
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, "SetBleachNamespace failed: " + ex.Message);
            }
        }

        public static void SetClkRate(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(ClkRateProperty, val);
        }

        public static void SetDwellTime(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(DwellTimeProperty, val);
        }

        public static void SetEpochCount(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(EpochCountProperty, val);
        }

        public static void SetFill(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(FillProperty, val);
        }

        public static void SetIterations(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(IterationsProperty, val);
        }

        public static void SetLongIdleTime(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(LongIdleTimeProperty, val);
        }

        public static void SetPixelMode(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(PixelModeProperty, val);
        }

        public static void SetPostCycleIdleMS(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(PostCycleIdleMSProperty, val);
        }

        public static void SetPostEpochIdleMS(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(PostEpochIdleMSProperty, val);
        }

        public static void SetPostIdleTime(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(PostIdleTimeProperty, val);
        }

        public static void SetPostPatIdleTime(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(PostPatIdleTimeProperty, val);
        }

        public static void SetPower(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(PowerProperty, val);
        }

        public static void SetPreCycleIdleMS(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(PreCycleIdleMSProperty, val);
        }

        public static void SetPreEpochIdleMS(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(PreEpochIdleMSProperty, val);
        }

        public static void SetPreIdleTime(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(PreIdleTimeProperty, val);
        }

        public static void SetPrePatIdleTime(UIElement element, string val)
        {
            if (element == null)
            {
                throw new ArgumentNullException("element");
            }
            element.SetValue(PrePatIdleTimeProperty, val);
        }

        #endregion Methods
    }
}