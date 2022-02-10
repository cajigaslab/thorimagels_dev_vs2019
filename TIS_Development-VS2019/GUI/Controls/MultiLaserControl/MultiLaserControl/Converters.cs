namespace MultiLaserControl
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using System.Windows.Controls;
    using System.Windows.Data;

    using ThorSharedTypes;

    public class PercentConverter : IMultiValueConverter
    {
        #region Fields

        private static double[] dMaxLaser = new double[4];
        private static double[] dMinLaser = new double[4];

        #endregion Fields

        #region Methods

        object IMultiValueConverter.Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            double dVal = Convert.ToDouble(values[0]);
            double dMin = Convert.ToDouble(values[1]);
            double dMax = Convert.ToDouble(values[2]);
            Decimal dec = new Decimal();

            if ((dMax - dMin) <= 0)
            {
                return 0.0;
            }

            dec = new Decimal((dVal - dMin) * 100 / (dMax - dMin));

            dMinLaser[Convert.ToInt32(parameter) - 1] = dMin;
            dMaxLaser[Convert.ToInt32(parameter) - 1] = dMax;

            if (targetType == typeof(string))
            {
                return Decimal.Round(dec, 2).ToString();
            }
            else
            {
                return Convert.ToDouble(Decimal.Round(dec, 2).ToString());
            }
        }

        object[] IMultiValueConverter.ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            double dVal = Convert.ToDouble(value);
            Decimal[] dec = new Decimal[3];//new Decimal();
            dec[1] = new Decimal(dMinLaser[Convert.ToInt32(parameter) - 1]);
            dec[2] = new Decimal(dMaxLaser[Convert.ToInt32(parameter) - 1]);
            dec[0] = new Decimal((dVal - 0) * (dMaxLaser[Convert.ToInt32(parameter) - 1] - dMinLaser[Convert.ToInt32(parameter) - 1]) / 100 + dMinLaser[Convert.ToInt32(parameter) - 1]);

            if (targetTypes[0] == typeof(string))
            {
                string[] ret = new string[3];
                ret[0] = Decimal.Round(dec[0], 2).ToString();
                ret[1] = Decimal.Round(dec[1], 2).ToString();
                ret[2] = Decimal.Round(dec[2], 2).ToString();
                return ret;
            }
            else
            {
                Double[] dRet = new Double[3];
                dRet[0] = Convert.ToDouble(Decimal.Round(dec[0], 2).ToString());
                dRet[1] = Convert.ToDouble(Decimal.Round(dec[0], 2).ToString());
                dRet[2] = Convert.ToDouble(Decimal.Round(dec[0], 2).ToString());
                object[] ret = new object[dRet.Length];
                dRet.CopyTo(ret, 0);
                return ret;
            }
        }

        #endregion Methods
    }
}