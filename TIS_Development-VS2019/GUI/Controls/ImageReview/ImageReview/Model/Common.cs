using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace ImageReviewModule.Model
{
    public enum Shape
    {
        SHAPE_UNDEFINED = -1,
        SHAPE_RECTANGLE = 1,
        SHAPE_ELLIPSE = 2,
    }

    public enum DistanceUnit
    {
        DISTANCE_UNDEFINED = -1,
        DISTANCE_KILOMETER = 1,
        DISTANCE_METER = 2,
        DISTANCE_MILLIMETER = 3,
        DISTANCE_MICROMETER = 4,
        DISTANCE_NANOMETER = 5,
        DISTANCE_PICOMETER = 6,
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct SampleInfo
    {
        public string name;
        public float width;
        public float height;
        public string type; // SampleType
        public ushort rowSize;
        public ushort columnSize;
        public float centerToCenterX;
        public float centerToCenterY;
        public float topLeftCenterOffsetX;
        public float topLeftCenterOffsetY;
        public int wellShape;
        public float diameter; // for SHAPE_ELLIPSE
        public float wellWidth; // for SHAPE_RECTANGLE
        public float wellHeight; // for SHAPE_RECTANGLE
    }

    public class Common
    {
    }
}
