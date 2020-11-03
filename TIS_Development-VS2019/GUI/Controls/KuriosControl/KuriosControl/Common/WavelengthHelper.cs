namespace KuriosControl.Common
{
    using System;
    using System.Windows.Media;

    public static class WavelengthHelper
    {
        #region Fields

        public static Range<int> DeviceWavelengthRange = new Range<int>(420, 730);
        public static Range<int> WavelengthRange = new Range<int>(380, 700);

        #endregion Fields

        #region Methods

        public static Color Wavelength2Color(int wavelength)
        {
            double R = 0,
                G = 0,
                B = 0,
                alpha,
                wl = wavelength;

            if (wl < 440)
            {
                R = -1 * (wl - 440) / (440 - 380);
                G = 0;
                B = 1;
            }
            else if (wl >= 440 && wl < 490)
            {
                R = 0;
                G = (wl - 440) / (490 - 440);
                B = 1;
            }
            else if (wl >= 490 && wl < 510)
            {
                R = 0;
                G = 1;
                B = -1 * (wl - 510) / (510 - 490);
            }
            else if (wl >= 510 && wl < 580)
            {
                R = (wl - 510) / (580 - 510);
                G = 1;
                B = 0;
            }
            else if (wl >= 580 && wl < 645)
            {
                R = 1;
                G = -1 * (wl - 645) / (645 - 580);
                B = 0.0;
            }
            else if (wl >= 645)
            {
                R = 1;
                G = 0;
                B = 0;
            }

            //// intensty is lower at the edges of the visible spectrum.

            if (wl > 700)
            {
                alpha = 0.3 + 0.7 * (780 - wl) / (780 - 700);
            }
            else if (wl < 420)
            {
                alpha = 0.3 + 0.7 * (wl - 380) / (420 - 380);
            }
            else
            {
                alpha = 1;
            }
            alpha = alpha > 0 ? alpha : 0;
            var color = Color.FromRgb(
                (byte)Convert.ToInt32(alpha * R * 255),
                (byte)Convert.ToInt32(alpha * G * 255),
                (byte)Convert.ToInt32(alpha * B * 255));

            return color;
        }

        #endregion Methods
    }
}