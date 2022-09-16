// ***********************************************************************
// Assembly         : ThorImageLS
// Author           : Thorlabs
// Created          : 04-27-2015
//
// Last Modified By : Thorlabs
// Last Modified On : 07-21-2015
// ***********************************************************************
// <copyright file="App.xaml.cs" company="Thorlabs">
//     Copyright ©  2010
// </copyright>
// <summary></summary>
// ***********************************************************************
/// <summary>
/// The ThorImage namespace.
/// </summary>
namespace ThorImage
{
    using System;
    using System.Collections.Generic;
    using System.Configuration;
    using System.Data;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Net;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading;
    using System.Windows;
    using System.Xml;

    using CustomMessageBox;

    using FolderDialogControl;

    using Microsoft.Practices;
    using Microsoft.Win32;

    using ThorLogging;

    using ThorSharedTypes;

    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        #region Fields

        /// <summary>
        /// The application start time
        /// </summary>
        private DateTime _appStartTime;
        private Bootstrapper _bootstrapper = null;

        #endregion Fields

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the App class.
        /// </summary>
        public App()
        {
            //Terminate any existing version of the app
            try
            {
                Application.Current.ShutdownMode = ShutdownMode.OnExplicitShutdown;

                System.Diagnostics.Process[] process = System.Diagnostics.Process.GetProcessesByName("ThorImageLS");

                System.Diagnostics.Process myprocess = System.Diagnostics.Process.GetCurrentProcess();

                //only try to delete files if no other instance of ThorImageLS is running
                if (process.Length <= 1)
                {
                    HandleLogFiles();
                }

                foreach (System.Diagnostics.Process p in process)
                {
                    StringBuilder sb = new StringBuilder();

                    sb.AppendFormat("Process id {0} MyProcess id {1}", p.Id, myprocess.Id);

                    ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + sb.ToString());

                    if (false == p.Id.Equals(myprocess.Id))
                    {
                        if (MessageBoxResult.Yes == MessageBox.Show("An existing version of ThorImageLS is running and must be shutdown. Do you want to close the existing ThorImageLS.exe?", "Close existing application", MessageBoxButton.YesNo, MessageBoxImage.Warning, MessageBoxResult.Yes))
                        {
                            ProcessUtility.KillTree(p.Id);
                            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Killing process");
                        }
                        else
                        {
                            ProcessUtility.KillTree(myprocess.Id);
                            return;
                        }
                    }

                }
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
            }

            //Search alternate paths for dependency files
            SetDependenciesPath();

            //assign the documents folder
            string docFolder = LocateDocumentsFolder();

            if (string.IsNullOrEmpty(docFolder))
            {
                Application.Current.Shutdown();
                return;
            }

            //assign other Resource strings
            string hwSettingsFile = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\Application Settings\\HardwareSettings.xml";
            string appSettingsFile = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\Application Settings\\ApplicationSettings.xml";
            string appSettingsFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\Application Settings";
            string templatesFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\Capture Templates";
            string thorDatabase = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\Application Settings\\ThorDatabase.db";
            string zStackCacheFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\ZStackCache";
            string tilesCacheFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\TilesCache";
            string appRootFolder = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder;
            string imageProcessSettingsFile = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString() + "\\" + docFolder + "\\Application Settings\\ImageProcessSettings.xml";
            string LightPathsFolder = templatesFolder + "\\LightPaths";
            Application.Current.Resources.Add("HardwareSettingsFile", hwSettingsFile);
            Application.Current.Resources.Add("ApplicationSettingsFile", appSettingsFile);
            Application.Current.Resources.Add("ImageProcessSettingsFile", imageProcessSettingsFile);
            Application.Current.Resources.Add("ApplicationSettingsFolder", appSettingsFolder);
            Application.Current.Resources.Add("TemplatesFolder", templatesFolder);
            Application.Current.Resources.Add("ThorDatabase", thorDatabase);
            Application.Current.Resources.Add("ZStackCacheFolder", zStackCacheFolder);
            Application.Current.Resources.Add("TilesCacheFolder", tilesCacheFolder);
            Application.Current.Resources.Add("AppRootFolder", appRootFolder);
            Application.Current.Resources.Add("LightPathListFolder", LightPathsFolder);

            System.Reflection.Assembly assembly = System.Reflection.Assembly.GetExecutingAssembly();

            Version version = assembly.GetName().Version;

            bool _hardwareConnectionsOpened = false;

            //Check that the hardware settings exists and is valid
            if (false == VerifyHardwareSettings())
            {
                MessageBox.Show("Application failed to load the HardwareSettings.xml file. Application exiting ...", appSettingsFolder, MessageBoxButton.OK, MessageBoxImage.Error);
                Application.Current.Shutdown();
                return;
            }

            //Check that the application settings exists and is valid
            if (false == VerifyApplicationSettings())
            {
                MessageBox.Show("Application failed to load the ApplicationSettings.xml file. Application exiting ...", appSettingsFolder, MessageBoxButton.OK, MessageBoxImage.Error);
                Application.Current.Shutdown();
                return;
            }

            if (false == VerifyImageProcessSettings())
            {
                MessageBox.Show("Application failed to load the ImageProcessSettings.xml file. Application exiting ...", appSettingsFolder, MessageBoxButton.OK, MessageBoxImage.Error);
                Application.Current.Shutdown();
                return;
            }

            if (false == SetActiveExperimentPath(templatesFolder))
            {
                return;
            }

            if (false == VerifyCTempFolder())
            {
                Application.Current.Shutdown();
                return;
            }

            if (false == VerifyThorDatabase())
            {
                MessageBox.Show("Application failed to load the experiment database. Application exiting ...", appSettingsFolder, MessageBoxButton.OK, MessageBoxImage.Error);
                Application.Current.Shutdown();
                return;
            }

            //check the fiji location
            VerifyFijiExe();

            //Register to the application exit event before the SelectHarware Window is displayed
            //to allow teardown of all devices in case the user exits the app from the SelectHardware Window
            Application.Current.Exit += new ExitEventHandler(Current_Exit);
            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                AboutDll.TabletSplashScreen splash = new AboutDll.TabletSplashScreen(String.Format("v{0}.{1}.{2}.{3}", version.Major, version.Minor, version.Build, version.Revision));

                splash.Hides = true;

                //Check for ThorImageLS updates
                UpdateCheck();

                try
                {
                    //Load the tablet splash screen
                    splash.Show();
                    if (0 == SelectHardwareSetupCommand())
                    {
                        splash.Close();
                        MessageBox.Show("Hardware failed to load. Could not connect to system.");
                    }

                    SetRuntimeStartStatistics();

                    StringBuilder sb = new StringBuilder();

                    SetupHardware setupHardware = new SetupHardware();

                    splash.Hide();
                    if (setupHardware.ShowDialog() == false)
                    {
                        Application.Current.Shutdown();
                        return;
                    }

                    splash.Show();

                    System.Threading.Thread.Sleep(2000);
                }
                catch (Exception ex)
                {
                    splash.Close();
                    MessageBox.Show(ex.Message, "Hardware failed to load. Could not connect to system.");
                }

                splash.Close();
            }
            else
            {
                _hardwareConnectionsOpened = false;
                AboutDll.SplashScreen splash = new AboutDll.SplashScreen(String.Format("v{0}.{1}.{2}.{3}", version.Major, version.Minor, version.Build, version.Revision), _hardwareConnectionsOpened);

                splash.Hides = true;

                //Check for ThorImageLS updates
                UpdateCheck();

                try
                {
                    //Load the splash screen
                    splash.Show();
                    if (0 == SelectHardwareSetupCommand())
                    {
                        splash.Close();
                        MessageBox.Show("Hardware failed to load. Could not connect to system.");
                    }

                    //Try to connect to PMT Switch Box at startup
                    ResourceManagerCS.Instance.ConnectToPMTSwitchBox();

                    SetRuntimeStartStatistics();

                    StringBuilder sb = new StringBuilder();

                    SetupHardware setupHardware = new SetupHardware();

                    splash.Hide();
                    if (setupHardware.ShowDialog() == false)
                    {
                        Application.Current.Shutdown();
                        return;
                    }

                    splash.Close();
                    _hardwareConnectionsOpened = true;
                    AboutDll.SplashScreen splashNew = new AboutDll.SplashScreen(String.Format("v{0}.{1}.{2}.{3}", version.Major, version.Minor, version.Build, version.Revision), _hardwareConnectionsOpened);
                    splashNew.Hides = true;
                    try
                    {
                        splashNew.Show();
                        System.Threading.Thread.Sleep(2000);
                    }
                    catch (Exception ex)
                    {
                        splashNew.Close();
                        MessageBox.Show(ex.Message, "Hardware failed to load. Could not connect to system.");
                    }
                    splashNew.Close();
                }
                catch (Exception ex)
                {
                    splash.Close();
                    MessageBox.Show(ex.Message, "Hardware failed to load. Could not connect to system.");
                }
            }

            //Deregister from Exit event to allow all other modules to be registed first.
            //This will ensure that the hardware teardown is done last.
            Application.Current.Exit -= new ExitEventHandler(Current_Exit);

            Application.Current.ShutdownMode = ShutdownMode.OnMainWindowClose;
            //Re-Register to Exit Event to do a harware teardown when exiting the app

            if (_bootstrapper  == null)
            {
                _bootstrapper = new Bootstrapper();
            }
            _bootstrapper.Run();

            _bootstrapper.ShowInitialPanel();

            Application.Current.MainWindow.WindowState = WindowState.Maximized;

            if (ResourceManagerCS.Instance.TabletModeEnabled)
            {
                Application.Current.MainWindow.WindowStyle = WindowStyle.None;
            }

            //Re-Register to Exit Event to do a harware teardown when exiting the app

            Application.Current.Exit += new ExitEventHandler(Current_Exit);

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Initialized");
        }

        #endregion Constructors

        #region Properties

        /*
         * Getter and Setter for the Attribute "FijiDontShowAgain" in the Application Settings
         * file. If the checkbox don't show again is checked then it sets the attribute to 1
         * otherwise it is set to 0 for false.
         */
        public bool FijiDontShowAgain
        {
            get
            {

                const bool DEFAULT_VALUE = false;

                XmlDocument appSettings = new XmlDocument();
                string appSettingsFile = GetApplicationSettingsFileString();
                appSettings.Load(appSettingsFile);

                XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/MsgBoxChk");

                if (null != node)
                {
                    string dontAskXmlValue = string.Empty;
                    GetAttribute(node, appSettings, "FijiDontShowAgain", ref dontAskXmlValue);
                    return dontAskXmlValue == "1";
                }
                else
                {
                    return DEFAULT_VALUE;
                }

            }

            set
            {

                //Open Document
                XmlDocument appSettings = new XmlDocument();
                string appSettingsFile = GetApplicationSettingsFileString();
                appSettings.Load(appSettingsFile);

                //Get Value String
                string valueString;
                if (value)
                    valueString = "1";
                else
                    valueString = "0";

                XmlNode node = appSettings.SelectSingleNode("/ApplicationSettings/MsgBoxChk");

                if (null != node)
                {
                    SetAttribute(node, appSettings, "FijiDontShowAgain", valueString);
                    appSettings.Save(appSettingsFile);
                }
                else
                {
                    CreateNode(appSettings, "MsgBoxChk");
                    node = appSettings.SelectSingleNode("/ApplicationSettings/MsgBoxChk");
                    if (null != node)
                    {
                        SetAttribute(node, appSettings, "FijiDontShowAgain", valueString);
                        appSettings.Save(appSettingsFile);
                    }
                }
            }
        }

        #endregion Properties

        #region Methods

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetApplicationSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetApplicationSettingsFilePathAndName(StringBuilder sb, int length);

        [DllImport(".\\ResourceManager.dll", EntryPoint = "GetHardwareSettingsFilePathAndName", CharSet = CharSet.Unicode)]
        public static extern int GetHardwareSettingsFilePathAndName(StringBuilder sb, int length);

        public string GetApplicationSettingsFileString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetApplicationSettingsFilePathAndName(sb, PATH_LENGTH);

            return sb.ToString();
        }

        public string GetHardwareSettingsFileString()
        {
            const int PATH_LENGTH = 261;
            StringBuilder sb = new StringBuilder(PATH_LENGTH);
            GetHardwareSettingsFilePathAndName(sb, PATH_LENGTH);

            return sb.ToString();
        }

        /// <summary>
        /// Determines whether [is valid XML] [the specified XML file].
        /// </summary>
        /// <param name="xmlFile">The XML file.</param>
        /// <returns><c>true</c> if [is valid XML] [the specified XML file]; otherwise, <c>false</c>.</returns>
        public bool IsValidXml(string xmlFile)
        {
            using (System.Xml.XmlTextReader xmlTextReader = new XmlTextReader(xmlFile))
            {
                try
                {
                    while (xmlTextReader.Read()) ;
                }
                catch
                {
                    return false;
                }
            }
            return true;
        }

        /// <summary>
        /// Sets the dependencies path.
        /// </summary>
        public void SetDependenciesPath()
        {
            try
            {
                string appPath = System.IO.Path.GetDirectoryName(System.Diagnostics.Process.GetCurrentProcess().MainModule.FileName);

                if (Directory.Exists(appPath))
                    SetDllDirectory(appPath + "\\Lib");

               String oldPath = Environment.GetEnvironmentVariable("PATH", EnvironmentVariableTarget.Process);

               if (false == oldPath.Contains(appPath + "\\Modules_Native"))
               {
                   Environment.SetEnvironmentVariable("PATH", oldPath + ";" + appPath + "\\Modules_Native", EnvironmentVariableTarget.Process);
               }

            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        /// <summary>
        /// Gets the camera identifier.
        /// </summary>
        /// <returns>System.Int32.</returns>
        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "GetCameraID")]
        private static extern int GetCameraID();

        /// <summary>
        /// Gets the device identifier.
        /// </summary>
        /// <param name="deviceType">Type of the device.</param>
        /// <returns>System.Int32.</returns>
        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "GetDeviceID")]
        private static extern int GetDeviceID(int deviceType);

        /// <summary>
        /// Locates the documents folder.
        /// </summary>
        /// <returns>System.String.</returns>
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
                node = rmDoc.SelectSingleNode("/ResourceManager/TabletMode");
                if (null != node && null != node.Attributes.GetNamedItem("value"))
                {
                    int tabletMode = 0;
                    if (Int32.TryParse(node.Attributes["value"].Value, out tabletMode))
                    {
                        ResourceManagerCS.Instance.TabletModeEnabled = (1 == tabletMode);
                    }
                }
            }
            return docFolder;
        }

        /// <summary>
        /// Selects the hardware setup command.
        /// </summary>
        /// <returns>System.Int32.</returns>
        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "SetupCommand")]
        private static extern int SelectHardwareSetupCommand();

        /// <summary>
        /// Selects the hardware teardown command.
        /// </summary>
        /// <returns>System.Int32.</returns>
        [DllImport(".\\Modules_Native\\SelectHardware.dll", EntryPoint = "TeardownCommand")]
        private static extern int SelectHardwareTeardownCommand();

        /// <summary>
        /// Sets the active experiment.
        /// </summary>
        /// <param name="path">The path.</param>
        /// <returns>System.Int32.</returns>
        [DllImport(".\\ExperimentManager.dll", EntryPoint = "SetActiveExperiment", CharSet = CharSet.Unicode)]
        private static extern int SetActiveExperiment(string path);

        /// <summary>
        /// Sets the DLL directory.
        /// </summary>
        /// <param name="lpPathName">Name of the lp path.</param>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern bool SetDllDirectory(string lpPathName);

        /// <summary>
        /// Browses the temporary folder.
        /// </summary>
        /// <param name="hardwareDoc">The hardware document.</param>
        /// <param name="node">The node.</param>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        private bool BrowseTempFolder(XmlDocument hardwareDoc, XmlNode node)
        {
            FolderDialogControl.BrowseForFolderDialog folderDlg = new BrowseForFolderDialog();

            do
            {
                if (true == folderDlg.ShowDialog())
                {
                    if (Directory.Exists(folderDlg.SelectedFolder))
                    {
                        SetAttribute(node, hardwareDoc, "path", folderDlg.SelectedFolder);
                        break;
                    }
                }
            }
            while (true);

            return true;
        }

        //Function that downloads the ThorImageLS.xml to compare current version to newest version release
        void CheckForUpdates()
        {
            ServicePointManager.Expect100Continue = true;
            ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12;

            using (WebClient client = new WebClient())
            {
                client.DownloadDataAsync(new Uri("https://www.thorlabs.com/Software/ThorImageLS%20Reference/ThorImageLS.xml"), ResourceManagerCS.GetMyDocumentsThorImageFolderString());
                client.DownloadDataCompleted += client_DownloadDataCompleted;
            }
        }

        //Function that compares current version to newest release version and opens a messagebox informing the user of an available update
        void client_DownloadDataCompleted(object sender, DownloadDataCompletedEventArgs e)
        {
            if (e.Error != null)
            {
                MessageBox.Show("Unable to check for updates. Please check your network connection.");
                return;
            }

            byte[] raw = e.Result;
            string result = System.Text.Encoding.UTF8.GetString(raw);

            XmlDocument doc = new XmlDocument();
            XmlDocument doc2 = new XmlDocument();

            doc.LoadXml(result);
            doc2.Load(".//ResourceManager.xml");

            XmlNodeList ndList = doc.SelectNodes("/Software/Version");
            XmlNode node = doc2.SelectSingleNode("/ResourceManager/ThorImageVersionCheck");

            string str = string.Empty;
            string str2 = string.Empty;

            XmlManager.GetAttribute(node, doc2, "stopUpdateCheck", ref str2);

            if (XmlManager.GetAttribute(ndList[0], doc, "value", ref str))
            {
                Version v = Assembly.GetExecutingAssembly().GetName().Version;
                Version v_p = new Version(str);

                string strMessage = string.Format("");

                //Only displays the message box if a new version is available
                if (v_p.CompareTo(v) > 0)
                {
                    strMessage = string.Format("New Version Available!\rThorImageLS version available {0}\rCurrent ThorImageLS version {1}.{2}.{3}.{4}\rContact ImagingTechSupport@thorlabs.com today to schedule your free software upgrade", str, v.Major, v.Minor, v.Build, v.Revision);

                    Dispatcher.Invoke(() =>
                    {
                        CustomMessageBox messageBox = new CustomMessageBox(strMessage, "Check For Updates", "Stop displaying this message", "Ok");
                        //Displays Check For Updates message box and changes the stopUpdateCheck to 1 if the checkbox was pressed upon closing the window
                        if (messageBox.ShowDialog().GetValueOrDefault(true))
                        {
                            if (messageBox.CheckBoxChecked)
                            {
                                node.Attributes[1].Value = "1";
                                doc2.Save("ResourceManager.xml");
                            }
                        }
                    }, System.Windows.Threading.DispatcherPriority.Normal);
                }
            }
        }

        //Function that takes the date since the checkbox was checked and returns true if 90 days have passed since then
        int CompareDates(string newDate, string oldDate)
        {
            //splits the dates from string "month/day/year" into three separate int values
            string[] splitNew = newDate.Split('/');
            string[] splitOld = oldDate.Split('/');
            int[] newDateInt = new int[3];
            int[] oldDateInt = new int[3];
            for (int i = 0; i < 3; i++)
            {
                newDateInt[i] = int.Parse(splitNew[i]);
                oldDateInt[i] = int.Parse(splitOld[i]);
            }

            //Assigning each date indice to a more recognizable name
            int newDateYear = newDateInt[2];
            int newDateMonth = newDateInt[0];
            int newDateDay = newDateInt[1];
            int oldDateYear = oldDateInt[2];
            int oldDateMonth = oldDateInt[0];
            int oldDateDay = oldDateInt[1];

            //number of days in each month (no leap year)
            int[] dayNumber = new int[] { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

            //Get the total number of days that have passed for the new and old dates since BC (assuming 365 days in a year)
            int newDaysInMonths = 0;
            for (int i = 0; i < (newDateMonth - 1); i++)
            {
                newDaysInMonths += dayNumber[i];
            }
            int numberNewDays = (newDateYear * 365) + newDaysInMonths + newDateDay;

            int oldDaysInMonths = 0;
            for (int i = 0; i < (oldDateMonth - 1); i++)
            {
                oldDaysInMonths += dayNumber[i];
            }
            int numberOldDays = (oldDateYear * 365) + oldDaysInMonths + oldDateDay;

            //If the number of days since the last update exceeds the cutoff, return TRUE
            int cutoffDays = 90;
            if ((numberNewDays - numberOldDays) >= cutoffDays)
            {
                return 1;
            }
            return 0;
        }

        /*
         *  CreateNode function creates a new node in an XML file, currently is only being used
         *  in the FijiDontShowAgain setter.
         */
        private void CreateNode(XmlDocument doc, string nodeName)
        {
            XmlNode node = doc.CreateNode(XmlNodeType.Element, nodeName, null);
            doc.DocumentElement.AppendChild(node);
        }

        /// <summary>
        /// Handles the Exit event of the Current control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="ExitEventArgs"/> instance containing the event data.</param>
        void Current_Exit(object sender, ExitEventArgs e)
        {
            if (0 == SelectHardwareTeardownCommand())
            {
                MessageBox.Show("Could not disconnect all the hardware system properly.");
            }

            SetRuntimeEndStatistics();

            ThorLog.Instance.TraceEvent(TraceEventType.Verbose, 1, this.GetType().Name + " Exiting");
        }

        /// <summary>
        /// Gets the attribute.
        /// </summary>
        /// <param name="node">The node.</param>
        /// <param name="doc">The document.</param>
        /// <param name="attrName">Name of the attribute.</param>
        /// <param name="attrValue">The attribute value.</param>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        private bool GetAttribute(XmlNode node, XmlDocument doc, string attrName, ref string attrValue)
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

        /// <summary>
        /// Handles the log files
        /// </summary>
        void HandleLogFiles()
        {
            try
            {
                string appPath = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
                string pleoraDmp = appPath + "\\pleora_ebus.dmp";

                if (File.Exists(pleoraDmp))
                {
                    File.Delete(pleoraDmp);
                }
            }
            catch(Exception ex)
            {
                ex.ToString();
            }

            try
            {
                string appPath = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
                string thordaqLog = appPath +  "\\ThorLog.log";
                const long MAX_LOG_FILE_SIZE = 536870912; //512MB
                if (File.Exists(thordaqLog))
                {
                    long fileSize = new FileInfo(thordaqLog).Length;
                    if (MAX_LOG_FILE_SIZE <= fileSize)
                    {
                        var lastModified = File.GetLastWriteTime(thordaqLog);
                        string newThorLogName = appPath + "\\ThorLog" + lastModified.ToString("yyyyMMdd_HHmmss") + ".log";
                        System.IO.File.Move(thordaqLog, newThorLogName);
                    }
                }
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        /// <summary>
        /// Sets the active experiment path.
        /// </summary>
        /// <param name="folder">The folder.</param>
        /// <returns>Boolean.</returns>
        Boolean SetActiveExperimentPath(string folder)
        {
            string activeFile = folder + "\\Active.xml";

            if (File.Exists(activeFile))
            {
                if (IsValidXml(activeFile))
                {
                    SetActiveExperiment(activeFile);
                    return true;
                }
                else
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " the system has an invalid Active.xml file.");
                    File.Delete(activeFile);
                    File.Copy(folder + "\\Default.xml", activeFile);
                    SetActiveExperiment(activeFile);
                    return true;
                }
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " the system is missing the Active.xml file.");
                MessageBox.Show("Application failed to load the Active.xml file. Application exiting ...", folder, MessageBoxButton.OK, MessageBoxImage.Error);
                Application.Current.Shutdown();
                return false;

            }
        }

        /// <summary>
        /// Sets the attribute.
        /// </summary>
        /// <param name="node">The node.</param>
        /// <param name="doc">The document.</param>
        /// <param name="attrName">Name of the attribute.</param>
        /// <param name="attrValue">The attribute value.</param>
        private void SetAttribute(XmlNode node, XmlDocument doc, string attrName, string attrValue)
        {
            if (null == node.Attributes.GetNamedItem(attrName))
            {
                XmlAttribute attr = doc.CreateAttribute(attrName);
                attr.Value = attrValue;
                node.Attributes.Append(attr);
            }

            node.Attributes[attrName].Value = attrValue;
        }

        /// <summary>
        /// Sets the runtime end statistics.
        /// </summary>
        void SetRuntimeEndStatistics()
        {
            XmlDocument appDoc = new XmlDocument();

            string appFile = GetApplicationSettingsFileString();

            appDoc.Load(appFile);

            XmlNodeList ndList = appDoc.SelectNodes("/ApplicationSettings/Runtime");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if (GetAttribute(ndList[0], appDoc, "end", ref str))
                {
                    int end = Convert.ToInt32(str);

                    end++;

                    SetAttribute(ndList[0], appDoc, "end", end.ToString());
                }

                if (GetAttribute(ndList[0], appDoc, "runTime", ref str))
                {
                    int endTotalSeconds = (int)DateTime.Now.Subtract(_appStartTime).TotalSeconds;
                    int previousTotal = Convert.ToInt32(str);
                    previousTotal += endTotalSeconds;
                    SetAttribute(ndList[0], appDoc, "runTime", previousTotal.ToString());
                }

                string appSettPath = GetApplicationSettingsFileString();
                appDoc.Save(appSettPath);
            }
        }

        /// <summary>
        /// Sets the runtime start statistics.
        /// </summary>
        void SetRuntimeStartStatistics()
        {
            string appFile = GetApplicationSettingsFileString();
            if (!File.Exists(appFile))
                return;
            _appStartTime = DateTime.Now;

            XmlDocument appDoc = new XmlDocument();

            appDoc.Load(appFile);

            XmlNodeList ndList = appDoc.SelectNodes("/ApplicationSettings/Runtime");

            if (ndList.Count > 0)
            {
                string str = string.Empty;

                if(GetAttribute(ndList[0],appDoc, "start", ref str))
                {
                    int start = Convert.ToInt32(str);

                    start++;

                    SetAttribute(ndList[0],appDoc, "start", start.ToString());

                    string appSettPath = GetApplicationSettingsFileString();
                    appDoc.Save(appSettPath);
                }
            }
        }

        void UpdateCheck()
        {
            //load the ResourceManager xml for the lastCheckDate and stopUpdateCheck values
            XmlDocument doc = new XmlDocument();
            doc.Load(".//ResourceManager.xml");
            XmlNode node = doc.SelectSingleNode("/ResourceManager/ThorImageVersionCheck");
            string str = string.Empty;
            string str2 = string.Empty;
            DateTime thisDay = DateTime.Today;
            string newDay = thisDay.ToString("d");
            int xmlUpdateCheck = 0;
            if (XmlManager.GetAttribute(node, doc, "lastCheckDate", ref str))
            {
                //If the last update date is blank (default), fill it in with today's date
                if (String.IsNullOrEmpty(str))
                {
                    node.Attributes[0].Value = newDay;
                    doc.Save("ResourceManager.xml");
                }

                if (XmlManager.GetAttribute(node, doc, "stopUpdateCheck", ref str2))
                {
                    xmlUpdateCheck = int.Parse(str2);
                    //If the Checkbox is checked, run CheckForUpdates, reset the stopUpdateCheck value to 0, and update the lastCheckDate to today- only if 90 days have passed
                    if (xmlUpdateCheck == 1)
                    {
                        if (CompareDates(newDay, str) == 1)
                        {
                            node.Attributes[1].Value = "0";
                            node.Attributes[0].Value = newDay;
                            doc.Save("ResourceManager.xml");
                            CheckForUpdates();
                        }
                    }
                    else
                    {
                        //If the Checkbox is not checked, run CheckForUpdates and update the lastCheckDate to today
                        node.Attributes[0].Value = newDay;
                        doc.Save("ResourceManager.xml");
                        CheckForUpdates();
                    }
                }
            }
        }

        /// <summary>
        /// Updates the fiji check application setting.
        /// </summary>
        void UpdateFijiCheckAppSetting()
        {
            XmlDocument appSettingsDoc = new XmlDocument();

            string appSettPath = GetApplicationSettingsFileString();

            if(false == File.Exists(appSettPath))
            {
                return;
            }
            appSettingsDoc.Load(appSettPath);

            XmlNode node = appSettingsDoc.SelectSingleNode("/ApplicationSettings/BrowseFijiExePath");

            if (node == null)
            {
                node = appSettingsDoc.CreateNode(XmlNodeType.Element, "BrowseFijiExePath", null);
                appSettingsDoc.DocumentElement.AppendChild(node);
            }

            SetAttribute(node, appSettingsDoc, "value", "NO");
            appSettingsDoc.Save(appSettPath);
        }

        /// <summary>
        /// Verifies the application settings.
        /// </summary>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        bool VerifyApplicationSettings()
        {
            XmlDocument appDoc = new XmlDocument();

            string appFile = GetApplicationSettingsFileString();

            if (File.Exists(appFile))
            {
                if (IsValidXml(appFile))
                {
                    appDoc.Load(appFile);
                }
                else
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " the system has an invalid ApplicationSettings.xml file.");
                    return false;
                }
            }
            else
            {
                return false;
            }
            return true;
        }

        /// <summary>
        /// Verifies the c temporary folder.
        /// </summary>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        bool VerifyCTempFolder()
        {
            XmlDocument hardwareDoc = new XmlDocument();

            string hardwareFile = GetHardwareSettingsFileString();

            hardwareDoc.Load(hardwareFile);

            XmlNodeList ndList = hardwareDoc.SelectNodes("/HardwareSettings/Streaming");

            //check the directory stored in the settings file
            if (ndList.Count > 0)
            {
                string tempFolder = string.Empty;

                if (true == GetAttribute(ndList[0], hardwareDoc, "path", ref tempFolder))
                {
                    //if the directory exists return
                    if (Directory.Exists(tempFolder))
                    {
                        return true;
                    }
                }
            }

            XmlNode node = hardwareDoc.SelectSingleNode("/HardwareSettings");

            //create the streaming node if it is missing
            if (node == null)
            {
                node = hardwareDoc.CreateNode(XmlNodeType.Element, "Streaming", null);
                hardwareDoc.DocumentElement.AppendChild(node);
            }

            node = hardwareDoc.SelectSingleNode("/HardwareSettings/Streaming");

            MessageBox.Show("Temporary folder is missing. You must select/create one (Ex.C:\\Temp) before starting the application.", "Temporary Folder", MessageBoxButton.OK);

            BrowseTempFolder(hardwareDoc, node);

            hardwareDoc.Save(hardwareFile);

            return true;
        }

        /// <summary>
        /// Verifies the fiji executable.
        /// </summary>
        void VerifyFijiExe()
        {
            XmlDocument commandListDoc = new XmlDocument();

            string commandListPath = Application.Current.Resources["ApplicationSettingsFolder"].ToString() + "\\CommandList.xml";

            if (false == File.Exists(commandListPath))
            {
                return;
            }

            commandListDoc.Load(commandListPath);

            XmlNode node2 = commandListDoc.SelectSingleNode("/CommandList/Command/FijiExe");

            string fijiExePath;

            if (null != node2 && null != node2.Attributes.GetNamedItem("value"))
            {
                fijiExePath = node2.Attributes["value"].Value;
                if (File.Exists(fijiExePath))
                {
                }
                else
                {
                    if (!FijiDontShowAgain)
                    {
                        CustomMessageBox messageBox = new CustomMessageBox("The location for the Fiji Excutable is wrong or no location has been provided. Do you wish to update the location now?", "Incorrect path for Fiji", "Don't ask again", "Yes", "No");
                        if (messageBox.ShowDialog().GetValueOrDefault(false))
                        {
                            OpenFileDialog dlg = new OpenFileDialog();
                            dlg.Filter = "Executable|*.exe";
                            if (true == dlg.ShowDialog())
                            {
                                SetAttribute(node2, commandListDoc, "value", dlg.FileName);
                                commandListDoc.Save(commandListPath);
                            }
                        }
                        else
                        {
                            FijiDontShowAgain = messageBox.CheckBoxChecked;
                        }
                    }
                }
            }
            else
            {
                return;
            }
        }

        /// <summary>
        /// Verifies the hardware settings.
        /// </summary>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        bool VerifyHardwareSettings()
        {
            XmlDocument hardwareDoc = new XmlDocument();

            string hardwareFile = GetHardwareSettingsFileString();

            if (File.Exists(hardwareFile))
            {
                if (IsValidXml(hardwareFile))
                {
                    hardwareDoc.Load(hardwareFile);
                }
                else
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " the system has an invalid HardwareSettings.xml file.");
                    return false;
                }
            }
            else
            {
                return false;
            }
            return true;
        }

        bool VerifyImageProcessSettings()
        {
            XmlDocument imageProcessDoc = new XmlDocument();

            string imageProcessFile = Application.Current.Resources["ImageProcessSettingsFile"].ToString();

            if (File.Exists(imageProcessFile))
            {
                if (IsValidXml(imageProcessFile))
                {
                    imageProcessDoc.Load(imageProcessFile);
                }
                else
                {
                    ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " the system has an invalid ImageProcessSettings.xml file.");
                    return false;
                }
            }
            else
            {
                return false;
            }
            return true;
        }

        /// <summary>
        /// Verifies the thor database.
        /// </summary>
        /// <returns><c>true</c> if XXXX, <c>false</c> otherwise.</returns>
        bool VerifyThorDatabase()
        {
            XmlDocument appDoc = new XmlDocument();

            string appFile = Application.Current.Resources["ThorDatabase"].ToString();

            if (File.Exists(appFile))
            {
                return true;
            }
            else
            {
                ThorLog.Instance.TraceEvent(TraceEventType.Error, 1, this.GetType().Name + " the system was unable to load the ThorDatabase.db file.");
                return false;
            }
        }

        #endregion Methods
    }
}