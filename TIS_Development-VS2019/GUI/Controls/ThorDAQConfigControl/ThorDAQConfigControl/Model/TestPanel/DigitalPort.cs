using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ThorDAQConfigControl.Model
{
    public class DigitalPort
    {
        public string Name;
        public List<DigitalLine> LineList;

        public DigitalPort(string name)
        {
            Name = name;
            LineList = new List<DigitalLine>();
        }
    }
}
