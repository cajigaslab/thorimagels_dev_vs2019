using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Data.Odbc;

namespace ThorDataAccessLayer
{
    public class Wavelength
    { 
        internal long waveLengthId;
        internal int index1;
        internal string name;
        internal double exposureTimeMS;
        internal long experimentId; 
    }
}
