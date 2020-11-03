using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Diagnostics;
using System.IO.Ports;
using System.Threading;

namespace JPEGer_Process
{
    class Program
    {
        static int imgRows, imgCols;
		static int rows, cols;
		static Bitmap bmpOut = null;
		static Graphics g = null;
		static string[] folders = new string[] { "1x\\", "2x\\", "4x\\", "8x\\", "16x\\" };
        static string[] ext = new string[] { ".jpg", ".jpg", ".jpg", ".jpg", ".jpg" };
        static int[] factor = new int[] { 1, 2, 4, 8, 16 };
		static string path;
		static string outPath;
		static List<int> colList;
        static List<Point> expTime;
        static bool correctStrobe = false;
        static int targetExpUs;
        static SerialPort port = null;

        static bool killProc = false;
        static DateTime lastFileCheck;
        static int lastFileCnt = 0;

        static double[,] ffImg = null;
        static StreamWriter sw;

        static unsafe void Main(string[] args)
        {
            AppDomain.CurrentDomain.ProcessExit += new EventHandler(CurrentDomain_ProcessExit);

            string appPath = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);

            if (File.Exists(appPath + @"\FFImage.bin"))
            {
                UpdateLog("FFImage.bin was found", false);

                try
                {
                    BinaryReader br = new BinaryReader(new FileStream(appPath + @"\FFImage.bin", FileMode.Open));
                    int ffRows = br.ReadInt32();
                    int ffCols = br.ReadInt32();

                    ffImg = new double[ffRows, ffCols];

                    for (int r = 0; r < ffRows; r++)
                    {
                        for (int c = 0; c < ffCols; c++)
                        {
                            ffImg[r, c] = br.ReadDouble();
                        }
                    }
                    br.Close();

                    UpdateLog("FFImage.bin loaded properly", true);
                }
                catch
                {
                    UpdateLog("FFImage.bin failed to load", true);
                }
            }
            else
            {
                UpdateLog("FFImage.bin was NOT found", true);
            }

            int lastStrip = 0;
            bool done = false;

            UpdateLog("Args Null - " + (args == null), true);

            for (int a = 0; a < args.Length; a++)
            {
                UpdateLog("Args[" + a.ToString() + "] -" + args[a].ToString(), true);
            }

            string path = args[0] + @"\jpeg\1x\";
            int rows = Int32.Parse(args[1]);
            int cols = Int32.Parse(args[2]);
            targetExpUs = Int32.Parse(args[3]);

            UpdateLog("Path - " + path, true);
            UpdateLog("Rows - " + rows.ToString(), true);
            UpdateLog("Cols - " + cols.ToString(), true);
            UpdateLog("TargetExpUs - " + targetExpUs.ToString(), true);

            List<string> portStrs = new List<string>();
            string portName = Strobe_Monitor_Port(ref portStrs);

            UpdateLog("Serial Port - " + portName, true);

            if (portName != "")
            {
                try
                {
                    port = new SerialPort(portName);
                    port.BaudRate = 115200;
                    port.Open();
                    port.ReadTimeout = 100;
                    correctStrobe = true;
                    port.Write("r");
                    UpdateLog("Arduino is ready", true);
                }
                catch
                {
                    UpdateLog("Arduino failed to engage", true);
                }
            }

            try
            {
                JPEGer_Initialize(args[0], rows, cols);
                expTime = new List<Point>();
                UpdateLog("JPEGer_Initialized SUCCESS", true);
            }
            catch 
            {
                UpdateLog("JPEGer_Initialized FAILED", true);
            }

            lastFileCheck = DateTime.Now;
            Thread thread = new Thread(MT_CheckFolder);
            thread.Start();
            //lastUpdate = DateTime.Now;
            while (!done && !killProc)// (DateTime.Now - lastUpdate) < timeOut)
            {
                string[] files = Directory.GetFiles(path, "*" + ext[0]);

                if (files.Length > 0)
                {
                    string lastFile = Path.GetFileNameWithoutExtension(files[files.Length - 1]);
                    string[] parts = lastFile.Split(new char[] { '_' });
                    int currentStrip = Int32.Parse(parts[1]);
                    int currentCol = Int32.Parse(parts[2]);

                    if (correctStrobe)
                    {
                        try
                        {
                            while (port.BytesToRead > 0)
                            {
                                string[] vals = port.ReadLine().Trim().Split(new char[] { ':' });

                                if (vals.Length == 2)
                                    expTime.Add(new Point(Int32.Parse(vals[0]), Int32.Parse(vals[1])));

                                UpdateLog("Arduino Data : {" + vals[0] + ", " + vals[1] + "}", true);
                            }
                        }
                        catch
                        {
                            UpdateLog("Failed to read from Arduino", true);
                        }
                    }

                    if (currentStrip > lastStrip)
                    {
                        for (int i =lastStrip; i < currentStrip; i++)
                        {
                            Console.WriteLine("Processing Column : " + i);

                            try
                            {
                                JPEGer_ProcessColumn(i);
                                UpdateLog("Process Column - " + i.ToString(), true);
                            }
                            catch
                            {
                                UpdateLog("FAILED to Process Column - " + i.ToString(), true);
                            }
                        }
                        lastStrip = currentStrip;
                        //lastUpdate = DateTime.Now;
                    }

                    if (currentStrip == cols - 1 && currentCol == rows - 1)
                    {
                        try
                        {
                            Console.WriteLine("Processing Column : " + currentStrip);
                            JPEGer_ProcessColumn(currentStrip);
                            done = true;
                            Console.WriteLine("Done!");

                            if (correctStrobe)
                            {
                                port.Write("s");
                            }

                            Thread.Sleep(1000);

                            UpdateLog("Processed Last Column", true);
                        }
                        catch
                        {
                            UpdateLog("FAILED to Process Last Column", true);
                        }

                        //Spawn Viewer
                        //if (false)//File.Exists("TileViewer.exe"))
                        //{
                        //    Process proc = new Process();
                        //    proc.StartInfo.CreateNoWindow = false;
                        //    proc.StartInfo.UseShellExecute = false;
                        //    proc.StartInfo.FileName = "TileViewer.exe";
                        //    proc.StartInfo.WindowStyle = ProcessWindowStyle.Maximized;
                        //    proc.StartInfo.Arguments = args[0].Replace(@"\", @"\\") + " " + rows.ToString() + " " + cols.ToString();

                        //    proc.Start();
                        //}
                    }
                }
            }

            if (correctStrobe)
            {
                port.Write("s");

                while (port.BytesToRead > 0)
                {
                    port.ReadLine();
                }

                port.Close();
                port.Dispose();
                correctStrobe = false;
            }

            thread.Abort();
        }

        unsafe private static void UpdateLog(string Message, bool Append)
        {
            sw = new StreamWriter("JPEGer_Log.txt", Append);
            sw.WriteLine(Message);
            sw.Close();
        }

        static void CurrentDomain_ProcessExit(object sender, EventArgs e)
        {
            if (correctStrobe)
            {
                port.Write("s");

                while (port.BytesToRead > 0)
                {
                    port.ReadLine();
                }

                port.Close();
                port.Dispose();
            }
        }

        static void JPEGer_Initialize(string ImagePath, int Rows, int Cols)
		{
			colList = new List<int>();
			rows = Rows;
			cols = Cols;
			path = ImagePath;
			path += "\\jpeg\\";
			outPath = path;

			if (!Directory.Exists(outPath + "2x\\"))
			{
				Directory.CreateDirectory(outPath + "2x\\");
				Directory.CreateDirectory(outPath + "4x\\");
				Directory.CreateDirectory(outPath + "8x\\");
				Directory.CreateDirectory(outPath + "16x\\");
			}
		}

        static void JPEGer_ProcessColumn(int c)
        {
			if(c == 2)
			{
                ////Figure out if 1x is tiff or jpeg
                //string[] tmpJpeg = Directory.GetFiles(path + @"1x", "*.jpg");
                //string[] tmpTiff = Directory.GetFiles(path + @"1x", "*.tif");
                //if (tmpJpeg.Length > tmpTiff.Length)
                //    ext[0] = ".jpg";
                //else
                //    ext[0] = ".tif";

				Bitmap bmp_1 = Get_Tile_Image(path + "1x\\", 0, 0, ext[0], targetExpUs, false);
                
                //Configure FF-map for the number of shifts
                if(ffImg != null)
                    Configure_FF_Map(bmp_1.Height, bmp_1.Width);

				imgRows = bmp_1.Height / 32;
				imgCols = bmp_1.Width / 32;
                bmp_1.Dispose();
                bmp_1 = null;

				bmpOut = new Bitmap(cols * imgCols, rows * imgRows);
				g = Graphics.FromImage(bmpOut);
				g.FillRectangle(Brushes.Black, new Rectangle(0, 0, bmpOut.Width, bmpOut.Height));
			}

            if (c % 2 == 0)
                Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, c - factor[1], 1);
            if (c % 4 == 0)
                Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, c - factor[2], 2);
            if (c % 8 == 0)
                Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, c - factor[3], 3);
            if (c % 16 == 0)
                Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, c - factor[4], 4);

			if(c == cols-1)
			{
				if (cols % 2 != 0)
				{
					Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, factor[1] * (cols / factor[1]), 1);
					Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, factor[2] * (cols / factor[2]), 2);
					Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, factor[3] * (cols / factor[3]), 3);
					Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, factor[4] * (cols / factor[4]), 4);
				}
				else
				{
					if (cols % factor[1] != 0)
						Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, factor[1] * (cols / factor[1]), 1);
					else
						Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, factor[1] * (cols / factor[1]) - factor[1], 1);

					if (cols % factor[2] != 0)
						Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, factor[2] * (cols / factor[2]), 2);
					else
						Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, factor[2] * (cols / factor[2]) - factor[2], 2);

					if (cols % factor[3] != 0)
						Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, factor[3] * (cols / factor[3]), 3);
					else
						Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, factor[3] * (cols / factor[3]) - factor[3], 3);

					if (cols % factor[4] != 0)
						Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, factor[4] * (cols / factor[4]), 4);
					else
						Create_Tiles(outPath, rows, cols, path, folders, factor, imgRows, imgCols, g, factor[4] * (cols / factor[4]) - factor[4], 4);
				}

				bmpOut.Save(outPath + "Full.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
				bmpOut = null;
			}
        }

        private static void Configure_FF_Map(int ImgRows, int ImgCols)
        {
            int shifts = 0;

            if (ImgCols == 1392)
                shifts = 1040 - ImgRows;
            else if (ImgCols == 3320)
                shifts = 2496 - ImgRows;

            double[,] tmp = new double[ImgRows, ImgCols];

            for (int R = 0; R < ImgRows; R++)
            {
                for (int C = 0; C < ImgCols; C++)
                {
                    for (int i = 0; i < shifts; i++)
                    {
                        tmp[R, C] += ffImg[R + i, C];
                    }
                }
            }

            for (int R = 0; R < ImgRows; R++)
            {
                for (int C = 0; C < ImgCols; C++)
                {
                    tmp[R, C] /= (double)shifts;
                }
            }

            //Calculate max
            int[] hist = new int[16384];
            int cnt = 0;
            for (int R = 0; R < ImgRows; R++)
            {
                for (int C = 0; C < ImgCols; C++)
                {
                    hist[(ushort)tmp[R, C]]++;
                    cnt++;
                }
            }
            int thresh = (int)(0.9995f * (float)cnt);
            double max = -1;
            int sum = 0;
            for (int i = 0; i < hist.Length; i++)
            {
                sum += hist[i];
                if (sum > thresh)
                {
                    max = i;
                    break;
                }
            }

            //Normaize the mask
            for (int R = 0; R < ImgRows; R++)
            {
                for (int C = 0; C < ImgCols; C++)
                {
                    tmp[R, C] /= max;
                    tmp[R, C] = (tmp[R, C] <= 1) ? tmp[R, C] : 1;
                }
            }

            ffImg = tmp;
        }

        static Bitmap Get_Tile_Image(string path, int Row, int Col, string Ext, int TargetExpUs, bool Correct)
        {
            string preRow = "";
            string preCol = "";

            if (Row < 10)
                preRow = "000";
            else if (Row < 100)
                preRow = "00";
            else if (Row < 1000)
                preRow = "0";

            if (Col < 10)
                preCol = "000";
            else if (Col < 100)
                preCol = "00";
            else if (Col < 1000)
                preCol = "0";

            string name = path + "Tile_" + preCol + Col + "_" + preRow + Row + Ext;

            if (!File.Exists(name))
                return null;
            else
            {
                if (Correct && correctStrobe && path.Contains("1x"))
                {
                    int index = Col * rows + Row;
                    float scaler = (float)TargetExpUs / (float)expTime[index].Y;
                    Bitmap bmp = new Bitmap(name);
                    Scale_Bitmap(ref bmp, scaler);
                    Bitmap tmp = new Bitmap(bmp);
                    bmp.Dispose();
                    if(ext[0] == ".jpg")
                        tmp.Save(name, ImageFormat.Jpeg);
                    else if (ext[0] == ".tif")
                        tmp.Save(name, ImageFormat.Tiff);

                    return tmp;
                }
                else
                    return new Bitmap(name);
            }
        }

		static void Create_Tiles(string OutPath, int rows, int cols, string path, string[] folders, int[] factor, int imgRows, int imgCols, Graphics g, int c, int factorIdx)
        {
            for (int r = 0; r < rows; r += factor[factorIdx])
            {
                Bitmap[,] bmp1xArray = new Bitmap[2, 2];
                for (int subR = 0; subR < 2; subR++)
                {
                    for (int subC = 0; subC < 2; subC++)
                    {
                        bmp1xArray[subR, subC] = Get_Tile_Image(path + folders[factorIdx - 1], r + subR * factor[factorIdx - 1], c + subC * factor[factorIdx - 1], ext[factorIdx-1], targetExpUs, true);
                        if (factorIdx == 1 && bmp1xArray[subR, subC] != null)
                            g.DrawImage(bmp1xArray[subR, subC], (c + subC) * imgCols, (r + subR) * imgRows, imgCols, imgRows);
                    }
                }

                if (bmp1xArray[0, 0] == null)
                    return;

                Size imgSize = bmp1xArray[0, 0].Size;

                int incX = imgSize.Width / 2;
                int incY = imgSize.Height / 2;
                Bitmap tmp = new Bitmap(incX * 2, incY * 2);
				Graphics gTile = Graphics.FromImage(tmp);

				gTile.FillRectangle(Brushes.Black, 0, 0, imgSize.Width, imgSize.Height);
                bool painted = false;

                for (int tR = 0; tR < 2; tR++)
                {
                    for (int tC = 0; tC < 2; tC++)
                    {
                        if (bmp1xArray[tR, tC] != null)
                        {
							gTile.DrawImage(bmp1xArray[tR, tC], new Rectangle(tC * incX, tR * incY, incX, incY));
                            painted = true;
                        }
                    }
                }

                if (painted)
                {
                    string preRow = "";
                    string preCol = "";

                    if (r < 10)
                        preRow = "000";
                    else if (r < 100)
                        preRow = "00";
                    else if (r < 1000)
                        preRow = "0";

                    if (c < 10)
                        preCol = "000";
                    else if (c < 100)
                        preCol = "00";
                    else if (c < 1000)
                        preCol = "0";

					tmp.Save(OutPath + folders[factorIdx] + "Tile_" + preCol + (c) + "_" + preRow + (r) + ".jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
                }


                for (int subR = 0; subR < 2; subR++)
                {
                    for (int subC = 0; subC < 2; subC++)
                    {
                        if (bmp1xArray[subR, subC] != null)
                        {
							//bmp1xArray[subR, subC].Dispose();
                            bmp1xArray[subR, subC] = null;
                        }
                    }
                }
                //tmp.Dispose();
                tmp = null;

				GC.Collect(GC.MaxGeneration, GCCollectionMode.Forced);
            }
        }

        static unsafe void Scale_Bitmap(ref Bitmap BMP, float Scaler)
        {
            int dx = (BMP.PixelFormat == PixelFormat.Format32bppArgb) ? 4 : 3;
            int width = BMP.Width;
            int height = BMP.Height;
            BitmapData bmpData = BMP.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.ReadWrite, BMP.PixelFormat);
            byte* ptr = (byte*)bmpData.Scan0.ToPointer();

            for (int r = 0; r < height; r++)
            {
                int offset = r * bmpData.Stride;
                for (int c = 0; c < width; c++)
                {
                    float val = *(ptr + offset + c * dx) * Scaler;

                    //Do flat-fielding if present
                    if (ffImg != null)
                        val /= (float)ffImg[height - (r) - 1, c];

                    val = (float)Math.Round(val);
                    val = (val <= 255) ? val : 255;
                    val = (val >= 0) ? val : 0;

                    for (int i = 0; i < dx; i++)
                    {
                        *(ptr + offset + c * dx + i) = (byte)val;
                    }
                }
            }

            BMP.UnlockBits(bmpData);
        }

        static string Strobe_Monitor_Port(ref List<string> outStrs)
        {
            string[] portnames = SerialPort.GetPortNames();
            string outName = "";
            SerialPort port = null;

            for (int i = 0; i < portnames.Length; i++)
            {
                try
                {
                    port = new SerialPort(portnames[i], 115200, Parity.None, 8, StopBits.One);
                    port.Open();
                    port.ReadTimeout = 100;

                    port.Write("?");
                    
                    string str = port.ReadLine().Trim();
                    outStrs.Add(portnames[i] + "--->" + str);

                    if (str == "ArduinoStrobe")
                    {
                        outName = portnames[i];
                    }
                }
                catch
                {
                }
                finally
                {
                    if (port != null && port.IsOpen)
                        port.Close();
                }
            }

            return outName;
        }

        static void MT_CheckFolder()
        {
            while (true)
            {
                string[] files = Directory.GetFiles(path + @"1x\", "*" + ext[0]);

                if (files.Length > lastFileCnt)
                {
                    lastFileCnt = files.Length;
                    lastFileCheck = DateTime.Now;
                }
                else if (files.Length < rows*cols && (DateTime.Now - lastFileCheck) > TimeSpan.FromSeconds(10))
                {
                    killProc = true;
                }

                Thread.Sleep(2000);
            }
        }
    }
}
