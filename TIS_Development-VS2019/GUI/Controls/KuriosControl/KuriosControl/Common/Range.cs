namespace KuriosControl.Common
{
    public class Range<T>
    {
        #region Constructors

        public Range()
        {
        }

        public Range(T min, T max)
        {
            Min = min;
            Max = max;
        }

        #endregion Constructors

        #region Properties

        public T Max
        {
            get;
            set;
        }

        public T Min
        {
            get;
            set;
        }

        #endregion Properties
    }
}