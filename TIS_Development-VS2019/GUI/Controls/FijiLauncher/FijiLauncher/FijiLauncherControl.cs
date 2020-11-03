#region Header

//-------------------------------------------------------------------------------
//to use this assembly, three arguments need to be clarified in main application:
//
//  1. _fijiExeFile
//      -- the location of your fiji application.
//          ex. _fijiExeFile = "D:\\fiji\fiji.exe";
//
//  2. _ijmFile
//      -- the ImageJ/fiji macro file you'd like to run.
//          ex. _ijmFile = "D:\\fiji\macros\\FFTBatch.ijm";
//
//  3. _inputDir
//      -- the folder containing the image you'd like to process with macro.
//          ex. _inputDir = "D:\\\ImageJMacroTestImages\\";
//--------------------------------------------------------------------------------

#endregion Header

namespace FijiLauncher
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Windows;

    #region Delegates

    // a delegate type for hooking up change notifications.
    public delegate void IjmFinishedEventHandler(object sender, EventArgs e);

    #endregion Delegates

    public class FijiLauncherControl
    {
        #region Fields

        private string _doneStr; // progress indicator string in log file, ex "Procedure is finished."
        private string _fijiExeFile; // location of fiji.exe/imageJ.exe
        private FileStream _fileStream = null;
        private bool _headless; // option of headless mode
        private string _ijmFile; // location of macro file
        private string _inputDir; // input directory
        private bool _isLogOn; // option of logging
        private string _logFile; // location of log file (ex. log.txt)
        System.Timers.Timer _logFileCheckTimer; // log file status check timer; reading every 1/4 second
        private string _outputDir; // output directory
        private string _outputName;
        private bool _processFinished; // macro process finished
        private bool _processOn; // processing carrying on
        StringBuilder _sbNewDir;
        private StreamReader _streamReader = null;
        private string _streamString;

        #endregion Fields

        #region Constructors

        public FijiLauncherControl()
        {
            _ijmFile = "";
            _fijiExeFile = "";
            _logFile = "";
            _inputDir = "";
            _outputDir = "";
            _isLogOn = true;
            _processOn = false;
            _processFinished = false;               // flag showing if the macro is done executing
            _headless = true;                       // headless mode, without see ImageJ UI
            _doneStr = "Procedure is finished.";    // string that gets written in ijm to tell the macro is done
            _outputName = "Output001";

            _logFileCheckTimer = new System.Timers.Timer(250);  // periodically reading log.txt 4 times per sec
            _logFileCheckTimer.Enabled = true;
            _logFileCheckTimer.Elapsed += new System.Timers.ElapsedEventHandler(_logFileCheckTimer_Elapsed);
        }

        #endregion Constructors

        #region Events

        public event IjmFinishedEventHandler IjmFinished;

        #endregion Events

        #region Properties

        public string FijiExeFile
        {
            get { return _fijiExeFile; }
            set { _fijiExeFile = value; }
        }

        public bool Headless
        {
            get { return _headless; }
            set { _headless = value; }
        }

        public string IjmFile
        {
            get { return _ijmFile; }
            set { _ijmFile = value; }
        }

        public string InputDir
        {
            get { return _inputDir; }
            set
            {
                _inputDir = value;
                _inputDir = _inputDir.Remove(_inputDir.Length - 1);
            }
        }

        public bool IsLogOn
        {
            get { return _isLogOn; }
            set { _isLogOn = value; }
        }

        public string LogFile
        {
            get { return _logFile; }
            set { _logFile = value; }
        }

        public string OutputDir
        {
            get { return _outputDir; }
            set { _outputDir = value; }
        }

        public bool ProcessFinished
        {
            get { return _processFinished; }
            set { _processFinished = value; }
        }

        public bool ProcessOn
        {
            get { return _processOn; }
            set { _processOn = value; }
        }

        public string StatusString
        {
            get
            {
                return _streamString;
            }
        }

        #endregion Properties

        #region Methods

        public bool CheckStatuts()
        {
            bool ret;

            if (null == _fileStream)
            {
                _fileStream = new FileStream(_logFile, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);

                _streamReader = new StreamReader(_fileStream);
            }

            if (!_streamReader.EndOfStream)
            {
                _streamString += _streamReader.ReadLine();
                _streamString += "\n";
            }

            if (_streamString.Contains(_doneStr))
            {
                ret = true;
            }
            else
            {
                ret = false;
            }

            //            var lastLine = File.ReadAllLines(_logFile).Last();

            //            ret = String.Compare(lastLine, _doneStr) == 0 ? true : false;

            return ret;
        }

        public void EchoMessageBox()
        {
        }

        public long LaunchFiji()
        {
            long ret = 0;

            string headlessOption;

            _processFinished = false;

            _logFileCheckTimer.Start();

            if (_headless)
            {
                headlessOption = "--headless";
            }
            else
            {
                headlessOption = "";
            }

            _streamString = string.Empty;

            //  _outputDir = _inputDir + "\\Output";
            //  _outputDir = _outputDir.Replace("\\\\", "\\");

            string oldFolder = _outputDir.Replace(_outputName + "", "");
            string newFolder = _inputDir + "\\";

            if (newFolder != oldFolder)
            {
                _outputName = "Output001";
            }

            _sbNewDir = new StringBuilder(_inputDir + "\\" + _outputName);

            if (Directory.Exists(_sbNewDir.ToString()))
            {
                string str = _outputName;
                do
                {
                    str = CreateUniqueName(str);
                }
                while (Directory.Exists(_inputDir + "\\" + str));

                _outputName = str;
                _sbNewDir = new StringBuilder(_inputDir + "\\" + str);
            }

            //Create the new experiment directory(s)
            _outputDir = _sbNewDir.ToString();
            Directory.CreateDirectory(_outputDir);

            _inputDir = _inputDir + "?" + _outputDir;   // ? as the input output path seperator

            string redirStr = "";
            if (_isLogOn)
            {
                _logFile = _outputDir + "\\log.txt";
              //  _logFile = _outputDir.Replace("\\\\", "\\");
                redirStr = ">";
            }
            else
            {
                _logFile = "";
                redirStr = "";
            }

            /*------------------------------- command line argument format -------------------------------
               * in this c# code, the file paths have to be
              1. /C                   command line parameter, exit after execute
              2. \"                   handles escape characters in entire command line
              3. \"{0}\"              double quoted path for fiji/imagej exe file
              4. {1}                  headless option, --headless, or nothing
              5. \"{2}\"              double quoted path for imagej/fiji macro file
              6. \"{3}\"              double quoted path for input file/images
              7. {4}                  >
              8. \"{5}\"              double quoted path for log.txt file
               * -------------------------------------------------------------------------------------------
               * in imagej/fiji macro, the argument has to be with a file seperator,
               * for example
               *
               * macro "blobCounter"
            {
                name = getArgument;
                input_dir = name + File.separator;
                output_dir = input_dir + "Output\\";
                ...
            }
             ---------------------------------------------------------------------------------------------*/

            string fijiCmdText = string.Format("/C \"\"{0}\" {1} -macro \"{2}\" \"{3}\" {4} \"{5}\"", _fijiExeFile, headlessOption, _ijmFile, _inputDir, redirStr, _logFile);

            //SequentualOutputFolderName();

            //if (_isLogOn)
            //{
            //    _logFile = _outputDir + "\\log.txt";
            //    _logFile = _logFile.Replace("\\\\", "\\");

            //    fijiCmdText += " > " + _logFile;
            //}

            try
            {
                System.Diagnostics.Process process = new System.Diagnostics.Process();
                System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
                startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
                startInfo.FileName = "cmd.exe";
                //startInfo.Arguments = "/C \"\"C:\\Users\\ncui.THORLABS\\Favorites\\Downloads\\fiji.app (1)\\fiji.exe\" -macro \"C:\\Users\\ncui.THORLABS\\Favorites\\Downloads\\fiji.app (1)\\macros\\partana.txt\" \"C:\\Users\\ncui.THORLABS\\Favorites\\Downloads\\fiji.app (1)\\blobimg?C:\\Users\\ncui.THORLABS\\Favorites\\Downloads\\fiji.app (1)\\blobimg\\Output009\" > \"C:\\Users\\ncui.THORLABS\\Favorites\\Downloads\\fiji.app (1)\\blobimg\\log.txt\"";

                startInfo.Arguments = fijiCmdText;
                process.StartInfo = startInfo;
                process.Start();
                _processOn = true;
                //   process.WaitForExit();

                ret = 1;
            }
            catch (Exception ex)
            {
                string str = ex.Message;
                ret = 0;
            }

            return ret;
        }

        // Invoke the Changed event; called whenever list changes
        protected virtual void OnIjmFinished(EventArgs e)
        {
            if (IjmFinished != null)
                IjmFinished(this, e);
        }

        private string CreateUniqueName(string str)
        {
            Match match = Regex.Match(str, "(.*)([0-9]{3,})");

            if (match.Groups.Count > 2)
            {
                int val = Convert.ToInt32(match.Groups[2].Value);

                val++;

                str = match.Groups[1].Value + String.Format("{0:000}", val);
            }
            else
            {
                str = str + "001";
            }

            return str;
        }

        void _logFileCheckTimer_Elapsed(object sender, EventArgs e)
        {
            if (_processOn && IsLogOn)
            {
                try
                {
                    _processFinished = CheckStatuts();

                    if (_processFinished)
                    {
                        OnIjmFinished(EventArgs.Empty);
                        _processOn = false;
                        _logFileCheckTimer.Stop();
                        _streamReader.Close();
                        _fileStream.Close();
                        _fileStream = null;
                        _streamReader = null;

                    }
                }
                catch (Exception ex)
                {
                    string str = ex.Message;

                }
            }
        }

        #endregion Methods
    }
}