namespace GeometryUtilities
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Media;
    using System.Windows.Shapes;
    using System.Xml;

    using HDF5CS;

    using OverlayManager;

    using ThorSharedTypes;

    public static class WaveformBuilder
    {
        #region Fields

        public const double GALVO_RATE = 1000.0; //Galvo: 1000Hz
        public const float INV_DIRECTION = -1;
        public const double MS_TO_S = 1000.0;
        public const double PULSE_MIN_US = 50.0; //[us]
        public const double US_TO_S = 1000000.0;

        private static double _field2Volts = 0.0901639344; // == FIELD2THETA
        static GGalvoWaveformParams _gWaveParams = new GGalvoWaveformParams() { GalvoWaveformXY = IntPtr.Zero, GalvoWaveformPockel = IntPtr.Zero, DigBufWaveform = IntPtr.Zero };
        static ThorDAQGGWaveformParams _thorDAQGGWaveformParams = new ThorDAQGGWaveformParams() { GalvoWaveformXY = IntPtr.Zero, GalvoWaveformPockel = IntPtr.Zero, DigBufWaveform = IntPtr.Zero };
        private static bool _inSaving;
        static PixelArray _pixelArray = new PixelArray();
        private static bool _saveSuccessed;
        static BleachWaveform _waveform = null;
        static BackgroundWorker _waveformSaver;

        #endregion Fields

        #region Delegates

        private delegate bool waveformSaverStateChecker();

        #endregion Delegates

        #region Properties

        public static int ClkRate
        {
            get;
            set;
        }

        public static double DeltaX_Px
        {
            get;
            set;
        }

        public static double Field2Volts
        {
            get
            {
                return _field2Volts;
            }
            set
            {
                _field2Volts = value;
            }
        }

        public static WAVEFORM_FILETYPE FileType
        {
            get;
            set;
        }

        /// <summary>
        /// minimum acceptable dwell time, unit: [us]
        /// </summary>
        public static double MinDwellTime
        {
            get
            {
                return (US_TO_S / ClkRate);
            }
        }

        public static string PathAndFilename
        {
            get;
            set;
        }

        public static double[] PowerIdle
        {
            get;
            set;
        }

        private static double DeltaX_volt
        {
            get;
            set;
        }

        private static double DeltaY_volt
        {
            get;
            set;
        }

        private static int HorizontalScanDirection
        {
            get;
            set;
        }

        private static double ImgRefX_volt
        {
            get;
            set;
        }

        private static double ImgRefY_volt
        {
            get;
            set;
        }

        private static double PixelX_Volt
        {
            get;
            set;
        }

        private static double PixelY_Volt
        {
            get;
            set;
        }

        /// <summary>
        /// Determine save [GalvoXY, Pockels, DigitalLines] or not.
        /// </summary>
        private static bool[] SaveType
        {
            get;
            set;
        }

        private static int VerticalScanDirection
        {
            get;
            set;
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Build contour waveform outside-in ellipse/circle region.
        /// </summary>
        /// <param name="bwParams"></param>
        /// <param name="Power"></param>
        public static void BuildEllipse(BleachWaveParams bwParams, double[] Power)
        {
            int preIdleCnt = (int)Math.Round(bwParams.PreIdleTime * (double)ClkRate / MS_TO_S);
            int dwellCnt = (int)Math.Floor(bwParams.DwellTime * (double)ClkRate / US_TO_S);
            int postIdleCnt = (int)Math.Round(bwParams.PostIdleTime * (double)ClkRate / MS_TO_S);

            double xRadiusPx = bwParams.ROIWidth / 2;
            double yRadiusPx = bwParams.ROIHeight / 2;
            double dTheta = 0, startPx = 0, newxRadiusPx = 0, newyRadiusPx = 0;
            int XSteps = (int)Math.Abs(Math.Ceiling(xRadiusPx * PixelX_Volt / DeltaX_volt));
            int YSteps = (int)Math.Abs(Math.Ceiling(yRadiusPx * PixelY_Volt / DeltaY_volt));
            int targetIteration = (int)Math.Min(XSteps, YSteps);
            GeometryTypes.EllipseType type = (xRadiusPx == yRadiusPx) ? GeometryTypes.EllipseType.Circle : GeometryTypes.EllipseType.Ellipse;
            List<Point> localVertices = new List<Point>();

            for (int it = 0; it <= targetIteration; it++)
            {
                localVertices.Clear();
                switch (type)
                {
                    case GeometryTypes.EllipseType.Circle:
                        newxRadiusPx = newyRadiusPx = xRadiusPx - (DeltaX_Px * it);
                        if (newxRadiusPx < 0)
                        {
                            return;
                        }
                        startPx = bwParams.Center.X + newxRadiusPx;
                        localVertices.Add(new Point(startPx, bwParams.Center.Y));
                        dTheta = Math.Acos(1 - ((Math.Pow((DeltaX_Px / newxRadiusPx), 2)) / 2));
                        for (int id = 1; id < (int)(2 * Math.PI / dTheta); id++)
                        {
                            localVertices.Add(new Point(newxRadiusPx * Math.Cos(id * dTheta) + bwParams.Center.X, newyRadiusPx * Math.Sin(id * dTheta) + bwParams.Center.Y));
                        }
                        break;
                    case GeometryTypes.EllipseType.Ellipse:
                        newxRadiusPx = xRadiusPx - (DeltaX_Px * it);
                        newyRadiusPx = yRadiusPx - (DeltaX_Px * it);
                        if ((newxRadiusPx < 0) || (newyRadiusPx < 0))
                        {
                            return;
                        }
                        startPx = bwParams.Center.X + newxRadiusPx;
                        localVertices.Add(new Point(startPx, bwParams.Center.Y));
                        dTheta = Math.Acos((Math.Pow(newxRadiusPx, 2) - Math.Sqrt(Math.Pow(newyRadiusPx, 4) + Math.Pow(DeltaX_Px, 2) * (Math.Pow(newxRadiusPx, 2) - Math.Pow(newyRadiusPx, 2)))) / (Math.Pow(newxRadiusPx, 2) - Math.Pow(newyRadiusPx, 2)));

                        for (int id = 1; id < (int)(2 * Math.PI / dTheta); id++)
                        {
                            localVertices.Add(new Point(newxRadiusPx * Math.Cos(id * dTheta) + bwParams.Center.X, newyRadiusPx * Math.Sin(id * dTheta) + bwParams.Center.Y));
                        }
                        break;
                }

                //start build:
                foreach (Point pt in localVertices)
                {
                    if (bwParams.PixelMode)
                    {
                        BuildTransitionPixelMode(ImgRefX_volt - HorizontalScanDirection * (pt.X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (pt.Y * PixelY_Volt), preIdleCnt, dwellCnt, postIdleCnt, Power);
                    }
                    else
                    {
                        BuildTransition(ImgRefX_volt - HorizontalScanDirection * (pt.X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (pt.Y * PixelY_Volt), dwellCnt, Power, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
                    }
                }

                //trace:
                if ((0 == bwParams.Fill))   //|| (it == targetIteration)
                {
                    //close polygon:
                    if (bwParams.PixelMode)
                    {
                        BuildTransitionPixelMode(ImgRefX_volt - HorizontalScanDirection * (localVertices[0].X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (localVertices[0].Y * PixelY_Volt), preIdleCnt, dwellCnt, postIdleCnt, Power);
                    }
                    else
                    {
                        BuildTransition(ImgRefX_volt - HorizontalScanDirection * (localVertices[0].X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (localVertices[0].Y * PixelY_Volt), dwellCnt, Power, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
                    }

                    return;
                }
            }
        }

        /// <summary>
        /// Build waveform along a straight line with modulation power.
        /// </summary>
        /// <param name="endPt"></param>
        /// <param name="bwParams"></param>
        /// <param name="Power"></param>
        public static void BuildLine(Point endPt, BleachWaveParams bwParams, double[] Power)
        {
            double TargetVecVx = ImgRefX_volt - HorizontalScanDirection * endPt.X * PixelX_Volt;
            double TargetVecVy = ImgRefY_volt + VerticalScanDirection * endPt.Y * PixelY_Volt;
            int preIdleCnt = (int)Math.Floor(bwParams.PreIdleTime * (double)ClkRate / MS_TO_S);
            int dwellCnt = (int)Math.Floor(bwParams.DwellTime * (double)ClkRate / US_TO_S);
            int postIdleCnt = (int)Math.Floor(bwParams.PostIdleTime * (double)ClkRate / MS_TO_S);
            if (bwParams.PixelMode)
            {
                BuildTransitionPixelMode(TargetVecVx, TargetVecVy, preIdleCnt, dwellCnt, postIdleCnt, Power);
            }
            else
            {
                BuildTransition(TargetVecVx, TargetVecVy, dwellCnt, Power, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
            }
        }

        /// <summary>
        /// Build Contour fill in Polygon, can be concave, convex, or self-intercepted.
        /// </summary>
        /// <param name="bwParams"></param>
        /// <param name="Power"></param>
        public static void BuildPolygon(BleachWaveParams bwParams, double[] Power)
        {
            int dwellCnt = (int)Math.Floor(bwParams.DwellTime * (double)ClkRate / US_TO_S);

            if (0 == bwParams.Fill)
            {
                BuildPolyTrace(bwParams, true, Power);
            }
            else
            {
                List<Point> localVertices = bwParams.Vertices;
                //polygon fill:
                Point rootTranslate = new Point();
                System.Drawing.Bitmap RootMap = ProcessBitmap.CreateBitmap(localVertices, true, true, 1, ref rootTranslate);
                List<System.Drawing.Bitmap> intermediateMap1 = new List<System.Drawing.Bitmap>();
                List<System.Drawing.Bitmap> intermediateMap2 = new List<System.Drawing.Bitmap>();
                List<System.Drawing.Bitmap> intermediateMap3 = new List<System.Drawing.Bitmap>();
                RootMap = RootMap.OpenPolygonBinaryFilter();
                intermediateMap1.Add(RootMap);

                while (intermediateMap1.Count > 0)
                {
                    for (int id = 0; id < intermediateMap1.Count; id++)
                    {
                        intermediateMap3 = BuildSinglePolygon(intermediateMap1[id], rootTranslate, bwParams, Power);

                        for (int it = 0; it < intermediateMap3.Count; it++)
                        {
                            intermediateMap2.Add(intermediateMap3[it]);
                        }
                    }
                    intermediateMap1.Clear();

                    for (int it = 0; it < intermediateMap2.Count; it++)
                    {
                        intermediateMap1.Add(intermediateMap2[it]);
                    }
                    intermediateMap2.Clear();
                    intermediateMap3.Clear();
                }
            }
        }

        /// <summary>
        /// Build trace following polygon vertices order, can be enclosed or not.
        /// </summary>
        /// <param name="bwParams"></param>
        /// <param name="closedPoly"></param>
        /// <param name="Power"></param>
        public static void BuildPolyTrace(BleachWaveParams bwParams, bool closedPoly, double[] Power)
        {
            int preIdleCnt = (int)Math.Round(bwParams.PreIdleTime * (double)ClkRate / MS_TO_S);
            int dwellCnt = (int)Math.Floor(bwParams.DwellTime * (double)ClkRate / US_TO_S);
            int postIdleCnt = (int)Math.Round(bwParams.PostIdleTime * (double)ClkRate / MS_TO_S);

            List<Point> localVertices = bwParams.Vertices;

            foreach (Point pt in localVertices)
            {
                if (bwParams.PixelMode)
                {
                    BuildTransitionPixelMode(ImgRefX_volt - HorizontalScanDirection * (pt.X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (pt.Y * PixelY_Volt), preIdleCnt, dwellCnt, postIdleCnt, Power);
                }
                else
                {
                    BuildTransition(ImgRefX_volt - HorizontalScanDirection * (pt.X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (pt.Y * PixelY_Volt), dwellCnt, Power, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
                }
            }
            if (closedPoly)
            {
                if (bwParams.PixelMode)
                {
                    BuildTransitionPixelMode(ImgRefX_volt - HorizontalScanDirection * (localVertices[0].X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (localVertices[0].Y * PixelY_Volt), preIdleCnt, dwellCnt, postIdleCnt, Power);
                }
                else
                {
                    BuildTransition(ImgRefX_volt - HorizontalScanDirection * (localVertices[0].X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (localVertices[0].Y * PixelY_Volt), dwellCnt, Power, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
                }
            }
        }

        public static void BuildPostIdle(BleachWaveParams bwParams)
        {
            //dwell time first:
            int postIdleCnt = (int)Math.Floor(bwParams.PostIdleTime * (double)ClkRate / MS_TO_S);
            int count = (postIdleCnt > 1) ? (postIdleCnt - 1) : postIdleCnt;
            WaveformBuilder.BuildModulation(count, PowerIdle, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);

            //prepare digital lines' transition:
            WaveformBuilder.BuildModulation(1, PowerIdle, 0, (byte)1, (byte)0, (byte)1, (byte)0, (byte)1, (byte)1);
        }

        public static void BuildPostPatIdle(BleachWaveParams bwParams, bool epochEnd, bool cycleEnd)
        {
            //leave PULSE_MIN_US for pattern complete trigger pulse:
            double pulseTime = Math.Max((US_TO_S / WaveformBuilder.ClkRate), PULSE_MIN_US);
            int pulseClkCount = Math.Max((int)(pulseTime * WaveformBuilder.ClkRate / US_TO_S), (int)1);

            //dwell time for post pattern idle:
            int postPatIdleCnt = (int)Math.Floor(bwParams.PostPatIdleTime * (double)ClkRate / MS_TO_S);
            int count = (postPatIdleCnt > pulseClkCount) ? (postPatIdleCnt - pulseClkCount) : postPatIdleCnt;
            WaveformBuilder.BuildModulation(count, PowerIdle, 0, (byte)1, (byte)0, (byte)1, (byte)1, (byte)1, (byte)1);

            //dwell pattern complete trigger pulse:
            WaveformBuilder.BuildModulation(pulseClkCount, PowerIdle, 0, (byte)1, (byte)0, (byte)0, (byte)1, (byte)1, (byte)1);

            //dwell time for post epoch idle:
            if (epochEnd)
            {
                int postActiveIdleCnt = (int)Math.Floor(bwParams.PostEpochIdleMS * (double)ClkRate / MS_TO_S);
                int actCount = (postActiveIdleCnt > pulseClkCount) ? (postActiveIdleCnt - pulseClkCount) : postActiveIdleCnt;
                WaveformBuilder.BuildModulation(actCount, PowerIdle, 0, (byte)1, (byte)0, (byte)0, (byte)1, (byte)1, (byte)1);
            }

            //dwell time for post cycle idle:
            if (cycleEnd)
            {
                int postCycleIdleCnt = (int)Math.Floor(bwParams.PostCycleIdleMS * (double)ClkRate / MS_TO_S);
                int cyCount = (postCycleIdleCnt > pulseClkCount) ? (postCycleIdleCnt - pulseClkCount) : postCycleIdleCnt;
                WaveformBuilder.BuildModulation(cyCount, PowerIdle, 0, (byte)1, (byte)0, (byte)0, (byte)1, (byte)0, (byte)1);
            }

            //set cycle digital line transition:
            byte cycleEnv = (cycleEnd) ? (byte)0 : (byte)1;
            byte epochEnv = (epochEnd) ? (byte)0 : (byte)1;
            WaveformBuilder.BuildModulation(1, PowerIdle, 0, cycleEnv, (byte)0, (byte)0, (byte)1, epochEnv, (byte)1);
        }

        public static void BuildPreIdle(BleachWaveParams bwParams)
        {
            //prepare digital lines' transition first:
            WaveformBuilder.BuildModulation(1, PowerIdle, 0, (byte)1, (byte)0, (byte)1, (byte)0, (byte)1, (byte)1);

            //dwell time:
            int preIdleCnt = (int)Math.Floor(bwParams.PreIdleTime * (double)ClkRate / MS_TO_S);
            int count = (preIdleCnt > 1) ? (preIdleCnt - 1) : preIdleCnt;
            WaveformBuilder.BuildModulation(count, PowerIdle, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
        }

        public static void BuildPrePatIdle(BleachWaveParams bwParams, bool cycleBegin, bool epochBegin)
        {
            //set cycle line transition first:
            byte cycleEnv = (cycleBegin) ? (byte)0 : (byte)1;
            byte epochEnv = (epochBegin) ? (byte)0 : (byte)1;
            WaveformBuilder.BuildModulation(1, PowerIdle, 0, cycleEnv, (byte)0, (byte)0, (byte)0, epochEnv, (byte)1);

            //dwell time for cycle idle:
            if (cycleBegin)
            {
                int preCycleIdleCnt = (int)Math.Floor(bwParams.PreCycleIdleMS * (double)ClkRate / MS_TO_S);
                int cyCount = (preCycleIdleCnt > 1) ? (preCycleIdleCnt - 1) : preCycleIdleCnt;
                WaveformBuilder.BuildModulation(cyCount, PowerIdle, (byte)0, (byte)1, (byte)0, (byte)0, (byte)0, epochEnv, (byte)1);
            }

            //set epoch line transition:
            WaveformBuilder.BuildModulation(1, PowerIdle, 0, (byte)1, (byte)0, (byte)0, (byte)0, epochEnv, (byte)1);

            //dwell time for epoch idle:
            if (epochBegin)
            {
                int preActiveIdleCnt = (int)Math.Floor(bwParams.PreEpochIdleMS * (double)ClkRate / MS_TO_S);
                int actCount = (preActiveIdleCnt > 1) ? (preActiveIdleCnt - 1) : preActiveIdleCnt;
                WaveformBuilder.BuildModulation(actCount, PowerIdle, (byte)0, (byte)1, (byte)0, (byte)0, (byte)0, (byte)1, (byte)1);
            }

            //dwell time for pattern idle:
            int prePatIdleCnt = (int)Math.Floor(bwParams.PrePatIdleTime * (double)ClkRate / MS_TO_S);
            int count = (prePatIdleCnt > 1) ? (prePatIdleCnt - 1) : prePatIdleCnt;
            WaveformBuilder.BuildModulation(count, PowerIdle, (byte)0, (byte)1, (byte)0, (byte)1, (byte)0, (byte)1, (byte)1);
        }

        /// <summary>
        /// Build contour waveform outside-in rectangle region.
        /// </summary>
        /// <param name="bwParams"></param>
        /// <param name="Power"></param>
        public static void BuildRectContour(BleachWaveParams bwParams, double[] Power)
        {
            int preIdleCnt = (int)Math.Round(bwParams.PreIdleTime * (double)ClkRate / MS_TO_S);
            int dwellCnt = (int)Math.Floor(bwParams.DwellTime * (double)ClkRate / US_TO_S);
            int postIdleCnt = (int)Math.Round(bwParams.PostIdleTime * (double)ClkRate / MS_TO_S);

            List<Point> localVertices = bwParams.Vertices;

            double midX_Px = (localVertices[1].X + localVertices[0].X) / 2;
            double midY_Px = (localVertices[2].Y + localVertices[1].Y) / 2;
            int targetIteration = (int)Math.Min(Math.Ceiling((localVertices[2].Y - localVertices[1].Y) * PixelY_Volt / DeltaY_volt / 2), Math.Ceiling((localVertices[1].X - localVertices[0].X) * PixelX_Volt / DeltaX_volt / 2));
            bool done = false;
            List<Point> finalVertices = new List<Point>();

            for (int it = 0; it <= targetIteration; it++)
            {
                //check if past boundary:
                if ((localVertices[0].X + (it * DeltaX_Px) > midX_Px) || (localVertices[0].Y + (it * DeltaX_Px) > midY_Px))
                {
                    return;
                }

                finalVertices.Clear();
                finalVertices.Add(new Point(localVertices[0].X + (it * DeltaX_Px), localVertices[0].Y + (it * DeltaX_Px)));
                finalVertices.Add(new Point(localVertices[1].X - (it * DeltaX_Px), localVertices[1].Y + (it * DeltaX_Px)));
                finalVertices.Add(new Point(localVertices[2].X - (it * DeltaX_Px), localVertices[2].Y - (it * DeltaX_Px)));
                finalVertices.Add(new Point(localVertices[3].X + (it * DeltaX_Px), localVertices[3].Y - (it * DeltaX_Px)));
                //one step short to avoid double visit at Top-Left corner:
                if (finalVertices[0].Y + (DeltaX_Px) < finalVertices[3].Y)
                {
                    finalVertices.Add(new Point(finalVertices[0].X, finalVertices[0].Y + (DeltaX_Px)));
                }

                //sub-grid filling for the last contour:
                if (((localVertices[1].X - localVertices[0].X - (2 * it * DeltaX_Px)) < DeltaX_Px) || ((localVertices[2].Y - localVertices[1].Y - (2 * it * DeltaX_Px)) < DeltaX_Px))
                {
                    done = true;
                }

                //start build:
                foreach (Point pt in finalVertices)
                {
                    if (bwParams.PixelMode)
                    {
                        BuildTransitionPixelMode(ImgRefX_volt - HorizontalScanDirection * (pt.X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (pt.Y * PixelY_Volt), preIdleCnt, dwellCnt, postIdleCnt, Power);
                    }
                    else
                    {
                        BuildTransition(ImgRefX_volt - HorizontalScanDirection * (pt.X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (pt.Y * PixelY_Volt), dwellCnt, Power, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
                    }
                }
                if (done)
                {
                    return;
                }

                //trace:
                if (0 == bwParams.Fill)
                {
                    //close rect:
                    if (bwParams.PixelMode)
                    {
                        BuildTransitionPixelMode(ImgRefX_volt - HorizontalScanDirection * (localVertices[0].X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (localVertices[0].Y * PixelY_Volt), preIdleCnt, dwellCnt, postIdleCnt, Power);
                    }
                    else
                    {
                        BuildTransition(ImgRefX_volt - HorizontalScanDirection * (localVertices[0].X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (localVertices[0].Y * PixelY_Volt), dwellCnt, Power, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
                    }
                    return;
                }
            }
        }

        /// <summary>
        /// Build Zig-Zag waveform top to down across rectangle region; equal or smaller than target rectangle.
        /// </summary>
        /// <param name="bwParams"></param>
        /// <param name="Power"></param>
        public static void BuildRectTopDown(BleachWaveParams bwParams, double[] Power)
        {
            int preIdleCnt = (int)Math.Round(bwParams.PreIdleTime * (double)ClkRate / MS_TO_S);
            int dwellCnt = (int)Math.Floor(bwParams.DwellTime * (double)ClkRate / US_TO_S);
            int postIdleCnt = (int)Math.Round(bwParams.PostIdleTime * (double)ClkRate / MS_TO_S);
            int XStepsPerLine = (int)Math.Abs(Math.Floor((bwParams.ROIRight - bwParams.ROILeft) / DeltaX_Px));
            int YStepsPerIteration = (int)Math.Abs(Math.Floor((bwParams.ROIBottom - bwParams.ROITop) / DeltaX_Px));

            float xDirection = 1;
            float yDirection = 1;
            double startPx = 0, targetPx = 0, targetPy = 0, rightEndPx = bwParams.ROILeft + (XStepsPerLine * DeltaX_Px);

            //Build
            for (int iy = 0; iy <= YStepsPerIteration; iy++)
            {
                //horizontal transit:
                xDirection = (iy % 2 == 0) ? 1 : INV_DIRECTION;
                startPx = (iy % 2 == 0) ? bwParams.ROILeft : rightEndPx;
                targetPx = startPx + (xDirection * XStepsPerLine * DeltaX_Px);
                targetPy = bwParams.ROITop + (iy * yDirection * DeltaX_Px);

                if (bwParams.PixelMode)
                {
                    BuildTransitionPixelMode(ImgRefX_volt - HorizontalScanDirection * (targetPx * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (targetPy * PixelY_Volt), preIdleCnt, dwellCnt, postIdleCnt, Power);
                }
                else
                {
                    BuildTransition(ImgRefX_volt - HorizontalScanDirection * (targetPx * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (targetPy * PixelY_Volt), dwellCnt, Power, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
                }

                //vertical transit, skip at the last y step:
                if (iy < Math.Abs(YStepsPerIteration))
                {
                    targetPy = bwParams.ROITop + ((iy + 1) * yDirection * DeltaX_Px);
                    if (bwParams.PixelMode)
                    {
                        BuildTransitionPixelMode(ImgRefX_volt - HorizontalScanDirection * (targetPx * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (targetPy * PixelY_Volt), preIdleCnt, dwellCnt, postIdleCnt, Power);
                    }
                    else
                    {
                        BuildTransition(ImgRefX_volt - HorizontalScanDirection * (targetPx * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (targetPy * PixelY_Volt), dwellCnt, Power, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
                    }
                }
            }
        }

        public static void BuildSpot(BleachWaveParams bwParams, double[] Power)
        {
            int preIdleCnt = (int)Math.Floor(bwParams.PreIdleTime * (double)ClkRate / MS_TO_S);
            int dwellCnt = (int)Math.Floor(bwParams.DwellTime * (double)ClkRate / US_TO_S);
            int postIdleCnt = (int)Math.Floor(bwParams.PostIdleTime * (double)ClkRate / MS_TO_S);
            if (bwParams.PixelMode)
            {
                WaveformBuilder.BuildModulation(preIdleCnt, PowerIdle, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
            }

            WaveformBuilder.BuildModulation(dwellCnt, Power, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);

            if (bwParams.PixelMode)
            {
                WaveformBuilder.BuildModulation(postIdleCnt, PowerIdle, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
            }
        }

        /// <summary>
        /// travel without power.
        /// </summary>
        /// <param name="target"></param>
        public static void BuildTravel(Point target, byte cycleEnvelope, byte epochEnvelope, byte patnEnvelope)
        {
            double TargetVecVx = ImgRefX_volt - HorizontalScanDirection * target.X * PixelX_Volt;
            double TargetVecVy = ImgRefY_volt + VerticalScanDirection * target.Y * PixelY_Volt;
            BuildTransition(TargetVecVx, TargetVecVy, 0, PowerIdle, 0, cycleEnvelope, (byte)0, patnEnvelope, (byte)0, epochEnvelope, (byte)1);
        }

        public static bool CheckSaveState()
        {
            if (_waveformSaver != null)
            {
                if (_inSaving)
                {
                    waveformSaverStateChecker stillWorking = () => { return _waveformSaver.IsBusy; };
                    return !(bool)Application.Current.Dispatcher.Invoke(stillWorking, null);
                }
                else
                {
                    return true;
                }
            }
            else
            {
                return (_inSaving) ? false : true;
            }
        }

        /// <summary>
        /// This function will copy an unsigned short array to an IntPtr Buffer.
        /// There is no Mashal.Copy function for an unsigned Short array, so this function
        /// bridges that gap
        /// </summary>
        /// <param name="source"></param>
        /// <param name="sourceOffset"></param>
        /// <param name="ptrDest"></param>
        /// <param name="elements"></param>
        public static unsafe void CopyUShortToIntPtr(ushort[] source, int sourceOffset, IntPtr ptrDest, int elements)
        {
            fixed (ushort* ptrSource = &source[sourceOffset])
            {
                CopyMemory(ptrDest, (IntPtr)ptrSource, (uint)elements * 2);    // 2 bytes per element
            }
        }

        /// <summary>
        /// Call this function to free the unmanaged memory allocated for the _gWaveParams and _thorDAQGGWaveformParams structures
        /// </summary>
        public static void FreeGWaveParamsMemory()
        {
            if (_gWaveParams.GalvoWaveformXY != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(_gWaveParams.GalvoWaveformXY);
                _gWaveParams.GalvoWaveformXY = IntPtr.Zero;
            }

            if (_gWaveParams.GalvoWaveformPockel != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(_gWaveParams.GalvoWaveformPockel);
                _gWaveParams.GalvoWaveformPockel = IntPtr.Zero;
            }

            if (_gWaveParams.DigBufWaveform != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(_gWaveParams.DigBufWaveform);
                _gWaveParams.DigBufWaveform = IntPtr.Zero;
            }

            if (_thorDAQGGWaveformParams.GalvoWaveformXY != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(_thorDAQGGWaveformParams.GalvoWaveformXY);
                _thorDAQGGWaveformParams.GalvoWaveformXY = IntPtr.Zero;
            }

            if (_thorDAQGGWaveformParams.GalvoWaveformPockel != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(_thorDAQGGWaveformParams.GalvoWaveformPockel);
                _thorDAQGGWaveformParams.GalvoWaveformPockel = IntPtr.Zero;
            }

            if (_thorDAQGGWaveformParams.DigBufWaveform != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(_thorDAQGGWaveformParams.DigBufWaveform);
                _thorDAQGGWaveformParams.DigBufWaveform = IntPtr.Zero;
            }
        }

        public static PixelArray GetPixelArray()
        {
            return _pixelArray;
        }

        /// <summary>
        /// Calculate pockels power in voltage based on response type
        /// </summary>
        /// <param name="percent"></param>
        /// <param name="minV"></param>
        /// <param name="maxV"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        public static double GetPockelsPowerValue(double percent, double minV, double maxV, PockelsResponseType type)
        {
            const double AREA_UNDER_CURVE = 2.0;
            double linearVal = 0;
            switch (type)
            {
                case PockelsResponseType.LINEAR_RESPONSE:   //linear response
                    linearVal = percent / 100.0;
                    return (minV + linearVal * (maxV - minV));
                case PockelsResponseType.SINE_RESPONSE:    //linearize the sine wave response of the pockels cell

                    linearVal = (Math.Acos(1 - AREA_UNDER_CURVE * Math.Max(0, Math.Min(100.0, percent)) / 100.0) / Math.PI);
                    return (minV + linearVal * (maxV - minV));
                default:
                    return -1.0;
            }
        }

        public static bool GetSaveResult()
        {
            return _saveSuccessed;
        }

        public static BleachWaveform GetWaveform()
        {
            return _waveform;
        }

        public static bool InitializeParams(int fieldSize, double[] fieldScaleFine, int[] Pixel, int[] OffsetActual, double[] OffsetFine, double ScaleYScan, int verticalFlipScan, int horizontalFlipScan, double[] PwIdle, WaveformDriverType waveformDriverType)
        {
            //Calculate parameters:
            if ((fieldScaleFine.Count() != 2) || (Pixel.Count() != 2) || (OffsetActual.Count() != 2) || (OffsetFine.Count() != 2))
            {
                return false;
            }

            HorizontalScanDirection = (1 == horizontalFlipScan) ? (-1) : 1;
            //top-down: 1, bottom-up: -1
            VerticalScanDirection = (1 == verticalFlipScan) ? (-1) : 1;
            PowerIdle = PwIdle;

            double halfCenterFieldX_volt = (fieldSize * Field2Volts / 2) * fieldScaleFine[0];
            double halfCenterFieldY_volt = (fieldSize * Field2Volts / 2) * ((double)Pixel[1] / (double)Pixel[0]) * ScaleYScan * fieldScaleFine[1];
            double offsetX_volt_Dir = (double)OffsetActual[0] * Field2Volts + OffsetFine[0];
            double offsetY_volt_Dir = VerticalScanDirection * (double)OffsetActual[1] * Field2Volts + OffsetFine[1];
            double LeftX_volt = offsetX_volt_Dir - halfCenterFieldX_volt;
            double RightX_volt = offsetX_volt_Dir + halfCenterFieldX_volt;
            double TopY_volt = offsetY_volt_Dir - halfCenterFieldY_volt;
            double BotY_volt = offsetY_volt_Dir + halfCenterFieldY_volt;

            ImgRefX_volt = (1 == HorizontalScanDirection) ? RightX_volt : LeftX_volt;
            ImgRefY_volt = (1 == VerticalScanDirection) ? TopY_volt : BotY_volt;

            DeltaX_volt = 2 * halfCenterFieldX_volt * GALVO_RATE / ClkRate;
            DeltaY_volt = DeltaX_volt;
            PixelX_Volt = 2 * halfCenterFieldX_volt / Pixel[0];
            PixelY_Volt = 2 * halfCenterFieldY_volt / Pixel[1];
            DeltaX_Px = DeltaX_volt / WaveformBuilder.PixelX_Volt;

            _waveform = new BleachWaveform(waveformDriverType);

            return true;
        }

        public static List<BleachWaveParams> LoadBleachWaveParams(string pathAndName, ROICapsule roiCapsule, double umPerPixel, double umPerPixelRatio, int binX, int binY)
        {
            List<BleachWaveParams> bParamList = new List<BleachWaveParams>();

            XmlDocument doc = new XmlDocument();
            XmlTextReader reader = new XmlTextReader(pathAndName);
            doc.Load(reader);
            XmlNodeList xnodes = doc.DocumentElement.ChildNodes.Item(0).ChildNodes.Item(0).ChildNodes;

            uint idx = 0;
            string str = string.Empty;

            foreach (XmlNode node in xnodes)
            {
                BleachWaveParams rec = new BleachWaveParams();
                rec.PreIdleTime = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 0, ref str)) ? (Convert.ToDouble(str, CultureInfo.CurrentCulture)) : 0;
                rec.ClockRate = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 7, ref str)) ? (Convert.ToInt32(str)) : 80000;
                rec.UMPerPixel = umPerPixel;
                rec.UMPerPixelRatio = umPerPixelRatio;
                ClkRate = rec.ClockRate;
                rec.DwellTime = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 1, ref str)) ? Math.Max(MinDwellTime, (Convert.ToDouble(str, CultureInfo.CurrentCulture))) : MinDwellTime;
                rec.PostIdleTime = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 2, ref str)) ? (Convert.ToDouble(str, CultureInfo.CurrentCulture)) : 0;
                rec.Iterations = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 3, ref str)) ? (Convert.ToInt32(str)) : 1;
                if (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 4, ref str))
                { rec.Fill = Convert.ToInt32(str, CultureInfo.CurrentCulture); }
                else
                { rec.Fill = (int)BleachWaveParams.FillChoice.Tornado; }
                if (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 5, ref str))
                { rec.PixelMode = (1 == Convert.ToInt32(str, CultureInfo.CurrentCulture)) ? true : false; }
                else
                { rec.PixelMode = false; }
                rec.Power = new double[1] { (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 6, ref str)) ? (Convert.ToDouble(str, CultureInfo.CurrentCulture)) : 0 };
                rec.LongIdleTime = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 8, ref str)) ? (Convert.ToDouble(str, CultureInfo.CurrentCulture)) : 0.0;
                rec.PrePatIdleTime = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 9, ref str)) ? (Convert.ToDouble(str, CultureInfo.CurrentCulture)) : 0.0;
                rec.PostPatIdleTime = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 10, ref str)) ? (Convert.ToDouble(str, CultureInfo.CurrentCulture)) : 0.0;
                rec.PreCycleIdleMS = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 11, ref str)) ? (Convert.ToDouble(str, CultureInfo.CurrentCulture)) : 0.0;
                rec.PostCycleIdleMS = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 12, ref str)) ? (Convert.ToDouble(str, CultureInfo.CurrentCulture)) : 0.0;
                rec.PreEpochIdleMS = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 13, ref str)) ? (Convert.ToDouble(str, CultureInfo.CurrentCulture)) : 0.0;
                rec.PostEpochIdleMS = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 14, ref str)) ? (Convert.ToDouble(str, CultureInfo.CurrentCulture)) : 0.0;
                rec.EpochCount = (OverlayManager.BleachClass.GetBleachAttribute(node, doc, 15, ref str)) ? (Convert.ToInt32(str)) : 1;

                //ROIs Params: STATSONLY mode, PATTERN_NOSTATS is for SLM:
                if ((ThorSharedTypes.Mode.STATSONLY != (ThorSharedTypes.Mode)(OverlayManager.OverlayManagerClass.GetTagByteSection(roiCapsule.ROIs[idx].Tag, ThorSharedTypes.Tag.MODE, OverlayManager.OverlayManagerClass.SecR))))
                {
                    idx++;
                    continue;
                }
                if (roiCapsule.ROIs[idx].GetType() == typeof(OverlayManager.ROIRect))
                {
                    rec.shapeType = "Rectangle";
                    rec.ROIWidth = Math.Round(Convert.ToDouble(((OverlayManager.ROIRect)(roiCapsule.ROIs[idx])).ROIWidth * binX, CultureInfo.CurrentCulture), 2);
                    rec.ROIHeight = Math.Round(Convert.ToDouble(((OverlayManager.ROIRect)(roiCapsule.ROIs[idx])).ROIHeight * binY, CultureInfo.CurrentCulture), 2);
                    rec.ROITop = ((OverlayManager.ROIRect)(roiCapsule.ROIs[idx])).TopLeft.Y * binY;
                    rec.ROILeft = ((OverlayManager.ROIRect)(roiCapsule.ROIs[idx])).TopLeft.X * binX;
                    rec.ROIBottom = ((OverlayManager.ROIRect)(roiCapsule.ROIs[idx])).BottomRight.Y * binY;
                    rec.ROIRight = ((OverlayManager.ROIRect)(roiCapsule.ROIs[idx])).BottomRight.X * binX;
                    rec.Vertices.Clear();
                    rec.Vertices.Add(new Point(rec.ROILeft, rec.ROITop));
                    rec.Vertices.Add(new Point(rec.ROIRight, rec.ROITop));
                    rec.Vertices.Add(new Point(rec.ROIRight, rec.ROIBottom));
                    rec.Vertices.Add(new Point(rec.ROILeft, rec.ROIBottom));
                    rec.VerticeCount = 4;
                }
                else if (roiCapsule.ROIs[idx].GetType() == typeof(OverlayManager.ROIPoly))
                {
                    rec.shapeType = "Polygon";
                    //make copy:
                    Point tmp;
                    Application.Current.Dispatcher.Invoke((Action)(() =>
                    {
                        int cnt = ((OverlayManager.ROIPoly)(roiCapsule.ROIs[idx])).Points.Count;
                        for (int i = 0; i < cnt; i++)
                        {
                            double px = ((OverlayManager.ROIPoly)(roiCapsule.ROIs[idx])).Points[i].X * binX;
                            double py = ((OverlayManager.ROIPoly)(roiCapsule.ROIs[idx])).Points[i].Y * binY;
                            tmp = new Point(px, py);
                            rec.Vertices.Add(tmp);
                        }
                    }));
                    rec.VerticeCount = rec.Vertices.Count;
                    rec.ROIWidth = Math.Round(Convert.ToDouble(rec.GetPolyRight() - rec.GetPolyLeft(), CultureInfo.CurrentCulture), 2);
                    rec.ROIHeight = Math.Round(Convert.ToDouble(rec.GetPolyBottom() - rec.GetPolyTop(), CultureInfo.CurrentCulture), 2);
                    rec.ROITop = rec.GetPolyTop();
                    rec.ROILeft = rec.GetPolyLeft();
                    rec.ROIBottom = rec.GetPolyBottom();
                    rec.ROIRight = rec.GetPolyRight();
                }
                else if (roiCapsule.ROIs[idx].GetType() == typeof(OverlayManager.ROICrosshair))
                {
                    rec.shapeType = "Crosshair";
                    rec.ROIWidth = 1 * binX;
                    rec.ROIHeight = 1 * binY;
                    rec.ROITop = ((OverlayManager.ROICrosshair)(roiCapsule.ROIs[idx])).CenterPoint.Y * binY;
                    rec.ROILeft = ((OverlayManager.ROICrosshair)(roiCapsule.ROIs[idx])).CenterPoint.X * binX;
                    rec.ROIBottom = rec.ROITop;
                    rec.ROIRight = rec.ROILeft;
                    rec.ZValue = ((OverlayManager.ROICrosshair)(roiCapsule.ROIs[idx])).ZValue;
                }
                else if (roiCapsule.ROIs[idx].GetType() == typeof(Line))
                {
                    rec.shapeType = "Line";
                    rec.ROIWidth = 1 * binX;
                    rec.ROIHeight = 1 * binY;
                    rec.ROITop = ((Line)(roiCapsule.ROIs[idx])).Y1 * binY;
                    rec.ROILeft = ((Line)(roiCapsule.ROIs[idx])).X1 * binX;
                    rec.ROIBottom = ((Line)(roiCapsule.ROIs[idx])).Y2 * binY;
                    rec.ROIRight = ((Line)(roiCapsule.ROIs[idx])).X2 * binX;
                }
                else if (roiCapsule.ROIs[idx].GetType() == typeof(Polyline))
                {
                    rec.shapeType = "Polyline";
                    Point tmp;
                    Application.Current.Dispatcher.Invoke((Action)(() =>
                    {
                        int cnt = ((Polyline)(roiCapsule.ROIs[idx])).Points.Count;
                        for (int i = 0; i < cnt; i++)
                        {
                            double px = ((Polyline)(roiCapsule.ROIs[idx])).Points[i].X * binX;
                            double py = ((Polyline)(roiCapsule.ROIs[idx])).Points[i].Y * binY;
                            tmp = new Point(px, py);
                            rec.Vertices.Add(tmp);
                        }
                    }));
                    rec.VerticeCount = rec.Vertices.Count;
                    rec.ROIWidth = 1 * binX;
                    rec.ROIHeight = 1 * binY;
                    rec.ROITop = rec.GetPolyTop();
                    rec.ROILeft = rec.GetPolyLeft();
                    rec.ROIBottom = rec.GetPolyBottom();
                    rec.ROIRight = rec.GetPolyRight();
                }
                else if (roiCapsule.ROIs[idx].GetType() == typeof(OverlayManager.ROIEllipse))
                {
                    rec.shapeType = "Ellipse";
                    rec.ROIWidth = Math.Round(Convert.ToDouble(((OverlayManager.ROIEllipse)(roiCapsule.ROIs[idx])).ROIWidth * binX, CultureInfo.CurrentCulture), 2);
                    rec.ROIHeight = Math.Round(Convert.ToDouble(((OverlayManager.ROIEllipse)(roiCapsule.ROIs[idx])).ROIHeight * binY, CultureInfo.CurrentCulture), 2);
                    rec.Center = new Point(((OverlayManager.ROIEllipse)(roiCapsule.ROIs[idx])).Center.X * binX, ((OverlayManager.ROIEllipse)(roiCapsule.ROIs[idx])).Center.Y * binY);
                    rec.ROITop = Math.Round((rec.Center.Y * binY - ((OverlayManager.ROIEllipse)(roiCapsule.ROIs[idx])).ROIHeight / 2), 2);
                    rec.ROILeft = Math.Round((rec.Center.X * binX - ((OverlayManager.ROIEllipse)(roiCapsule.ROIs[idx])).ROIWidth / 2), 2);
                    rec.ROIBottom = Math.Round((rec.Center.Y * binY + ((OverlayManager.ROIEllipse)(roiCapsule.ROIs[idx])).ROIHeight / 2), 2);
                    rec.ROIRight = Math.Round((rec.Center.X * binX + ((OverlayManager.ROIEllipse)(roiCapsule.ROIs[idx])).ROIWidth / 2), 2);
                    rec.ZValue = ((OverlayManager.ROIEllipse)(roiCapsule.ROIs[idx])).ZValue; ;
                }
                rec.ID = (uint)(((int[])roiCapsule.ROIs[idx].Tag)[(int)Tag.SUB_PATTERN_ID]);
                bParamList.Add(rec);
                idx++;
            }
            reader.Close();
            return bParamList;
        }

        public static void PixelEllipse(BleachWaveParams bwParams)
        {
            double xRadiusPx = bwParams.ROIWidth / 2;
            double yRadiusPx = bwParams.ROIHeight / 2;
            double dTheta = 0, startPx = 0, newxRadiusPx = 0, newyRadiusPx = 0;
            int XSteps = (int)Math.Abs(Math.Ceiling(xRadiusPx * PixelX_Volt / DeltaX_volt));
            int YSteps = (int)Math.Abs(Math.Ceiling(yRadiusPx * PixelY_Volt / DeltaY_volt));
            int targetIteration = (int)Math.Min(XSteps, YSteps);
            GeometryTypes.EllipseType type = (xRadiusPx == yRadiusPx) ? GeometryTypes.EllipseType.Circle : GeometryTypes.EllipseType.Ellipse;
            List<Point> localVertices = new List<Point>();
            BleachWaveParams tmpParams = new BleachWaveParams();

            for (int it = 0; it <= targetIteration; it++)
            {
                localVertices.Clear();
                tmpParams.Vertices.Clear();
                switch (type)
                {
                    case GeometryTypes.EllipseType.Circle:
                        newxRadiusPx = newyRadiusPx = xRadiusPx - (DeltaX_Px * it);
                        if (newxRadiusPx < 0)
                        {
                            return;
                        }
                        startPx = bwParams.Center.X + newxRadiusPx;
                        localVertices.Add(new Point(startPx, bwParams.Center.Y));
                        dTheta = Math.Acos(1 - ((Math.Pow((DeltaX_Px / newxRadiusPx), 2)) / 2));
                        for (int id = 1; id < (int)(2 * Math.PI / dTheta); id++)
                        {
                            localVertices.Add(new Point(newxRadiusPx * Math.Cos(id * dTheta) + bwParams.Center.X, newyRadiusPx * Math.Sin(id * dTheta) + bwParams.Center.Y));
                        }
                        break;
                    case GeometryTypes.EllipseType.Ellipse:
                        newxRadiusPx = xRadiusPx - (DeltaX_Px * it);
                        newyRadiusPx = yRadiusPx - (DeltaX_Px * it);
                        if ((newxRadiusPx < 0) || (newyRadiusPx < 0))
                        {
                            return;
                        }
                        startPx = bwParams.Center.X + newxRadiusPx;
                        localVertices.Add(new Point(startPx, bwParams.Center.Y));
                        dTheta = Math.Acos((Math.Pow(newxRadiusPx, 2) - Math.Sqrt(Math.Pow(newyRadiusPx, 4) + Math.Pow(DeltaX_Px, 2) * (Math.Pow(newxRadiusPx, 2) - Math.Pow(newyRadiusPx, 2)))) / (Math.Pow(newxRadiusPx, 2) - Math.Pow(newyRadiusPx, 2)));

                        for (int id = 1; id < (int)(2 * Math.PI / dTheta); id++)
                        {
                            localVertices.Add(new Point(newxRadiusPx * Math.Cos(id * dTheta) + bwParams.Center.X, newyRadiusPx * Math.Sin(id * dTheta) + bwParams.Center.Y));
                        }
                        break;
                }

                //start build:
                tmpParams.Vertices = localVertices;
                PixelPolyTrace(tmpParams, true);

                //trace:
                if ((0 == bwParams.Fill))
                {
                    return;
                }
            }
        }

        public static void PixelLine(BleachWaveParams bwParams)
        {
            _pixelArray.AddPoint(bwParams.ROILeft, bwParams.ROITop);
            PixelTransition(new Point(bwParams.ROIRight, bwParams.ROIBottom));
        }

        public static void PixelPolygon(BleachWaveParams bwParams)
        {
            if (0 == bwParams.Fill)
            {
                PixelPolyTrace(bwParams, true);
            }
            else
            {
                List<Point> localVertices = bwParams.Vertices;
                //polygon fill:
                Point rootTranslate = new Point();
                System.Drawing.Bitmap RootMap = ProcessBitmap.CreateBitmap(localVertices, true, true, 1, ref rootTranslate);
                List<System.Drawing.Bitmap> intermediateMap1 = new List<System.Drawing.Bitmap>();
                List<System.Drawing.Bitmap> intermediateMap2 = new List<System.Drawing.Bitmap>();
                List<System.Drawing.Bitmap> intermediateMap3 = new List<System.Drawing.Bitmap>();
                RootMap = RootMap.OpenPolygonBinaryFilter();
                intermediateMap1.Add(RootMap);

                while (intermediateMap1.Count > 0)
                {
                    for (int id = 0; id < intermediateMap1.Count; id++)
                    {
                        intermediateMap3 = PixelSinglePolygon(intermediateMap1[id], rootTranslate, bwParams);

                        for (int it = 0; it < intermediateMap3.Count; it++)
                        {
                            intermediateMap2.Add(intermediateMap3[it]);
                        }
                    }
                    intermediateMap1.Clear();

                    for (int it = 0; it < intermediateMap2.Count; it++)
                    {
                        intermediateMap1.Add(intermediateMap2[it]);
                    }
                    intermediateMap2.Clear();
                    intermediateMap3.Clear();
                }
            }
        }

        public static void PixelPolyTrace(BleachWaveParams bwParams, bool closeTrace)
        {
            List<Point> localVertices = bwParams.Vertices;

            foreach (Point pt in localVertices)
            {
                if (pt == localVertices[0])
                {
                    _pixelArray.AddPoint(pt);
                }
                else
                {
                    PixelTransition(pt);
                }
            }

            if (closeTrace)
            {
                PixelTransition(localVertices[0]);
            }
        }

        public static void PixelRectContour(BleachWaveParams bwParams)
        {
            List<Point> localVertices = bwParams.Vertices;

            double midX_Px = (localVertices[1].X + localVertices[0].X) / 2;
            double midY_Px = (localVertices[2].Y + localVertices[1].Y) / 2;
            int targetIteration = (int)Math.Min(Math.Ceiling((localVertices[2].Y - localVertices[1].Y) * PixelY_Volt / DeltaY_volt / 2), Math.Ceiling((localVertices[1].X - localVertices[0].X) * PixelX_Volt / DeltaX_volt / 2));
            bool done = false;
            List<Point> finalVertices = new List<Point>();
            BleachWaveParams tmpParams = new BleachWaveParams();

            _pixelArray.AddPoint(localVertices[0]);

            for (int it = 0; it <= targetIteration; it++)
            {
                //check if past boundary:
                if ((localVertices[0].X + (it * DeltaX_Px) > midX_Px) || (localVertices[0].Y + (it * DeltaX_Px) > midY_Px))
                {
                    return;
                }

                finalVertices.Clear();
                tmpParams.Vertices.Clear();
                finalVertices.Add(new Point(localVertices[0].X + (it * DeltaX_Px), localVertices[0].Y + (it * DeltaX_Px)));
                finalVertices.Add(new Point(localVertices[1].X - (it * DeltaX_Px), localVertices[1].Y + (it * DeltaX_Px)));
                finalVertices.Add(new Point(localVertices[2].X - (it * DeltaX_Px), localVertices[2].Y - (it * DeltaX_Px)));
                finalVertices.Add(new Point(localVertices[3].X + (it * DeltaX_Px), localVertices[3].Y - (it * DeltaX_Px)));
                //one step short to avoid double visit at Top-Left corner:
                if (finalVertices[0].Y + (DeltaX_Px) < finalVertices[3].Y)
                {
                    finalVertices.Add(new Point(finalVertices[0].X, finalVertices[0].Y + (DeltaX_Px)));
                }

                //sub-grid filling for the last contour:
                if (((localVertices[1].X - localVertices[0].X - (2 * it * DeltaX_Px)) < DeltaX_Px) || ((localVertices[2].Y - localVertices[1].Y - (2 * it * DeltaX_Px)) < DeltaX_Px))
                {
                    done = true;
                }

                //start build:
                tmpParams.Vertices = finalVertices;
                PixelPolyTrace(tmpParams, true);

                if (done)
                {
                    return;
                }

                //trace:
                if (0 == bwParams.Fill)
                {
                    return;
                }
            }
        }

        public static void PixelRectTopDown(BleachWaveParams bwParams)
        {
            int XStepsPerLine = (int)Math.Abs(Math.Floor((bwParams.ROIRight - bwParams.ROILeft) / DeltaX_Px));
            int YStepsPerIteration = (int)Math.Abs(Math.Floor((bwParams.ROIBottom - bwParams.ROITop) / DeltaX_Px));

            float xDirection = 1;
            float yDirection = 1;
            double startPx = 0, rightEndPx = bwParams.ROILeft + (XStepsPerLine * DeltaX_Px);
            Point pt;

            //Initial Top-Left:
            _pixelArray.AddPoint(bwParams.ROILeft, bwParams.ROITop);

            for (int iy = 0; iy <= YStepsPerIteration; iy++)                                                    //# YStepsPerIteration: 1 iteration
            {
                //horizontal transit:
                xDirection = (iy % 2 == 0) ? 1 : INV_DIRECTION;
                startPx = (iy % 2 == 0) ? bwParams.ROILeft : rightEndPx;
                pt = new Point(startPx + (xDirection * XStepsPerLine * DeltaX_Px), bwParams.ROITop + (iy * yDirection * DeltaX_Px));
                PixelTransition(pt);

                //vertical transit, skip at the last y step:
                if (iy < Math.Abs(YStepsPerIteration))
                {
                    pt = new Point(startPx + (xDirection * XStepsPerLine * DeltaX_Px), bwParams.ROITop + ((iy + 1) * yDirection * DeltaX_Px));
                    PixelTransition(pt);
                }
            }
        }

        public static void PixelSpot(BleachWaveParams bwParams)
        {
            _pixelArray.AddPoint(bwParams.ROILeft, bwParams.ROITop);
        }

        public static void ResetPixelArray()
        {
            _pixelArray.Clear();
        }

        public static void ResetWaveform()
        {
            if (null != _waveform)
            {
                //reset waveform:
                _waveform.Clear();
                _waveform.ClockRate = (UInt32)ClkRate;
                _waveform.PockelIdle = PowerIdle;
                //_waveform.AddValues(ImgRefX_volt, ImgRefY_volt, PowerIdle, 0);    //No need to visit reference.
            }
        }

        public static void ReturnHome(bool final)
        {
            Byte complete = (final) ? (Byte)1 : (Byte)0;
            Byte active = (final) ? (Byte)0 : (Byte)1;
            if (0 < _waveform.Count)
            {
                if ((0 >= _waveform.X_Volt.Count || _waveform.X_Volt.Last() == _waveform.X_Volt[0]) && (0 >= _waveform.Y_Volt.Count || _waveform.Y_Volt.Last() == _waveform.Y_Volt[0]))
                {
                    //already at home, create a falling sinal with one clock:
                    BuildModulation(1, PowerIdle, complete, 0, 0, 0, 0, 0, active);
                    return;
                }
                BuildTransition(_waveform.X_Volt[0], _waveform.Y_Volt[0], 0, PowerIdle, complete, 0, 0, 0, 0, 0, active);
            }
        }

        public static void SaveWaveform(string pathAndFilename, bool useBackgroundWorker, bool[] saveArray = null)
        {
            PathAndFilename = pathAndFilename;
            _saveSuccessed = false;
            _inSaving = true;
            FileType = WAVEFORM_FILETYPE.MEMORY_MAP;
            SaveType = (null != saveArray && (int)SignalType.SIGNALTYPE_LAST == saveArray.Length) ? saveArray : Enumerable.Repeat(true, (int)SignalType.SIGNALTYPE_LAST).ToArray();

            if (useBackgroundWorker)
            {
                _waveformSaver = new BackgroundWorker();
                _waveformSaver.DoWork += new DoWorkEventHandler(waveformSaver_DoWork);
                _waveformSaver.WorkerReportsProgress = false;
                _waveformSaver.WorkerSupportsCancellation = true;
                _waveformSaver.RunWorkerCompleted += new RunWorkerCompletedEventHandler(waveformSaver_RunWorkerCompleted);

                //alloc before start for memory map file:
                if (WAVEFORM_FILETYPE.MEMORY_MAP == FileType)
                {
                    SetupGWaveParams();
                }
                _waveformSaver.RunWorkerAsync();
            }
            else
            {
                SaveFile();
                _inSaving = false;
            }
        }

        public static void StopSave()
        {
            if ((_waveformSaver != null) && (_inSaving))
            {
                _waveformSaver.CancelAsync();
            }
        }

        /// <summary>
        /// Build waveform for modulation only.
        /// </summary>
        /// <param name="Counts"></param>
        /// <param name="Power"></param>
        /// <param name="cycleComplete"></param>
        private static void BuildModulation(int Counts, double[] Power, byte cycleComplete, byte cycleEnvelope, byte iterEnvelope, byte patnEnvelope, byte patnComplete, byte epochEnv, byte activeEnv)
        {
            if (Counts > 0)
            {
                for (int it = 0; it < Counts; it++)
                {
                    _waveform.AddValues(((0 < _waveform.X_Volt.Count) ? _waveform.X_Volt.Last() : (double?)null), ((0 < _waveform.Y_Volt.Count) ? _waveform.Y_Volt.Last() : (double?)null), Power, cycleComplete, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, epochEnv, activeEnv);
                }
            }
        }

        /// <summary>
        /// Try contour fill dissolved polygon until divergence.
        /// </summary>
        /// <param name="inputMap"></param>
        /// <param name="translate"></param>
        /// <param name="bwParams"></param>
        /// <param name="Power"></param>
        /// <returns></returns>
        private static List<System.Drawing.Bitmap> BuildSinglePolygon(System.Drawing.Bitmap inputMap, Point translate, BleachWaveParams bwParams, double[] Power)
        {
            byte[] buffer01, buffer02;
            Point childTranslate = new Point();
            List<System.Drawing.Bitmap> outMapList = new List<System.Drawing.Bitmap>();
            Dictionary<int, List<Point>> vGroups = new Dictionary<int, List<Point>>();
            List<Point> tracePoints = new List<Point>();
            System.Drawing.Bitmap tmap = inputMap;
            bool stop = false, initial = true;
            int dwellCnt = (int)Math.Floor(bwParams.DwellTime * (double)ClkRate / US_TO_S);
            BleachWaveParams localbwParam = bwParams.MakeCopy();

            while (!stop)
            {
                vGroups = tmap.FindConnectedObjects();
                switch (vGroups.Count)
                {
                    case 0:
                        stop = true;
                        break;
                    case 1:
                        buffer01 = ProcessBitmap.LoadBinaryBitmap(tmap);
                        buffer02 = ProcessBitmap.BinaryFilter(buffer01, tmap.Width, tmap.Height, ProcessBitmap.MatrixType.EdgeDetection, 1);
                        tmap = ProcessBitmap.OutputBinaryBitmap(buffer02, tmap.Width, tmap.Height);
                        //tmap.SaveMap("C:/Temp/Poly");
                        tracePoints = tmap.TraceConnectedBoundary(translate, (int)DeltaX_Px);
                        if (initial)
                        {
                            BuildTransition(ImgRefX_volt - HorizontalScanDirection * (tracePoints[0].X * PixelX_Volt), ImgRefY_volt + VerticalScanDirection * (tracePoints[0].Y * PixelY_Volt), 0, PowerIdle, 0, (byte)1, (byte)1, (byte)1, (byte)0, (byte)1, (byte)1);
                            initial = false;
                        }
                        localbwParam.Vertices = tracePoints;
                        localbwParam.VerticeCount = tracePoints.Count;
                        BuildPolyTrace(localbwParam, false, Power);
                        buffer02 = ProcessBitmap.BinaryFilter(buffer01, tmap.Width, tmap.Height, ProcessBitmap.MatrixType.Erosion, (int)DeltaX_Px);
                        tmap = ProcessBitmap.OutputBinaryBitmap(buffer02, tmap.Width, tmap.Height);
                        //tmap.SaveMap("C:/Temp/Poly");
                        break;
                    default:
                        foreach (KeyValuePair<int, List<Point>> child in vGroups)
                        {
                            tmap = ProcessBitmap.CreateBitmap(child.Value, false, false, (int)DeltaX_Px, ref childTranslate);
                            outMapList.Add(tmap);
                        }
                        stop = true;
                        break;
                }
            }
            return outMapList;
        }

        private static void BuildTransition(double targetVecVx, double targetVecVy, int dwellCnt, double[] Power, byte cycleComplete, byte cycleEnvelope, byte iterEnvelope, byte patnEnvelope, byte patnComplete, byte epochEnv, byte activeEnv)
        {
            //Set first if waveform is empty:
            if (0 == _waveform.Count)
            {
                _waveform.AddValues(targetVecVx, targetVecVy, Power, cycleComplete, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, epochEnv, activeEnv);
                return;
            }
            //Return if already arrived:
            if ((_waveform.X_Volt.Last() == targetVecVx) && (_waveform.Y_Volt.Last() == targetVecVy))
            {
                return;
            }
            //Travel along diagonal line with modulation ON/OFF:
            double DistanceVx = Math.Abs(targetVecVx - _waveform.X_Volt.Last());
            double DistanceVy = Math.Abs(targetVecVy - _waveform.Y_Volt.Last());
            double xDirection = ((targetVecVx - _waveform.X_Volt.Last()) > 0) ? 1 : INV_DIRECTION;      //DistanceVx = 0 will be xDirection = 0.
            double yDirection = ((targetVecVy - _waveform.Y_Volt.Last()) > 0) ? 1 : INV_DIRECTION;      //DistanceVy = 0 will be yDirection = 0.
            int Nd = Math.Max((int)Math.Floor(DistanceVx / DeltaX_volt), (int)Math.Floor(DistanceVy / DeltaY_volt));
            if (Nd > 0)
            {
                for (int id = 0; id < Nd; id++)
                {
                    _waveform.AddValues(_waveform.X_Volt.Last() + xDirection * (float)Math.Min(DeltaX_volt, DistanceVx / Nd), _waveform.Y_Volt.Last() + yDirection * Math.Min(DeltaY_volt, DistanceVy / Nd), Power, cycleComplete, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, epochEnv, activeEnv);
                    //dwell time:
                    BuildModulation(dwellCnt, Power, cycleComplete, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, epochEnv, activeEnv);
                }
            }
            //Travel residue distance:
            double dV_sub_X = targetVecVx - _waveform.X_Volt.Last();
            double dV_sub_Y = targetVecVy - _waveform.Y_Volt.Last();
            if (dV_sub_X != 0 || dV_sub_Y != 0)
            {
                _waveform.AddValues(_waveform.X_Volt.Last() + dV_sub_X, _waveform.Y_Volt.Last() + dV_sub_Y, Power, cycleComplete, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, epochEnv, activeEnv);
                //dwell time:
                BuildModulation(dwellCnt, Power, cycleComplete, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, epochEnv, activeEnv);
            }
        }

        private static void BuildTransitionPixelMode(double TargetVecVx, double TargetVecVy, int preIdleCnt, int dwellCnt, int postIdleCnt, double[] Power)
        {
            //Return if already arrived:
            if ((_waveform.X_Volt.Last() == TargetVecVx) && (_waveform.Y_Volt.Last() == TargetVecVy))
            {
                return;
            }
            //Travel along diagonal line with modulation ON/OFF:
            byte cycleEnvelope = (byte)1, iterEnvelope = (byte)1, patnEnvelope = (byte)1, patnComplete = (byte)0;   //All envelopes will always be HIGH in pixel modulation mode
            double DistanceVx = Math.Abs(TargetVecVx - _waveform.X_Volt.Last());
            double DistanceVy = Math.Abs(TargetVecVy - _waveform.Y_Volt.Last());
            double xDirection = ((TargetVecVx - _waveform.X_Volt.Last()) > 0) ? 1 : INV_DIRECTION;      //DistanceVx = 0 will be xDirection = 0.
            double yDirection = ((TargetVecVy - _waveform.Y_Volt.Last()) > 0) ? 1 : INV_DIRECTION;      //DistanceVy = 0 will be yDirection = 0.
            int Nd = Math.Max((int)Math.Floor(DistanceVx / DeltaX_volt), (int)Math.Floor(DistanceVy / DeltaY_volt));
            if (Nd > 0)
            {
                for (int id = 0; id < Nd; id++)
                {
                    _waveform.AddValues(_waveform.X_Volt.Last() + xDirection * (float)Math.Min(DeltaX_volt, DistanceVx / Nd), _waveform.Y_Volt.Last() + yDirection * Math.Min(DeltaY_volt, DistanceVy / Nd), PowerIdle, (byte)0, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, (byte)1, (byte)1);
                    //preIdle time:
                    BuildModulation(preIdleCnt, PowerIdle, (byte)0, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, (byte)1, (byte)1);
                    //dwell time:
                    BuildModulation(dwellCnt, Power, (byte)0, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, (byte)1, (byte)1);
                    //postIdle time:
                    BuildModulation(postIdleCnt, PowerIdle, (byte)0, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, (byte)1, (byte)1);
                }
            }
            //Travel residue distance:
            double dV_sub_X = TargetVecVx - _waveform.X_Volt.Last();
            double dV_sub_Y = TargetVecVy - _waveform.Y_Volt.Last();
            if (dV_sub_X != 0 || dV_sub_Y != 0)
            {
                _waveform.AddValues(_waveform.X_Volt.Last() + dV_sub_X, _waveform.Y_Volt.Last() + dV_sub_Y, PowerIdle, (byte)0, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, (byte)1, (byte)1);
                //preIdle time:
                BuildModulation(preIdleCnt, PowerIdle, (byte)0, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, (byte)1, (byte)1);
                //dwell time:
                BuildModulation(dwellCnt, Power, (byte)0, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, (byte)1, (byte)1);
                //postIdle time:
                BuildModulation(postIdleCnt, PowerIdle, (byte)0, cycleEnvelope, iterEnvelope, patnEnvelope, patnComplete, (byte)1, (byte)1);
            }
        }

        /// <summary>
        /// Use this function to copy The data to the gWaveParams structure
        /// </summary>
        static void CopyDataToGWaveformParams()
        {
            //copy the corresponding array depending on the waveform driver type
            switch (_waveform.WaveformDriverType)
            {
                case WaveformDriverType.WaveformDriver_NI:
                    {
                        if (IntPtr.Zero != _gWaveParams.GalvoWaveformXY && 0 < _waveform.X_Volt.Count)
                        {
                            //copy the XY analog lines into a single IntPtr buffer _gWaveParams.GalvoWaveformXY
                            //leave interleave of XY to lower level:
                            //offset the IntPtr using the IntPtr.Add() function
                            Marshal.Copy(_waveform.X_Volt.ToArray(), 0, _gWaveParams.GalvoWaveformXY, _waveform.Count);
                            Marshal.Copy(_waveform.Y_Volt.ToArray(), 0, IntPtr.Add(_gWaveParams.GalvoWaveformXY, _waveform.Count * sizeof(double)), _waveform.Count);
                        }
                        //copy the pockels analog line into a single IntPtr buffer _gWaveParams.GalvoWaveformPockel
                        for (int i = 0; i < _waveform.Pockel.Count; i++)
                        {
                            if (0 < _waveform.Pockel[i].Count)
                                Marshal.Copy(_waveform.Pockel[i].ToArray(), 0, IntPtr.Add(_gWaveParams.GalvoWaveformPockel, i * (int)_gWaveParams.analogPockelSize / _gWaveParams.pockelsCount * sizeof(double)), (int)_gWaveParams.analogPockelSize / _gWaveParams.pockelsCount);
                        }
                        //copy the digital lines into a single IntPtr buffer _gWaveParams.DigBufWaveform
                        //offset the IntPtr using the IntPtr.Add() function
                        if (IntPtr.Zero != _gWaveParams.DigBufWaveform)
                        {
                            Marshal.Copy(_waveform.PockelDig.ToArray(), 0, _gWaveParams.DigBufWaveform, _waveform.Count);
                            Marshal.Copy(_waveform.ActiveEnvelope.ToArray(), 0, IntPtr.Add(_gWaveParams.DigBufWaveform, 1 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.CycleComplete.ToArray(), 0, IntPtr.Add(_gWaveParams.DigBufWaveform, 2 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.CycleEnvelope.ToArray(), 0, IntPtr.Add(_gWaveParams.DigBufWaveform, 3 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.IterationEnvelope.ToArray(), 0, IntPtr.Add(_gWaveParams.DigBufWaveform, 4 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.PatternEnvelope.ToArray(), 0, IntPtr.Add(_gWaveParams.DigBufWaveform, 5 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.PatternComplete.ToArray(), 0, IntPtr.Add(_gWaveParams.DigBufWaveform, 6 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.EpochEnvelope.ToArray(), 0, IntPtr.Add(_gWaveParams.DigBufWaveform, 7 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.CycleComplementary.ToArray(), 0, IntPtr.Add(_gWaveParams.DigBufWaveform, 8 * _waveform.Count), _waveform.Count);
                        }
                    }
                    break;
                case WaveformDriverType.WaveformDriver_ThorDAQ:
                    {
                        if (IntPtr.Zero != _gWaveParams.GalvoWaveformXY && 0 < _waveform.X_Volt.Count)
                        {
                            //copy the XY analog lines into a single IntPtr buffer _thorDAQGGWaveformParams.GalvoWaveformXY
                            //offset the IntPtr using the IntPtr.Add() function
                            CopyUShortToIntPtr(_waveform.X_Volt_16bit.ToArray(), 0, _gWaveParams.GalvoWaveformXY, _waveform.Count);
                            CopyUShortToIntPtr(_waveform.Y_Volt_16bit.ToArray(), 0, IntPtr.Add(_gWaveParams.GalvoWaveformXY, _waveform.Count * sizeof(ushort)), _waveform.Count);
                        }
                        //copy the pockels analog line into a single IntPtr buffer _gWaveParams.GalvoWaveformPockel
                        for (int i = 0; i < _waveform.Pockel.Count; i++)
                        {
                            if (0 < _waveform.Pockel[i].Count)
                                Marshal.Copy(_waveform.Pockel[i].ToArray(), 0, IntPtr.Add(_gWaveParams.GalvoWaveformPockel, i * (int)_gWaveParams.analogPockelSize * sizeof(double)), (int)_gWaveParams.analogPockelSize);
                        }

                        //copy the digital lines into a single IntPtr buffer _thorDAQGGWaveformParams.DigBufWaveform
                        //offset the IntPtr using the IntPtr.Add() function
                        if (IntPtr.Zero != _thorDAQGGWaveformParams.DigBufWaveform)
                        {                            
                            Marshal.Copy(_waveform.PockelDig.ToArray(), 0, _thorDAQGGWaveformParams.DigBufWaveform, _waveform.Count);
                            Marshal.Copy(_waveform.ActiveEnvelope.ToArray(), 0, IntPtr.Add(_thorDAQGGWaveformParams.DigBufWaveform, _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.CycleComplete.ToArray(), 0, IntPtr.Add(_thorDAQGGWaveformParams.DigBufWaveform, 2 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.CycleEnvelope.ToArray(), 0, IntPtr.Add(_thorDAQGGWaveformParams.DigBufWaveform, 3 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.IterationEnvelope.ToArray(), 0, IntPtr.Add(_thorDAQGGWaveformParams.DigBufWaveform, 4 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.PatternEnvelope.ToArray(), 0, IntPtr.Add(_thorDAQGGWaveformParams.DigBufWaveform, 5 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.PatternComplete.ToArray(), 0, IntPtr.Add(_thorDAQGGWaveformParams.DigBufWaveform, 6 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.EpochEnvelope.ToArray(), 0, IntPtr.Add(_thorDAQGGWaveformParams.DigBufWaveform, 7 * _waveform.Count), _waveform.Count);
                            Marshal.Copy(_waveform.CycleComplementary.ToArray(), 0, IntPtr.Add(_thorDAQGGWaveformParams.DigBufWaveform, 8 * _waveform.Count), _waveform.Count);
                        }
                    }
                    break;
            }           
        }

        [DllImport("kernel32.dll", EntryPoint = "RtlMoveMemory", SetLastError = false)]
        static extern void CopyMemory(IntPtr Destination, IntPtr Source, uint Length);

        private static List<System.Drawing.Bitmap> PixelSinglePolygon(System.Drawing.Bitmap inputMap, Point translate, BleachWaveParams bwParams)
        {
            byte[] buffer01, buffer02;
            Point childTranslate = new Point();
            List<System.Drawing.Bitmap> outMapList = new List<System.Drawing.Bitmap>();
            Dictionary<int, List<Point>> vGroups = new Dictionary<int, List<Point>>();
            List<Point> tracePoints = new List<Point>();
            System.Drawing.Bitmap tmap = inputMap;
            bool stop = false;
            BleachWaveParams localbwParam = bwParams.MakeCopy();

            while (!stop)
            {
                vGroups = tmap.FindConnectedObjects();
                switch (vGroups.Count)
                {
                    case 0:
                        stop = true;
                        break;
                    case 1:
                        buffer01 = ProcessBitmap.LoadBinaryBitmap(tmap);
                        buffer02 = ProcessBitmap.BinaryFilter(buffer01, tmap.Width, tmap.Height, ProcessBitmap.MatrixType.EdgeDetection, 1);
                        tmap = ProcessBitmap.OutputBinaryBitmap(buffer02, tmap.Width, tmap.Height);
                        tracePoints = tmap.TraceConnectedBoundary(translate, (int)DeltaX_Px);
                        localbwParam.Vertices = tracePoints;
                        localbwParam.VerticeCount = tracePoints.Count;
                        PixelPolyTrace(localbwParam, false);

                        buffer02 = ProcessBitmap.BinaryFilter(buffer01, tmap.Width, tmap.Height, ProcessBitmap.MatrixType.Erosion, (int)DeltaX_Px);
                        tmap = ProcessBitmap.OutputBinaryBitmap(buffer02, tmap.Width, tmap.Height);
                        break;
                    default:
                        foreach (KeyValuePair<int, List<Point>> child in vGroups)
                        {
                            tmap = ProcessBitmap.CreateBitmap(child.Value, false, false, (int)DeltaX_Px, ref childTranslate);
                            outMapList.Add(tmap);
                        }
                        stop = true;
                        break;
                }
            }
            return outMapList;
        }

        private static void PixelTransition(Point targetPixel)
        {
            //Return if already exist:
            if ((_pixelArray.LastX == targetPixel.X) && (_pixelArray.LastY == targetPixel.Y))
            {
                return;
            }
            //Travel along diagonal line with modulation ON/OFF:
            double DistanceVx = Math.Abs(targetPixel.X - _pixelArray.LastX);
            double DistanceVy = Math.Abs(targetPixel.Y - _pixelArray.LastY);
            double xDirection = ((targetPixel.X - _pixelArray.LastX) > 0) ? 1 : INV_DIRECTION;      //DistanceVx = 0 will be xDirection = 0.
            double yDirection = ((targetPixel.Y - _pixelArray.LastY) > 0) ? 1 : INV_DIRECTION;      //DistanceVy = 0 will be yDirection = 0.
            int Nd = Math.Max((int)Math.Floor(DistanceVx / DeltaX_Px), (int)Math.Floor(DistanceVy / DeltaX_Px));
            if (Nd > 0)
            {
                for (int id = 0; id < Nd; id++)
                {
                    _pixelArray.AddPoint(_pixelArray.LastX + xDirection * (float)Math.Min(DeltaX_Px, DistanceVx / Nd), _pixelArray.LastY + yDirection * Math.Min(DeltaX_Px, DistanceVy / Nd));
                }
            }
            //Travel residue distance:
            double dV_sub_X = targetPixel.X - _pixelArray.LastX;
            double dV_sub_Y = targetPixel.Y - _pixelArray.LastY;
            //if (dV_sub_X > DeltaX_Px || dV_sub_Y > DeltaX_Px)
            if (dV_sub_X != 0 || dV_sub_Y != 0)		//allow to do residue step
            {
                _pixelArray.AddPoint(_pixelArray.LastX + dV_sub_X, _pixelArray.LastY + dV_sub_Y);
            }
        }

        private static void SaveFile()
        {
            switch (FileType)
            {
                //TODO: in the future we will cleanup/remove the h5 related code. We haven't used it and will probably never use it
                //of course, before removing any code we need to triple check if its used at all.
                case WAVEFORM_FILETYPE.H5:
                    SaveToH5();
                    _saveSuccessed = true;
                    break;
                case WAVEFORM_FILETYPE.MEMORY_MAP:
                    try
                    {
                        //set wavefor params and allocate memory
                        SetupGWaveParams();

                        CopyDataToGWaveformParams();

                        //Save the waveform using a different function depending on the waveform driver Type
                        switch (_waveform.WaveformDriverType)
                        {
                            case WaveformDriverType.WaveformDriver_NI:
                                _saveSuccessed = (1 == SaveWaveformDataStruct(PathAndFilename, _gWaveParams));
                                break;
                            case WaveformDriverType.WaveformDriver_ThorDAQ:
                                _saveSuccessed = (1 == SaveThorDAQWaveformDataStruct(PathAndFilename, _thorDAQGGWaveformParams));
                                break;
                        }
                    }
                    finally
                    {
                        //free memory allocated for the GWaveformParams
                        FreeGWaveParamsMemory();
                    }
                    break;
            }
        }

        [DllImport(".\\Modules_Native\\GeometryUtilitiesCPP.dll", EntryPoint = "SaveThorDAQWaveformDataStruct", CharSet = CharSet.Unicode)]
        private static extern int SaveThorDAQWaveformDataStruct(string waveformPathName, GGalvoWaveformParams waveformParams);

        private static void SaveToH5()
        {
            H5CSWrapper h5io = new H5CSWrapper();
            h5io.SetSavePathAndFileName(PathAndFilename);
            if (!h5io.CreateH5())
            {
                return;
            }
            string[] grpName = { "", "/Analog", "/Digital" };
            string[] rtNames = { "/ClockRate" };
            List<string> aDsetNames = new List<string>(); //{ "/X", "/Y", "/Pockel" };
            aDsetNames.Add("/X");
            aDsetNames.Add("/Y");
            for (int i = 0; i < _waveform.Pockel.Count; i++)
            {
                aDsetNames.Add("/Pockel" + i);
            }
            string[] dDsetNames = { "/PockelDig", "/ActiveEnvelope", "/CycleComplete", "/CycleEnvelope", "/IterationEnvelope",
                                              "/PatternEnvelope", "/PatternComplete", "/EpochEnvelope", "/CycleInverse" };
            UInt32[] clk = { _waveform.ClockRate };

            //Copy the different array depending on the waveform driver Type
            switch (_waveform.WaveformDriverType)
            {
                case WaveformDriverType.WaveformDriver_NI:
                    {
                        h5io.CreateGroupDatasetNames<UInt32>(grpName[0], rtNames, rtNames.Length);
                        h5io.CreateGroupDatasetNames<Double>(grpName[1], aDsetNames.ToArray(), aDsetNames.Count);
                        h5io.CreateGroupDatasetNames<Byte>(grpName[2], dDsetNames, dDsetNames.Length);
                        h5io.WriteDataset<UInt32>(grpName[0], rtNames[0], clk, 0, (UInt32)1);
                        h5io.ExtendDataset<Double>(grpName[1], aDsetNames[0], _waveform.X_Volt.ToArray(), true, (UInt32)_waveform.X_Volt.Count);
                        h5io.ExtendDataset<Double>(grpName[1], aDsetNames[1], _waveform.Y_Volt.ToArray(), true, (UInt32)_waveform.Y_Volt.Count);
                        for (int i = 0; i < _waveform.Pockel.Count; i++)
                        {
                            h5io.ExtendDataset<Double>(grpName[1], aDsetNames[i + 2], _waveform.Pockel[i].ToArray(), true, (UInt32)_waveform.Pockel.Count);
                        }
                    }
                    break;
                case WaveformDriverType.WaveformDriver_ThorDAQ:
                    {
                        h5io.CreateGroupDatasetNames<UInt32>(grpName[0], rtNames, rtNames.Length);
                        h5io.CreateGroupDatasetNames<UInt16>(grpName[1], aDsetNames.ToArray(), aDsetNames.Count);
                        h5io.CreateGroupDatasetNames<Byte>(grpName[2], dDsetNames, dDsetNames.Length);
                        h5io.WriteDataset<UInt32>(grpName[0], rtNames[0], clk, 0, (UInt32)1);
                        h5io.ExtendDataset<UInt16>(grpName[1], aDsetNames[0], _waveform.X_Volt_16bit.ToArray(), true, (UInt32)_waveform.X_Volt_16bit.Count);
                        h5io.ExtendDataset<UInt16>(grpName[1], aDsetNames[1], _waveform.Y_Volt_16bit.ToArray(), true, (UInt32)_waveform.Y_Volt_16bit.Count);
                        h5io.ExtendDataset<UInt16>(grpName[1], aDsetNames[2], _waveform.Pockel_16bit.ToArray(), true, (UInt32)_waveform.Pockel_16bit.Count);
                    }
                    break;
            }

            h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[0], _waveform.PockelDig.ToArray(), true, (UInt32)_waveform.PockelDig.Count);
            h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[1], _waveform.ActiveEnvelope.ToArray(), true, (UInt32)_waveform.ActiveEnvelope.Count);
            h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[2], _waveform.CycleComplete.ToArray(), true, (UInt32)_waveform.CycleComplete.Count);
            h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[3], _waveform.CycleEnvelope.ToArray(), true, (UInt32)_waveform.CycleEnvelope.Count);
            h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[4], _waveform.IterationEnvelope.ToArray(), true, (UInt32)_waveform.IterationEnvelope.Count);
            h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[5], _waveform.PatternEnvelope.ToArray(), true, (UInt32)_waveform.PatternEnvelope.Count);
            h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[6], _waveform.PatternComplete.ToArray(), true, (UInt32)_waveform.PatternComplete.Count);
            h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[7], _waveform.EpochEnvelope.ToArray(), true, (UInt32)_waveform.EpochEnvelope.Count);
            h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[8], _waveform.CycleComplementary.ToArray(), true, (UInt32)_waveform.CycleComplementary.Count);
            h5io.CloseH5();
        }

        /// <summary>
        /// Use this function to save the waveform to the H5 file when using a background worker
        /// </summary>
        /// <param name="e"></param>
        /// <returns></returns>
        private static bool SaveToH5_background(DoWorkEventArgs e)
        {
            H5CSWrapper h5io = new H5CSWrapper();
            h5io.SetSavePathAndFileName(PathAndFilename);
            if (!h5io.CreateH5())
            {
                return false;
            }
            string[] grpName = { "", "/Analog", "/Digital" };
            string[] rtNames = { "/ClockRate" };
            List<string> aDsetNames = new List<string>(); //{ "/X", "/Y", "/Pockel" };
            aDsetNames.Add("/X");
            aDsetNames.Add("/Y");
            for (int i = 0; i < _waveform.Pockel.Count; i++)
            {
                aDsetNames.Add("/Pockel" + i);
            }
            string[] dDsetNames = { "/PockelDig", "/ActiveEnvelope", "/CycleComplete", "/CycleEnvelope", "/IterationEnvelope", "/PatternEnvelope", "/PatternComplete", "/EpochEnvelope", "/CycleInverse" };
            UInt32[] clk = { _waveform.ClockRate };

            //Save the waveform using a different data types depending on the waveform driver Type
            switch (_waveform.WaveformDriverType)
            {
                //TODO: in the future we will cleanup/remove the h5 related code. We haven't used it and will probably never use it
                case WaveformDriverType.WaveformDriver_NI:
                    {
                        h5io.CreateGroupDatasetNames<UInt32>(grpName[0], rtNames, rtNames.Length);
                        h5io.CreateGroupDatasetNames<Double>(grpName[1], aDsetNames.ToArray(), aDsetNames.Count);
                        h5io.CreateGroupDatasetNames<Byte>(grpName[2], dDsetNames, dDsetNames.Length);

                        if (_waveformSaver.CancellationPending != true)
                        {
                            h5io.WriteDataset<UInt32>(grpName[0], rtNames[0], clk, 0, (UInt32)1);
                        }
                        else
                        {
                            h5io.CloseH5();
                            e.Cancel = true;
                            return false;
                        }
                        if (_waveformSaver.CancellationPending != true)
                        {
                            h5io.ExtendDataset<Double>(grpName[1], aDsetNames[0], _waveform.X_Volt.ToArray(), true, (UInt32)_waveform.X_Volt.Count);
                        }
                        else
                        {
                            h5io.CloseH5();
                            e.Cancel = true;
                            return false;
                        }
                        if (_waveformSaver.CancellationPending != true)
                        {
                            h5io.ExtendDataset<Double>(grpName[1], aDsetNames[1], _waveform.Y_Volt.ToArray(), true, (UInt32)_waveform.Y_Volt.Count);
                        }
                        else
                        {
                            h5io.CloseH5();
                            e.Cancel = true;
                            return false;
                        }
                        for (int i = 0; i < _waveform.Pockel.Count; i++)
                        {
                            if (_waveformSaver.CancellationPending != true)
                            {
                                h5io.ExtendDataset<Double>(grpName[1], aDsetNames[i + 2], _waveform.Pockel[i].ToArray(), true, (UInt32)_waveform.Pockel.Count);
                            }
                            else
                            {
                                h5io.CloseH5();
                                e.Cancel = true;
                                return false;
                            }
                        }
                    }
                    break;
                case WaveformDriverType.WaveformDriver_ThorDAQ:
                    {
                        h5io.CreateGroupDatasetNames<UInt32>(grpName[0], rtNames, rtNames.Length);
                        h5io.CreateGroupDatasetNames<UInt16>(grpName[1], aDsetNames.ToArray(), aDsetNames.Count);
                        h5io.CreateGroupDatasetNames<Byte>(grpName[2], dDsetNames, dDsetNames.Length);

                        if (_waveformSaver.CancellationPending != true)
                        {
                            h5io.WriteDataset<UInt32>(grpName[0], rtNames[0], clk, 0, (UInt32)1);
                        }
                        else
                        {
                            h5io.CloseH5();
                            e.Cancel = true;
                            return false;
                        }
                        if (_waveformSaver.CancellationPending != true)
                        {
                            h5io.ExtendDataset<UInt16>(grpName[1], aDsetNames[0], _waveform.X_Volt_16bit.ToArray(), true, (UInt32)_waveform.X_Volt_16bit.Count);
                        }
                        else
                        {
                            h5io.CloseH5();
                            e.Cancel = true;
                            return false;
                        }
                        if (_waveformSaver.CancellationPending != true)
                        {
                            h5io.ExtendDataset<UInt16>(grpName[1], aDsetNames[1], _waveform.Y_Volt_16bit.ToArray(), true, (UInt32)_waveform.Y_Volt_16bit.Count);
                        }
                        else
                        {
                            h5io.CloseH5();
                            e.Cancel = true;
                            return false;
                        }
                        if (_waveformSaver.CancellationPending != true)
                        {
                            h5io.ExtendDataset<UInt16>(grpName[1], aDsetNames[2], _waveform.Pockel_16bit.ToArray(), true, (UInt32)_waveform.Pockel_16bit.Count);
                        }
                        else
                        {
                            h5io.CloseH5();
                            e.Cancel = true;
                            return false;
                        }
                    }
                    break;
            }
            if (_waveformSaver.CancellationPending != true)
            {
                h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[0], _waveform.PockelDig.ToArray(), true, (UInt32)_waveform.PockelDig.Count);
            }
            else
            {
                h5io.CloseH5();
                e.Cancel = true;
                return false;
            }
            if (_waveformSaver.CancellationPending != true)
            {
                h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[1], _waveform.ActiveEnvelope.ToArray(), true, (UInt32)_waveform.ActiveEnvelope.Count);
            }
            else
            {
                h5io.CloseH5();
                e.Cancel = true;
                return false;
            }
            if (_waveformSaver.CancellationPending != true)
            {
                h5io.WriteDataset<Byte>(grpName[2], dDsetNames[2], _waveform.CycleComplete.ToArray(), 0, (UInt32)_waveform.CycleComplete.Count);
            }
            else
            {
                h5io.CloseH5();
                e.Cancel = true;
                return false;
            }
            if (_waveformSaver.CancellationPending != true)
            {
                h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[3], _waveform.CycleEnvelope.ToArray(), true, (UInt32)_waveform.CycleEnvelope.Count);
            }
            else
            {
                h5io.CloseH5();
                e.Cancel = true;
                return false;
            }
            if (_waveformSaver.CancellationPending != true)
            {
                h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[4], _waveform.IterationEnvelope.ToArray(), true, (UInt32)_waveform.IterationEnvelope.Count);
            }
            else
            {
                h5io.CloseH5();
                e.Cancel = true;
                return false;
            }
            if (_waveformSaver.CancellationPending != true)
            {
                h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[5], _waveform.PatternEnvelope.ToArray(), true, (UInt32)_waveform.PatternEnvelope.Count);
            }
            else
            {
                h5io.CloseH5();
                e.Cancel = true;
                return false;
            }
            if (_waveformSaver.CancellationPending != true)
            {
                h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[6], _waveform.PatternComplete.ToArray(), true, (UInt32)_waveform.PatternComplete.Count);
            }
            else
            {
                h5io.CloseH5();
                e.Cancel = true;
                return false;
            }
            if (_waveformSaver.CancellationPending != true)
            {
                h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[7], _waveform.EpochEnvelope.ToArray(), true, (UInt32)_waveform.EpochEnvelope.Count);
            }
            else
            {
                h5io.CloseH5();
                e.Cancel = true;
                return false;
            }
            if (_waveformSaver.CancellationPending != true)
            {
                h5io.ExtendDataset<Byte>(grpName[2], dDsetNames[8], _waveform.CycleComplementary.ToArray(), true, (UInt32)_waveform.CycleComplementary.Count);
            }
            else
            {
                h5io.CloseH5();
                e.Cancel = true;
                return false;
            }
            h5io.CloseH5();

            return true;
        }

        [DllImport(".\\Modules_Native\\GeometryUtilitiesCPP.dll", EntryPoint = "SaveThorDAQWaveformDataStruct", CharSet = CharSet.Unicode)]
        private static extern int SaveThorDAQWaveformDataStruct(string waveformPathName, ThorDAQGGWaveformParams waveformParams);

        [DllImport(".\\Modules_Native\\GeometryUtilitiesCPP.dll", EntryPoint = "SaveWaveformDataStruct", CharSet = CharSet.Unicode)]
        private static extern int SaveWaveformDataStruct(string waveformPathName, GGalvoWaveformParams waveformParams);

        private static void SetupGWaveParams()
        {
            //Alloc the unmanaged memory in the corresponding size depending on the waveform driver Type
            switch (_waveform.WaveformDriverType)
            {
                case WaveformDriverType.WaveformDriver_NI:
                    {
                        _gWaveParams.clockRate = (UInt64)_waveform.ClockRate;
                        _gWaveParams.analogXYSize = SaveType[(int)SignalType.ANALOG_XY] ? (UInt64)(_waveform.Count) * 2 : 0;
                        _gWaveParams.pockelsCount = SaveType[(int)SignalType.ANALOG_POCKEL] ? (byte)(_waveform.Pockel.Count) : (byte)0;
                        _gWaveParams.analogPockelSize = SaveType[(int)SignalType.ANALOG_POCKEL] ? (UInt64)(_waveform.Count) * _gWaveParams.pockelsCount : 0;
                        _gWaveParams.digitalSize = SaveType[(int)SignalType.DIGITAL_LINES] ? (UInt64)(_waveform.Count) * ((int)(ThorSharedTypes.BLEACHSCAN_DIGITAL_LINENAME.DIGITAL_LINENAME_LAST) - 1) : 0; //no need to save dummy
                        _gWaveParams.stepVolt = DeltaX_volt;
                        _gWaveParams.digitalLineCnt = SaveType[(int)SignalType.DIGITAL_LINES] ? ((int)(ThorSharedTypes.BLEACHSCAN_DIGITAL_LINENAME.DIGITAL_LINENAME_LAST) - 1) : 0;
                        //alloc here and release when completed:

                        //Alloc the unmanaged memory in the corresponding size depending on the waveform driver Type
                        _gWaveParams.GalvoWaveformXY = SaveType[(int)SignalType.ANALOG_XY] ? Marshal.AllocHGlobal((int)_gWaveParams.analogXYSize * sizeof(double)) : IntPtr.Zero;
                        _gWaveParams.GalvoWaveformPockel = SaveType[(int)SignalType.ANALOG_POCKEL] ? Marshal.AllocHGlobal((int)_gWaveParams.analogPockelSize *  sizeof(double)) : IntPtr.Zero;
                        _gWaveParams.DigBufWaveform = SaveType[(int)SignalType.DIGITAL_LINES] ? Marshal.AllocHGlobal((int)_gWaveParams.digitalSize * sizeof(byte)) : IntPtr.Zero;
                    }
                    break;
                case WaveformDriverType.WaveformDriver_ThorDAQ:
                    {
                        _thorDAQGGWaveformParams.clockRate = (UInt64)_waveform.ClockRate;
                        _thorDAQGGWaveformParams.analogXYSize = SaveType[(int)SignalType.ANALOG_XY] ? (UInt64)(_waveform.Count) * 2 : 0;
                        _thorDAQGGWaveformParams.pockelsCount = SaveType[(int)SignalType.ANALOG_POCKEL] ? (byte)(_waveform.Pockel.Count) : (byte)0;
                        _thorDAQGGWaveformParams.analogPockelSize = SaveType[(int)SignalType.ANALOG_POCKEL] ? (UInt64)(_waveform.Count) * _gWaveParams.pockelsCount : 0;
                        _thorDAQGGWaveformParams.digitalSize = SaveType[(int)SignalType.DIGITAL_LINES] ? (UInt64)(_waveform.Count) * ((int)(ThorSharedTypes.BLEACHSCAN_DIGITAL_LINENAME.DIGITAL_LINENAME_LAST) - 1) : 0; //no need to save dummy
                        _thorDAQGGWaveformParams.stepVolt = DeltaX_volt;
                        _thorDAQGGWaveformParams.digitalLineCnt = SaveType[(int)SignalType.DIGITAL_LINES] ? ((int)(ThorSharedTypes.BLEACHSCAN_DIGITAL_LINENAME.DIGITAL_LINENAME_LAST) - 1) : 0;
                        //alloc here and release when completed:

                        //Alloc the unmanaged memory in the corresponding size depending on the waveform driver Type
                        _thorDAQGGWaveformParams.GalvoWaveformXY = SaveType[(int)SignalType.ANALOG_XY] ? Marshal.AllocHGlobal((int)_thorDAQGGWaveformParams.analogXYSize * sizeof(ushort)) : IntPtr.Zero;
                        _thorDAQGGWaveformParams.GalvoWaveformPockel = SaveType[(int)SignalType.ANALOG_POCKEL] ? Marshal.AllocHGlobal((int)_thorDAQGGWaveformParams.analogPockelSize * sizeof(ushort)) : IntPtr.Zero;
                        _thorDAQGGWaveformParams.DigBufWaveform = SaveType[(int)SignalType.DIGITAL_LINES] ? Marshal.AllocHGlobal((int)_thorDAQGGWaveformParams.digitalSize * sizeof(byte)) : IntPtr.Zero;
                    }
                    break;
            }
        }

            private static void waveformSaver_DoWork(object sender, DoWorkEventArgs e)
        {
            switch (FileType)
            {
                case WAVEFORM_FILETYPE.H5:
                    _saveSuccessed = SaveToH5_background(e);
                    break;
                case WAVEFORM_FILETYPE.MEMORY_MAP:
                default:
                    //prepare memory:
                    CopyDataToGWaveformParams();

                    //Save the waveform using a different function depending on the waveform driver Type
                    if (_waveformSaver.CancellationPending != true)
                    {
                        switch (_waveform.WaveformDriverType)
                        {
                            case WaveformDriverType.WaveformDriver_NI:
                                _saveSuccessed = (1 == SaveWaveformDataStruct(PathAndFilename, _gWaveParams));
                                break;
                            case WaveformDriverType.WaveformDriver_ThorDAQ:
                                _saveSuccessed = (1 == SaveThorDAQWaveformDataStruct(PathAndFilename, _thorDAQGGWaveformParams));
                                break;
                        }
                    }
                    break;
            }
        }

        private static void waveformSaver_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (WAVEFORM_FILETYPE.MEMORY_MAP == FileType)
            {
                FreeGWaveParamsMemory();
            }
            _waveformSaver.DoWork -= new DoWorkEventHandler(waveformSaver_DoWork);
            _waveformSaver.RunWorkerCompleted -= new RunWorkerCompletedEventHandler(waveformSaver_RunWorkerCompleted);
            _inSaving = false;
        }

        #endregion Methods
    }
}