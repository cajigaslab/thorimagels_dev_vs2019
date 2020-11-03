using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.Odbc;
using System.Data;

namespace ThorDataAccessLayer
{
    public class User
    { 
        internal long UserID;
        internal string FirstName;
        internal string LastName;
        internal string EmailID;
        internal string LoginID;
        internal string Status;
        internal string Role;
    }
}
