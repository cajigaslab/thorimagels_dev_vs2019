﻿namespace OverlayManager
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Media;
    using System.Windows.Shapes;
    using System.Xml;

    using Microsoft.Win32;

    public class ROIPoly : ScalableShape
    {
        #region Fields

        private readonly Path _path;

        private double _arcRoundness;
        private Rect _bounds;
        private Rect _statsBounds;
        private bool _closed;
        private PointCollection _points;
        private bool _useRoundnessPercentage;

        private PointCollection _statsPoints = new PointCollection();
        private double _imageScaleX;
        private double _imageScaleY;
        private double _toAdjustX;
        private double _toAdjustY;
        #endregion Fields

        #region Constructors

        // ROIPoly constructor
        public ROIPoly()
        {
            var geometry = new PathGeometry();
            geometry.Figures.Add(new PathFigure());
            _path = new Path { Data = geometry };
            _bounds = new Rect();
            _statsBounds = new Rect();
            Points = new PointCollection();
            _statsPoints = new PointCollection();
            Points.Changed += Points_Changed;

            _imageScaleX = 1.0;
            _imageScaleY = 1.0;
            _toAdjustX = 1.0;
            _toAdjustY = 1.0;
        }

        public ROIPoly(PointCollection pts, double xScale, double yScale)
        {
            var geometry = new PathGeometry();
            geometry.Figures.Add(new PathFigure());
            _statsPoints = new PointCollection();
            _path = new Path { Data = geometry };
            _bounds = new Rect();
            _statsBounds = new Rect();
            Points = pts;
            Points.Changed += Points_Changed;

            _imageScaleX = xScale;
            _imageScaleY = yScale;
            _toAdjustX = 1.0 / xScale;
            _toAdjustY = 1.0 / yScale;
        }

        #endregion Constructors

        #region Properties

        /// <summary>
        /// Gets or sets a value that specifies the arc roundness.
        /// </summary>
        public double ArcRoundness
        {
            get
            {
                return _arcRoundness;
            }
            set
            {
                _arcRoundness = value;
                RedrawShape();
            }
        }

        public Rect Bounds
        {
            get
            {
                return _bounds;
            }
        }

        /// <summary>
        /// Gets or sets a value that specifies if the polygon will be closed or not.
        /// </summary>
        public bool Closed
        {
            get
            {
                return _closed;
            }
            set
            {
                _closed = value;
                RedrawShape();
            }
        }

        public Geometry Data
        {
            get
            {
                return _path.Data;
            }
        }

        /// <summary>
        /// Gets or sets a collection that contains the points of the polygon.
        /// </summary>
        public PointCollection Points
        {
            get { return _points; }
            set
            {
                _points = value;
                RedrawShape();
            }
        }

        public Rect StatsBounds
        {
            get => _statsBounds;
        }

        public PointCollection StatsPoints
        {
            get { return _statsPoints; }
        }

        public override double ImageScaleX
        {
            get
            {
                return _imageScaleX;
            }
            set
            {
                _imageScaleX = value > 0 ? value : 1.0;
            }
        }

        public override double ImageScaleY
        {
            get
            {
                return _imageScaleY;
            }
            set
            {
                _imageScaleY = value > 0 ? value : 1.0;
            }
        }

        /// <summary>
        /// Gets or sets a value that specifies if the ArcRoundness property value will be used as a percentage of the connecting segment or not.
        /// </summary>
        public bool UseRoundnessPercentage
        {
            get
            {
                return _useRoundnessPercentage;
            }
            set
            {
                _useRoundnessPercentage = value;
                RedrawShape();
            }
        }

        protected override Geometry DefiningGeometry
        {
            get
            {
                return _path.Data;
            }
        }

        #endregion Properties

        #region Methods

        public override void ApplyScaleUpdate(double xScale, double yScale)
        {
            _toAdjustX = xScale / _imageScaleX;
            _toAdjustY = yScale / _imageScaleY;

            _imageScaleX = xScale;
            _imageScaleY = yScale;

            double updatedX;
            double updatedY;

            for(int i = 0; i < _points.Count; i++)
            {
                updatedX = _points[i].X * _toAdjustX;
                updatedY = _points[i].Y * _toAdjustY;
                _points[i] = new Point(updatedX, updatedY);
            }

            RedrawShape();
        }

        /// <summary>
        /// Method used to connect 2 segments with a common point, defined by 3 points and aplying an arc segment between them
        /// </summary>
        /// <param name="pathFigure"></param>
        /// <param name="p1">First point, of the first segment</param>
        /// <param name="p2">Second point, the common point</param>
        /// <param name="p3">Third point, the second point of the second segment</param>
        /// <param name="roundness">The roundness of the arc</param>
        /// <param name="usePercentage">A value that indicates if the roundness of the arc will be used as a percentage or not</param>
        private static void ConnectLinePoints(PathFigure pathFigure, Point p1, Point p2, Point p3, double roundness, bool usePercentage)
        {
            //The point on the first segment where the curve will start.
            Point backPoint;
            //The point on the second segment where the curve will end.
            Point nextPoint;
            if (usePercentage)
            {
                backPoint = GetPointAtDistancePercent(p1, p2, roundness, false);
                nextPoint = GetPointAtDistancePercent(p2, p3, roundness, true);
            }
            else
            {
                backPoint = GetPointAtDistance(p1, p2, roundness, false);
                nextPoint = GetPointAtDistance(p2, p3, roundness, true);
            }

            int lastSegmentIndex = pathFigure.Segments.Count - 1;
            //Set the ending point of the first segment.
            ((LineSegment)(pathFigure.Segments[lastSegmentIndex])).Point = backPoint;
            //Create and add the curve.
            var curve = new QuadraticBezierSegment { Point1 = p2, Point2 = nextPoint };
            pathFigure.Segments.Add(curve);
            //Create and add the new segment.
            var line = new LineSegment { Point = p3 };
            pathFigure.Segments.Add(line);
        }

        /// <summary>
        /// Gets a point on a segment, defined by two points, at a given distance.
        /// </summary>
        /// <param name="p1">First point of the segment</param>
        /// <param name="p2">Second point of the segment</param>
        /// <param name="distance">Distance  to the point</param>
        /// <param name="firstPoint">A value that indicates if the distance is calculated by the first or the second point</param>
        /// <returns>The point calculated.</returns>
        private static Point GetPointAtDistance(Point p1, Point p2, double distance, bool firstPoint)
        {
            double segmentLength = Math.Sqrt(Math.Pow((p2.X - p1.X), 2) + Math.Pow((p2.Y - p1.Y), 2));
            //The distance cannot be greater than half of the length of the segment
            if (distance > (segmentLength / 2))
                distance = segmentLength / 2;
            double rap = firstPoint ? distance / segmentLength : (segmentLength - distance) / segmentLength;
            return new Point(p1.X + (rap * (p2.X - p1.X)), p1.Y + (rap * (p2.Y - p1.Y)));
        }

        /// <summary>
        /// Gets a point on a segment, defined by two points, at a given distance.
        /// </summary>
        /// <param name="p1">First point of the segment</param>
        /// <param name="p2">Second point of the segment</param>
        /// <param name="distancePercent">Distance percent to the point</param>
        /// <param name="firstPoint">A value that indicates if the distance is calculated by the first or the second point</param>
        /// <returns></returns>
        private static Point GetPointAtDistancePercent(Point p1, Point p2, double distancePercent, bool firstPoint)
        {
            double rap = firstPoint ? distancePercent / 100 : (100 - distancePercent) / 100;
            return new Point(p1.X + (rap * (p2.X - p1.X)), p1.Y + (rap * (p2.Y - p1.Y)));
        }

        /// <summary>
        /// Adds a point to the shape
        /// </summary>
        /// <param name="currentPoint">The current point added</param>
        /// <param name="prevPoint">Previous point</param>
        /// <param name="prevPrevPoint">The point before the previous point</param>
        private void AddPointToPath(Point currentPoint, Point? prevPoint, Point? prevPrevPoint)
        {
            if (Points.Count == 0)
                return;

            var pathGeometry = _path.Data as PathGeometry;
            if (pathGeometry == null) return;

            var pathFigure = pathGeometry.Figures[0];

            //the first point of a polygon
            if (prevPoint == null)
            {
                pathFigure.StartPoint = currentPoint;
            }
            //second point of the polygon, only a line will be drawn
            else if (prevPrevPoint == null)
            {
                var lines = new LineSegment { Point = currentPoint };
                pathFigure.Segments.Add(lines);
            }
            //third point and above
            else
            {
                ConnectLinePoints(pathFigure, prevPrevPoint.Value, prevPoint.Value, currentPoint, ArcRoundness, UseRoundnessPercentage);
            }
        }

        /// <summary>
        /// Adds the segments necessary to close the shape
        /// </summary>
        /// <param name="pathFigure"></param>
        private void CloseFigure(PathFigure pathFigure)
        {
            //No need to visually close the figure if we don't have at least 3 points.
            if (Points.Count < 3)
                return;
            Point backPoint, nextPoint;
            if (UseRoundnessPercentage)
            {
                backPoint = GetPointAtDistancePercent(Points[Points.Count - 1], Points[0], ArcRoundness, false);
                nextPoint = GetPointAtDistancePercent(Points[0], Points[1], ArcRoundness, true);
            }
            else
            {
                backPoint = GetPointAtDistance(Points[Points.Count - 1], Points[0], ArcRoundness, false);
                nextPoint = GetPointAtDistance(Points[0], Points[1], ArcRoundness, true);
            }
            ConnectLinePoints(pathFigure, Points[Points.Count - 2], Points[Points.Count - 1], backPoint, ArcRoundness, UseRoundnessPercentage);
            var line2 = new QuadraticBezierSegment { Point1 = Points[0], Point2 = nextPoint };
            pathFigure.Segments.Add(line2);
            pathFigure.StartPoint = nextPoint;
            double ROIminX = int.MaxValue, ROIminY = int.MaxValue, ROImaxX = int.MinValue, ROImaxY = int.MinValue;
            for (int i = 0; i < _points.Count; i++)
            {
                if (_points[i].X < ROIminX)
                {
                    ROIminX = _points[i].X;
                }
                if (_points[i].Y < ROIminY)
                {
                    ROIminY = _points[i].Y;
                }
                if (_points[i].X > ROImaxX)
                {
                    ROImaxX = _points[i].X;
                }
                if (_points[i].Y > ROImaxY)
                {
                    ROImaxY = _points[i].Y;
                }
            }
            Point p1 = new Point(ROIminX, ROIminY);
            Point p2 = new Point(ROImaxX, ROImaxY);

            Point sp1 = new Point(ROIminX/_imageScaleX, ROIminY/_imageScaleY);
            Point sp2 = new Point(ROImaxX/_imageScaleX, ROImaxY/_imageScaleY);

            _bounds = new Rect(p1, p2);
            _statsBounds = new Rect(sp1, sp2);
        }

        private void Points_Changed(object sender, EventArgs e)
        {
            RedrawShape();
        }

        /// <summary>
        /// Redraws the entire shape.
        /// </summary>
        private void RedrawShape()
        {
            var pathGeometry = _path.Data as PathGeometry;
            if (pathGeometry == null) return;

            var pathFigure = pathGeometry.Figures[0];

            pathFigure.Segments.Clear();
            _statsPoints.Clear();

            for (int counter = 0; counter < Points.Count; counter++)
            {
                switch (counter)
                {
                    case 0:
                        AddPointToPath(Points[counter], null, null);
                        break;
                    case 1:
                        AddPointToPath(Points[counter], Points[counter - 1], null);
                        break;
                    default:
                        AddPointToPath(Points[counter], Points[counter - 1], Points[counter - 2]);
                        break;
                }
            }

            if (_closed)
                CloseFigure(pathFigure);

            foreach (Point p in Points)
            {
                _statsPoints.Add(new Point()
                {
                    X = p.X / _imageScaleX, 
                    Y = p.Y / _imageScaleY
                });
            }
        }

        #endregion Methods
    }
}