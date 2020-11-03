namespace UpdateFirmwareWindow
{
    using System;
    using System.ComponentModel;
    using System.IO.Ports;
    using System.Threading;
    using System.Windows;
    using System.Windows.Media;

    /// <summary>
    /// Interaction logic for UpdateFirmwareWindow.xaml
    /// </summary>
    public partial class UpdateFirmware : Window, INotifyPropertyChanged
    {
        #region Fields

        const byte HOST_ID = 0x01; //Grid Margin="0,0,0,214"
        const int MAX_PAGES = 3564;
        const int MGMSG_BL_GET_FIRMWARE = 0x00A5;
        const int MGMSG_BL_GET_FIRMWAREVER = 0x0030;
        const int MGMSG_BL_REQ_FIRMWARE = 0x00A4;
        const int MGMSG_BL_REQ_FIRMWAREVER = 0x002F;
        const int MGMSG_BL_SET_FIRMWARE = 0x00A3;
        const int MGMSG_GET_UPDATE_FIRMWARE = 0x00A6;
        const int MGMSG_RESET_FIRMWARE_LOADCOUNT = 0x00A7;
        const byte MOTHERBOARD_ID = 0x11; // 0x11 => MCM 1, 0x12 => MCM2 ...

        private string _connectionStatus = string.Empty;
        private int _connectionStatusColor = (int)Status.GREEN;
        private int _dei;
        private bool _firmware_error = false;
        private byte[][] _firmware_output; //up to 1 MB flash space in 64byte pages, leading 4 bytes are 32bit address
        private string _fw_file = string.Empty;
        private byte[][] _fw_op_rst_cmd;
        private string _portName;
        private SerialPort _serialPort;
        private bool _usb_connected;
        private BackgroundWorker _worker;

        #endregion Fields

        #region Constructors

        public UpdateFirmware(string portName, string firmwareFile, string fileName)
        {
            InitializeComponent();
            this.Loaded += UpdateFirmwareWindow_Loaded;
            _portName = portName;
            _fw_file = firmwareFile;
            fileNameLabel.Content = fileName;
        }

        #endregion Constructors

        #region Enumerations

        public enum Status
        {
            NONE,
            GREEN,
            YELLOW,
            RED
        }

        #endregion Enumerations

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public string ConnectionStatus
        {
            get
            {
                return _connectionStatus;
            }
            set
            {
                _connectionStatus = value;
                OnPropertyChanged("ConnectionStatus");
            }
        }

        // 1: Green, 2: Yellow, 3: Red
        public int ConnectionStatusColor
        {
            get
            {
                return _connectionStatusColor;
            }
            set
            {
                _connectionStatusColor = value;
                OnPropertyChanged("ConnectionStatusColor");
            }
        }

        public string FirmwareFile
        {
            get
            {
                return _fw_file;
            }
            set
            {
                _fw_file = value;
            }
        }

        #endregion Properties

        #region Methods

        public byte[] usb_wait_read(int bytes)
        {
            try
            {
                if (_serialPort.IsOpen)
                {
                    byte[] data_back = new byte[bytes];
                    while (_serialPort.BytesToRead < bytes) { };
                    _serialPort.Read(data_back, 0, bytes);
                    return data_back;
                }
                else
                {
                    return new byte[0];
                }
            }
            catch
            {
                return new byte[0];
            }
        }

        /// <summary>
        /// Handles the Click event of the btnExit control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RoutedEventArgs"/> instance containing the event data.</param>
        private void btnExit_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        /// <summary>
        /// Handles the Exit event of the Current control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="ExitEventArgs"/> instance containing the event data.</param>
        void Current_Exit(object sender, ExitEventArgs e)
        {
            this.Close();
        }

        private void display_version()
        {
            // request bootloader version info
            byte[] bytesToSend = new byte[10] { 0x2F, 0x00, 0x04, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00 };
            usb_write(bytesToSend, 10);
            Thread.Sleep(20);
            byte[] data_back = usb_wait_read(10);
            labelFirmware_rev_val.Content = Convert.ToString(data_back[9]);
            labelFirmware_rev_val.Content += ".";
            labelFirmware_rev_val.Content += Convert.ToString(data_back[8]);
        }

        private void function1(string f_name)
        {
            _dei = 0;
            _firmware_output = new byte[16535][]; //up to 1 MB flash space in 64byte pages, leading 4 bytes are 32bit address
            _fw_op_rst_cmd = new byte[16535][];

            uint page_cnt = intel_hex_conversion(f_name);
            byte[] out_buffer = new byte[74];
            byte[] test_buffer = new byte[70];
            out_buffer[0] = 0xA3;  //command lower byte
            out_buffer[1] = 0x00;  //command upper byte
            out_buffer[2] = 0x44;  //number of data bytes lower  68
            out_buffer[3] = 0x00;  //number of data bytes upper
            out_buffer[4] = 0x80;  //destination
            out_buffer[5] = 0x00;  //source

            byte[] reset_load_command_buffer = new byte[6];
            reset_load_command_buffer[0] = 0xA7;  //command lower byte
            reset_load_command_buffer[1] = 0x00;  //command upper byte
            reset_load_command_buffer[2] = 0x00;  //number of data bytes lower  00
            reset_load_command_buffer[3] = 0x00;  //number of data bytes upper
            reset_load_command_buffer[4] = 0x80;  //destination
            reset_load_command_buffer[5] = 0x00;  //source

            bool nextAct = false;
            while (!nextAct)
            {
                //TODO address
                int start_address = _firmware_output[0][0] | _firmware_output[0][1] << 8 | _firmware_output[0][2] << 16 | _firmware_output[0][3] << 24;
                if (start_address == 0x8000)
                    nextAct = true;
                else
                    break;
            }
            double totalProcessCount = page_cnt * 2 + 16535 / 10.0;
            double progressCount = 0;
            if (nextAct)
            {
                if (page_cnt < MAX_PAGES)
                {
                    for (int c = 0; c < page_cnt; c++)
                    {
                        Array.Copy(_firmware_output[c], 0, out_buffer, 6, 68); // cpying firmware output into out_buffer
                        usb_write(out_buffer, 74); // out_buffer = THORLABS APT + page_adrs + 64 bytes data
                        Thread.Sleep(20);
                        progressCount++;
                        if (0 == progressCount % 100)
                        {
                            UpdateProgress((int)(progressCount / totalProcessCount * 100));
                            System.Threading.Thread.Sleep(5);
                        }
                    }
                    //Check till here first
                    //First step correct

                    out_buffer[0] = 0xA4;  //reset command to request firmware
                    out_buffer[2] = 0x04;  //set to 4 data bytes

                    //initialize array to 0xff
                    for (int ii = 0; ii < 16535; ii++)
                    {
                        _fw_op_rst_cmd[ii] = new byte[70];
                        for (int d = 0; d < 70; d++)
                        {
                            _fw_op_rst_cmd[ii][d] = 0xff;
                        }
                        if (0 == (ii + 1) % 10)
                        {
                            progressCount++;
                        }

                        if (0 == progressCount % 100)
                        {
                            UpdateProgress((int)(progressCount / totalProcessCount * 100));
                            System.Threading.Thread.Sleep(5);
                        }

                    }
                    for (int c = 0; c < page_cnt; c++)
                    {
                        Array.Copy(_firmware_output[c], 0, out_buffer, 6, 4);  //copy the address to the buffer to request the info
                        usb_write(out_buffer, 10);
                        Thread.Sleep(20);
                        byte[] input = usb_wait_read(70);

                        //Check till here now. Second Step
                        for (int d = 0; d < 64; d++)
                        {
                            if (input[d + 6] != _firmware_output[c][d + 4])
                            {
                                MessageBox.Show("Error at" + c.ToString());
                                _firmware_error = true;
                            }

                            if (_firmware_error)
                                break;
                        }
                        if (_firmware_error)
                            break;
                        get_cdc_ip(input, c); // get 70 bytes for all the pages with A5 command here
                        progressCount++;
                        if (0 == progressCount % 100)
                        {
                            UpdateProgress((int)(progressCount / totalProcessCount * 100));
                            System.Threading.Thread.Sleep(5);
                        }

                    }

                    //Check till here now. Third Step
                    if (_firmware_error)
                    {
                        MessageBox.Show("Firmware Upgrade failed");
                    }
                    else
                    {
                        usb_write(reset_load_command_buffer, 6);
                        UpdateProgress(100);
                        System.Threading.Thread.Sleep(5);
                        try
                        {
                            _serialPort.Dispose();
                        }
                        catch
                        {
                        }
                    }
                }
                else
                {
                    MessageBox.Show("Firmware Overwriting to flash addresses. Firmware Upgrade failed");
                }
            }
            else
            {
                MessageBox.Show("Firmware Starts at incorrect address. Firmware Upgrade failed");
            }
        }

        void get_cdc_ip(byte[] input, int c)
        {
            for (int j = 0; j < 70; j++)
                _fw_op_rst_cmd[_dei][j] = input[j];
            _dei++;
        }

        uint intel_hex_conversion(string f_name)
        {
            //initialize array to 0xff
            for (int c = 0; c < 16535; c++)
            {
                _firmware_output[c] = new byte[68];
                for (int d = 0; d < 68; d++)
                {
                    _firmware_output[c][d] = 0xff;
                }
            }

            byte[] temp_buffer = new byte[1058240];  //up to 1MB flash
            for (int c = 0; c < 1058240; c++)
            {
                temp_buffer[c] = 0xff;
            }

            System.IO.StreamReader file_in = new System.IO.StreamReader(@f_name);

            string temp_line;
            uint byte_count, address, record_type, checksum, address_offset = 0;
            uint min_address = 0xffffffff, max_address = 0x00000000;

            do
            {
                temp_line = file_in.ReadLine();  //read next line from intel hex file

                byte_count = Convert.ToUInt16(temp_line.Substring(1, 2), 16);  //first byte after :
                address = Convert.ToUInt16(temp_line.Substring(3, 4), 16);  //second and third bytes
                record_type = Convert.ToUInt16(temp_line.Substring(7, 2), 16);  //fourth byte
                checksum = Convert.ToUInt16(temp_line.Substring(temp_line.Length - 2, 2), 16); //last byte

                address = address & 0xffff;  //set upper bytes of 32 bit int to zeros

                if (record_type == 0x02)  //it's an extended segment address
                {
                    address_offset = Convert.ToUInt16(temp_line.Substring(9, 4), 16);  //offset data from ext seg address
                    address_offset = address_offset << 4;
                }

                if (record_type == 0x00)  //data that needs to be sent
                {
                    for (int c = 0; c < byte_count; c++)  //convert all data bytes in the line and load the firmware buffer
                    {
                        temp_buffer[address + address_offset + c] = Convert.ToByte(temp_line.Substring(9 + c * 2, 2), 16);  //convert each pair of chars to byte
                    }
                    if ((address + address_offset) < min_address) { min_address = address + address_offset; }
                    if ((address + address_offset) > max_address) { max_address = address + address_offset; }
                }

            } while (record_type != 0x01);  //run til eof

            uint total_byte_count = max_address - min_address;
            uint total_pages = (total_byte_count - 1) / 64 + 1;  //-1 and +1 are to round up the next whole page

            address = min_address;

            for (int c = 0; c < total_pages; c++)
            {
                Array.Copy(BitConverter.GetBytes(address), _firmware_output[c], 4);
                Array.Copy(temp_buffer, address, _firmware_output[c], 4, 64);
                address += 64;
            }

            file_in.Close();
            return total_pages;
        }

        /// <summary>
        /// Called when [property changed].
        /// </summary>
        /// <param name="propertyName">Name of the property.</param>
        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private void startFirmwareUpdate_Click(object sender, RoutedEventArgs e)
        {
            startFirmwareUpdate.IsEnabled = false;
            updateStatus.Content = "Updating Firmware, Do Not close the Window.";
            _worker = new BackgroundWorker();
            _worker.WorkerReportsProgress = true;
            _worker.DoWork += new DoWorkEventHandler(_worker_DoWork);
            _worker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(_worker_RunWorkerCompleted);
            _worker.ProgressChanged += new ProgressChangedEventHandler(_worker_ProgressChanged);
            _worker.RunWorkerAsync();
        }

        void UpdateFirmwareWindow_Loaded(object sender, RoutedEventArgs e)
        {
            usb_connect();
        }

        /// <summary>
        /// Updates the progress.
        /// </summary>
        /// <param name="percent">The percent.</param>
        /// <param name="deviceIndex">Index of the device.</param>
        private void UpdateProgress(int percent)
        {
            if (null != _worker)
            {
                _worker.ReportProgress(percent);
            }
        }

        private void usb_connect()
        {
            _serialPort = new SerialPort();
            _serialPort.PortName = _portName;
            lbl_con_status.Dispatcher.Invoke(new Action(() =>
            {
                lbl_con_status.Content = "Disconnected";
                startFirmwareUpdate.IsEnabled = false;
                updateStatus.Content = "Device not found, close the app and retry.";
                lbl_con_status.Background = new SolidColorBrush(Colors.Red);
            }));
            try
            {
                _serialPort.Open();

                Thread.Sleep(500);

                //attach interrupt handler
                _usb_connected = true;

                lbl_con_status.Dispatcher.Invoke(new Action(() =>
                {
                    lbl_con_status.Content = "Connected";
                    startFirmwareUpdate.IsEnabled = true;
                    updateStatus.Content = "";
                    lbl_con_status.Background = new SolidColorBrush(Colors.Green);
                }));

                display_version();

            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void usb_disconnect()
        {
            lbl_con_status.Dispatcher.Invoke(new Action(() =>
            {
                lbl_con_status.Content = "Disconnected";
                startFirmwareUpdate.IsEnabled = false;
                updateStatus.Content = "Device not found, close the app and retry.";
                lbl_con_status.Background = new SolidColorBrush(Colors.Red);
            }));

            try
            {
                if (_serialPort.IsOpen == true)
                {
                    _serialPort.Close();
                }
                _usb_connected = false;
            }
            catch (Exception ex)
            {
                ex.ToString();
            }
        }

        private void usb_write(byte[] bytesToSend, int length)
        {
            if (_usb_connected)
            {
                try
                {
                    _serialPort.Write(bytesToSend, 0, length);
                }
                catch (Exception ex)
                {
                    ex.ToString();
                    usb_disconnect();
                }
            }
        }

        /// <summary>
        /// Handles the DoWork event of the _worker control.
        /// Here the firmware is updated
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="DoWorkEventArgs"/> instance containing the event data.</param>
        void _worker_DoWork(object sender, DoWorkEventArgs e)
        {
            function1(_fw_file);

            UpdateProgress(100); // -1 is the index used to update the status the label
            System.Threading.Thread.Sleep(1500);
        }

        /// <summary>
        /// Handles the ProgressChanged event of the _worker control.
        /// the index (userState) is used to update the right progress bar
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="ProgressChangedEventArgs"/> instance containing the event data.</param>
        void _worker_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            pbar.Value = e.ProgressPercentage;
            lbl1pbar.Content = e.ProgressPercentage.ToString() + "%";
        }

        /// <summary>
        /// Handles the RunWorkerCompleted event of the _worker control.
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">The <see cref="RunWorkerCompletedEventArgs"/> instance containing the event data.</param>
        void _worker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            MessageBox.Show("Firmware update completed, please power cycle the device and disconnect-reconnect its USB cable.", "Firmware Update finished", MessageBoxButton.OK);
            startFirmwareUpdate.IsEnabled = true;
            this.Close();
        }

        #endregion Methods
    }
}