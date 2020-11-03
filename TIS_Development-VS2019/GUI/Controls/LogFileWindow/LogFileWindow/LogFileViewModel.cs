namespace LogFileWindow.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Media;

    using ThorSharedTypes;

    public class LogMessage
    {
        #region Fields

        private Brush _color;
        private string _logText;

        #endregion Fields

        #region Properties

        public Brush DisplayColor
        {
            get
            {
                return _color;
            }
            set
            {
                _color = value;
            }
        }

        public string LogText
        {
            get
            {
                return _logText;
            }
            set
            {
                _logText = value;
            }
        }

        #endregion Properties
    }

    class LogFileViewModel : ThorSharedTypes.VMBase
    {
        #region Fields

        const int MAX_LINES_BEHIND = 10000;

        BackgroundWorker _bw = new BackgroundWorker();
        private ObservableCollection<LogMessage> _logMessages;
        private string _logText = string.Empty;
        private int _messageIndex = 0;
        private bool _run;

        #endregion Fields

        #region Constructors

        public LogFileViewModel()
        {
            _logMessages = new ObservableCollection<LogMessage>();
        }

        #endregion Constructors

        #region Properties

        public ObservableCollection<LogMessage> LogMessages
        {
            get
            {
                return _logMessages;
            }
            set
            {
                _logMessages = value;
                OnPropertyChanged("LogMessages");
            }
        }

        public int MessageIndex
        {
            get
            {
                return _messageIndex;
            }
            set
            {
                _messageIndex = value;
                OnPropertyChanged("MessageIndex");
            }
        }

        #endregion Properties

        #region Methods

        public void Start()
        {
            _bw.DoWork += _bw_DoWork;
            _bw.RunWorkerAsync();
        }

        public void Stop()
        {
            _run = false;
        }

        private void AddMessage(List<string> strList)
        {
            while (LogMessages.Count > MAX_LINES_BEHIND)
            {
                LogMessages.RemoveAt(0);
            }

            foreach (string str in strList)
            {
                LogMessages.Add(new LogMessage() { DisplayColor = GetMessageColor(str), LogText = str });
            }

            if (LogMessages.Count > 0)
            {
                MessageIndex = LogMessages.Count - 1;
            }
        }

        private Brush GetMessageColor(string str)
        {
            String s = str;
            if (s.Contains("INFO"))
            {
                return new SolidColorBrush(Colors.LimeGreen);
            }
            else if (s.Contains("WARNING"))
            {
                return new SolidColorBrush(Colors.PaleGoldenrod);
            }
            else if (s.Contains("ERROR"))
            {
                return new SolidColorBrush(Colors.Tomato);
            }
            else
            {
                return new SolidColorBrush(Colors.SkyBlue);
            }
        }

        void _bw_DoWork(object sender, DoWorkEventArgs e)
        {
            if (!File.Exists("Thorlog.log"))
            {
                return;
            }

            _run = true;
            long offset = 0;

            using (FileStream file = File.Open(
                                                "Thorlog.log",
                                                FileMode.Open,
                                                FileAccess.Read,
                                                FileShare.Write))
            {
                //Move to the end of the file
                offset = file.Seek(0, SeekOrigin.End);
                using (StreamReader reader = new StreamReader(file))
                {
                    DateTime dt = DateTime.Now;
                    List<string> strList = new List<string>();
                    while (_run)
                    {
                        try
                        {
                            if (!reader.EndOfStream)
                            {
                               if (((DateTime)DateTime.Now - dt).TotalMilliseconds > 500)
                                {
                                    string str = string.Empty;
                                    strList.Clear();
                                    do
                                    {
                                        str = reader.ReadLine();

                                        if (strList.Count > MAX_LINES_BEHIND)
                                        {
                                            strList.RemoveAt(0);
                                        }

                                        strList.Add(str);

                                        System.Threading.Thread.Sleep(1);
                                    }
                                    while (!reader.EndOfStream && _run);
                                    // strlist may be changed while it's being referenced inside the AddMessage function
                                    // and this would result in a crash. So, make a copy of strlist for the AddMessage.
                                    Application.Current.Dispatcher.BeginInvoke(new Action(() => AddMessage(strList.ToList())));

                                    dt = DateTime.Now;
                                }
                            }
                        }
                        catch (IOException ex)
                        {
                            //Sharing violations to the log file can still occur
                            ex.ToString();
                        }

                        System.Threading.Thread.Sleep(2);
                    }
                }
            }
        }

        #endregion Methods
    }
}