using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ThorDataAccessLayer
{
    public class Gate
    {
        public long GateId
        { get; set; }
        public string GateName
        { get; set; }
        public string Description
        { get; set; }
        public List<GatingParameter> GatingParameters
         { get; set; }
    }
}
