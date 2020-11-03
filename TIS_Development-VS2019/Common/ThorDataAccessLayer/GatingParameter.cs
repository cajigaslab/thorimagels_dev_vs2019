using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;


namespace ThorDataAccessLayer
{
    public class GatingParameter
    {
       
        public long ParameterId { get; set; }
        public string Statistic { get; set; }
        public RangeTypes RangeType
        { get; set; }
        public string RangeValueFrom
        { get; set; }
        public string RangeValueTo
        { get; set; }
        public long GeteId { get; set; }
       
    }
    public enum RangeTypes
    {
        EqualTo = 1,
        NotEqualTo,
        LessThan,
        GreaterThan,
        Between,
        OutSideOf
    }
}
