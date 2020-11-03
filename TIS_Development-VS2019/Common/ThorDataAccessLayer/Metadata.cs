using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ThorDataAccessLayer
{
  
    public class Metadata
    {
        public enum MetaDataType
        {
            TypeString = 1,
            TypeInteger,
            TypeDouble
        }
        internal long metadataId;
        internal int metadataType;
        internal string metadataName;

        
    }
}
