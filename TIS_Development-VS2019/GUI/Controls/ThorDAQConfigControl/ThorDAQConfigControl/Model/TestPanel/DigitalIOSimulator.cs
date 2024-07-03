using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;

namespace ThorDAQConfigControl.Model
{
    public class DigitalIOSimulator
    {
        private List<DigitalPort> diPortList;

        public DigitalIOSimulator()
        {
            diPortList = new List<DigitalPort>();
            var port = new DigitalPort("port0");
            var line = new DigitalLine(0, 7);
            line.SetAllDirection(0);
            port.LineList.Add(line);
            diPortList.Add(port);

            port = new DigitalPort("port1");
            line = new DigitalLine(0, 3);
            line.SetAllDirection(0);
            port.LineList.Add(line);
            diPortList.Add(port);
        }

        public List<string> GetPortNames()
        {
            List<string> res = new List<string>();
            foreach(var item in diPortList)
            {
                res.Add(item.Name);
            }
            return res;
        }

        public List<string> GetLineNames(int portIndex)
        {
            List<string> res = new List<string>();
            foreach (var item in diPortList[portIndex].LineList)
            {
                res.Add(diPortList[portIndex].Name+"/line"+item.StartPos+":"+item.EndPos);
            }
            return res;
        }
    }
}
