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
    using ThorSharedTypes;

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
            string hwSettingsFile = docFolder + "Application Settings\\HardwareSettings.xml";
            string appSettingsFile = docFolder + "Application Settings\\ApplicationSettings.xml";
            string appSettingsFolder = docFolder + "Application Settings";
            string templatesFolder = docFolder + "Capture Templates";
            string thorDatabase = docFolder + "Application Settings\\ThorDatabase.db";
            string zStackCacheFolder = docFolder + "ZStackCache";
            string tilesCacheFolder = docFolder + "TilesCache";
            string appRootFolder = docFolder;

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
                //Get the path to the documents folder from the Resource Manager
                //Resource Manager finds location of documents folder based on location given in XML. This can be different than the system documents folder. 
                string documentsFolderPathString = ResourceManagerCS.GetMyDocumentsThorImageFolderString();
                if (false == Directory.Exists(documentsFolderPathString))
                {
                    MessageBox.Show(String.Format("Application failed to locate the Documents Folder {0}. Application exiting.", docFolder), "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                    docFolder = string.Empty;
                }
                else 
                {
                    docFolder = documentsFolderPathString;
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