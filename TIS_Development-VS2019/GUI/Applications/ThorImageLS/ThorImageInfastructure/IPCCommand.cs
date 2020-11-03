namespace ThorImageInfastructure
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    public class IPCCommand
    {
        #region Properties

        public string CommandType
        {
            get; set;
        }

        public string Data
        {
            get; set;
        }

        public string Destination
        {
            get; set;
        }

        public string Source
        {
            get; set;
        }

        #endregion Properties
    }
}