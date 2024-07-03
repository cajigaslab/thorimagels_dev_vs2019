namespace ThorSharedTypes
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    public class ImageIdentifier : IEquatable<ImageIdentifier>
    {
        #region Constructors

        public ImageIdentifier(int channel, int plane)
        {
            Channel = channel;
            Plane = plane;
        }

        #endregion Constructors

        #region Properties

        public int Channel
        {
            get; set;
        }

        public int Plane
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        public static ImageIdentifier MakeFromKeyString(string key)
        {
            var splitKey = key.Split('-');

            int channel = 0;
            int plane = 0;

            foreach (var splitKeyEntry in splitKey)
            {
                if (splitKeyEntry.Length < 2)
                {
                    continue;
                }

                var subString = splitKeyEntry.Substring(1);

                switch (splitKeyEntry[0])
                {
                    case 'C':
                        {
                            _ = int.TryParse(subString, out channel);
                        }
                        break;
                    case 'P':
                        {
                            _ = int.TryParse(subString, out plane);
                        }
                        break;
                }
            }

            return new ImageIdentifier(channel, plane);
        }

        public string AsKeyString()
        {
            return string.Format("C{0}-P{1}", Channel, Plane);
        }

        public bool Equals(ImageIdentifier imageIdentifier)
        {
            return imageIdentifier.Channel == Channel && imageIdentifier.Plane == Plane;
        }

        #endregion Methods
    }
}