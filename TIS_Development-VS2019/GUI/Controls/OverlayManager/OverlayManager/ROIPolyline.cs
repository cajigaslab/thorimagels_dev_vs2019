namespace OverlayManager
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

    public class ROIPolyline : ScalableShape
    {
        #region Fields

        private readonly Path _path;
        private PointCollection _points;

        private PointCollection _statsPoints;
        private double _imageScaleX;
        private double _imageScaleY;
        private double _toAdjustX;
        private double _toAdjustY;
        #endregion Fields

        #region Constructors
        public ROIPolyline()
        {
            var geometry = new PathGeometry();
            geometry.Figures.Add(new PathFigure());
            _path = new Path { Data = geometry };
            Points = new PointCollection();
            _statsPoints = new PointCollection();
            Points.Changed += Points_Changed;

            _imageScaleX = 1.0;
            _imageScaleY = 1.0;
            _toAdjustX = 1.0;
            _toAdjustY = 1.0;
            RedrawShape();
        }

        public ROIPolyline(PointCollection pts, double xScale, double yScale)
        {
            var geometry = new PathGeometry();
            geometry.Figures.Add(new PathFigure());
            _path = new Path { Data = geometry };
            Points = pts;
            Points.Changed += Points_Changed;
            _statsPoints = new PointCollection();

            _imageScaleX = xScale;
            _imageScaleY = yScale;
            _toAdjustX = 1.0 / xScale;
            _toAdjustY = 1.0 / yScale;
            RedrawShape();
        }
        #endregion Constructors


        #region Properties
        protected override Geometry DefiningGeometry
        {
            get
            {
                return _path.Data;
            }
        }

        public PointCollection Points
        {
            get { return _points; }
            set
            {
                _points = value;
                RedrawShape();
            }
        }

        public PointCollection StatsPoints
        {
            get => _statsPoints;
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

        #endregion Properties


        #region Methods

        //TODO probably don't need else if. Just add segment
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
            else
            {
                var line = new LineSegment() { Point = currentPoint };
                pathFigure.Segments.Add(line);
            }
        }

        public override void ApplyScaleUpdate(double xScale, double yScale)
        {
            _toAdjustX = xScale / _imageScaleX;
            _toAdjustY = yScale / _imageScaleY;

            _imageScaleX = xScale;
            _imageScaleY = yScale;

            double updatedX;
            double updatedY;

            for (int i = 0; i < _points.Count; i++)
            {
                updatedX = _points[i].X * _toAdjustX;
                updatedY = _points[i].Y * _toAdjustY;
                _points[i] = new Point(updatedX, updatedY);
            }

            RedrawShape();
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
            _statsPoints?.Clear();

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

            foreach (Point p in Points)
            {
                _statsPoints?.Add(new Point()
                {
                    X = p.X / _imageScaleX,
                    Y = p.Y / _imageScaleY
                });
            }
        }
    }
}

#endregion Methods
