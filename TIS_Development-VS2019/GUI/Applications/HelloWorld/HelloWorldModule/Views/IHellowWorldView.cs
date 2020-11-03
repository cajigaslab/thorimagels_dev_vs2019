using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace HelloWorldModule
{
    public interface IHellowWorldView
    {
        void ShowDialog(string content);
        string Message {  set; }
    }
}
