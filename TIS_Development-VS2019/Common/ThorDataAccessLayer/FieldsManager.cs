namespace ThorDataAccessLayer
{
    using System;
    using System.Collections.Generic;
    using System.Data;
    using System.Data.Odbc;
    using System.Linq;
    using System.Text;

    public class FieldsManager
    {
        #region Methods

        public static void DeleteField(int fieldId)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "DELETE FROM thorlabs.fields WHERE id = " + fieldId;
            MyComm.CommandText = strCommandText;
            MyComm.ExecuteNonQuery();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="FilterId"></param>
        /// <returns></returns>
        public static DataSet GetFieldsForFilter(int FilterId)
        {
            OdbcCommand MyCmd = new OdbcCommand();
            OdbcConnection MyConn = Connection.GetConnection();
            MyCmd.Connection = MyConn;
            string strCommand = "SELECT * FROM `thorlabs`.`fields` WHERE FilterId =" + FilterId;
            MyCmd.CommandText = strCommand;
            OdbcDataAdapter adp = new OdbcDataAdapter(strCommand, MyConn);
            DataSet dsFields = new DataSet();
            adp.Fill(dsFields);
            return dsFields;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="fieldType"></param>
        /// <param name="value"></param>
        /// <param name="filterId"></param>
        /// <returns></returns>
        public static int SaveFieldForFilter(int fieldType,
            string value,
            int filterId)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "INSERT INTO thorlabs.`fields` (FieldType,FieldValue,FilterId) VALUES(" + fieldType + ",'" + value + "'," + filterId + ")";
            MyComm.CommandText = strCommandText;
            MyComm.ExecuteNonQuery();

            ///Get compiled search id
            strCommandText = "SELECT Max(id) FROM `thorlabs`.`fields` WHERE FilterId =" + filterId;
            MyComm.CommandText = strCommandText;

            object objFieldId = MyComm.ExecuteScalar();
            return int.Parse(objFieldId.ToString());
        }

        public static void UpdateField(int fieldId,
            int fieldType,
            string value)
        {
            //only value should be updated
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "Update thorlabs.fields set fieldValue = '" + value + "' where Id = " + fieldId;
            MyComm.CommandText = strCommandText;
            MyComm.ExecuteNonQuery();
        }

        #endregion Methods
    }
}