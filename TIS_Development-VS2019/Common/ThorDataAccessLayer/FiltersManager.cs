namespace ThorDataAccessLayer
{
    using System;
    using System.Collections.Generic;
    using System.Data;
    using System.Data.Odbc;
    using System.Linq;
    using System.Text;

    public class FiltersManager
    {
        #region Methods

        /// <summary>
        /// Delete filter from filter page
        /// </summary>
        /// <param name="filterId"></param>
        public static void DeleteFilter(int filterId)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "DELETE FROM thorlabs.filters WHERE id = " + filterId;
            MyComm.CommandText = strCommandText;
            MyComm.ExecuteNonQuery();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="compiledSearchId"></param>
        /// <returns></returns>
        public static DataSet GetFiltersForCompiledSearch(int compiledSearchId)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();
            MyCmd.Connection = MyConn;
            string strCommand = "SELECT * FROM `thorlabs`.`filters` WHERE CompiledSearch_Id =" + compiledSearchId;
            MyCmd.CommandText = strCommand;
            OdbcDataAdapter adp = new OdbcDataAdapter(strCommand, MyConn);
            DataSet dsFilters = new DataSet();
            adp.Fill(dsFilters);
            return dsFilters;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="filterType"></param>
        /// <param name="compiledSearchID"></param>
        /// <returns></returns>
        public static int SaveFilterForCompiledSearch(int compiledSearchId, int filter)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "INSERT INTO thorlabs.filters (Filter,CompiledSearch_Id) VALUES('" + filter + "'," + compiledSearchId + ")";
            MyComm.CommandText = strCommandText;
            MyComm.ExecuteNonQuery();

            ///Get compiled search id
            strCommandText = "SELECT Max(id) FROM `thorlabs`.`filters` WHERE CompiledSearch_Id =" + compiledSearchId;
            MyComm.CommandText = strCommandText;

            object objFilterId = MyComm.ExecuteScalar();
            return int.Parse(objFilterId.ToString());
        }

        public static int UpdateFilter(int filterId,
            int filterType)
        {
            //probably won't be required
            return int.MinValue;
        }

        #endregion Methods
    }
}