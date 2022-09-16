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
        const ushort THORDAQ_DIG_ACTIVE_ENVELOPE = 0x101;
        const ushort THORDAQ_DIG_CYCLE_COMPLETE = 0x202;
        const ushort THORDAQ_DIG_CYCLE_ENVELOPE = 0x404;
        const ushort THORDAQ_DIG_EPOCH_ENVELOPE = 0x1010;
        const ushort THORDAQ_DIG_ITERATION_ENVELOPE = 0x2020;
        const ushort THORDAQ_DIG_LOW = 0x0;
        const ushort THORDAQ_DIG_PATTERN_COMPLETE = 0x4040;
        const ushort THORDAQ_DIG_PATTERN_ENVELOPE = 0x8080;
        const ushort THORDAQ_DIG_POCKELS_DIGI = 0x808;
        const double VOLT_TO_THORDAQ_VAL = 65535.0 / 20.0;

        private readonly WaveformDriverType _waveformDriverType;

        private List<Byte> _activeEnvelope = null;
        private List<Byte> _cycleComplementary = null;
        private List<Byte> _cycleComplete = null;
        private List<Byte> _cycleEnvelope = null;
        private List<ushort> _digitalLines16bit = null;
        private List<Byte> _epochEnvelope = null;
        private List<Byte> _iterEnvelope = null;
        private List<Byte> _patnComplete = null;
        private List<Byte> _patnEnvelope = null;
        private List<List<double>> _pockel = null;
        private List<List<Byte>> _pockelDig = null;
        private List<List<ushort>> _pockel_16bit = null;
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
            _pockel = new List<List<double>>();
            _pockelDig = new List<List<Byte>>();
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
            _pockel_16bit = new List<List<ushort>>();
            _digitalLines16bit = new List<ushort>();
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
            get
            {
                if ((WaveformDriverType.WaveformDriver_ThorDAQ == _waveformDriverType))
                {
                    int count = _x_volt_16bit.Count;
                    for (int i = 0; i < _pockel_16bit.Count; i++)
                    {
                        count = Math.Max(count, _pockel_16bit[i].Count);
                    }
                    return count;
                }
                else
                {
                    int count = _x_volt.Count;
                    for (int i = 0; i < _pockel.Count; i++)
                    {
                        count = Math.Max(count, _pockel[i].Count);
                    }
                    return count;
                }
            }
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

        public List<ushort> DigitalLines16bit
        {
            get { return _digitalLines16bit; }
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

        public List<List<double>> Pockel
        {
            get { return _pockel; }
        }

        public List<List<byte>> PockelDig
        {
            get { return _pockelDig; }
        }

        public double[] PockelIdle
        {
            get;
            set;
        }

        public List<List<ushort>> Pockel_16bit
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

        public void AddValues(double? x_Volt, double? y_Volt, double[] pockel, byte cycleComp, byte cycleEnv, byte iterEnv, byte patnEnv, byte patnComp, byte epochEnv, byte activeEnv)
        {
            if (null != x_Volt)
            {
                _x_volt.Add(Math.Min(MAX_GALVO_VOLTAGE, Math.Max((double)x_Volt, MIN_GALVO_VOLTAGE)));
            }
            if (null != y_Volt)
            {
                _y_volt.Add(Math.Min(MAX_GALVO_VOLTAGE, Math.Max((double)y_Volt, MIN_GALVO_VOLTAGE)));
            }
            if (WaveformDriverType.WaveformDriver_NI == _waveformDriverType)
            {
                for (int i = 0; i < pockel.Length; i++)
                {
                    if (i + 1 > _pockel.Count)
                    {
                        _pockel.Add(new List<double>());
                        _pockelDig.Add(new List<byte>());
                    }
                    _pockel[i].Add(pockel[i]);
                    _pockelDig[i].Add(PockelIdle[i] < pockel[i] ? (byte)1 : (byte)0);

                }

                _activeEnvelope.Add(activeEnv);

                _cycleComplete.Add(cycleComp);

                _cycleEnvelope.Add(cycleEnv);

                _iterEnvelope.Add(iterEnv);

                _patnEnvelope.Add(patnEnv);

                _patnComplete.Add(patnComp);

                _epochEnvelope.Add(epochEnv);

                _cycleComplementary.Add(((byte)1 == cycleEnv) ? (byte)0 : (byte)1);
            }
            //when the waveform driver is thordaq then calculate and
            //add the unsigned 16bit equivalent voltage value
            else if (WaveformDriverType.WaveformDriver_ThorDAQ == _waveformDriverType)
            {

                if (null != x_Volt)
                {
                    ushort mid = x_Volt > 0 ? (ushort)0x7fff : (ushort)0x8000;
                    _x_volt_16bit.Add((ushort)(Math.Round(x_Volt.Value * VOLT_TO_THORDAQ_VAL) + mid));
                }
                if (null != y_Volt)
                {
                    ushort mid = y_Volt > 0 ? (ushort)0x7fff : (ushort)0x8000;
                    _y_volt_16bit.Add((ushort)(Math.Round(y_Volt.Value * VOLT_TO_THORDAQ_VAL) + mid));
                }
                ushort pockelDigi = THORDAQ_DIG_LOW;
                for (int i = 0; i < pockel.Length; i++)
                {
                    ushort mid = pockel[i] > 0 ? (ushort)0x7fff : (ushort)0x8000;

                    if (i + 1 > _pockel_16bit.Count)
                    {
                        _pockel_16bit.Add(new List<ushort>());
                    }

                    ushort pockel16bit = (ushort)(Math.Round(pockel[i] * VOLT_TO_THORDAQ_VAL) + mid);

                    _pockel_16bit[i].Add(pockel16bit);

                    if (PockelIdle[i] < pockel[i])
                    {
                        pockelDigi = THORDAQ_DIG_POCKELS_DIGI;
                    }
                }

                ushort digi = activeEnv == 0x1 ? THORDAQ_DIG_ACTIVE_ENVELOPE : THORDAQ_DIG_LOW;
                digi |= cycleComp == 0x1 ? THORDAQ_DIG_CYCLE_COMPLETE : THORDAQ_DIG_LOW;
                digi |= cycleEnv == 0x1 ? THORDAQ_DIG_CYCLE_ENVELOPE : THORDAQ_DIG_LOW;
                digi |= iterEnv == 0x1 ? THORDAQ_DIG_ITERATION_ENVELOPE : THORDAQ_DIG_LOW;
                digi |= patnEnv == 0x1 ? THORDAQ_DIG_PATTERN_ENVELOPE : THORDAQ_DIG_LOW;
                digi |= patnComp == 0x1 ? THORDAQ_DIG_PATTERN_COMPLETE : THORDAQ_DIG_LOW;
                digi |= epochEnv == 0x1 ? THORDAQ_DIG_EPOCH_ENVELOPE : THORDAQ_DIG_LOW;
                digi |= pockelDigi;

                _digitalLines16bit.Add(digi);
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

            _x_volt_16bit.Clear();
            _y_volt_16bit.Clear();
            _pockel_16bit.Clear();
            _digitalLines16bit.Clear();
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