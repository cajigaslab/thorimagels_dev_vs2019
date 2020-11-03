using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ThorImageInfastructure
{
    public class Command
    {
        public string Message { get; set; }
        public Guid CommandGUID { get; set; }
        public List<string> Payload { get; set; }
    }
}
