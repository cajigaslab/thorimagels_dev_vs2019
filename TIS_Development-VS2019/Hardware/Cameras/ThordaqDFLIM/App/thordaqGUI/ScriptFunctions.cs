namespace thordaqGUI
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Text.RegularExpressions;
    using System.Globalization;

    using NWLogic.DeviceLib;

    public partial class MainWindow : Window
    {
        public void LoadScript(string fileName)
        {
            System.IO.StreamReader file = new System.IO.StreamReader(fileName);
            while (!file.EndOfStream)
            {
                string line = file.ReadLine();
                for (int i = 0; i < THORDAQ_COMMANDLINE_COMMAND.Length; i++)
                {
                    if (line.ToLower().Contains(THORDAQ_COMMANDLINE_COMMAND[i].ToLower()))
                    {
                        string input = line.Trim(); // remove all white space start or end in the input string,
                        input = input.Replace("dmadriverclix64 ", string.Empty);
                        input = System.Text.RegularExpressions.Regex.Replace(input, @"\s{2,}", " ");// replace multiple space with one
                        ConsoleOutput.Add(">>> " + input);
                        UpdateConsoleCommand(input);
                        _last_recorded_command = ConsoleOutput.Count - 1;
                        break;
                    }
                }
            }
        }

        public void Save() 
        { 
        
        }
    }
}
