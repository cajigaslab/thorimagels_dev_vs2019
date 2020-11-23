namespace HardwareSetupUserControl
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
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

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for AddModality.xaml
    /// </summary>
    public partial class AddModality : Window, INotifyPropertyChanged
    {
        #region Fields

        private const string MODALITIES_FOLDER_NAME = "Modalities";

        private string _modalityName;

        #endregion Fields

        #region Constructors

        public AddModality()
        {
            InitializeComponent();
            DataContext = this;

            this.Loaded += AddModality_Loaded;
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string ModalityName
        {
            get
            {
                return _modalityName;
            }
            set
            {
                _modalityName = value;
                OnPropertyChanged("ModalityName");
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetMyDocumentsThorImageFolder", CharSet = CharSet.Unicode)]
        public static extern int GetMyDocumentsThorImageFolder(StringBuilder sb, int length);

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

        public string GetMyDocumentsThorImageFolderString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetMyDocumentsThorImageFolder(sb, PATH_LENGTH);

            return sb.ToString();
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

        protected virtual void OnPropertyChanged(String propertyName)
        {
            if (System.String.IsNullOrEmpty(propertyName))
            {
                return;
            }
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        void AddModality_Loaded(object sender, RoutedEventArgs e)
        {
            string modalitiesFolder = GetMyDocumentsThorImageFolderString() + "\\" + MODALITIES_FOLDER_NAME;

            if (Directory.Exists(modalitiesFolder))
            {
                string[] mods = Directory.GetDirectories(modalitiesFolder);

                foreach (string mod in mods)
                {
                    string str = new DirectoryInfo(mod).Name;
                    cbModality.Items.Add(str);
                }
            }
        }

        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            this.DialogResult = false;
            this.Close();
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            if (this.tbName.Text.Length > 0)
            {
                string basePath = GetMyDocumentsThorImageFolderString() + "\\" + MODALITIES_FOLDER_NAME;
                string newModalityPath;
                if (Directory.Exists(basePath + "\\" + tbName.Text))
                {
                    MessageBox.Show("Modality name already exists! Please choose a unique name");
                    return;
                }

                if (cbModality.SelectedIndex >= 0)
                {

                    try
                    {
                        newModalityPath = basePath + "\\" + tbName.Text;
                        Directory.CreateDirectory(newModalityPath);
                    }
                    catch (System.Exception ex)
                    {
                        MessageBox.Show("Modality folder creation failed!" + ex.Message);
                        return;
                    }

                    try
                    {
                        Copy(basePath + "\\" + cbModality.Items[cbModality.SelectedIndex].ToString(), newModalityPath);
                    }
                    catch (System.Exception ex)
                    {
                        MessageBox.Show("Modality folder copy failed!" + ex.Message);
                        return;
                    }
                }
                else
                {
                    MessageBox.Show("Please select a Copy Modality entry to add a new modality");
                    return;
                }
            }
            this.DialogResult = true;
            this.Close();
        }

        void Copy(string sourceDir, string targetDir)
        {
            Directory.CreateDirectory(targetDir);

            foreach (var file in Directory.GetFiles(sourceDir))
            {
                File.Copy(file, System.IO.Path.Combine(targetDir, System.IO.Path.GetFileName(file)));
                //remove TemplateScans node only for xml files
                string fileExtension = System.IO.Path.GetExtension(file);
                if (fileExtension == ".xml")
                {
                    XmlManager.RemoveNodeByName(System.IO.Path.Combine(targetDir, System.IO.Path.GetFileName(file)), "TemplateScans");
                }
            }

            foreach (var directory in Directory.GetDirectories(sourceDir))
                Copy(directory, System.IO.Path.Combine(targetDir, System.IO.Path.GetFileName(directory)));
        }

        #endregion Methods
    }
}