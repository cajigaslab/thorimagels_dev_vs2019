using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.Odbc;

namespace ThorDataAccessLayer
{
    public class Category
    {
        internal long categoryId;
        internal string categoryName;

        public long Id
        {
            get
            {
                return categoryId;
            }
        }

        public string Name
        {
            get
            {
                return categoryName;
            }
        }
    }
}
