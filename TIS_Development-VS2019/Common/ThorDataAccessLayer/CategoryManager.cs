namespace ThorDataAccessLayer
{
    using System;
    using System.Collections.Generic;
    using System.Data;
    using System.Data.Odbc;
    using System.Linq;
    using System.Text;

    public class CategoryManager
    {
        #region Methods

        /// <summary>
        ///     
        /// </summary>
        /// <returns></returns>
        public static DataSet GetCategories()
        {
            OdbcConnection MyConn = Connection.GetConnection();
            string strCommandText = "SELECT DISTINCT categoryName, categoryId FROM `thorlabs`.`category`;";
            OdbcDataAdapter adp = new OdbcDataAdapter(strCommandText, MyConn);
            DataSet dsCategories = new DataSet();
            adp.Fill(dsCategories);
            return dsCategories;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="catagoryName"></param>
        /// <returns></returns>
        public static int GetCategoryID(string catagoryName)
        {
            return int.MinValue;
        }

        #endregion Methods
    }
}