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

        public const byte BYTE_HIGH = (byte)1;
        public const byte BYTE_LOW = (byte)0;

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
        List<ushort> _pockelsOffset16bit = new List<ushort>();
        private List<List<ushort>> _pockel_16bit = null;
        ushort _xOffset16bit = 0;
        private List<double> _x_volt = null;
        private List<ushort> _x_volt_16bit = null;
        ushort _yOffset16bit = 0;
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

        public List<ushort> PockelsOffset16bit
        {
            get => _pockelsOffset16bit;
        }

        public List<List<ushort>> Pockel_16bit
        {
            get { return _pockel_16bit; }
        }

        public WaveformDriverType WaveformDriverType
        {
            get { return _waveformDriverType; }
        }

        public ushort XOffset16bit
        {
            get => _xOffset16bit;
        }

        public List<double> X_Volt
        {
            get { return _x_volt; }
        }

        public List<ushort> X_Volt_16bit
        {
            get { return _x_volt_16bit; }
        }

        public ushort YOffset16bit
        {
            get => _yOffset16bit;
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
                    if (_x_volt_16bit.Count == 0)
                    {
                        _x_volt_16bit.Add(0);
                        _xOffset16bit = (ushort)(Math.Round(x_Volt.Value * VOLT_TO_THORDAQ_VAL) + mid);
                    }
                    else
                    {
                        _x_volt_16bit.Add((ushort)(Math.Round(x_Volt.Value * VOLT_TO_THORDAQ_VAL) + mid - _xOffset16bit));
                    }
                }
                if (null != y_Volt)
                {
                    ushort mid = y_Volt > 0 ? (ushort)0x7fff : (ushort)0x8000;

                    if (_y_volt_16bit.Count == 0)
                    {
                        _y_volt_16bit.Add(0);
                        _yOffset16bit = (ushort)(Math.Round(y_Volt.Value * VOLT_TO_THORDAQ_VAL) + mid);
                    }
                    else
                    {
                        _y_volt_16bit.Add((ushort)(Math.Round(y_Volt.Value * VOLT_TO_THORDAQ_VAL) + mid - _yOffset16bit));
                    }
                }
                ushort pockelDigi = THORDAQ_DIG_LOW;
                for (int i = 0; i < pockel.Length; i++)
                {
                    ushort mid = pockel[i] > 0 ? (ushort)0x7fff : (ushort)0x8000;

                    if (i + 1 > _pockel_16bit.Count)
                    {
                        _pockel_16bit.Add(new List<ushort>());
                        _pockelsOffset16bit.Add(0);
                    }

                    ushort pockel16bit = (ushort)(Math.Round(pockel[i] * VOLT_TO_THORDAQ_VAL) + mid);

                    if (_pockel_16bit[i].Count == 0)
                    {
                        _pockel_16bit[i].Add(0);
                        _pockelsOffset16bit[i] = pockel16bit;
                    }
                    else
                    {
                        _pockel_16bit[i].Add((ushort)(pockel16bit - _pockelsOffset16bit[i]));
                    }

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

        /// <summary>
        /// Shift pockels waveform relative to galvo's waveform forward(-) or backward (+), startIndex: 1st index of waveform body (after travel)
        /// </summary>
        /// <param name="startIndex"></param>
        /// <param name="shiftValue"></param>
        public void ShiftPockel(bool final, int startIndex, int shiftValue)
        {
            if (0 == shiftValue)
                return;

            int shiftCount = Math.Abs(shiftValue);
            List<byte> lowBytes = Enumerable.Repeat(BYTE_LOW, shiftCount).ToList();
            List<byte> highBytes = Enumerable.Repeat(BYTE_HIGH, shiftCount).ToList();

            if (0 < shiftValue)
            {
                //**************************************************************************************************//
                //  delay pockels start relative to galvo, padding at beginning of pockels and at the end of galvo  //
                //**************************************************************************************************//
                List<double> xInsertVolts = Enumerable.Repeat(Math.Min(MAX_GALVO_VOLTAGE, Math.Max((double)_x_volt.Last(), MIN_GALVO_VOLTAGE)), shiftCount).ToList();
                List<double> yInsertVolts = Enumerable.Repeat(Math.Min(MAX_GALVO_VOLTAGE, Math.Max((double)_y_volt.Last(), MIN_GALVO_VOLTAGE)), shiftCount).ToList();
                _x_volt.AddRange(xInsertVolts);
                _y_volt.AddRange(yInsertVolts);

                switch (_waveformDriverType)
                {
                    case WaveformDriverType.WaveformDriver_NI:
                        {
                            for (int i = 0; i < _pockel.Count; i++)
                            {
                                List<double> pockelValues = Enumerable.Repeat(_pockel[i].ElementAt(startIndex), shiftCount).ToList();
                                List<byte> pockelDigValues = Enumerable.Repeat(PockelIdle[i] < _pockel[i].ElementAt(startIndex) ? (byte)1 : (byte)0, shiftCount).ToList();
                                _pockel[i].InsertRange(startIndex, pockelValues);
                                _pockelDig[i].InsertRange(startIndex, pockelDigValues);
                            }

                            _cycleComplete.InsertRange(startIndex, lowBytes);
                            _cycleComplementary.InsertRange(startIndex, highBytes);
                            _cycleEnvelope.InsertRange(startIndex, lowBytes);
                            _iterEnvelope.InsertRange(startIndex, lowBytes);
                            _patnEnvelope.InsertRange(startIndex, lowBytes);
                            _patnComplete.InsertRange(startIndex, lowBytes);
                            _epochEnvelope.InsertRange(startIndex, lowBytes);
                            _activeEnvelope.InsertRange(startIndex, highBytes);
                        }
                        break;
                    case WaveformDriverType.WaveformDriver_ThorDAQ:
                        {
                            double? targetXVolt = _x_volt.Last();
                            if (null != targetXVolt)
                            {
                                ushort mid = targetXVolt.Value > 0 ? (ushort)0x7fff : (ushort)0x8000;
                                List<ushort> targetXVolts = Enumerable.Repeat((ushort)(Math.Round(targetXVolt.Value * VOLT_TO_THORDAQ_VAL) + mid - _xOffset16bit), shiftCount).ToList();
                                _x_volt_16bit.AddRange(targetXVolts);
                            }
                            double? targetYVolt = _y_volt.Last();
                            if (null != targetYVolt)
                            {
                                ushort mid = targetYVolt.Value > 0 ? (ushort)0x7fff : (ushort)0x8000;
                                List<ushort> targetYVolts = Enumerable.Repeat((ushort)(Math.Round(targetYVolt.Value * VOLT_TO_THORDAQ_VAL) + mid - _yOffset16bit), shiftCount).ToList();
                                _x_volt_16bit.AddRange(targetYVolts);
                            }

                            ushort pockelDigi = (int)THORDAQ_DIG_POCKELS_DIGI == ((int)_digitalLines16bit.ElementAt(startIndex) & (int)THORDAQ_DIG_POCKELS_DIGI) ? THORDAQ_DIG_POCKELS_DIGI : THORDAQ_DIG_LOW;
                            for (int i = 0; i < _pockel_16bit.Count; i++)
                            {
                                List<ushort> pockel16bits = Enumerable.Repeat(_pockel_16bit[i].ElementAt(startIndex), shiftCount).ToList();
                                _pockel_16bit[i].InsertRange(startIndex, pockel16bits);
                            }
                            ushort digi = THORDAQ_DIG_ACTIVE_ENVELOPE;  //activeEnvelope
                            digi |= THORDAQ_DIG_LOW;    //cycleComplete
                            digi |= THORDAQ_DIG_LOW;    //cycleEnvelope
                            digi |= THORDAQ_DIG_LOW;    //iterEnvelope
                            digi |= THORDAQ_DIG_LOW;    //patnEnvelope
                            digi |= THORDAQ_DIG_LOW;    //patnComplete
                            digi |= THORDAQ_DIG_LOW;    //epochEnvelope
                            digi |= pockelDigi;
                            _digitalLines16bit.InsertRange(startIndex, Enumerable.Repeat(digi, shiftCount).ToList());
                        }
                        break;
                    default:
                        break;
                }
            }
            else
            {
                //**************************************************************************************************//
                //  delay galvo start relative to pockels, padding at beginning of galvo and at the end of pockels  //
                //**************************************************************************************************//
                List<double> xInsertVolts = Enumerable.Repeat(Math.Min(MAX_GALVO_VOLTAGE, Math.Max((double)_x_volt.ElementAt(startIndex), MIN_GALVO_VOLTAGE)), shiftCount).ToList();
                List<double> yInsertVolts = Enumerable.Repeat(Math.Min(MAX_GALVO_VOLTAGE, Math.Max((double)_y_volt.ElementAt(startIndex), MIN_GALVO_VOLTAGE)), shiftCount).ToList();
                _x_volt.InsertRange(startIndex, xInsertVolts);
                _y_volt.InsertRange(startIndex, yInsertVolts);

                switch (_waveformDriverType)
                {
                    case WaveformDriverType.WaveformDriver_NI:
                        {
                            for (int i = 0; i < _pockel.Count; i++)
                            {
                                List<double> pockelValues = Enumerable.Repeat(_pockel[i].Last(), shiftCount).ToList();
                                List<byte> pockelDigValues = Enumerable.Repeat(PockelIdle[i] < _pockel[i].Last() ? (byte)1 : (byte)0, shiftCount).ToList();
                                _pockel[i].AddRange(pockelValues);
                                _pockelDig[i].AddRange(pockelDigValues);
                            }

                            _cycleComplete.AddRange(Enumerable.Repeat(final ? BYTE_HIGH : BYTE_LOW, shiftCount).ToList());
                            _cycleComplementary.AddRange(Enumerable.Repeat(final ? BYTE_LOW : BYTE_HIGH, shiftCount).ToList());
                            _cycleEnvelope.AddRange(lowBytes);
                            _iterEnvelope.AddRange(lowBytes);
                            _patnEnvelope.AddRange(lowBytes);
                            _patnComplete.AddRange(lowBytes);
                            _epochEnvelope.AddRange(lowBytes);
                            _activeEnvelope.AddRange(Enumerable.Repeat(final ? BYTE_LOW : BYTE_HIGH, shiftCount).ToList());
                        }
                        break;
                    case WaveformDriverType.WaveformDriver_ThorDAQ:
                        {
                            double? targetXVolt = _x_volt.ElementAt(startIndex);
                            if (null != targetXVolt)
                            {
                                ushort mid = targetXVolt.Value > 0 ? (ushort)0x7fff : (ushort)0x8000;
                                ushort xOffset16bitTmp = (ushort)(Math.Round(targetXVolt.Value * VOLT_TO_THORDAQ_VAL) + mid);
                                if (_x_volt_16bit.Count == 0)
                                {
                                    _x_volt_16bit.AddRange(Enumerable.Repeat((ushort)0, shiftCount).ToList());
                                    _xOffset16bit = (ushort)(Math.Round(targetXVolt.Value * VOLT_TO_THORDAQ_VAL) + mid);
                                }
                                else
                                {
                                    _x_volt_16bit.InsertRange(startIndex, Enumerable.Repeat((ushort)(Math.Round(targetXVolt.Value * VOLT_TO_THORDAQ_VAL) + mid - xOffset16bitTmp), shiftCount).ToList());
                                }
                            }
                            double? targetYVolt = _y_volt.ElementAt(startIndex);
                            if (null != targetYVolt)
                            {
                                ushort mid = targetYVolt.Value > 0 ? (ushort)0x7fff : (ushort)0x8000;
                                ushort yOffset16bitTmp = (ushort)(Math.Round(targetYVolt.Value * VOLT_TO_THORDAQ_VAL) + mid);
                                if (_y_volt_16bit.Count == 0)
                                {
                                    _y_volt_16bit.AddRange(Enumerable.Repeat((ushort)0, shiftCount).ToList());
                                    _yOffset16bit = (ushort)(Math.Round(targetYVolt.Value * VOLT_TO_THORDAQ_VAL) + mid);
                                }
                                else
                                {
                                    _y_volt_16bit.InsertRange(startIndex, Enumerable.Repeat((ushort)(Math.Round(targetYVolt.Value * VOLT_TO_THORDAQ_VAL) + mid - yOffset16bitTmp), shiftCount).ToList());
                                }
                            }

                            ushort pockelDigi = (int)THORDAQ_DIG_POCKELS_DIGI == ((int)_digitalLines16bit.Last() & (int)THORDAQ_DIG_POCKELS_DIGI) ? THORDAQ_DIG_POCKELS_DIGI : THORDAQ_DIG_LOW;
                            for (int i = 0; i < _pockel_16bit.Count; i++)
                            {
                                List<ushort> pockel16bits = Enumerable.Repeat(_pockel_16bit[i].Last(), shiftCount).ToList();
                                _pockel_16bit[i].AddRange(pockel16bits);
                            }
                            ushort digi = final ? THORDAQ_DIG_LOW : THORDAQ_DIG_ACTIVE_ENVELOPE;  //activeEnvelope
                            digi |= final ? THORDAQ_DIG_CYCLE_COMPLETE : THORDAQ_DIG_LOW;   //cycleComplete
                            digi |= THORDAQ_DIG_LOW;    //cycleEnvelope
                            digi |= THORDAQ_DIG_LOW;    //iterEnvelope
                            digi |= THORDAQ_DIG_LOW;    //patnEnvelope
                            digi |= THORDAQ_DIG_LOW;    //patnComplete
                            digi |= THORDAQ_DIG_LOW;    //epochEnvelope
                            digi |= pockelDigi;
                            _digitalLines16bit.AddRange(Enumerable.Repeat(digi, shiftCount).ToList());
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        #endregion Methods
    }
}