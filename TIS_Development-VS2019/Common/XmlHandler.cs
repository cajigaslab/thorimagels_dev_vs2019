namespace XMLHandle
{
    using System;
    using System.IO;
    using System.Xml;

    public class XMLHandler
    {
        #region Fields

        private object _resource;
        private string _settingsFile = string.Empty;

        #endregion Fields

        #region Constructors

        public XMLHandler(object target)
        {
            this.Resource = target;
        }

        #endregion Constructors

        #region Properties

        public object Resource
        {
            get { return _resource; }
            set
            {
                _resource = value;
                if (null != _resource)
                {
                    _settingsFile = _resource.ToString();
                }
            }
        }

        #endregion Properties

        #region Methods

        public bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
        {
            bool ret;

            if (null == node.Attributes.GetNamedItem(attrName))
            {
                ret = false;
            }
            else
            {
                attrValue = node.Attributes[attrName].Value;
                ret = true;
            }

            return ret;
        }

        public bool Load(XmlDocument doc)
        {
            if (File.Exists(_settingsFile))
            {
                doc.Load(_settingsFile);
                return true;
            }
            else
            {
                return false;
            }
        }

        public bool Save(XmlDocument doc)
        {
            if (File.Exists(_settingsFile))
            {
                doc.Save(_settingsFile);
                return true;
            }
            else
            {
                return false;
            }
        }

        //assign the attribute value to the input node and document
        //if the attribute does not exist add it to the document
        public void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attValue)
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