using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Forms;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;

namespace ImageTilerControl
{
    /// <summary>
    /// Interaction logic for UserControl1.xaml
    /// </summary>
    public partial class UserControl1 : System.Windows.Controls.UserControl
    {
        private delegate void StandardDelegate();
        public delegate void ImageCoordChangedDelegate(int X, int Y);
        private System.Windows.Threading.DispatcherTimer timerGridUpdate;

        public UserControl1()
        {
            InitializeComponent();
        }

        #region From WinForms
        public event ImageCoordChangedDelegate ImageCoordChangedEvent;
        Bitmap fullBMP = null;
        RectangleF fullRect;
        Bitmap[,] grid = null;
        RectangleF[,] baseRects;
        RectangleF[,] rects;
        int[,] scales;
        RectangleF viewRect = new RectangleF(-1, -1, 1, 1);
        System.Drawing.Size[] sizes;
        int currentSize = 0;
        bool dragThumb = false;
        bool dragZoom = false;
        PointF ptGrab;
        string jpegPath;
        int totalRows, totalCols;
        string[] subJpegs = new string[] { @"32x\", @"16x\", @"8x\", @"4x\", @"2x\", @"1x\" };
        int[] scalers = new int[] { 32, 16, 8, 4, 2, 1 };
        string[] subFileTypes = new string[] { ".jpg", ".jpg", ".jpg", ".jpg", ".jpg", ".jpg" };
        double mag, pixelSizeUM, offsetXMM, offsetYMM, tdiWidthMM, tdiHeightMM, camHeight;
        bool moveStageOnClick = true;

        public void LoadData(string Folder)
        {
            Read_Experiment_File(Folder, out mag, out pixelSizeUM, out offsetXMM, out offsetYMM, out tdiWidthMM, out tdiHeightMM, out camHeight);

            jpegPath = Folder + @"jpeg\";
            //Figure out if 1x is tiff or jpeg
            string[] tmpJpeg = Directory.GetFiles(Folder + @"jpeg\1x", "*.jpg");
            string[] tmpTiff = Directory.GetFiles(Folder + @"jpeg\1x", "*.tif");
            if (tmpJpeg.Length > tmpTiff.Length)
                subFileTypes[5] = ".jpg";
            else
                subFileTypes[5] = ".tif";

            fullBMP = new Bitmap(Folder + @"jpeg\Full.jpg");

            pbThumbView.Height = fullBMP.Height * pbThumbView.Width / fullBMP.Width;
            gridLayout.RowDefinitions[0].Height = new GridLength(pbThumbView.Height);
            //gridLayout.RowDefinitions[1].Height = gridLayout.RowDefinitions[0].Height;
            pbThumbView.Invalidate();

            if (subFileTypes[5] == ".jpg")
            {
                string[] vars = System.IO.Path.GetFileNameWithoutExtension(tmpJpeg[tmpJpeg.Length - 1]).Split(new char[] { '_' });
                totalCols = Int32.Parse(vars[1]) + 1;
                totalRows = Int32.Parse(vars[2]) + 1;
            }
            else
            {
                string[] vars = System.IO.Path.GetFileNameWithoutExtension(tmpTiff[tmpTiff.Length - 1]).Split(new char[] { '_' });
                totalCols = Int32.Parse(vars[1]) + 1;
                totalRows = Int32.Parse(vars[2]) + 1;
            }

            grid = new Bitmap[totalRows, totalCols];
            scales = new int[totalRows, totalCols];
            rects = new RectangleF[totalRows, totalCols];
            baseRects = new RectangleF[totalRows, totalCols];

            for (int r = 0; r < baseRects.GetLength(0); r++)
            {
                for (int c = 0; c < baseRects.GetLength(1); c++)
                {
                    baseRects[r, c] = new RectangleF((float)c / (float)baseRects.GetLength(1), (float)r / (float)baseRects.GetLength(0), 1.0f / (float)baseRects.GetLength(1), 1.0f / (float)baseRects.GetLength(0));
                }
            }

            sizes = new System.Drawing.Size[5];
            Bitmap tmp = new Bitmap(jpegPath + @"16x\Tile_0000_0000" + subFileTypes[1]);
            sizes[0] = tmp.Size;
            tmp = new Bitmap(jpegPath + @"8x\Tile_0000_0000" + subFileTypes[2]);
            sizes[1] = tmp.Size;
            tmp = new Bitmap(jpegPath + @"4x\Tile_0000_0000" + subFileTypes[3]);
            sizes[2] = tmp.Size;
            tmp = new Bitmap(jpegPath + @"2x\Tile_0000_0000" + subFileTypes[4]);
            sizes[3] = tmp.Size;
            tmp = new Bitmap(jpegPath + @"1x\Tile_0000_0000" + subFileTypes[5]);
            sizes[4] = tmp.Size;

            float vHeight = (float)pbZoomView.Height / (float)pbZoomView.Width * (float)pbThumbView.Width / (float)pbThumbView.Height;
            viewRect = new RectangleF(0, 0, 0.5f, 0.5f * vHeight);

            Update_Rects();
            Update_PictureBoxes();

            timerGridUpdate = new System.Windows.Threading.DispatcherTimer();
            timerGridUpdate.Tick += new EventHandler(timerGridUpdate_Tick);
            timerGridUpdate.Interval = new TimeSpan(0, 0, 0, 0, 50);
            timerGridUpdate.Start();

            GC.Collect(GC.MaxGeneration, GCCollectionMode.Forced);
        }

        private void Update_PictureBoxes()
        {
            pbThumbView.Refresh();
            pbZoomView.Refresh();
        }

        private void Update_Rects()
        {
            if (sizes != null)
            {
                currentSize = 5;

                //Find minimum size of images to use so that the image size is >= the rectangle the image will be painted too
                for (int i = 0; i < scalers.Length; i++)
                {
                    float totWidth = grid.GetLength(1) * sizes[0].Width * viewRect.Width / scalers[i];

                    if (totWidth >= 1f * (float)pbZoomView.Width)
                    {
                        currentSize = i;
                        break;
                    }
                }

                //Get offset in terms of pictureBox coords from viewRect coords
                float offsetX = (int)Math.Round(viewRect.X * pbZoomView.Width / viewRect.Width);
                float offsetY = (int)Math.Round(viewRect.Y * pbZoomView.Height / viewRect.Height);
                //Determine width and height of each image rect based on viewRect size and magnification
                float incX = (int)Math.Round(scalers[currentSize] * pbZoomView.Width * baseRects[0, 0].Width / viewRect.Width);
                float incY = (int)Math.Round(scalers[currentSize] * pbZoomView.Height * baseRects[0, 0].Height / viewRect.Height);
                //Determine rectangle to paint full 32x image into
                fullRect = new RectangleF(-offsetX, -offsetY, incX * grid.GetLength(1) / scalers[currentSize], incY * grid.GetLength(0) / scalers[currentSize]);

                for (int r = 0; r < rects.GetLength(0); r++)
                {
                    for (int c = 0; c < rects.GetLength(1); c++)
                    {
                        rects[r, c] = new RectangleF(c * incX - offsetX, r * incY - offsetY, incX*1.001f, incY*1.001f);
                    }
                }
            }
        }

        private void Update_Grid()
        {
            //Load images outside of the view by 20% to pre-buffer for motion
            RectangleF inflatedView = RectangleF.Inflate(viewRect, 0.2f * viewRect.Width, 0.2f * viewRect.Height);
            bool updatePaint = false;

            RectangleF tmpRect = new RectangleF(0, 0, baseRects[0, 0].Width * scalers[currentSize], baseRects[0, 0].Height * scalers[currentSize]);
            for (int r = 0; r < rects.GetLength(0); r++)
            {
                for (int c = 0; c < rects.GetLength(1); c++)
                {
                    if (currentSize > 0 && r < rects.GetLength(0) / scalers[currentSize] + 1 && c < rects.GetLength(1) / scalers[currentSize] + 1 && inflatedView.IntersectsWith(new RectangleF(c * tmpRect.Width, r * tmpRect.Height, tmpRect.Width, tmpRect.Height)))
                    {
                        if (scales[r, c] != currentSize)
                        {
                            grid[r, c] = Thorlabs_IP_Library.Thorlabs_IP_Library.Get_Tile_Image(jpegPath + subJpegs[currentSize], r * scalers[currentSize], c * scalers[currentSize], subFileTypes[currentSize]);

                            if (grid[r, c] != null)
                            {
                                updatePaint = true;
                                scales[r, c] = currentSize;
                            }
                            else
                            {
                                scales[r, c] = 0;
                            }
                        }
                    }
                    else
                    {
                        if (grid[r, c] != null)
                        {
                            grid[r, c].Dispose();
                            grid[r, c] = null;
                        }

                        scales[r, c] = 0;
                    }
                }
            }

            GC.Collect(GC.MaxGeneration, GCCollectionMode.Forced);

            if (updatePaint)
                pbZoomView.Invoke(new StandardDelegate(Update_PictureBoxes));
        }

        //This should be modified to read in a more XML-friendly way
        private void Read_Experiment_File(string Folder, out double Magnification, out double PixelSizeUM, out double OffsetXMM, out double OffsetYMM, out double TDIWidthMM, out double TDIHeightMM, out double CameraHeight)
        {
            Magnification = 0;
            PixelSizeUM = 0;
            OffsetXMM = 0;
            OffsetYMM = 0;
            TDIHeightMM = 0;
            TDIWidthMM = 0;
            CameraHeight = 0;

            string file = Folder + "Experiment.xml";
            StreamReader sr = new StreamReader(file);

            while(sr.Peek() >= 0)
            {
                string line = sr.ReadLine();
                string[] vals = line.Trim().Split(new char[]{' '});

                if(line.Contains("Magnification"))
                {
                    for (int i = 0; i < vals.Length; i++)
			        {
                        if (vals[i].Contains("mag="))
                        {
                            string[] tmp = vals[i].Split(new char[] { '"' });
                            Magnification = double.Parse(tmp[1]);
                        }
			        }
                }

                if (line.Contains("pixelSizeUM"))
                {
                    for (int i = 0; i < vals.Length; i++)
                    {
                        if (vals[i].Contains("pixelSizeUM="))
                        {
                            string[] tmp = vals[i].Split(new char[] { '"' });
                            PixelSizeUM = double.Parse(tmp[1]);
                        }
                    }
                }

                if (line.Contains("tdiWidth"))
                {
                    for (int i = 0; i < vals.Length; i++)
                    {
                        if (vals[i].Contains("tdiWidth="))
                        {
                            string[] tmp = vals[i].Split(new char[] { '"' });
                            TDIWidthMM = double.Parse(tmp[1]);
                        }
                    }
                }

                if (line.Contains("tdiHeight"))
                {
                    for (int i = 0; i < vals.Length; i++)
                    {
                        if (vals[i].Contains("tdiHeight="))
                        {
                            string[] tmp = vals[i].Split(new char[] { '"' });
                            TDIHeightMM = double.Parse(tmp[1]);
                        }
                    }
                }

                if (line.Contains("offsetXMM"))
                {
                    for (int i = 0; i < vals.Length; i++)
                    {
                        if (vals[i].Contains("offsetXMM="))
                        {
                            string[] tmp = vals[i].Split(new char[] { '"' });
                            OffsetXMM = double.Parse(tmp[1]);
                        }
                    }
                }

                if (line.Contains("offsetYMM"))
                {
                    for (int i = 0; i < vals.Length; i++)
                    {
                        if (vals[i].Contains("offsetYMM="))
                        {
                            string[] tmp = vals[i].Split(new char[] { '"' });
                            OffsetYMM = double.Parse(tmp[1]);
                        }
                    }
                }

                if (line.Contains("<Camera"))
                {
                    for (int i = 0; i < vals.Length; i++)
                    {
                        if (vals[i].Contains("height="))
                        {
                            string[] tmp = vals[i].Split(new char[] { '"' });
                            CameraHeight = double.Parse(tmp[1]);
                        }
                    }
                }
            }

            sr.Close();
        }

        public bool MoveStageOnClick
        {
            get
            {
                return moveStageOnClick;
            }
            set
            {
                moveStageOnClick = value;
            }
        }

        #region Event
        private void pbThumbView_Paint(object sender, PaintEventArgs e)
        {
            if (fullBMP != null)
            {
                Graphics g = e.Graphics;

                g.DrawImage(fullBMP, pbThumbView.ClientRectangle);
                g.DrawRectangle(Pens.Yellow, viewRect.X * pbThumbView.Width, viewRect.Y * pbThumbView.Height, viewRect.Width * pbThumbView.Width, viewRect.Height * pbThumbView.Height);
            }
        }

        private void pbZoomView_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;

            g.FillRectangle(System.Drawing.Brushes.Black, pbZoomView.ClientRectangle);

            if (fullBMP != null)
            {
                g.DrawImage(fullBMP, fullRect);

                for (int r = 0; r < rects.GetLength(0); r++)
                {
                    for (int c = 0; c < rects.GetLength(1); c++)
                    {
                        if (scales[r, c] > 0)
                        {
                            g.DrawImage(grid[r, c], rects[r, c]);
                            //g.DrawRectangle(Pens.Yellow, rects[r, c].X, rects[r, c].Y, rects[r, c].Width, rects[r, c].Height);
                        }
                    }
                }
            }
        }

        private void pbThumbView_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            pbThumbView.Focus();

            dragThumb = true;
        }

        private void pbThumbView_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (dragThumb)
            {
                float x = (float)e.X / (float)pbThumbView.Width;
                float y = (float)e.Y / (float)pbThumbView.Height;

                viewRect = new RectangleF(x - viewRect.Width / 2, y - viewRect.Height / 2, viewRect.Width, viewRect.Height);

                Update_Rects();
                Update_PictureBoxes();
            }
        }

        private void pbThumbView_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            dragThumb = false;
        }

        private void pbZoomView_MouseWheel(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (fullBMP != null)
            {
                float newWidth;
                float newHeight;
                float mag = 0.9f;

                if (e.Delta < 0)
                {
                    mag = 1 / mag;
                }

                newWidth = mag * viewRect.Width;
                newHeight = mag * viewRect.Height;

                float mX = ((float)e.X / (float)pbZoomView.Width) * viewRect.Width + viewRect.X;
                float mY = ((float)e.Y / (float)pbZoomView.Height) * viewRect.Height + viewRect.Y;

                float cX1 = viewRect.X + viewRect.Width / 2;
                float cY1 = viewRect.Y + viewRect.Height / 2;

                float cX2 = mag * (cX1 - mX) + mX;
                float cY2 = mag * (cY1 - mY) + mY;

                viewRect = new RectangleF(cX2 - newWidth / 2, cY2 - newHeight / 2, newWidth, newHeight);

                Update_Rects();
                Update_Grid();
                Update_PictureBoxes();
            }
        }

        private void pbThumbView_MouseWheel(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (fullBMP != null)
            {
                float newWidth;
                float newHeight;

                if (e.Delta > 0)
                {
                    newWidth = 0.9f * viewRect.Width;
                    newHeight = 0.9f * viewRect.Height;
                }
                else
                {
                    newWidth = viewRect.Width / 0.9f;
                    newHeight = viewRect.Height / 0.9f;
                }

                float centerX = viewRect.X + viewRect.Width / 2;
                float centerY = viewRect.Y + viewRect.Height / 2;

                viewRect = new RectangleF(centerX - newWidth / 2, centerY - newHeight / 2, newWidth, newHeight);
                Update_Rects();
                Update_Grid();
                Update_PictureBoxes();
            }
        }

        private void pbZoomView_Resize(object sender, EventArgs e)
        {
            if (fullBMP != null)
            {
                viewRect = new RectangleF(viewRect.X, viewRect.Y, viewRect.Width, viewRect.Width * (float)pbThumbView.Width * (float)pbZoomView.Height / (float)(pbZoomView.Width * pbThumbView.Height));
                Update_Rects();
                Update_PictureBoxes();
            }
        }

        private void timerGridUpdate_Tick(object sender, EventArgs e)
        {
            Update_Grid();
        }

        private void pbThumbView_MouseEnter(object sender, EventArgs e)
        {
            pbThumbView.Focus();
        }

        private void pbThumbView_MouseLeave(object sender, EventArgs e)
        {
            pbThumbView.Focus();
        }

        private void pbZoomView_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            pbZoomView.Focus();

            if (e.Button == MouseButtons.Left)
            {
                dragZoom = true;
                ptGrab = e.Location;
            }
            else if(e.Button == MouseButtons.Right && moveStageOnClick)
            {
                double x01 = viewRect.X + viewRect.Width * (double)e.X / (double)pbZoomView.Width;
                double y01 = viewRect.Y + viewRect.Height * (double)e.Y / (double)pbZoomView.Height;

                System.Drawing.Size imgSize = sizes[4];//Image size at 1x

                double fullWidth = totalCols * imgSize.Width * (0.001 * pixelSizeUM / mag);
                double fullHeight = totalRows * imgSize.Height * (0.001 * pixelSizeUM / mag);

                //The extra math on the "x" is to account for the lines thrown away at the top of the scan
                double x = offsetXMM + y01 * fullHeight + ((camHeight - imgSize.Height - imgSize.Height/2) * (0.001 * pixelSizeUM / mag));
                double y = offsetYMM + x01 * fullWidth - ((imgSize.Width / 2) * (0.001 * pixelSizeUM / mag)); ;

                bool success = SetStagePosition(x, y);
            }
        }

        private void pbZoomView_MouseEnter(object sender, EventArgs e)
        {
            pbZoomView.Focus();
        }

        private void pbZoomView_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            if (dragZoom)
            {
                float dX = ((float)e.X - ptGrab.X) / (float)pbZoomView.Width;
                float dY = ((float)e.Y - ptGrab.Y) / (float)pbZoomView.Height;
                ptGrab = e.Location;

                viewRect = new RectangleF(viewRect.X - viewRect.Width * dX, viewRect.Y - viewRect.Height * dY, viewRect.Width, viewRect.Height);

                Update_Rects();
                Update_PictureBoxes();
            }
            else
            {
                //Get x and y in terms of whole slide by finding mouse location is terms of viewRect
                float x = viewRect.X + viewRect.Width * (float)e.X / (float)pbZoomView.Width;
                float y = viewRect.Y + viewRect.Height * (float)e.Y / (float)pbZoomView.Height;

                if (x >= 0 && x <= 1 && y >= 0 && y <= 1)
                {
                    //lbXCoord.Text = x.ToString("F3");
                    //lbYCoord.Text = y.ToString("F3");

                    //See what baseRect the x,y coord lies in
                    for (int r = 0; r < baseRects.GetLength(0); r++)
                    {
                        for (int c = 0; c < baseRects.GetLength(1); c++)
                        {
                            if (baseRects[r, c].Contains(x, y))
                            {
                                //lbCoord.Text = "(" + c.ToString() + "," + r.ToString() + ")";

                                if (ImageCoordChangedEvent != null)
                                    ImageCoordChangedEvent(c, r);

                                return;
                            }
                        }
                    }
                }
                else
                {
                    //lbXCoord.Text = @"N/A";
                    //lbYCoord.Text = @"N/A";
                }
            }
        }

        private void pbZoomView_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            dragZoom = false;
        }
        #endregion

        [DllImport(".\\Modules_Native\\LiveImageData.dll", EntryPoint = "SetStagePosition")]
        private static extern bool SetStagePosition(double x, double y);
        #endregion
    }
}
