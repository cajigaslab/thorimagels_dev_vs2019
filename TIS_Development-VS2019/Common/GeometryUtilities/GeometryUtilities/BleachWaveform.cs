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

    /// <summary>
    /// Analog waveform for X, Y, Pockel; Digital waveform for Pockel and CycleComplete.
    /// </summary>
    public class BleachWaveform
    {
        #region Fields

        const float MAX_GALVO_VOLTAGE = (float)10.0;
        const float MIN_GALVO_VOLTAGE = (float)-10.0;
        const ushort THORDAQ_VOLT_OFFSET = 10;
        const double VOLT_TO_THORDAQ_VAL = 65535 / 20;

        private readonly WaveformDriverType _waveformDriverType;

        private List<Byte> _activeEnvelope = null;
        private List<Byte> _cycleComplementary = null;
        private List<Byte> _cycleComplete = null;
        private List<Byte> _cycleEnvelope = null;
        private List<Byte> _epochEnvelope = null;
        private List<Byte> _iterEnvelope = null;
        private List<Byte> _patnComplete = null;
        private List<Byte> _patnEnvelope = null;
        private List<double> _pockel = null;
        private List<Byte> _pockelDig = null;
        private List<ushort> _pockel_16bit = null;
        private List<double> _x_volt = null;
        private List<ushort> _x_volt_16bit = null;
        private List<double> _y_volt = null;
        private List<ushort> _y_volt_16bit = null;

        #endregion Fields

        #region Constructors

        public BleachWaveform(WaveformDriverType waveformDriverType)
        {
            _x_volt = new List<double>();
            _y_volt = new List<double>();
            _pockel = new List<double>();
            _pockelDig = new List<Byte>();
            _activeEnvelope = new List<byte>();
            _cycleComplete = new List<Byte>();
            _cycleEnvelope = new List<Byte>();
            _cycleComplementary = new List<byte>();
            _iterEnvelope = new List<Byte>();
            _patnEnvelope = new List<Byte>();
            _patnComplete = new List<Byte>();
            _epochEnvelope = new List<byte>();

            _x_volt_16bit = new List<ushort>();
            _y_volt_16bit = new List<ushort>();
            _pockel_16bit = new List<ushort>();

            _waveformDriverType = waveformDriverType;
        }

        #endregion Constructors

        #region Properties

        public List<byte> ActiveEnvelope
        {
            get { return _activeEnvelope; }
        }

        public UInt32 ClockRate
        {
            get;
            set;
        }

        public int Count
        {
            get { return _x_volt.Count; }
        }

        public List<byte> CycleComplementary
        {
            get { return _cycleComplementary; }
        }

        public List<byte> CycleComplete
        {
            get { return _cycleComplete; }
        }

        public List<byte> CycleEnvelope
        {
            get { return _cycleEnvelope; }
        }

        public List<byte> EpochEnvelope
        {
            get { return _epochEnvelope; }
        }

        public List<byte> IterationEnvelope
        {
            get { return _iterEnvelope; }
        }

        public List<byte> PatternComplete
        {
            get { return _patnComplete; }
        }

        public List<byte> PatternEnvelope
        {
            get { return _patnEnvelope; }
        }

        public List<double> Pockel
        {
            get { return _pockel; }
        }

        public List<byte> PockelDig
        {
            get { return _pockelDig; }
        }

        public double PockelIdle
        {
            get;
            set;
        }

        public List<ushort> Pockel_16bit
        {
            get { return _pockel_16bit; }
        }

        public WaveformDriverType WaveformDriverType
        {
            get { return _waveformDriverType; }
        }

        public List<double> X_Volt
        {
            get { return _x_volt; }
        }

        public List<ushort> X_Volt_16bit
        {
            get { return _x_volt_16bit; }
        }

        public List<double> Y_Volt
        {
            get { return _y_volt; }
        }

        public List<ushort> Y_Volt_16bit
        {
            get { return _y_volt_16bit; }
        }

        #endregion Properties

        #region Methods

        public void AddValues(double x_Volt, double y_Volt, double pockel, byte cycleComp, byte cycleEnv, byte iterEnv, byte patnEnv, byte patnComp, byte epochEnv, byte activeEnv)
        {
            double valX = Math.Min(MAX_GALVO_VOLTAGE, Math.Max(x_Volt, MIN_GALVO_VOLTAGE));
            _x_volt.Add(valX);

            double valY = Math.Min(MAX_GALVO_VOLTAGE, Math.Max(y_Volt, MIN_GALVO_VOLTAGE));
            _y_volt.Add(valY);

            _pockel.Add(pockel);
            _pockelDig.Add((PockelIdle < pockel) ? (byte)(1) : (byte)(0));

            _activeEnvelope.Add(activeEnv);

            _cycleComplete.Add(cycleComp);

            _cycleEnvelope.Add(cycleEnv);

            _iterEnvelope.Add(iterEnv);

            _patnEnvelope.Add(patnEnv);

            _patnComplete.Add(patnComp);

            _epochEnvelope.Add(epochEnv);

            _cycleComplementary.Add(((byte)1 == cycleEnv) ? (byte)0 : (byte)1);

            //when the waveform driver is thordaq then calculate and
            //add the unsigned 16bit equivalent voltage value
            if (WaveformDriverType.WaveformDriver_ThorDAQ == _waveformDriverType)
            {
                _x_volt_16bit.Add((ushort)Math.Min(Math.Max(((valX + THORDAQ_VOLT_OFFSET) * VOLT_TO_THORDAQ_VAL), 0), ushort.MaxValue));
                _y_volt_16bit.Add((ushort)Math.Min(Math.Max(((valY + THORDAQ_VOLT_OFFSET) * VOLT_TO_THORDAQ_VAL), 0), ushort.MaxValue));
                _pockel_16bit.Add((ushort)Math.Min(Math.Max(((pockel + THORDAQ_VOLT_OFFSET) * VOLT_TO_THORDAQ_VAL), 0), ushort.MaxValue));
            }
        }

        public void Clear()
        {
            _x_volt.Clear();
            _y_volt.Clear();
            _pockel.Clear();
            _pockelDig.Clear();
            _activeEnvelope.Clear();
            _cycleComplete.Clear();
            _cycleEnvelope.Clear();
            _iterEnvelope.Clear();
            _patnEnvelope.Clear();
            _patnComplete.Clear();
            _epochEnvelope.Clear();
            _cycleComplementary.Clear();
        }

        public List<double> CreateXYInterList()
        {
            List<double> output = new List<double>();
            if (_x_volt.Count == _y_volt.Count)
            {
                for (int i = 0; i < _x_volt.Count; i++)
                {
                    output.Add(_x_volt[i]);
                    output.Add(_y_volt[i]);
                }
            }
            return output;
        }

        #endregion Methods
    }
}