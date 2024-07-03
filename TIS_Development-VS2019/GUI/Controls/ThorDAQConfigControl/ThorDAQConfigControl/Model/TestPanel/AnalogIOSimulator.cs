using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;

namespace ThorDAQConfigControl.Model
{
    public class AnalogIOSimulator
    {
        private string[] inputChannels;
        private string[] outputChannels;
        private double inputValue;
        private double[] outputValue;
        private bool isSimulating;

        // Params for Live Sin
        private double minInputLimit;
        private double maxInputLimit;

        public AnalogIOSimulator()
        {
            inputChannels = new string[] { "ai0", "ai1", "ai2", "ai3", "ai4", "ai5", "ai6", "ai7" };
            outputChannels = new string[] { "ao0", "ao1" };
            outputValue = new double[] { 0, 0 };

            isSimulating = false;
        }

        public void UpdateAIOLiveParams(double min, double max)
        {
            minInputLimit = min;
            maxInputLimit = max;
        }

        public List<string> GetInputChannelNames()
        {
            return inputChannels.ToList();
        }

        public List<string> GetOutputChannelNames()
        {
            return outputChannels.ToList();
        }

        public double GetInputValue()
        {
            return Math.Round(inputValue, 2);
        }

        public void SetOutputValue(int channelIndex, double value)
        {
            outputValue[channelIndex] = value;
        }

        public double GetOutputValue(int channelIndex)
        {
            return outputValue[channelIndex];
        }

        public void StartSimulating()
        {
            isSimulating = true;
            Task.Factory.StartNew(new Action(() =>
            {
                DoSimulating();
            }));
        }

        public void StopSimulating()
        {
            isSimulating = false;
        }

        private void DoSimulating()
        {
            float t = 0;
            while (isSimulating)
            {
                inputValue = (maxInputLimit - minInputLimit) / 2 * Math.Sin(2 * Math.PI * t / 1000) + (maxInputLimit + minInputLimit) / 2;
                t++;
                Thread.Sleep(200);
            }
        }

        public List<Point> GetFiniteValues(double min, double max, int t)
        {
            List<Point> pointList = new List<Point>();
            var A = (max - min) / 2;
            var k = (max + min) / 2;
            for (int i = 0; i < t; i++)
            {
                pointList.Add(new Point(i, Math.Round(A * Math.Sin(2 * Math.PI * i / t) + k, 2)));
            }
            return pointList;
        }
    }
}
