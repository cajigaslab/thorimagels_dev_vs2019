using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

// D. Zimmerman, 3/17/2020
// Sources: http://web.mit.edu/~gari-teaching/6.555/lectures/ch_DFT.pdf
//          http://practicalcryptography.com/miscellaneous/machine-learning/intuitive-guide-discrete-fourier-transform/
//          http://planetcalc.com/7543
//
// compute Discrete Fourier Transform based on a single column of text floating point numbers (in CSV file)
//namespace thordaqSDKGUI
namespace tdDFT
{
    // N     -- N is total number of discrete voltage samples
    // x[n]  -- x is voltage at sample number 'n', where  n goes from 0 to N-1
    // k     -- k is index of time domain, varies from 0 to N/2
    public class DiscreteFourierTrans
    {
        public long ComputeDFT( string Inputfilespec, long startingSample, int column, int samplingRate, bool HannWindow)  // startingSample is ORDINAL; up to 4 columns from Labview
        {
            bool doInverseDFT = false;
            long totalADCsamples;
            long sampleNum = startingSample - 1;
            string[] lines = File.ReadAllLines(Inputfilespec);

            double[] x = new double[lines.Length];
            long k;
            long n;

            for (totalADCsamples = 0; sampleNum < lines.Length; totalADCsamples++, sampleNum++)
            {
                // is it CSV file?  If not, assume single double number per line
                if( lines[sampleNum].Contains(","))
                {
                    string[] Field = lines[sampleNum].Split(',');
                    x[totalADCsamples] = double.Parse(Field[column]);
                }
                else
                {
                    x[totalADCsamples] = double.Parse(lines[sampleNum]);
                }
            }
            
            // suppose we measure an 8kHz signal at 500,000 samples/sec
            // 8kHz is period of 0.000125 seconds, so if we collect 100 complete periods,
            // the time is 0.0125 seconds.
            // At 500 kSample/sec * 0.0125 secs, that's 6250 samples

            long SampleLimitSize = 16*1024;
            if (totalADCsamples > SampleLimitSize) totalADCsamples = SampleLimitSize;
            // we have sample count and array of the floating point sample values
            // compute real and imaginary components
            double[] Xr = new double[totalADCsamples];  // "real" part
            double[] Xi = new double[totalADCsamples];  // "imaginary" part
            double[] Xamp = new double[totalADCsamples]; // RMS amplitude of real and imaginary
            // note that the resolution of the discrete frequency amplitudes depends on the number of samples
            // for instance, if we sample at 500,000 samples/sec, and have 16,384 total samples,
            // then each "bin" is separated by about 30.5 Hz, from 0 Hz up to 500kHz
            // A typical PXIe1071 measured (500 kSample/sec) ThorDAQ nominal 8 kHz sign wave
            // shows up on Agilent scope as 8059 Hz, and the DFT amplitude peak shows up
            // at 8056 Hz as expected.
            double[] binFreq = new double[totalADCsamples]; // actual HZ frequency of DFT coefficient "bins"

            // the real DFT (no "imaginary" component of sampled signals x[n])
            // is summation of N total samples for K total DFT coefficients
            // where both n and k go from 0 to N-1
            // x[] is array of DAC voltage samples (nominal range -10.0 to +10.0)

            // Sum x[n]*e^(-i(2*pi*k*n/N))

            // the crux of the calculation is Euler's identity, e^(-i*a) (where i is imaginary number and a is angle in radians)
            // expands to 
            // cos(a) - i*sin(a)
            // This inverse DFT (below) recombines the real and imaginary parts according to forumla referenced above
            // divided by N total samples
            long kLimit = totalADCsamples/2;
            if (doInverseDFT == true)
            {
                kLimit = totalADCsamples; // needed for Inverse function
            }

            double voltageSample; // for windowing...

            for (k = 0; k < kLimit; k++) // process only N/2 to save time (unless Inverse test needed)
            {
                double SumR = 0;
                double SumI = 0;
                for (n = 0; n < totalADCsamples; n++)
                {
                    double radianAngle = 2 * Math.PI * k * n / totalADCsamples;
                    if (HannWindow == true) // smooth out the rect() window?
                    {
                        voltageSample = x[n] * (0.5 * (1- Math.Cos( 2* Math.PI * n / totalADCsamples)) );
                    }
                    else
                        voltageSample = x[n];

                    SumR += voltageSample * Math.Cos(radianAngle);  // the "real" component
                    SumI += voltageSample * Math.Sin(radianAngle);  // the "imaginary" component, multiplied by "-i" (negative imaginary number)
                }
                Xr[k] = SumR;
                Xi[k] = SumI;
                Xamp[k] = Math.Sqrt( (Xr[k] * Xr[k]) + (Xi[k] * Xi[k])) / (double)totalADCsamples;
                binFreq[k] = k * samplingRate / totalADCsamples;
            }
            // inverse DFT
            if (doInverseDFT == true)
            {
                double OriginalVolts = 0;
                for (n = 0; n < totalADCsamples; n++)
                {
                    double SumR = 0;
                    double SumI = 0;
                    for (k = 0; k < totalADCsamples; k++)
                    {
                        double radianAngle = 2 * Math.PI * k * n / totalADCsamples;
                        SumR += Xr[k] * Math.Cos(radianAngle);
                        SumI += Xi[k] * Math.Sin(radianAngle);
                    }
                    OriginalVolts = (SumR + SumI) / totalADCsamples;
                }
            }

            return totalADCsamples; // return ordinal count
        }





    }
}
