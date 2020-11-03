namespace ThorDataAccessLayer
{
    using System;
    using System.Collections.Generic;
    using System.Data;
    using System.Data.Odbc;
    using System.Linq;
    using System.Text;

    public class CompiledSearchManager
    {
        #region Methods

        public static void DeleteCompiledSearch(int compiledSearchId)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "DELETE FROM thorlabs.compiledsearch WHERE id = " + compiledSearchId ;
            MyComm.CommandText = strCommandText;
            MyComm.ExecuteNonQuery();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="userId"></param>
        /// <returns></returns>
        public static DataSet GetCompiledSearchesForUser(int userId)
        {
            OdbcCommand MyCmd = new OdbcCommand();
            OdbcConnection MyConn = Connection.GetConnection();
            MyCmd.Connection = MyConn;
            string strCommand = "SELECT * FROM `thorlabs`.`compiledsearch` WHERE UserId =" + userId ;
            MyCmd.CommandText = strCommand;
            OdbcDataAdapter adp = new OdbcDataAdapter(strCommand, MyConn);
            DataSet dsCompiledSearch = new DataSet();
            adp.Fill(dsCompiledSearch);
            return dsCompiledSearch;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="name"></param>
        /// <param name="userID"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        public static int SaveCompiledSearchForUser(string name,
            int userID,
            int type)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "INSERT INTO thorlabs.compiledsearch (Name,UserId,SearchType) VALUES('" + name + "'," + userID + "," + type + ")";
            MyComm.CommandText = strCommandText;
            MyComm.ExecuteNonQuery();

            ///Get compiled search id
            strCommandText = "SELECT MAX(id) FROM `thorlabs`.`compiledsearch` WHERE userid =" + userID;
            MyComm.CommandText = strCommandText;

            object objID = MyComm.ExecuteScalar();

            return int.Parse(objID.ToString());
        }

        public static void UpdateCompiledSearch( string name,
            int compiledSearchId)
        {
            //only name should be updated
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "Update thorlabs.compiledsearch set Name = '" + name + "' where Id = " + compiledSearchId;
            MyComm.CommandText = strCommandText;
            MyComm.ExecuteNonQuery();
        }

        #endregion Methods
    }
}