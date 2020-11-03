using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ThorImageInfastructure
{
    public class ChangeEvent
    {
        public bool IsChanged { get; set; }
        public string ModuleName { get; set; } 
    }
}
