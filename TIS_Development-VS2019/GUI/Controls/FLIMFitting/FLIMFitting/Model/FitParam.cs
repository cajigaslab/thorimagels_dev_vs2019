namespace FLIMFitting.Model
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    public class FitParam : BindableBase
    {
        #region Fields

        public FlagDef FixedFlag = 0;
        public FlagDef SharedFlg = 0;

        private bool _isAmp1Fixed;
        private bool _isAmp1Shared;
        private bool _isAmp2Fixed;
        private bool _isAmp2Shared;
        private bool _isGaussWFixed;
        private bool _isGaussWShared;
        private bool _isScatterFixed;
        private bool _isScatterShared;
        private bool _isTua1Fixed;
        private bool _isTua1Shared;
        private bool _isTua2Fixed;
        private bool _isTua2Shared;
        private bool _isTZeroFixed;
        private bool _isTZeroShared;

        #endregion Fields

        #region Properties

        public bool IsAmp1Fixed
        {
            get { return _isAmp1Fixed; }
            set
            {
                SetProperty(ref _isAmp1Fixed, value);
                UpdateFixedFlag(FlagDef.AMP1, value);
                if (value) IsAmp1Shared = false;
            }
        }

        public bool IsAmp1Shared
        {
            get { return _isAmp1Shared; }
            set
            {
                SetProperty(ref _isAmp1Shared, value);
                UpdateShareFlag(FlagDef.AMP1, value);
                if (value) IsAmp1Fixed = false;
            }
        }

        public bool IsAmp2Fixed
        {
            get { return _isAmp2Fixed; }
            set
            {
                SetProperty(ref _isAmp2Fixed, value);
                UpdateFixedFlag(FlagDef.AMP2, value);
                if (value) IsAmp2Shared = false;
            }
        }

        public bool IsAmp2Shared
        {
            get { return _isAmp2Shared; }
            set
            {
                SetProperty(ref _isAmp2Shared, value);
                UpdateShareFlag(FlagDef.AMP2, value);
                if (value) IsAmp2Fixed = false;
            }
        }

        public bool IsGaussWFixed
        {
            get { return _isGaussWFixed; }
            set
            {
                SetProperty(ref _isGaussWFixed, value);
                UpdateFixedFlag(FlagDef.GAUSSW, value);
                if (value) IsGaussWShared = false;
            }
        }

        public bool IsGaussWShared
        {
            get { return _isGaussWShared; }
            set
            {
                SetProperty(ref _isGaussWShared, value);
                UpdateShareFlag(FlagDef.GAUSSW, value);
                if (value) IsGaussWFixed = false;
            }
        }

        public bool IsScatterFixed
        {
            get { return _isScatterFixed; }
            set
            {
                SetProperty(ref _isScatterFixed, value);
                UpdateFixedFlag(FlagDef.SCATTR, value);
                if (value) IsScatterShared = false;
            }
        }

        public bool IsScatterShared
        {
            get { return _isScatterShared; }
            set
            {
                SetProperty(ref _isScatterShared, value);
                UpdateShareFlag(FlagDef.SCATTR, value);
                if (value) IsScatterFixed = false;
            }
        }

        public bool IsTua1Fixed
        {
            get { return _isTua1Fixed; }
            set
            {
                SetProperty(ref _isTua1Fixed, value);
                UpdateFixedFlag(FlagDef.TAU1, value);
                if (value) IsTua1Shared = false;
            }
        }

        public bool IsTua1Shared
        {
            get { return _isTua1Shared; }
            set
            {
                SetProperty(ref _isTua1Shared, value);
                UpdateShareFlag(FlagDef.TAU1, value);
                if (value) IsTua1Fixed = false;
            }
        }

        public bool IsTua2Fixed
        {
            get { return _isTua2Fixed; }
            set
            {
                SetProperty(ref _isTua2Fixed, value);
                UpdateFixedFlag(FlagDef.TAU2, value);
                if (value) IsTua2Shared = false;
            }
        }

        public bool IsTua2Shared
        {
            get { return _isTua2Shared; }
            set
            {
                SetProperty(ref _isTua2Shared, value);
                UpdateShareFlag(FlagDef.TAU2, value);
                if (value) IsTua2Fixed = false;
            }
        }

        public bool IsTZeroFixed
        {
            get { return _isTZeroFixed; }
            set
            {
                SetProperty(ref _isTZeroFixed, value);
                UpdateFixedFlag(FlagDef.TZERO, value);
                if (value) IsTZeroShared = false;
            }
        }

        public bool IsTZeroShared
        {
            get { return _isTZeroShared; }
            set
            {
                SetProperty(ref _isTZeroShared, value);
                UpdateShareFlag(FlagDef.TZERO, value);
                if (value) IsTZeroFixed = false;
            }
        }

        #endregion Properties

        #region Methods

        public void Reset()
        {
            if (IsAmp1Fixed) IsAmp1Fixed = false;
            if (IsTua1Fixed) IsTua1Fixed = false;
            if (IsAmp2Fixed) IsAmp2Fixed = false;
            if (IsTua2Fixed) IsTua2Fixed = false;
            if (IsTZeroFixed) IsTZeroFixed = false;
            if (IsGaussWFixed) IsGaussWFixed = false;
            if (IsScatterFixed) IsScatterFixed = false;
            if (IsAmp1Shared) IsAmp1Shared = false;
            if (IsTua1Shared) IsTua1Shared = false;
            if (IsAmp2Shared) IsAmp2Shared = false;
            if (IsTua2Shared) IsTua2Shared = false;
            if (IsTZeroShared) IsTZeroShared = false;
            if (IsGaussWShared) IsGaussWShared = false;
            if (IsAmp1Fixed) IsScatterShared = false;
        }

        private void UpdateFixedFlag(FlagDef flag, bool v)
        {
            if (v)
            {
                FixedFlag |= flag;
            }
            else
            {
                FixedFlag &= ~flag;
            }
        }

        private void UpdateShareFlag(FlagDef flag, bool v)
        {
            if (v)
            {
                SharedFlg |= flag;
            }
            else
            {
                SharedFlg &= ~flag;
            }
        }

        #endregion Methods
    }
}