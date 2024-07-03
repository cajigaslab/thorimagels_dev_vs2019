namespace ThorDetector_Control
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    using Microsoft.Win32;

    /// <summary>
    /// Class ComPort.
    /// </summary>
    /// Name and friendly name of a COM port.
    public class ComPort
    {
        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="ComPort"/> class.
        /// </summary>
        /// <param name="portName">Name of the port.</param>
        /// <param name="friendlyName">Name of the friendly.</param>
        public ComPort(string portName, string friendlyName)
        {
            PortName = portName;
            FriendlyName = friendlyName;
        }

        #endregion Constructors

        #region Properties

        /// <summary>
        /// Gets the name of the friendly.
        /// </summary>
        /// <value>The name of the friendly.</value>
        /// Friendly name of the port.
        public string FriendlyName
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the name of the port.
        /// </summary>
        /// <value>The name of the port.</value>
        /// Name of the port (example: "COM1").
        public string PortName
        {
            get;
            private set;
        }

        #endregion Properties
    }

    public class COMPortManager
    {
        #region Fields

        public static System.Threading.EventWaitHandle ewh = new System.Threading.EventWaitHandle(false, System.Threading.EventResetMode.AutoReset);

        #endregion Fields

        #region Constructors

        public COMPortManager()
        {
        }

        #endregion Constructors

        #region Properties

        public static string bootloaderNum
        {
            get;
            set;
        }

        public static List<ComPort> comPorts
        {
            get;
            set;
        }

        public static string firmwareNum
        {
            get;
            set;
        }

        public static int selectedDetectorIndex
        {
            get;
            set;
        }

        public static string serialNum
        {
            get;
            set;
        }

        public static string type
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Gets the Detector_Bootloader COM ports. 
        /// </summary>
        /// <returns>ComPort[].</returns>
        public static ComPort[] GetCOMPorts(string friendlyName)
        {
            // list of all ports found

            return comPorts.ToArray();
        }

        /// <summary>
        /// Finds the ports.
        /// </summary>
        /// <param name="list">The list.</param>
        /// <param name="keyName">Name of the key.</param>
        private static void FindPorts(List<ComPort> list, string keyName)
        {
            using (RegistryKey key = Registry.LocalMachine.OpenSubKey(keyName))
            {
                int count = (int)key.GetValue("Count", 0);
                for (int i = 0; i < count; i++)
                {
                    string location = (string)key.GetValue(i.ToString());
                    string portName = (string)Registry.GetValue(@"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\" + location + @"\Device parameters", "PortName", string.Empty);
                    string friendlyName = (string)Registry.GetValue(@"HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\" + location, "FriendlyName", string.Empty);
                    list.Add(new ComPort(portName, friendlyName));
                }
            }
        }

        #endregion Methods
    }
}