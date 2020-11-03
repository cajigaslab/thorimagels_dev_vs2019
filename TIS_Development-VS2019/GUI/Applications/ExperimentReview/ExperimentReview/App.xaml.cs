namespace ExperimentReview
{
    using System;
    using System.Collections.Generic;
    using System.Configuration;
    using System.Data;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Xml;

    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        #region Constructors

        public App()
        {
            string docFolder = LocateDocumentsFolder();

            if (string.IsNullOrEmpty(docFolder))
            {
                Application.Current.Shutdown();
                return;
            }
            SetDependenciesPath();
            string hwSettingsFile = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\Application Settings\\HardwareSettings.xml";
            string appSettingsFile = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\Application Settings\\ApplicationSettings.xml";
            string appSettingsFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\Application Settings";
            string templatesFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\Capture Templates";
            string thorDatabase = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\Application Settings\\ThorDatabase.db";
            string zStackCacheFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\ZStackCache";
            string tilesCacheFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\TilesCache";
            string appRootFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder;

            Application.Current.Resources.Add("HardwareSettingsFile", hwSettingsFile);
            Application.Current.Resources.Add("ApplicationSettingsFile", appSettingsFile);
            Application.Current.Resources.Add("ApplicationSettingsFolder", appSettingsFolder);
            Application.Current.Resources.Add("TemplatesFolder", templatesFolder);
            Application.Current.Resources.Add("ThorDatabase", thorDatabase);
            Application.Current.Resources.Add("ZStackCacheFolder", zStackCacheFolder);
            Application.Current.Resources.Add("TilesCacheFolder", tilesCacheFolder);
            Application.Current.Resources.Add("AppRootFolder", appRootFolder);
        }

        #endregion Constructors

        #region Methods

        private static string LocateDocumentsFolder()
        {
            XmlDocument rmDoc = new XmlDocument();

            string docFolder = string.Empty;

            string rmPath = ".\\ResourceManager.xml";

            if (true == File.Exists(rmPath))
            {
                rmDoc.Load(rmPath);

                XmlNode node = rmDoc.SelectSingleNode("/ResourceManager/DocumentsFolder");

                if (null != node && null != node.Attributes.GetNamedItem("value"))
                {
                    docFolder = node.Attributes["value"].Value;
                    if (false == Directory.Exists(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder))
                    {
                        MessageBox.Show(String.Format("Application failed to locate the Documents Folder {0}. Application exiting.", docFolder), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                        docFolder = string.Empty;
                    }
                }
            }
            return docFolder;
        }

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern bool SetDllDirectory(string lpPathName);

        private void SetDependenciesPath()
        {
            try
            {
                string appPath = System.IO.Path.GetDirectoryName(System.Diagnostics.Process.GetCurrentProcess().MainModule.FileName);

                if (Directory.Exists(appPath))
                    SetDllDirectory(appPath + "\\Lib");

            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        #endregion Methods
    }
}