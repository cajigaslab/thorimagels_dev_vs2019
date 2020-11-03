namespace ThorSharedTypes
{
    using System;
    using System.Collections.Generic;
    using System.Collections.Specialized;
    using System.ComponentModel;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Xml;

    #region Enumerations

    /// <summary>
    /// Carrier Type enumeration
    /// </summary>
    public enum CarrierType : int
    {
        Slide,
        Multiwell,
        Multislide,
        PetriDish
    }

    /// <summary>
    /// Well shape enumeration
    /// </summary>
    public enum WellShape : int
    {
        CircleWell,
        RectangleWell
    }

    #endregion Enumerations

    /// <summary>
    /// Tile carrier information
    /// </summary>
    public class Carrier
    {
        #region Properties

        public double Height
        {
            get;
            set;
        }

        public String Name
        {
            get;
            set;
        }

        public WellPlateTemplate Template
        {
            get;
            set;
        }

        public CarrierType Type
        {
            get;
            set;
        }

        public double Width
        {
            get;
            set;
        }

        #endregion Properties
    }

    public static class SampleDimensions
    {
        #region Methods

        public static int Columns(SampleType type)
        {
            switch (type)
            {
                case SampleType.WELL6: return 3;
                case SampleType.WELL24: return 6;
                case SampleType.WELL96: return 12;
                case SampleType.WELL384: return 24;
                case SampleType.WELL1536: return 48;
                case SampleType.SLIDE: return 1;
                default: return 1;
            }
        }

        public static int Rows(SampleType type)
        {
            switch (type)
            {
                case SampleType.WELL6: return 2;
                case SampleType.WELL24: return 4;
                case SampleType.WELL96: return 8;
                case SampleType.WELL384: return 16;
                case SampleType.WELL1536: return 32;
                case SampleType.SLIDE: return 1;
                default: return 1;
            }
        }

        public static double WellOffsetMM(SampleType type)
        {
            switch (type)
            {
                case SampleType.WELL6: return 36;
                case SampleType.WELL24: return 18;
                case SampleType.WELL96: return 9;
                case SampleType.WELL384: return 4.5;
                case SampleType.WELL1536: return 2.25;
                case SampleType.SLIDE: return 0;
                default: return 0;
            }
        }

        #endregion Methods
    }

    public static class SampleNames
    {
        #region Fields

        const int ALPHABET_LENGTH = 26;

        #endregion Fields

        #region Methods

        public static string GetName(SampleType type)
        {
            switch (type)
            {
                case SampleType.WELL6: return "6 Well Plate";
                case SampleType.WELL24: return "24 Well Plate";
                case SampleType.WELL96: return "96 Well Plate";
                case SampleType.WELL384: return "384 Well Plate";
                case SampleType.WELL1536: return "1536 Well Plate";
                case SampleType.SLIDE: return "Slide";
                default: return "Unknown";
            }
        }

        public static SampleType GetSampleType(int number)
        {
            switch (number)
            {
                case 0: return SampleType.WELL6;
                case 1: return SampleType.WELL24;
                case 2: return SampleType.WELL96;
                case 3: return SampleType.WELL384;
                case 4: return SampleType.WELL1536;
                case 5: return SampleType.SLIDE;
                default: return SampleType.WELL96;
            }
        }

        public static string WellName(SampleType type, int index)
        {
            //calculate row
            int val = Convert.ToInt32(Math.Floor((double)index / SampleDimensions.Columns(type)));

            string strRow;

            int valAlpha = (val / ALPHABET_LENGTH);

            if (valAlpha > 0)
            {
                char a = Convert.ToChar((Convert.ToInt32('A') + (valAlpha)));
                char b = Convert.ToChar(Convert.ToInt32('a') + val - valAlpha * ALPHABET_LENGTH);
                strRow = a.ToString() + b.ToString();
            }
            else
            {
                char a = Convert.ToChar((Convert.ToInt32('A') + (val)));
                strRow = a.ToString();
            }

            //calculate column using remainder
            int col = index - val * SampleDimensions.Columns(type) + 1;

            return String.Format("{0}{1:00}", strRow, col);
        }

        public static int WellNumber(SampleType type, string wellName)
        {
            int number = 0;
            string alphabet = null;
            int wellnumber = 0;

            if (wellName.Length == 3)
            {
                number = int.Parse(wellName.Substring(1));
                alphabet = wellName.Substring(0, 1);
                wellnumber = ((Convert.ToInt32(alphabet) - (Convert.ToInt32('A'))) * SampleDimensions.Columns(type)) + number;
            }
            else if (wellName.Length == 4) //for 1536 wellplate
            {
                number = int.Parse(wellName.Substring(2));
                alphabet = wellName.Substring(0, 2);
                wellnumber = (((Convert.ToInt32(alphabet) - (Convert.ToInt32('a'))) * SampleDimensions.Columns(type)) + number) + (ALPHABET_LENGTH * SampleDimensions.Columns(type));
            }

            return wellnumber;
        }

        #endregion Methods
    }

    /// <summary>
    /// Record WellPlate information
    /// </summary>
    public class WellPlateTemplate
    {
        #region Properties

        //The horizontal distance between two wells nearby
        public double CenterToCenterX
        {
            get;
            set;
        }

        //The vertical distance between two wells nearby
        public double CenterToCenterY
        {
            get;
            set;
        }

        //The total number of columns of wells in the gird
        public int Col
        {
            get;
            set;
        }

        //The diameter of each single well; only enabled when the type of each well is circle
        public double Diameter
        {
            get;
            set;
        }

        //The Height of each single well; only enabled when the type of each well is rectangle
        public double Height
        {
            get;
            set;
        }

        //The total number of rows of wells in the gird
        public int Row
        {
            get;
            set;
        }

        //The type of each well: circle or rectangle
        public WellShape Shape
        {
            get;
            set;
        }

        // The horizontal distance between top-left bound point of wellplate and center point of top-left well
        public double TopLeftCenterOffsetX
        {
            get;
            set;
        }

        // The vectical distance between top-left bound point of wellplate and center point of top-left well
        public double TopLeftCenterOffsetY
        {
            get;
            set;
        }

        //The Width of each single well; only enabled when the type of each well is rectangle
        public double Width
        {
            get;
            set;
        }

        #endregion Properties
    }

    /// <summary>
    /// Tile position Information
    /// </summary>
    public class XYPosition
    {
        #region Fields

        private string overlapX = "0";
        private string overlapY = "0";
        private string tileCol = "1";
        private string tileRow = "1";
        private string x = "0";
        private string y = "0";
        private string z = "0";

        #endregion Fields

        #region Properties

        //
        public bool IsEnabled
        {
            get;
            set;
        }

        //The discription of each tiled are
        public string Name
        {
            get;
            set;
        }

        public string OverlapX
        {
            get
            {
                return overlapX;
            }
            set
            {
                Double n;
                if (Double.TryParse(value, out n))
                {
                    overlapX = value;
                }
            }
        }

        public string OverlapY
        {
            get
            {
                return overlapY;
            }
            set
            {
                Double n;
                if (Double.TryParse(value, out n))
                {
                    overlapY = value;
                }
            }
        }

        public string TileCol
        {
            get
            {
                return tileCol;
            }
            set
            {
                int n;
                if (int.TryParse(value, out n))
                {
                    tileCol = value;
                }
            }
        }

        public string TileRow
        {
            get
            {
                return tileRow;
            }
            set
            {
                int n;
                if (int.TryParse(value, out n))
                {
                    tileRow = value;
                }
            }
        }

        public string Well
        {
            get;
            set;
        }

        public string X
        {
            get
            {
                return x;
            }
            set
            {
                Double n;
                if (Double.TryParse(value, out n))
                {
                    x = value;
                }
            }
        }

        public string Y
        {
            get
            {
                return y;
            }
            set
            {
                Double n;
                if (Double.TryParse(value, out n))
                {
                    y = value;
                }
            }
        }

        public string Z
        {
            get
            {
                return z;
            }
            set
            {
                Double n;
                if (Double.TryParse(value, out n))
                {
                    z = value;
                }
            }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Override fucntion. Determines whether the specified <see cref="System.Object" />, is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals(object obj)
        {
            // If parameter is null return false.
            if (obj == null)
            {
                return false;
            }
            // If parameter cannot be cast to Point return false.
            XYPosition p = obj as XYPosition;
            if ((System.Object)p == null)
            {
                return false;
            }
            // Return true if the fields match:
            return ((OverlapX == p.OverlapX) && (OverlapY == p.OverlapY) &&
                    (TileCol == p.TileCol) && (TileRow == p.TileRow) && (X == p.X) && (Y == p.Y)); // Ignore the isenable property
        }

        /// <summary>
        /// Equalses the specified p.
        /// </summary>
        /// <param name="p">The p.</param>
        /// <returns></returns>
        public bool Equals(XYPosition p)
        {
            // If parameter is null return false:
            if ((object)p == null)
            {
                return false;
            }

            // Return true if the fields match:
            return ((OverlapX == p.OverlapX) && (OverlapY == p.OverlapY) &&
                    (TileCol == p.TileCol) && (TileRow == p.TileRow) && (X == p.X) && (Y == p.Y));
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table. 
        /// </returns>
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        #endregion Methods
    }
}