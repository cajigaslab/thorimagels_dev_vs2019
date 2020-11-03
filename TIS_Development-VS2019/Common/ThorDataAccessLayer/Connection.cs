namespace ThorDataAccessLayer
{
    using System;
    using System.Collections.Generic;
    using System.Data.Odbc;
    using System.Linq;
    using System.Text;

    class Connection
    {
        #region Fields

        static OdbcConnection MyConn = null;
        static string myConnectionString = "DSN=ThorLabDSN;UID=root;PWD=virginia";

        #endregion Fields

        #region Constructors

        private Connection()
        {
        }

        #endregion Constructors

        #region Methods

        public static OdbcConnection GetConnection()
        {
            if (MyConn == null)
            {
                MyConn = new OdbcConnection(myConnectionString);
                MyConn.Open();
            }
            return MyConn;
        }

        #endregion Methods
    }
}