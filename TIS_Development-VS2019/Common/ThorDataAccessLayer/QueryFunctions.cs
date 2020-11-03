using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.Odbc;
using System.Data;

namespace ThorDataAccessLayer
{  
    public class QueryFunctions
    {
        #region Fields

        public enum AggregateFunctions
        {
            Avg = 1,
            Min,
            Max,
            StdDev,
            Median
        }

        #endregion

        #region Query functions
        
        public static string GetColumnAverage(string algorithm,
                                        string[] wells,
                                        long resultId,
                                        string column,
                                        AggregateFunctions function)
        {
            string sMessage = "Value:";

            try
            {
                long AlgorithmId = ExperimentManager.GetAlgorithmIdByAlgName(algorithm);
                if (AlgorithmId > 0)
                {
                    string AlgorithmTable = ExperimentManager.GetResultsTableName(AlgorithmId);
                    if (AlgorithmTable != String.Empty)
                    {
                        string WellIds = GetWellIds(wells);
                        if (WellIds != String.Empty)
                        {
                            string ExperimentAlgorithmWellIDs = GetExperimentAlgorithmWellIDs(WellIds, resultId);

                            if (ExperimentAlgorithmWellIDs != String.Empty)
                            {
                                string columnExpression = GetAggregateColumnExpression(function, column);
                                string strCommandText = "SELECT " + columnExpression +
                                                        "FROM `" + AlgorithmTable + "` " +
                                                        "WHERE ExperimentAlgorithmWellId IN (" +
                                                        ExperimentAlgorithmWellIDs + ")";
                                OdbcConnection MyConn = Connection.GetConnection();
                                OdbcCommand MyComm = new OdbcCommand();
                                MyComm.Connection = MyConn;
                                MyComm.CommandText = strCommandText;
                                object result = MyComm.ExecuteScalar();
                                if (result != null)
                                {
                                    sMessage = sMessage + Convert.ToString(result);
                                }
                            }
                            else
                            {
                                sMessage = "Error:No data available for given results id";
                            }
                        }
                        else
                        {
                            sMessage = "Error:No data available for given well/s";
                        }
                    }

                    else
                    {
                        sMessage = "Error:No data available for given algorithm";
                    }
                }
                else
                {
                    sMessage = "Error:Algorithm does not exist";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        public static DataSet GetTableAverages(string algorithm,
                                        string[] wells,
                                        long resultId,
                                        AggregateFunctions function)
        {
            try
            {
                long AlgorithmId = ExperimentManager.GetAlgorithmIdByAlgName(algorithm);
                string AlgorithmTable = ExperimentManager.GetResultsTableName(AlgorithmId);
                if (AlgorithmTable != String.Empty)
                {
                    string WellIds = GetWellIds(wells);
                    if (WellIds != String.Empty)
                    {
                        string ExperimentAlgorithmWellIDs = GetExperimentAlgorithmWellIDs(WellIds, resultId);
                        if (ExperimentAlgorithmWellIDs != String.Empty)
                        {
                            string strCommandText = "SELECT COLUMN_NAME FROM information_schema.`COLUMNS` C " +
                                                    "WHERE table_name = '" + AlgorithmTable + "'";
                            string AvgColumns = String.Empty;
                            OdbcConnection MyConn = Connection.GetConnection();
                            OdbcCommand MyComm = new OdbcCommand();
                            MyComm.Connection = MyConn;
                            MyComm.CommandText = strCommandText;
                            OdbcDataReader rdr = MyComm.ExecuteReader();

                            if (rdr.HasRows)
                            {
                                while (rdr.Read())
                                {
                                    if (rdr.GetString(0) != "DataId" &&
                                        rdr.GetString(0) != "ExperimentAlgorithmWellId" &&
                                        rdr.GetString(0) != "CellID" &&
                                        rdr.GetString(0) != "Well" &&
                                        !rdr.GetString(0).StartsWith("Is"))
                                    {
                                        string columnName = rdr.GetString(0);
                                        columnName = GetAggregateColumnExpression(function, columnName);
                                        AvgColumns += "," + columnName;
                                    }
                                }
                            }
                            if (AvgColumns != String.Empty) AvgColumns = AvgColumns.Remove(0, 1);
                            rdr.Close();

                            strCommandText = "SELECT " + AvgColumns +
                                                    "FROM `" + AlgorithmTable + "` " +
                                                    "WHERE ExperimentAlgorithmWellId IN (" + ExperimentAlgorithmWellIDs + ")";

                            MyComm.CommandText = strCommandText;
                            OdbcDataAdapter adp = new OdbcDataAdapter(strCommandText, MyConn);
                            DataSet dsAvg = new DataSet();

                            adp.Fill(dsAvg, "Values");

                            return dsAvg;
                        }
                        else
                        {
                            return null;
                        }
                    }
                    else
                    {
                        return null;
                    }
                }
                else
                {
                    return null;
                }
            }
            catch
            {
                return null;
            }
        }

        #endregion

        #region Private functions

        private static string GetWellIds(string[] wellNames)
        {
            string WellIds = String.Empty;
            string Wells = String.Empty;

            foreach (string w in wellNames)
            {
                string well = ",'" + w + "'";
                Wells = Wells + well;
            }
            if(Wells != String.Empty) Wells = Wells.Remove(0, 1);

            string strCommandText = "SELECT WellId FROM `wells` " +
                                    "WHERE WellName IN (" + Wells + ")";
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            MyComm.CommandText = strCommandText;
            OdbcDataReader rdr = MyComm.ExecuteReader();

            if (rdr.HasRows)
            {
                while (rdr.Read())
                {
                    WellIds += "," + rdr.GetString(0);
                }
            }
            if (WellIds != String.Empty) WellIds = WellIds.Remove(0, 1);
            rdr.Close();         

            return WellIds;
        }

        private static string GetExperimentAlgorithmWellIDs(string wellIds, long resultId)
        {
            string ExperimentAlgorithmWellIDs = String.Empty;

            string strCommandText = "SELECT ExperimentAlgorithmWellId FROM `experimentalgorithmwell` " +
                                    "WHERE WellId IN (" + wellIds + ") AND " +
                                    "ResultsId = '" + resultId + "'";
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            MyComm.CommandText = strCommandText;
            OdbcDataReader rdr = MyComm.ExecuteReader();

            if (rdr.HasRows)
            {
                while (rdr.Read())
                {
                    ExperimentAlgorithmWellIDs += "," + rdr.GetString(0);
                }
            }
            if (ExperimentAlgorithmWellIDs != String.Empty) ExperimentAlgorithmWellIDs = ExperimentAlgorithmWellIDs.Remove(0, 1);
            rdr.Close();       

            return ExperimentAlgorithmWellIDs;
        }

        private static string GetAggregateColumnExpression(AggregateFunctions function, string column)
        {
            string ColumnExpression = String.Empty;

            switch (function)
            {
                case AggregateFunctions.Avg:
                    ColumnExpression = "Avg(`" + column + "`) ";
                    break;
                case AggregateFunctions.Min:
                    ColumnExpression = "Min(`" + column + "`) ";
                    break;
                case AggregateFunctions.Max:
                    ColumnExpression = "Max(`" + column + "`) ";
                    break;
                case AggregateFunctions.StdDev:
                    ColumnExpression = "STD(`" + column + "`)";
                    break;
                case AggregateFunctions.Median:
                    ColumnExpression = "Median(`" + column + "`)";
                    break;  
            }
            return ColumnExpression;
        }

        #endregion

    }
}
