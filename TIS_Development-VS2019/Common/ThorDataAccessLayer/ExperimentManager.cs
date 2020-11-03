namespace ThorDataAccessLayer
{
    using System;
    using System.Collections.Generic;
    using System.Data;
    using System.Data.Odbc;
    using System.Linq;
    using System.Text;
    using System.Net;
    using System.IO;
    using System.Xml;

    public class ExperimentManager
    {
        string ftpUserName = String.Empty;
        string ftpPassword = String.Empty;
        string ftpLocation = String.Empty;
        FtpWebResponse response;
        XmlTextReader reader;
        
        #region Methods

        public static DataSet GetAllExperiment()
        {
            OdbcConnection MyConn = Connection.GetConnection();

            string strCommand = "SELECT * FROM `thorlabs`.`experiment`";

            OdbcDataAdapter adp = new OdbcDataAdapter(strCommand, MyConn);
            DataSet dsExperiment = new DataSet();

            adp.Fill(dsExperiment, "Experiment");

            return dsExperiment;
        }

        public static void GetExperimentDetailsForExperiment(int experimentID, ref string comments, ref double magnification, ref int sampleType,
            ref int sampleRows, ref int sampleColumns, ref int mosaicRows, ref int mosaicColumns, ref string dateTime, ref string userName, ref List<string> categories)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();
            MyCmd.Connection = MyConn;
            string strCommand = "SELECT * FROM `thorlabs`.`sample` WHERE sampleId = " + experimentID.ToString();
            OdbcDataAdapter adp = new OdbcDataAdapter(strCommand, MyConn);
            DataSet dsSample = new DataSet();

            adp.Fill(dsSample, "Sample");

            if (dsSample.Tables[0].Rows.Count > 0)
            {
                const int SAMPLE_INDEX_ROWS = 4;
                const int SAMPLE_INDEX_COLUMNS = 5;
                const int SAMPLE_INDEX_MOSAIC_ROWS = 8;
                const int SAMPLE_INDEX_MOSAIC_COLUMNS = 9;
                const int SAMPLE_INDEX_SAMPLE_INDEX_TYPE = 1;

                sampleRows = Convert.ToInt32(dsSample.Tables[0].Rows[0].ItemArray[SAMPLE_INDEX_ROWS].ToString());
                sampleColumns = Convert.ToInt32(dsSample.Tables[0].Rows[0].ItemArray[SAMPLE_INDEX_COLUMNS].ToString());
                mosaicRows = Convert.ToInt32(dsSample.Tables[0].Rows[0].ItemArray[SAMPLE_INDEX_MOSAIC_ROWS].ToString());
                mosaicColumns = Convert.ToInt32(dsSample.Tables[0].Rows[0].ItemArray[SAMPLE_INDEX_MOSAIC_COLUMNS].ToString());
                sampleType = Convert.ToInt32(dsSample.Tables[0].Rows[0].ItemArray[SAMPLE_INDEX_SAMPLE_INDEX_TYPE].ToString());

            }

            strCommand = "SELECT * FROM `thorlabs`.`experiment` WHERE experimentId = " + experimentID.ToString();
            DataSet dsExp = new DataSet();

            adp = new OdbcDataAdapter(strCommand, MyConn);

            adp.Fill(dsExp, "Experiment");

            if (dsExp.Tables[0].Rows.Count > 0)
            {
                const int SAMPLE_INDEX_DATE_TIME = 2;
                const int SAMPLE_INDEX_USER = 3;
                const int SAMPLE_INDEX_MAGNIFICATION = 6;
                const int SAMPLE_INDEX_COMMENTS = 7;

                magnification = Convert.ToDouble(dsExp.Tables[0].Rows[0].ItemArray[SAMPLE_INDEX_MAGNIFICATION].ToString());

                DateTime dt = Convert.ToDateTime(dsExp.Tables[0].Rows[0].ItemArray[SAMPLE_INDEX_DATE_TIME].ToString());
                dateTime = String.Format("{0:dddd, MMMM d, yyyy}", dt);
                userName = dsExp.Tables[0].Rows[0].ItemArray[SAMPLE_INDEX_USER].ToString();
                comments = dsExp.Tables[0].Rows[0].ItemArray[SAMPLE_INDEX_COMMENTS].ToString();
            }

            strCommand = "SELECT distinct `c`.`categoryName` FROM `category` c right outer join experimentcategory e  on (`c`.`categoryId` = `e`.`categoryId`) where experimentId =" + experimentID.ToString();;

            DataSet dsCategory = new DataSet();

            adp = new OdbcDataAdapter(strCommand, MyConn);

            adp.Fill(dsCategory, "Category");

            if (dsCategory.Tables[0].Rows.Count > 0)
            {
                const int CATEGORY_NAME = 0;

                for (int i = 0; i < dsCategory.Tables[0].Rows.Count; i++)
                {
                    categories.Add(dsCategory.Tables[0].Rows[i].ItemArray[CATEGORY_NAME].ToString());
                }
            }
            return;
        }

        /// <summary>
        /// Dynamic query to get experiments depending on search criteria
        /// </summary>
        /// <param name="userName">User name OR String.Empty</param>
        /// <param name="dateRange">Date range array of from date and to date OR Null</param>
        /// <param name="category">Category Name OR Strin.Empty</param>
        /// <param name="comment">Comment OR String.Empty</param>
        /// <param name="sampleType">Sample type OR Int.MinValue</param>
        /// <param name="userNameCount">User name matches</param>
        /// <param name="dtCount">Date range matches</param>
        /// <param name="categorycount">Category matches</param>
        /// <param name="commentcount">Comment matches</param>
        /// <param name="Samplecount">Sample matches</param>
        /// <param name="dsResult">Result dataset</param>
        public static void GetExperiments(string userName,
            DateTime[] dateRange,
            int category,
            string comment,
            int sampleType,
            out int userNameCount,
            out int dtCount,
            out int categorycount,
            out int commentcount,
            out int sampleCount,
            out DataSet dsResult)
        {
            userNameCount = 0;
            dtCount = 0;
            categorycount = 0;
            commentcount = 0;
            sampleCount = 0;
            dsResult = null;

            DataRow[] dr;
            string filter = string.Empty;

            ///Get all experiments from database
            DataSet dsExperiment = GetAllExperiment();

            ////UserName filter
            if (!userName.Equals(string.Empty))
            {
                filter = "userName = '" + userName + "'";
                dr = dsExperiment.Tables[0].Select(filter);
                userNameCount = dr.Length;
            }

            ///Date filter
            if (dateRange != null)
            {
                if (dateRange.Length == 2)
                {
                    if (filter.Length != 0)
                    {
                        filter += " AND";
                    }
                    filter += " date >= #" + dateRange[0].Month + "/" + dateRange[0].Day + "/" + dateRange[0].Year +
                                        "# AND date <= #" + dateRange[1].Month + "/" + dateRange[1].Day + "/" + dateRange[1].Year + "#";
                    dr = dsExperiment.Tables[0].Select(filter);
                    dtCount = dr.Length;
                }
            }

            ///Category filter
            if (!category.Equals(int.MinValue))
            {
                OdbcDataReader drExperiments = GetExperimentsForCategory(category);
                string CategoyFilter = "(";
                while (drExperiments.Read())
                {
                    CategoyFilter += "," + drExperiments["experimentId"].ToString();
                }
                CategoyFilter += ")";
                CategoyFilter = CategoyFilter.Replace("(,", "(");
                if (CategoyFilter.Length > 2)
                {
                    if (filter.Length != 0)
                    {
                        filter += " AND";
                    }
                    filter += " experimentId in " + CategoyFilter;
                    dr = dsExperiment.Tables[0].Select(filter);
                    categorycount = dr.Length;
                }
            }

            ///Comment filter
            if (!comment.Equals("#@987"))
            {
                if (!comment.Equals(string.Empty))
                {
                    if (filter.Length != 0)
                    {
                        filter += " AND";
                    }
                    filter += " comments like '" + comment + "%'";
                    dr = dsExperiment.Tables[0].Select(filter);
                    commentcount = dr.Length;
                }
                else
                {
                    if (filter.Length != 0)
                    {
                        filter += " AND";
                    }
                    filter += " comments like '#@987%'";
                    dr = dsExperiment.Tables[0].Select(filter);
                    commentcount = dr.Length;
                }
            }

            ///Sample type filter
            if (sampleType != int.MinValue)
            {
                OdbcDataReader drSample = GetSampleData(sampleType);
                string SampleFilter = "(";
                while (drSample.Read())
                {
                    SampleFilter += "," + drSample["SampleId"].ToString();
                }
                SampleFilter += ")";
                SampleFilter = SampleFilter.Replace("(,", "(");
                if (SampleFilter.Length > 2)
                {
                    if (filter.Length != 0)
                    {
                        filter += " AND";
                    }
                    filter += " sampleid in " + SampleFilter;
                    dr = dsExperiment.Tables[0].Select(filter);
                    sampleCount = dr.Length;
                }
                else
                {
                    if (filter.Length != 0)
                    {
                        filter += " AND ";
                    }
                    filter += "sampleid in (0)";
                    dr = dsExperiment.Tables[0].Select(filter);
                    sampleCount = dr.Length;
                }
            }

            dsExperiment.Tables[0].DefaultView.RowFilter = filter;
            dsResult = dsExperiment;
        }

        public static OdbcDataReader GetExperimentsForCategory(int category)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();
            MyCmd.Connection = MyConn;
            string strCommand = "SELECT * FROM `thorlabs`.`experimentcategory` ec, thorlabs.category c WHERE ec.categoryId = c.categoryId AND ec.categoryId = '" + category + "'";
            MyCmd.CommandText = strCommand;
            OdbcDataReader result = MyCmd.ExecuteReader(CommandBehavior.CloseConnection);

            return result;
        }

        public static DataSet GetMinMaxDates()
        {
            OdbcConnection MyConn = Connection.GetConnection();

            string strCommand = "SELECT min(date) as minDate, max(date) as maxDate FROM `thorlabs`.`experiment`;";

            OdbcDataAdapter adp = new OdbcDataAdapter(strCommand, MyConn);
            DataSet dsDates = new DataSet();
            adp.Fill(dsDates, "Dates");

            return dsDates;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="categoryName"></param>
        /// <returns></returns>
        public static int GetOnlyCategoryMatches(int categoryId)
        {
            ///get category id
            //int categoryId = CategoryManager.GetCategoryID(categoryName);

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();
            MyCmd.Connection = MyConn;
            //string strCommand = "SELECT * FROM `thorlabs`.`experiment` where userName = '" + categoryName + "'";
            string strCommand = @"SELECT COUNT(experimentid) FROM `thorlabs`.`experiment`
                                where experimentId in
                                (SELECT experimentId from `thorlabs`.`experimentcategory`
                                where categoryId  = '" + categoryId + "')";
            MyCmd.CommandText = strCommand;
            Object objCategoryMatches = MyCmd.ExecuteScalar();

            return int.Parse(objCategoryMatches.ToString());
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="comment"></param>
        /// <returns></returns>
        public static int GetOnlyCommentsMatches(string comment)
        {
            int commentMatchCount = 0;
            if (comment.Length > 0)
            {

                OdbcCommand MyCmd = new OdbcCommand();
                OdbcConnection MyConn = Connection.GetConnection();
                MyCmd.Connection = MyConn;
                string strCommand = "SELECT COUNT(experimentId) FROM `experiment` WHERE comments like '" + comment + "%'";
                MyCmd.CommandText = strCommand;
                Object objCommentsCount = MyCmd.ExecuteScalar();

                commentMatchCount = int.Parse(objCommentsCount.ToString());

            }
            return commentMatchCount;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="fromDate"></param>
        /// <param name="toDate"></param>
        /// <returns></returns>
        public static int GetOnlyDateRangeMatches(DateTime fromDate,
            DateTime toDate)
        {
            string strFromDate = fromDate.Year + "-" + fromDate.Month + "-" + fromDate.Day;
            string strToDate = toDate.Year + "-" + toDate.Month + "-" + toDate.Day;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();

            MyCmd.Connection = MyConn;
            string strCommand = "SELECT COUNT(experimentid) FROM `experiment` e WHERE e.date >= '" + strFromDate + "' AND e.date <= '" + strToDate + "'";
            MyCmd.CommandText = strCommand;
            Object objCommentsCount = MyCmd.ExecuteScalar();

            return int.Parse(objCommentsCount.ToString());
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sampleType"></param>
        /// <returns></returns>
        public static int GetOnlySampleTypeMatches(int sampleType)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();

            MyCmd.Connection = MyConn;
            string strCommand = "SELECT COUNT(experimentId) FROM `experiment` e, sample s WHERE e.sampleId = s.sampleId AND s.type = " + sampleType;
            MyCmd.CommandText = strCommand;
            Object objSampleCount = MyCmd.ExecuteScalar();

            return int.Parse(objSampleCount.ToString());
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="userName"></param>
        /// <returns></returns>
        public static int GetOnlyUserNameMatches(string userName)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();

            MyCmd.Connection = MyConn;
            string strCommand = "SELECT COUNT(experimentid) FROM `thorlabs`.`experiment` where userName = '" + userName + "'";
            MyCmd.CommandText = strCommand;
            Object objUserNamecount = MyCmd.ExecuteScalar();

            return int.Parse(objUserNamecount.ToString());
        }

        public static OdbcDataReader GetSampleData(int SampleType)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();

            MyCmd.Connection = MyConn;
            string strCommand = "SELECT SampleId FROM `thorlabs`.`sample` WHERE type = " + SampleType;
            MyCmd.CommandText = strCommand;
            OdbcDataReader result = MyCmd.ExecuteReader(CommandBehavior.CloseConnection);

            return result;
        }

        /// <summary>
        ///     
        /// </summary>
        /// <returns></returns>
        public static DataSet GetUserNames()
        {
            OdbcConnection MyConn = Connection.GetConnection();
            string strCommandText = "SELECT DISTINCT userName FROM `thorlabs`.`experiment`";
            OdbcDataAdapter adp = new OdbcDataAdapter(strCommandText, MyConn);
            DataSet dsUsername = new DataSet();
            adp.Fill(dsUsername);
            return dsUsername;
        }

        public static void GetWavelengthsForExperiment(int experimentID, ref List<string> wlName, ref List<string> wlExp)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();
            MyCmd.Connection = MyConn;
            string strCommand = "SELECT * FROM `thorlabs`.`wavelength` WHERE experimentId = " + experimentID.ToString();
            OdbcDataAdapter adp = new OdbcDataAdapter(strCommand, MyConn);
            DataSet dsWavelengths = new DataSet();

            adp.Fill(dsWavelengths, "Wavelength");

            for (int i = 0; i < dsWavelengths.Tables[0].Rows.Count; i++)
            {
                wlName.Add(dsWavelengths.Tables[0].Rows[i].ItemArray[2].ToString());
                wlExp.Add(dsWavelengths.Tables[0].Rows[i].ItemArray[3].ToString());
            }

            return;
        }

        public static void GetPathForExperiment(int experimentID, ref string expPath)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();
            MyCmd.Connection = MyConn;
            string strCommand = "SELECT * FROM `thorlabs`.`experiment` WHERE experimentId = " + experimentID.ToString();
            OdbcDataAdapter adp = new OdbcDataAdapter(strCommand, MyConn);
            DataSet dsExperiment = new DataSet();

            adp.Fill(dsExperiment, "Experiment");

            if (dsExperiment.Tables[0].Rows.Count > 0)
            {
                const int EXPIMENT_PATH_COLUMN = 15;
                expPath = dsExperiment.Tables[0].Rows[0].ItemArray[EXPIMENT_PATH_COLUMN].ToString();
            }
        }


        public static void GetNameOfExperiment(int experimentID, ref string expName)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();
            MyCmd.Connection = MyConn;
            string strCommand = "SELECT * FROM `thorlabs`.`experiment` WHERE experimentId = " + experimentID.ToString();
            OdbcDataAdapter adp = new OdbcDataAdapter(strCommand, MyConn);
            DataSet dsExperiment = new DataSet();

            adp.Fill(dsExperiment, "Experiment");

            if (dsExperiment.Tables[0].Rows.Count > 0)
            {
                const int EXPIMENT_NAME_COLUMN = 1;
                expName = dsExperiment.Tables[0].Rows[0].ItemArray[EXPIMENT_NAME_COLUMN].ToString();
            }
        }

        public static long GetExperimentAlgorithmWellId(long experimentId, long algorithmId, long wellId)
        {
            long lExperimentAlgorithmWellId = 0;

            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();

            Cmd.Connection = Conn;
            string strCommand = "SELECT ExperimentAlgorithmWellId " +
                                "FROM `ExperimentAlgorithmWell` " +
                                "WHERE AlgorithmId = '" + algorithmId + "' AND " +
                                "ExperimentId = '" + experimentId + "' AND " +
                                "WellId = '" + wellId + "'";
            Cmd.CommandText = strCommand;
            object Id = Cmd.ExecuteScalar();
            if (Id != null)
            {
                lExperimentAlgorithmWellId = Convert.ToInt64(Id);
            }

            return lExperimentAlgorithmWellId;
        }       

        public static string GetResultsLocation(long lExperimentAlgorithmWellId)
        {
            string sResultsLocation = String.Empty;

            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();

            Cmd.Connection = Conn;
            string strCommand = "SELECT ResultsLocation " +
                                "FROM `ExperimentAlgorithmResults` " +
                                "WHERE ResultsId = (SELECT ResultsId FROM `ExperimentAlgorithmWell` " +
                                "WHERE ExperimentAlgorithmWellId = '" + lExperimentAlgorithmWellId + "')";
            Cmd.CommandText = strCommand;
            object location = Cmd.ExecuteScalar();
            if (location != DBNull.Value)
            {
                sResultsLocation = Convert.ToString(location);
            }

            strCommand = "SELECT WellName " +
                         "FROM `Wells` " +
                         "WHERE WellId = (SELECT WellId FROM `ExperimentAlgorithmWell` " +
                         "WHERE ExperimentAlgorithmWellId = '" + lExperimentAlgorithmWellId + "')";
            Cmd.CommandText = strCommand;
            object well = Cmd.ExecuteScalar();
            if (well != null)
            {
                if (well != DBNull.Value)
                {
                    sResultsLocation = sResultsLocation + "/" + Convert.ToString(well);
                }
            }

            return sResultsLocation;
        }

        public static long getResultsId(long experimentId, long algorithmId, string resultsLocation)
        {
            long lResId = 0;

            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();

            Cmd.Connection = Conn;
            string strCommand = "SELECT ResultsId FROM thorlabs.experimentalgorithmresults WHERE " +
                                "AlgorithmId = '" + algorithmId + 
                                "' AND ExperimentId = '" + experimentId + 
                                "' AND ResultsLocation = '" + resultsLocation + "'";
            Cmd.CommandText = strCommand;
            object res = Cmd.ExecuteScalar();
            if (res != null)
            {
                lResId = Convert.ToInt64(res);
            }

            return lResId;
        }

        public static string GetResultsIdsForExperimentAlgorithm(long experimentId, long algorithmId)
        {
            string sResultsIds = String.Empty;

            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();

            Cmd.Connection = Conn;
            string strCommand = "SELECT ResultsId FROM thorlabs.experimentalgorithmresults WHERE " +
                                "AlgorithmId = '" + algorithmId + "' AND ExperimentId = '" + experimentId + "'";
            Cmd.CommandText = strCommand;
            OdbcDataReader rdr = Cmd.ExecuteReader();

            if (rdr.HasRows)
            {
                while (rdr.Read())
                {
                    sResultsIds += "," + rdr.GetString(0);
                }
            }
            if (sResultsIds != String.Empty) sResultsIds = sResultsIds.Remove(0, 1);
            rdr.Close();

            return sResultsIds;
        }

        public static DataSet GetResultsTableForExperiment(long experimentId)
        {
            string sResultsIds = String.Empty;

            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();

            Cmd.Connection = Conn;
            string strCommand = "SELECT * FROM thorlabs.experimentalgorithmresults WHERE " +
                                "ExperimentId = '" + experimentId + "'";


            OdbcDataAdapter adp = new OdbcDataAdapter(strCommand, Conn);
            DataSet dsResults = new DataSet();

            adp.Fill(dsResults, "experimentalgorithmresults");

            return dsResults;
        }

        public static string GetAlgorithmNameForAlgorithmId(long algorithmId)
        {
            string sAlgorithmName = String.Empty;

            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();

            Cmd.Connection = Conn;
            string strCommand = "SELECT AlgorithmName FROM `algorithms` " +
                                "WHERE AlgorithmId = '" + algorithmId + "'";
            Cmd.CommandText = strCommand;
            OdbcDataReader rdr = Cmd.ExecuteReader();

            if (rdr.HasRows)
            {
                while (rdr.Read())
                {
                    sAlgorithmName += "," + rdr.GetString(0);
                }
            }
            if (sAlgorithmName != String.Empty) sAlgorithmName = sAlgorithmName.Remove(0, 1);
            rdr.Close();

            return sAlgorithmName;
        }

        public static string GetAlgorithmResultsTableNameForAlgorithmId(long algorithmId)
        {
            string sAlgorithmResultsTableName = String.Empty;

            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();

            Cmd.Connection = Conn;
            string strCommand = "SELECT ResultsTable FROM `algorithms` " +
                                "WHERE AlgorithmId = '" + algorithmId + "'";
            Cmd.CommandText = strCommand;
            OdbcDataReader rdr = Cmd.ExecuteReader();

            if (rdr.HasRows)
            {
                while (rdr.Read())
                {
                    sAlgorithmResultsTableName += "," + rdr.GetString(0);
                }
            }
            if (sAlgorithmResultsTableName != String.Empty) sAlgorithmResultsTableName = sAlgorithmResultsTableName.Remove(0, 1);
            rdr.Close();

            return sAlgorithmResultsTableName;
        }

        public static List<string> GetColumnNamesForAlgorithmId(long algorithmId)
        {
            string algorithmName = GetAlgorithmResultsTableNameForAlgorithmId(algorithmId);
            List<string> columnNames = new List<string>();
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SHOW COLUMNS FROM thorlabs." + algorithmName;
            MyComm.CommandText = strCommandText;
            OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
            DataSet ds = new DataSet();
            adapter.Fill(ds);

            if (ds != null)
            {
                if (ds.Tables.Count > 0)
                {
                    if (ds.Tables[0].Rows.Count > 0)
                    {
                        for (int i = 0; i < ds.Tables[0].Rows.Count; i++)
                        {
                            if (ds.Tables[0].Rows[i].ItemArray[0] != DBNull.Value)
                            {
                                columnNames.Add(ds.Tables[0].Rows[i].ItemArray[0].ToString());
                            }
                        }
                    }
                }
            }

            return columnNames;
        }

        public static DataSet GetAverageResultsTableForExperimentAlgorithmResults(long algorithmId, long resultsId)
        {
            string sResultsIds = String.Empty;

            string algorithmName = GetAlgorithmResultsTableNameForAlgorithmId(algorithmId);

            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();

            Cmd.Connection = Conn;
            string strCommand = "SELECT * FROM thorlabs.experimentalgorithmwell WHERE " +
                                "ResultsId = '" + resultsId + "'";

            Cmd.CommandText = strCommand;
            OdbcDataReader rdr = Cmd.ExecuteReader();

            DataSet results = new DataSet();

            //building the query from the algorithm table with all the column names
            OdbcCommand Cmd1 = new OdbcCommand();
            Cmd1.Connection = Conn;

            string strCommand1 = "select column_name from information_schema.columns where table_name='" + algorithmName + "'";
            Cmd1.CommandText = strCommand1;
            OdbcDataReader columnRdr = Cmd1.ExecuteReader();

            StringBuilder sb = new StringBuilder();

            if (columnRdr.HasRows)
            {
                while (columnRdr.Read())
                {
                    //special case for columnname "Well" which is used in the groupby cause in the query
                    if (columnRdr.GetString(0) != "Well")
                    {
                        //example: Avg(`Y-CentroidNm`) as 'Y-CentroidNm',
                        sb.Append("Avg(`");
                        sb.Append(columnRdr.GetString(0));
                        sb.Append("`) as '");
                        sb.Append(columnRdr.GetString(0));
                        sb.Append("',");
                    }
                    else
                    {
                        sb.Append(columnRdr.GetString(0));
                        sb.Append(",");
                    }                    
                }
            }

            sb.Remove(sb.Length-1, 1); //remove the last columns comma separator

            columnRdr.Close();

            if (rdr.HasRows)
            {
                while (rdr.Read())
                {
                    //strCommand = "SELECT Avg(DataId) as DataId, Avg(ExperimentAlgorithmWellId) as ExperimentAlgorithmWellId, Avg(CellID) as CellID, Well," + 
                    //             " Avg(AreaNm) as AreaNm, Avg(MeanRadiusNm) as MeanRadiusNm, Avg(PerimeterNm) as PerimeterNm, Avg(RoundnessNm) as RoundnessNm," + 
                    //             " IsBoundaryNm, avg(`X-CentroidNm`) as 'X-CentroidNm', Avg(`Y-CentroidNm`) as 'Y-CentroidNm', Avg(MajorAxisNm)as MajorAxisNm," + 
                    //             " Avg(MinorAxisNm)as MinorAxisNm, Avg(ElongationNm)as ElongationNm, Avg(TIINiNm)as TIINiNm, Avg(APINiNm)as APINiNm," + 
                    //             " Avg(SPINiNm)as SPINiNm, Avg(SkewNiNm)as SkewNiNm, Avg(KurtosisNiNm)as KurtosisNiNm, Avg(`Ratio:(TIINiNm)/(SPINiNm)`)as 'Ratio:(TIINiNm)/(SPINiNm)'," + 
                    //             " Avg(AreaWm)as AreaWm FROM thorlabs." + algorithmName + " WHERE ExperimentAlgorithmWellId = '" + rdr.GetString(0) + "' group by Well ";

                    strCommand = "SELECT "+sb.ToString()+" FROM thorlabs." + algorithmName + " WHERE ExperimentAlgorithmWellId = '" + rdr.GetString(0) + "' group by Well ";                    


                    OdbcDataAdapter adapter = new OdbcDataAdapter(strCommand, Conn);
                    DataSet dsCurrent = new DataSet();
                    adapter.Fill(dsCurrent);

                    if (dsCurrent != null)
                    {
                        results.Merge(dsCurrent);
                    }
                }
            }

            rdr.Close();

            return results;
        }

        public static DataSet GetResultsTableForExperimentAlgorithmResults(long algorithmId, long resultsId)
        {
            string sResultsIds = String.Empty;

            string algorithmName = GetAlgorithmResultsTableNameForAlgorithmId(algorithmId);

            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();

            Cmd.Connection = Conn;
            string strCommand = "SELECT * FROM thorlabs.experimentalgorithmwell WHERE " +
                                "ResultsId = '" + resultsId + "'";

            Cmd.CommandText = strCommand;
            OdbcDataReader rdr = Cmd.ExecuteReader();

            DataSet results = new DataSet();

            if (rdr.HasRows)
            {
                while (rdr.Read())
                {
                    strCommand = "SELECT * FROM thorlabs." + algorithmName + " WHERE " +
                                "ExperimentAlgorithmWellId = '" + rdr.GetString(0) + "'";

                    OdbcDataAdapter adapter = new OdbcDataAdapter(strCommand, Conn);
                    DataSet dsCurrent = new DataSet();
                    adapter.Fill(dsCurrent);

                    if (dsCurrent != null)
                    {
                        results.Merge(dsCurrent);
                    }
                }
            }
            
            rdr.Close();

            return results;
        }

        public static string GetMaskNamesForAlgorithm(long algorithmId)
        {
            string sMaskNames = String.Empty;

            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();

            Cmd.Connection = Conn;
            string strCommand = "SELECT MaskName FROM `algorithmmasks` " +
                                "WHERE AlgorithmId = '" + algorithmId + "'";
            Cmd.CommandText = strCommand;
            OdbcDataReader rdr = Cmd.ExecuteReader();

            if (rdr.HasRows)
            {
                while (rdr.Read())
                {
                    sMaskNames += "," + rdr.GetString(0);
                }
            }
            if (sMaskNames != String.Empty) sMaskNames = sMaskNames.Remove(0, 1);
            rdr.Close();

            return sMaskNames;
        }

        public static void GetImagesPerMask(long experimentAlgorithmWellId,ref int subRows,ref int subCols)
        {
            int experimentId = 0;

            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();

            Cmd.Connection = Conn;
            string strCommand = "SELECT ExperimentId FROM `experimentalgorithmresults` " +
                                "WHERE ResultsId = (SELECT ResultsId FROM `experimentalgorithmwell` " +
                                "WHERE ExperimentAlgorithmWellId = '" + experimentAlgorithmWellId + "')";
            Cmd.CommandText = strCommand;

            object ipm = Cmd.ExecuteScalar();
            if (ipm != DBNull.Value)
            {
                experimentId = Convert.ToInt32(ipm);
            }

            long sampleId = 0;

            string strCommandText = "SELECT sampleId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";

            Cmd.CommandText = strCommand;

            ipm = Cmd.ExecuteScalar();
            if (ipm != DBNull.Value)
            {
                sampleId = Convert.ToInt32(ipm);
            }

            strCommandText = "SELECT subRows,subColumns FROM thorlabs.sample WHERE sampleId = '" + sampleId + "'";

            Cmd.CommandText = strCommand;

            ipm = Cmd.ExecuteScalar();
            if (ipm != DBNull.Value)
            {
                sampleId = Convert.ToInt32(ipm);
            }

            Cmd.CommandText = strCommandText;
            OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, Conn);
            DataSet ds = new DataSet();
            adapter.Fill(ds);

            int sr = 1;
            int sc = 1;
            string strSubRows = String.Empty, strSubColumns = String.Empty;

            if (ds != null)
            {
                if (ds.Tables.Count > 0)
                {
                    if (ds.Tables[0].Rows.Count > 0)
                    {
                        if (ds.Tables[0].Rows[0].ItemArray[0] != DBNull.Value) strSubRows = ds.Tables[0].Rows[0].ItemArray[0].ToString();
                        if (ds.Tables[0].Rows[0].ItemArray[1] != DBNull.Value) strSubColumns = ds.Tables[0].Rows[0].ItemArray[1].ToString();
                        sr = Convert.ToInt32(strSubRows);
                        sc = Convert.ToInt32(strSubColumns);
                    }
                }
            }

            subRows = sr;
            subCols = sc;
        }

        public static string GetResultsTableName(long algorithmId)
        {
            string tableName = String.Empty;

            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();

            Cmd.Connection = Conn;
            string strCommand = "SELECT ResultsTable " +
                                "FROM `algorithms` " +
                                "WHERE AlgorithmId = '" + algorithmId + "'";
            Cmd.CommandText = strCommand;
            object table = Cmd.ExecuteScalar();
            if (table != DBNull.Value)
            {
                tableName = Convert.ToString(table);
            }

            return tableName;
        }

        public static DataSet GetWellData(long experimentAlgorithmWellId, string resultsTable)
        {
            OdbcConnection Conn = Connection.GetConnection();
            OdbcCommand Cmd = new OdbcCommand();
            DataSet dsWell = new DataSet();

            Cmd.Connection = Conn;
            string strCommand = "SELECT * FROM `" + resultsTable + "` " +
                                "WHERE ExperimentAlgorithmWellId = '" + experimentAlgorithmWellId + "'";
            Cmd.CommandText = strCommand;
            OdbcDataAdapter adapter = new OdbcDataAdapter(strCommand, Conn);
            adapter.Fill(dsWell);

            return dsWell;
        }


        public static string GetXCentroidColumn(string maskName, long AlgorithmId)
        {
            string xCentroid = String.Empty;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT XCentroidColumn FROM thorlabs.algorithmmasks WHERE " +
                                    "AlgorithmId = '" + AlgorithmId + "' AND " +
                                    "MaskName = '" + maskName + "'";
            MyComm.CommandText = strCommandText;
            object centroid = MyComm.ExecuteScalar();
            if (centroid != null)
            {
                if (centroid != DBNull.Value)
                {
                    xCentroid = Convert.ToString(centroid);
                }
            }

            return xCentroid;
        }
        
        public static void GetFTPDetails(ref string ftpUserName, ref string ftpPassword, ref string ftpLocation)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT userName,password,location FROM thorlabs.imageftplocation";
            MyComm.CommandText = strCommandText;
            OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
            DataSet ds = new DataSet();
            adapter.Fill(ds);

            if (ds != null)
            {
                if (ds.Tables.Count > 0)
                {
                    if (ds.Tables[0].Rows.Count > 0)
                    {
                        if (ds.Tables[0].Rows[0].ItemArray[0] != DBNull.Value) ftpUserName = ds.Tables[0].Rows[0].ItemArray[0].ToString();
                        if (ds.Tables[0].Rows[0].ItemArray[1] != DBNull.Value) ftpPassword = ds.Tables[0].Rows[0].ItemArray[1].ToString();
                        if (ds.Tables[0].Rows[0].ItemArray[2] != DBNull.Value) ftpLocation = ds.Tables[0].Rows[0].ItemArray[2].ToString();
                    }
                }
            }
        }

        /**
         * This private method is used to get and set ftp username and password details from DB
         * @param none
         * @return none
         */
        private void GetAndSetFtpDetails()
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT userName,password,location FROM thorlabs.imageftplocation";
            MyComm.CommandText = strCommandText;
            OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
            DataSet ds = new DataSet();
            adapter.Fill(ds);

            if (ds != null)
            {
                if (ds.Tables.Count > 0)
                {
                    if (ds.Tables[0].Rows.Count > 0)
                    {
                        if (ds.Tables[0].Rows[0].ItemArray[0] != DBNull.Value) ftpUserName = ds.Tables[0].Rows[0].ItemArray[0].ToString();
                        if (ds.Tables[0].Rows[0].ItemArray[1] != DBNull.Value) ftpPassword = ds.Tables[0].Rows[0].ItemArray[1].ToString();
                        if (ds.Tables[0].Rows[0].ItemArray[2] != DBNull.Value) ftpLocation = ds.Tables[0].Rows[0].ItemArray[2].ToString();
                    }
                }
            }
        }

        
        /**
	     * This private method is used to get xml reader for ftp.
	     * @param string. xml path.
	     * @return none
	     */
        private void GetFTPXMLReader(string xmlPath)
        {
            FtpWebRequest request = (FtpWebRequest)WebRequest.Create("ftp://" + xmlPath);
            request.Method = WebRequestMethods.Ftp.DownloadFile;

            request.Credentials = new NetworkCredential(ftpUserName, ftpPassword);

            response = (FtpWebResponse)request.GetResponse();

            Stream responseStream = response.GetResponseStream();
            reader = new XmlTextReader(responseStream);
        }

        /**
	     * This method is use to add the experiment by reading from xml.
	     * @param xmlPath. The FTP path where the xml file is stored.
	     * @return {status}
	     */
        public string addExperimentInformationFromXML(string xmlPath)
        {
            string sMessage = "Value:";
            string imageWavelength = string.Empty, imageName = string.Empty;
            int imageWells = 0, imageSubImages = 0;
            double imageZStage = 0, imageTimelapse = 0;

            try
            {
                //Get and set FTP user name and password
                GetAndSetFtpDetails();

                //Get XML reader for FTP
                string sPhysicalPath = ftpLocation + xmlPath;
                GetFTPXMLReader(sPhysicalPath);

                Experiment expcols = new Experiment();
                Camera camcols = new Camera();
                List<Wavelength> wavelengths = new List<Wavelength>();
                Sample samplecols = new Sample();
                List<Category> categories = new List<Category>();
                long experimentID = 0;
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText;

                string[] delim = new string[2] { @"\", "/" };
                string[] substrings = sPhysicalPath.Split(delim, StringSplitOptions.None);
                for (int i = 0; i < substrings.Length - 1; i++)
                {
                    expcols.ftpPath = expcols.ftpPath + substrings[i] + "/";
                }

                //Add experiment
                while (reader.Read())
                {
                    if (reader.NodeType == XmlNodeType.Element)
                    {
                        switch (reader.Name)
                        {
                            case "Name":
                                expcols.name = reader.GetAttribute(0);
                                strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE name = '" + expcols.name + "'";
                                MyComm.CommandText = strCommandText;
                                object exp = MyComm.ExecuteScalar();
                                if (exp != null)
                                {
                                    sMessage = "Error:Experiment name already in use";
                                    if (reader != null) reader.Close();
                                    if (response != null) response.Close();
                                    return sMessage;
                                }
                                break;

                            case "Date":
                                DateTime dt = Convert.ToDateTime(reader.GetAttribute(0));
                                string y = dt.Year.ToString(); string m = dt.Month.ToString(); string d = dt.Day.ToString();
                                string sdt = y + "-" + m + "-" + d;
                                expcols.date = sdt;
                                break;

                            case "User":
                                expcols.userName = reader.GetAttribute(0);
                                break;

                            case "Computer":
                                expcols.computerName = reader.GetAttribute(0);
                                break;

                            case "Software":
                                expcols.softwareVersion = Convert.ToDouble(reader.GetAttribute(0));
                                break;

                            case "Camera":
                                camcols.name = reader.GetAttribute(0);
                                camcols.width = Convert.ToInt32(reader.GetAttribute(1));
                                camcols.height = Convert.ToInt32(reader.GetAttribute(2));
                                camcols.pixelSizeUM = Convert.ToDouble(reader.GetAttribute(3));
                                camcols.binning = Convert.ToInt32(reader.GetAttribute(4));
                                camcols.gain = Convert.ToInt32(reader.GetAttribute(5));
                                camcols.lightmode = Convert.ToInt32(reader.GetAttribute(6));

                                strCommandText = "INSERT INTO thorlabs.camera (name,width,height,pixelSizeUM,binning,gain,lightmode) VALUES ('" + camcols.name + "','" + camcols.width + "','" + camcols.height + "','" + camcols.pixelSizeUM + "','" + camcols.binning + "','" + camcols.gain + "','" + camcols.lightmode + "')";
                                MyComm.CommandText = strCommandText;
                                MyComm.ExecuteNonQuery();

                                strCommandText = "SELECT MAX(cameraId) FROM thorlabs.camera";
                                MyComm.CommandText = strCommandText;
                                object camera = MyComm.ExecuteScalar();
                                expcols.cameraId = Convert.ToInt32(camera);
                                break;

                            case "Magnification":
                                expcols.magnificationMag = Convert.ToDouble(reader.GetAttribute(0));
                                break;

                            case "Wavelengths":
                                break;
                            case "Wavelength":
                                Wavelength wavelength = new Wavelength();
                                wavelength.name = reader.GetAttribute(0);
                                wavelength.exposureTimeMS = Convert.ToDouble(reader.GetAttribute(1));
                                wavelengths.Add(wavelength);

                                imageWavelength = wavelength.name;
                                break;

                            case "ZStage":
                                expcols.zStagename = reader.GetAttribute(0);
                                expcols.zStageSteps = Convert.ToDouble(reader.GetAttribute(1));
                                expcols.zStageStepSize = Convert.ToDouble(reader.GetAttribute(2));

                                imageZStage = expcols.zStageSteps;

                                break;

                            case "Timelapse":
                                expcols.timepoints = Convert.ToDouble(reader.GetAttribute(0));
                                expcols.intervalSec = Convert.ToDouble(reader.GetAttribute(1));

                                imageTimelapse = expcols.timepoints;

                                break;

                            case "Sample":
                                samplecols.type = Convert.ToInt32(reader.GetAttribute(0));
                                samplecols.offsetXMM = Convert.ToDouble(reader.GetAttribute(1));
                                samplecols.offsetYMM = Convert.ToDouble(reader.GetAttribute(2));

                                strCommandText = "INSERT INTO thorlabs.sample (type,offsetXMM,offsetYMM)" +
                                                 "VALUES ('" + samplecols.type + "','" + samplecols.offsetXMM + "','" + samplecols.offsetYMM + "')";
                                MyComm.CommandText = strCommandText;
                                MyComm.ExecuteNonQuery();

                                strCommandText = "SELECT MAX(sampleId) FROM thorlabs.sample";
                                MyComm.CommandText = strCommandText;
                                object sample = MyComm.ExecuteScalar();
                                expcols.sampleId = Convert.ToInt32(sample);
                                break;
                            case "Wells":
                                samplecols.startRow = Convert.ToInt32(reader.GetAttribute(0));
                                samplecols.startColumn = Convert.ToInt32(reader.GetAttribute(1));
                                samplecols.rows = Convert.ToInt32(reader.GetAttribute(2));
                                samplecols.columns = Convert.ToInt32(reader.GetAttribute(3));
                                samplecols.wellOffsetXMM = Convert.ToDouble(reader.GetAttribute(4));
                                samplecols.wellOffsetYMM = Convert.ToDouble(reader.GetAttribute(5));
                                strCommandText = "UPDATE thorlabs.sample SET " +
                                                 "startRow='" + samplecols.startRow + "'," +
                                                 "startColumn='" + samplecols.startColumn + "'," +
                                                 "rows='" + samplecols.rows + "'," +
                                                 "columns='" + samplecols.columns + "'," +
                                                 "wellOffsetXMM='" + samplecols.wellOffsetXMM + "'," +
                                                 "wellOffsetYMM='" + samplecols.wellOffsetYMM + "' " +
                                                 "WHERE sampleId = '" + expcols.sampleId + "'";
                                MyComm.CommandText = strCommandText;
                                MyComm.ExecuteNonQuery();

                                imageWells = samplecols.rows * samplecols.columns;

                                break;
                            case "SubImages":
                                samplecols.subRows = Convert.ToInt32(reader.GetAttribute(0));
                                samplecols.subColumns = Convert.ToInt32(reader.GetAttribute(1));
                                samplecols.transOffsetXMM = Convert.ToDouble(reader.GetAttribute(2));
                                samplecols.transOffsetYMM = Convert.ToDouble(reader.GetAttribute(3));
                                samplecols.subOffsetXMM = Convert.ToDouble(reader.GetAttribute(4));
                                samplecols.subOffsetYMM = Convert.ToDouble(reader.GetAttribute(5));
                                strCommandText = "UPDATE thorlabs.sample SET " +
                                                 "subRows='" + samplecols.subRows + "'," +
                                                 "subColumns='" + samplecols.subColumns + "'," +
                                                 "transOffsetXMM='" + samplecols.transOffsetXMM + "'," +
                                                 "transOffsetYMM='" + samplecols.transOffsetYMM + "'," +
                                                 "subOffsetXMM='" + samplecols.subOffsetXMM + "'," +
                                                 "subOffsetYMM='" + samplecols.subOffsetYMM + "' " +
                                                 "WHERE sampleId = '" + expcols.sampleId + "'";
                                MyComm.CommandText = strCommandText;
                                MyComm.ExecuteNonQuery();

                                imageSubImages = samplecols.subRows * samplecols.subColumns;

                                break;

                            case "Comments":
                                expcols.comments = reader.GetAttribute(0);
                                break;

                            case "Category":
                                Category category = new Category();
                                category.categoryName = reader.GetAttribute(0);
                                categories.Add(category);
                                break;
                        }

                    }
                }

                strCommandText = "INSERT INTO thorlabs.experiment (" +
                                 "name,date,userName,computerName,softwareVersion,magnificationMag," +
                                 "comments,cameraId,timepoints,intervalSec,sampleId,zStagename," +
                                 "zStageSteps,zStageStepSize,ftpPath,imageFTPLocationID" +
                                 ")" + " VALUES (" +
                                 "'" + expcols.name + "'," +
                                 "'" + expcols.date + "'," +
                                 "'" + expcols.userName + "'," +
                                 "'" + expcols.computerName + "'," +
                                 "'" + expcols.softwareVersion + "'," +
                                 "'" + expcols.magnificationMag + "'," +
                                 "'" + expcols.comments + "'," +
                                 "'" + expcols.cameraId + "'," +
                                 "'" + expcols.timepoints + "'," +
                                 "'" + expcols.intervalSec + "'," +
                                 "'" + expcols.sampleId + "'," +
                                 "'" + expcols.zStagename + "'," +
                                 "'" + expcols.zStageSteps + "'," +
                                 "'" + expcols.zStageStepSize + "'," +
                                 "'" + expcols.ftpPath + "'," +
                                 "'" + expcols.imageFTPLocationID + "'" +
                                 ")";
                MyComm.CommandText = strCommandText;
                MyComm.ExecuteNonQuery();

                strCommandText = "SELECT MAX(experimentId) FROM thorlabs.experiment";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                experimentID = Convert.ToInt64(experiment);
                sMessage = sMessage + experimentID.ToString();

                if (wavelengths.Count > 0)
                {
                    foreach (Wavelength wave in wavelengths)
                    {
                        strCommandText = "INSERT INTO thorlabs.wavelength (name,exposureTimeMS,experimentId) VALUES ('" + wave.name + "','" + wave.exposureTimeMS + "','" + experimentID + "')";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                }

                if (categories.Count > 0)
                {
                    foreach (Category cat in categories)
                    {
                        strCommandText = "SELECT categoryId FROM thorlabs.category WHERE categoryName = '" + cat.categoryName + "'";
                        MyComm.CommandText = strCommandText;
                        object category = MyComm.ExecuteScalar();
                        if (category == null)
                        {
                            strCommandText = "INSERT INTO thorlabs.category (categoryName) VALUES ('" + cat.categoryName + "')";
                            MyComm.CommandText = strCommandText;
                            MyComm.ExecuteNonQuery();
                            strCommandText = "SELECT categoryId FROM thorlabs.category WHERE categoryName = '" + cat.categoryName + "'";
                            MyComm.CommandText = strCommandText;
                            category = MyComm.ExecuteScalar();
                        }
                        int categoryId = Convert.ToInt32(category);
                        strCommandText = "INSERT INTO thorlabs.experimentcategory (experimentId, categoryId) VALUES ('" + experimentID + "','" + categoryId + "')";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                }

                for (int i = 1; i <= imageWells; i++)
                {
                    for (int j = 1; j <= imageSubImages; j++)
                    {
                        imageName = imageWavelength;

                        string sWell = i.ToString();
                        int padWidth = 5 - sWell.Length;
                        if (padWidth > 0) sWell = sWell.PadLeft(padWidth, '0');
                        imageName = imageName + "_" + sWell;

                        string subImage = j.ToString();
                        padWidth = 5 - subImage.Length;
                        if (padWidth > 0) subImage = subImage.PadLeft(padWidth, '0');
                        imageName = imageName + "_" + subImage;

                        string sZstage = imageZStage.ToString();
                        padWidth = 5 - sZstage.Length;
                        if (padWidth > 0) sZstage = sZstage.PadLeft(padWidth, '0');
                        imageName = imageName + "_" + sZstage;

                        string sTimelapse = imageTimelapse.ToString();
                        padWidth = 5 - sTimelapse.Length;
                        if (padWidth > 0) sTimelapse = sTimelapse.PadLeft(padWidth, '0');
                        imageName = imageName + "_" + sTimelapse;

                        imageName = imageName + ".tif";

                        strCommandText = "INSERT INTO thorlabs.image (name,experimentID) VALUES ('" + imageName + "','" + experimentID + "')";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                }
            }
            catch (Exception ex)
            {
                sMessage = ex.Message;

            }

            if (reader != null) reader.Close();
            if (response != null) response.Close();

            return sMessage;
        }

        /**
	     * This method is use to update the experiment by reading from xml.
	     * @param xmlPath. The FTP path where the xml file is stored.
	     * @return {status}
	     */
        public string updateExperimentInformationFromXML(string xmlPath)
        {
            string sMessage = "Success:Experiment is updated successfully";
            string imageWavelength = string.Empty, imageName = string.Empty;
            int imageWells = 0, imageSubImages = 0;
            double imageZStage = 0, imageTimelapse = 0;

            try
            {
                //Get and set FTP user name and password
                GetAndSetFtpDetails();

                //Get XML reader for FTP
                string sPhysicalPath = ftpLocation + xmlPath;
                GetFTPXMLReader(sPhysicalPath);

                Experiment expcols = new Experiment();
                Camera camcols = new Camera();
                List<Wavelength> wavelengths = new List<Wavelength>();
                Sample samplecols = new Sample();
                List<Category> categories = new List<Category>();
                long experimentID = 0;
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText;

                string[] delim = new string[2] { @"\", "/" };
                string[] substrings = sPhysicalPath.Split(delim, StringSplitOptions.None);
                for (int i = 0; i < substrings.Length - 1; i++)
                {
                    expcols.ftpPath = expcols.ftpPath + substrings[i] + "/";
                }

                while (reader.Read())
                {
                    if (reader.NodeType == XmlNodeType.Element)
                    {
                        switch (reader.Name)
                        {
                            case "Name":
                                expcols.name = reader.GetAttribute(0);
                                strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE name = '" + expcols.name + "'";
                                MyComm.CommandText = strCommandText;
                                object exp = MyComm.ExecuteScalar();
                                if (exp != null)
                                {
                                    experimentID = Convert.ToInt64(exp);
                                }
                                else
                                {
                                    sMessage = "Error:Experiment is not available";
                                    if (reader != null) reader.Close();
                                    if (response != null) response.Close();
                                    return sMessage;
                                }
                                break;

                            case "Date":
                                DateTime dt = Convert.ToDateTime(reader.GetAttribute(0));
                                string y = dt.Year.ToString(); string m = dt.Month.ToString(); string d = dt.Day.ToString();
                                string sdt = y + "-" + m + "-" + d;
                                expcols.date = sdt;
                                break;

                            case "User":
                                expcols.userName = reader.GetAttribute(0);
                                break;

                            case "Computer":
                                expcols.computerName = reader.GetAttribute(0);
                                break;

                            case "Software":
                                expcols.softwareVersion = Convert.ToDouble(reader.GetAttribute(0));
                                break;

                            case "Camera":
                                camcols.name = reader.GetAttribute(0);
                                camcols.width = Convert.ToInt32(reader.GetAttribute(1));
                                camcols.height = Convert.ToInt32(reader.GetAttribute(2));
                                camcols.pixelSizeUM = Convert.ToDouble(reader.GetAttribute(3));

                                strCommandText = "SELECT cameraId FROM thorlabs.experiment WHERE experimentId = '" + experimentID + "'";
                                MyComm.CommandText = strCommandText;
                                object cam = MyComm.ExecuteScalar();
                                int camId = Convert.ToInt32(cam);
                                if (camId > 0)
                                {
                                    strCommandText = "UPDATE thorlabs.camera SET " +
                                                     "name = '" + camcols.name + "', " +
                                                     "width = '" + camcols.width + "', " +
                                                     "height = '" + camcols.height + "', " +
                                                     "pixelSizeUM = '" + camcols.pixelSizeUM + "' " +
                                                     "WHERE cameraId = '" + camId + "'";
                                    MyComm.CommandText = strCommandText;
                                    MyComm.ExecuteNonQuery();
                                    expcols.cameraId = camId;
                                }
                                else
                                {
                                    strCommandText = "INSERT INTO thorlabs.camera (name,width,height,pixelSizeUM) VALUES ('" + camcols.name + "','" + camcols.width + "','" + camcols.height + "','" + camcols.pixelSizeUM + "')";
                                    MyComm.CommandText = strCommandText;
                                    MyComm.ExecuteNonQuery();

                                    strCommandText = "SELECT MAX(cameraId) FROM thorlabs.camera";
                                    MyComm.CommandText = strCommandText;
                                    object camera = MyComm.ExecuteScalar();
                                    expcols.cameraId = Convert.ToInt32(camera);
                                }
                                break;

                            case "Magnification":
                                expcols.magnificationMag = Convert.ToDouble(reader.GetAttribute(0));
                                break;

                            case "Wavelengths":
                                break;
                            case "Wavelength":
                                Wavelength wavelength = new Wavelength();
                                wavelength.name = reader.GetAttribute(0);
                                wavelength.exposureTimeMS = Convert.ToDouble(reader.GetAttribute(1));
                                wavelengths.Add(wavelength);

                                imageWavelength = wavelength.name;

                                break;

                            case "ZStage":
                                expcols.zStagename = reader.GetAttribute(0);
                                expcols.zStageSteps = Convert.ToDouble(reader.GetAttribute(1));
                                expcols.zStageStepSize = Convert.ToDouble(reader.GetAttribute(2));

                                imageZStage = expcols.zStageSteps;

                                break;

                            case "Timelapse":
                                expcols.timepoints = Convert.ToDouble(reader.GetAttribute(0));
                                expcols.intervalSec = Convert.ToDouble(reader.GetAttribute(1));

                                imageTimelapse = expcols.timepoints;

                                break;

                            case "Sample":
                                samplecols.type = Convert.ToInt32(reader.GetAttribute(0));
                                samplecols.offsetXMM = Convert.ToDouble(reader.GetAttribute(1));
                                samplecols.offsetYMM = Convert.ToDouble(reader.GetAttribute(2));

                                strCommandText = "SELECT sampleId FROM thorlabs.experiment WHERE experimentId = '" + experimentID + "'";
                                MyComm.CommandText = strCommandText;
                                object sam = MyComm.ExecuteScalar();
                                int samId = Convert.ToInt32(sam);
                                if (samId > 0)
                                {
                                    strCommandText = "UPDATE thorlabs.sample SET " +
                                                     "type = '" + samplecols.type + "', " +
                                                     "offsetXMM = '" + samplecols.offsetXMM + "', " +
                                                     "offsetYMM = '" + samplecols.offsetYMM + "' " +
                                                     "WHERE sampleId = '" + samId + "'";
                                    MyComm.CommandText = strCommandText;
                                    MyComm.ExecuteNonQuery();
                                    expcols.sampleId = samId;
                                }
                                else
                                {
                                    strCommandText = "INSERT INTO thorlabs.sample (type,offsetXMM,offsetYMM)" +
                                                     "VALUES ('" + samplecols.type + "','" + samplecols.offsetXMM + "','" + samplecols.offsetYMM + "')";
                                    MyComm.CommandText = strCommandText;
                                    MyComm.ExecuteNonQuery();

                                    strCommandText = "SELECT MAX(sampleId) FROM thorlabs.sample";
                                    MyComm.CommandText = strCommandText;
                                    object sample = MyComm.ExecuteScalar();
                                    expcols.sampleId = Convert.ToInt32(sample);
                                }
                                break;
                            case "Wells":
                                samplecols.startRow = Convert.ToInt32(reader.GetAttribute(0));
                                samplecols.startColumn = Convert.ToInt32(reader.GetAttribute(1));
                                samplecols.rows = Convert.ToInt32(reader.GetAttribute(2));
                                samplecols.columns = Convert.ToInt32(reader.GetAttribute(3));
                                samplecols.wellOffsetXMM = Convert.ToDouble(reader.GetAttribute(4));
                                samplecols.wellOffsetYMM = Convert.ToDouble(reader.GetAttribute(5));
                                strCommandText = "UPDATE thorlabs.sample SET " +
                                                 "startRow='" + samplecols.startRow + "'," +
                                                 "startColumn='" + samplecols.startColumn + "'," +
                                                 "rows='" + samplecols.rows + "'," +
                                                 "columns='" + samplecols.columns + "'," +
                                                 "wellOffsetXMM='" + samplecols.wellOffsetXMM + "'," +
                                                 "wellOffsetYMM='" + samplecols.wellOffsetYMM + "' " +
                                                 "WHERE sampleId = '" + expcols.sampleId + "'";
                                MyComm.CommandText = strCommandText;
                                MyComm.ExecuteNonQuery();

                                imageWells = samplecols.rows * samplecols.columns;

                                break;
                            case "SubImages":
                                samplecols.subRows = Convert.ToInt32(reader.GetAttribute(0));
                                samplecols.subColumns = Convert.ToInt32(reader.GetAttribute(1));
                                samplecols.transOffsetXMM = Convert.ToDouble(reader.GetAttribute(2));
                                samplecols.transOffsetYMM = Convert.ToDouble(reader.GetAttribute(3));
                                samplecols.subOffsetXMM = Convert.ToDouble(reader.GetAttribute(4));
                                samplecols.subOffsetYMM = Convert.ToDouble(reader.GetAttribute(5));
                                strCommandText = "UPDATE thorlabs.sample SET " +
                                                 "subRows='" + samplecols.subRows + "'," +
                                                 "subColumns='" + samplecols.subColumns + "'," +
                                                 "transOffsetXMM='" + samplecols.transOffsetXMM + "'," +
                                                 "transOffsetYMM='" + samplecols.transOffsetYMM + "'," +
                                                 "subOffsetXMM='" + samplecols.subOffsetXMM + "'," +
                                                 "subOffsetYMM='" + samplecols.subOffsetYMM + "' " +
                                                 "WHERE sampleId = '" + expcols.sampleId + "'";
                                MyComm.CommandText = strCommandText;
                                MyComm.ExecuteNonQuery();

                                imageSubImages = samplecols.subRows * samplecols.subColumns;

                                break;

                            case "Comments":
                                expcols.comments = reader.GetAttribute(0);
                                break;

                            case "Category":
                                Category category = new Category();
                                category.categoryName = reader.GetAttribute(0);
                                categories.Add(category);
                                break;
                        }

                    }
                }

                strCommandText = "UPDATE thorlabs.experiment SET " +
                                 "date = '" + expcols.date + "', " +
                                 "userName = '" + expcols.userName + "', " +
                                 "computerName = '" + expcols.computerName + "', " +
                                 "softwareVersion = '" + expcols.softwareVersion + "', " +
                                 "magnificationMag = '" + expcols.magnificationMag + "', " +
                                 "comments = '" + expcols.comments + "', " +
                                 "cameraId = '" + expcols.cameraId + "', " +
                                 "timepoints = '" + expcols.timepoints + "', " +
                                 "intervalSec = '" + expcols.intervalSec + "', " +
                                 "sampleId = '" + expcols.sampleId + "', " +
                                 "zStagename = '" + expcols.zStagename + "', " +
                                 "zStageSteps = '" + expcols.zStageSteps + "', " +
                                 "zStageStepSize = '" + expcols.zStageStepSize + "', " +
                                 "ftpPath = '" + expcols.ftpPath + "', " +
                                 "imageFTPLocationID = '" + expcols.imageFTPLocationID + "' " +
                                 "WHERE experimentId = '" + experimentID + "'";

                MyComm.CommandText = strCommandText;
                MyComm.ExecuteNonQuery();

                if (wavelengths.Count > 0)
                {
                    strCommandText = "DELETE FROM thorlabs.wavelength WHERE experimentId = '" + experimentID + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();

                    foreach (Wavelength wave in wavelengths)
                    {
                        strCommandText = "INSERT INTO thorlabs.wavelength (name,exposureTimeMS,experimentId) VALUES ('" + wave.name + "','" + wave.exposureTimeMS + "','" + experimentID + "')";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                }

                if (categories.Count > 0)
                {
                    strCommandText = "DELETE FROM thorlabs.experimentcategory WHERE experimentId = '" + experimentID + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();

                    foreach (Category cat in categories)
                    {
                        strCommandText = "SELECT categoryId FROM thorlabs.category WHERE categoryName = '" + cat.categoryName + "'";
                        MyComm.CommandText = strCommandText;
                        object category = MyComm.ExecuteScalar();
                        if (category == null)
                        {
                            strCommandText = "INSERT INTO thorlabs.category (categoryName) VALUES ('" + cat.categoryName + "')";
                            MyComm.CommandText = strCommandText;
                            MyComm.ExecuteNonQuery();
                            strCommandText = "SELECT categoryId FROM thorlabs.category WHERE categoryName = '" + cat.categoryName + "'";
                            MyComm.CommandText = strCommandText;
                            category = MyComm.ExecuteScalar();
                        }
                        int categoryId = Convert.ToInt32(category);
                        strCommandText = "INSERT INTO thorlabs.experimentcategory (experimentId, categoryId) VALUES ('" + experimentID + "','" + categoryId + "')";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                }

                strCommandText = "DELETE FROM thorlabs.image WHERE experimentId = '" + experimentID + "'";
                MyComm.CommandText = strCommandText;
                MyComm.ExecuteNonQuery();

                for (int i = 1; i <= imageWells; i++)
                {
                    for (int j = 1; j <= imageSubImages; j++)
                    {
                        imageName = imageWavelength;

                        string sWell = i.ToString();
                        int padWidth = 5 - sWell.Length;
                        if (padWidth > 0) sWell = sWell.PadLeft(padWidth, '0');
                        imageName = imageName + "_" + sWell;

                        string subImage = j.ToString();
                        padWidth = 5 - subImage.Length;
                        if (padWidth > 0) subImage = subImage.PadLeft(padWidth, '0');
                        imageName = imageName + "_" + subImage;

                        string sZstage = imageZStage.ToString();
                        padWidth = 5 - sZstage.Length;
                        if (padWidth > 0) sZstage = sZstage.PadLeft(padWidth, '0');
                        imageName = imageName + "_" + sZstage;

                        string sTimelapse = imageTimelapse.ToString();
                        padWidth = 5 - sTimelapse.Length;
                        if (padWidth > 0) sTimelapse = sTimelapse.PadLeft(padWidth, '0');
                        imageName = imageName + "_" + sTimelapse;

                        imageName = imageName + ".tif";

                        strCommandText = "INSERT INTO thorlabs.image (name,experimentID) VALUES ('" + imageName + "','" + experimentID + "')";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            if (reader != null) reader.Close();
            if (response != null) response.Close();

            return sMessage;
        }

        /**
	     * This method fetches the user for the experiment
	     * @param experimentId - The id of the experiment .
	     * @return {User name}
	     */
        public string getUser(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT userName FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object userName = MyComm.ExecuteScalar();
                if (userName != null)
                {
                    if (userName != DBNull.Value)
                    {
                        sMessage = sMessage + userName.ToString();
                    }
                    else
                    {
                        sMessage = "Error:User name is not set";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method sets the name of the user.
	     * @param experimentId. The id of the experiment .
	     * @param pUserName. The user name to be set
	     * @return {Status}
	     */
        public string setUser(long experimentId, string userName)
        {
            string sMessage = "Success:User is set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "UPDATE thorlabs.experiment SET userName = '" + userName + "' WHERE experimentId = '" + experimentId + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method is used to get the name of the experiment
	     * @param pExperimentId. The id of the experiment whose name is to be found.
	     * @return {experiment name}
	     */
        public string getName(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT name FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        sMessage = sMessage + experiment.ToString();
                    }
                    else
                    {
                        sMessage = "Error:Name is not set";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method replaces the old experiment name with the new name
	     * @param existingName. Existing experiment name
	     * @param newName. New Experiment name
	     * @return {status}
	     */
        public string setName(string newName, string existingName)
        {
            string sMessage = "Success:Experiment name is set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT name FROM thorlabs.experiment WHERE name = '" + existingName + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "SELECT name FROM thorlabs.experiment WHERE name = '" + newName + "'";
                    MyComm.CommandText = strCommandText;
                    experiment = MyComm.ExecuteScalar();
                    if (experiment == null)
                    {
                        strCommandText = "UPDATE thorlabs.experiment SET name = '" + newName + "' WHERE name = '" + existingName + "'";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                    else
                    {
                        sMessage = "Error:Experiment name already in use";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the date
	     * @param experimentId. The id of the experiment .
	     * @return {experiment date}
	     */
        public string getDate(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT date FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        sMessage = sMessage + experiment.ToString();
                    }
                    else
                    {
                        sMessage = "Error:Date is not set";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method sets the date for the experiment
	     * @param pDate. The date to be set.
	     * @param experimentId. The id of the experiment .
	     * @return {status}
	     */
        public string setDate(string date, long experimentId)
        {
            string sMessage = "Success:Experiment date is set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    DateTime dtExp = DateTime.MinValue;
                    if (DateTime.TryParse(date, out dtExp))
                    {
                        strCommandText = "UPDATE thorlabs.experiment SET date = '" + date + "' WHERE experimentId = '" + experimentId + "'";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                    else
                    {
                        sMessage = "Error:Date not valid";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the computer name
	     * @param experimentId - The id of the experiment .
	     * @return {computer name}
	     */
        public string getComputer(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT computerName FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        sMessage = sMessage + experiment.ToString();
                    }
                    else
                    {
                        sMessage = "Error:Computer is not set";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method sets the computer name.
	     * @param pComputerName. The name of the computer
	     * @param experimentId - The id of the experiment .           
	     * @return {status}
	     */
        public string setComputer(string ComputerName, long experimentId)
        {
            string sMessage = "Success:Experiment computer is set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "UPDATE thorlabs.experiment SET computerName = '" + ComputerName + "' WHERE experimentId = '" + experimentId + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the software version
	     * @param experimentId - The id of the experiment . 
	     * @return {sortware version}
	     */
        public string getSoftware(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT softwareVersion FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        sMessage = sMessage + experiment.ToString();
                    }
                    else
                    {
                        sMessage = "Error:Software is not set";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method sets the software version.
	     * @param pSoftwareVersion. The software version
	     * @param experimentId - The id of the experiment .
	     * @return {status}
	     */
        public string setSoftware(double softwareVersion, long experimentId)
        {
            string sMessage = "Success:Experiment software is set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "UPDATE thorlabs.experiment SET softwareVersion = '" + softwareVersion + "' WHERE experimentId = '" + experimentId + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the details of the camera
	     * @param experimentId. The Id of the experiment
	     * @return {camera info}
	     */
        public string getCamera(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT cameraId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        int cameraId = Convert.ToInt32(experiment);
                        strCommandText = "SELECT name,width,height,pixelSizeUM,binning,gain,lightmode FROM thorlabs.camera WHERE cameraId = '" + cameraId + "'";
                        MyComm.CommandText = strCommandText;
                        OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                        DataSet ds = new DataSet();
                        adapter.Fill(ds);

                        if (ds != null)
                        {
                            if (ds.Tables.Count > 0)
                            {
                                if (ds.Tables[0].Rows.Count > 0)
                                {
                                    string name = String.Empty, width = String.Empty, height = String.Empty, pixelSizeUM = String.Empty, binning = String.Empty, gain = String.Empty, lightmode = String.Empty;
                                    if (ds.Tables[0].Rows[0].ItemArray[0] != DBNull.Value) name = ds.Tables[0].Rows[0].ItemArray[0].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[1] != DBNull.Value) width = ds.Tables[0].Rows[0].ItemArray[1].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[2] != DBNull.Value) height = ds.Tables[0].Rows[0].ItemArray[2].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[3] != DBNull.Value) pixelSizeUM = ds.Tables[0].Rows[0].ItemArray[3].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[4] != DBNull.Value) binning = ds.Tables[0].Rows[0].ItemArray[4].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[5] != DBNull.Value) gain = ds.Tables[0].Rows[0].ItemArray[5].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[6] != DBNull.Value) lightmode = ds.Tables[0].Rows[0].ItemArray[6].ToString();
                                    sMessage = name + "," + width + "," + height + "," + pixelSizeUM + "," + binning + "," + gain + "," + lightmode;
                                }
                            }
                        }                       
                    }
                    else
                    {
                        sMessage = "Error:Camera is not set";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method sets the camera details.
	     * @param name. The name of camera.
	     * @param width. The width of camera.
	     * @param height. The height of camera.
	     * @param pixelSizeUM. The pixel size in UM of camera
	     * @param experimentName. The name of the experiment
	     * @return {status}
	     */
        public string setCamera(string name, int width, int height,
                double pixelSizeUM, long experimentId)
        {
            string sMessage = "Success:Experiment camera is set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT cameraId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        int cameraId = Convert.ToInt32(experiment);
                        strCommandText = "SELECT cameraId from camera where cameraId = '" + cameraId + "'";
                        MyComm.CommandText = strCommandText;
                        object camera = MyComm.ExecuteScalar();
                        if (camera != null)
                        {
                            strCommandText = "UPDATE thorlabs.camera SET name = '" + name + "', width = '" + width + "', height = '" + height + "', pixelSizeUM = '" + pixelSizeUM + "' WHERE cameraId = '" + cameraId + "'";
                            MyComm.CommandText = strCommandText;
                            MyComm.ExecuteNonQuery();
                        }
                        else
                        {
                            sMessage = "Error:Camera is not available";
                        }
                    }
                    else
                    {
                        sMessage = "Error:Camera not available for the experiment";
                    }

                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the magnification value.
	     * @param experimentId. The name of the experiment name
	     * @return {magnificaion value}
	     */
        public string getMagnification(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT magnificationMag FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        sMessage = sMessage + experiment.ToString();
                    }
                    else
                    {
                        sMessage = "Error:Magnification is not set";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method sets the magnification value. 
	     * @param mag. The magnification value
	     * @param experimentId. The Id of the experiment
	     * @return {status}
	     */
        public string setMagnification(double mag, long experimentId)
        {
            string sMessage = "Success:Experiment magnification is set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "UPDATE thorlabs.experiment SET magnificationMag = '" + mag + "' WHERE experimentId = '" + experimentId + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
         * This method fetches the details of Z stage 
         * @param experimentId. The Id of the experiment
         * @return {z stage}
         */
        public string getZStage(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "SELECT zStagename,zStageSteps,zStageStepSize FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                    MyComm.CommandText = strCommandText;
                    OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                    DataSet ds = new DataSet();
                    adapter.Fill(ds);

                    if (ds != null)
                    {
                        if (ds.Tables.Count > 0)
                        {
                            if (ds.Tables[0].Rows.Count > 0)
                            {
                                string zStagename = String.Empty, zStageSteps = String.Empty, zStageStepSize = String.Empty;
                                if (ds.Tables[0].Rows[0].ItemArray[0] != DBNull.Value) zStagename = ds.Tables[0].Rows[0].ItemArray[0].ToString();
                                if (ds.Tables[0].Rows[0].ItemArray[1] != DBNull.Value) zStageSteps = ds.Tables[0].Rows[0].ItemArray[1].ToString();
                                if (ds.Tables[0].Rows[0].ItemArray[2] != DBNull.Value) zStageStepSize = ds.Tables[0].Rows[0].ItemArray[2].ToString();
                                sMessage = sMessage + zStagename + "," + zStageSteps + "," + zStageStepSize;
                            }
                        }
                    }
                }
                else
                {
                    sMessage = "Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method sets Z Stage 
	     * @param name. Name of Z stage
	     * @param steps. Z stage names
	     * @param stepSize. Z stage size
	     * @param experimentId. The Id of the experiment
	     * @return {status}
	     */
        public string setZStage(string name,
                                double steps,
                                double stepSize,
                                long experimentId)
        {
            string sMessage = "Success:Experiment Zstage is set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "UPDATE thorlabs.experiment SET zStagename = '" + name + "', zStageSteps = '" + steps + "', zStageStepSize = '" + stepSize + "' WHERE experimentId = '" + experimentId + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the details of the timelapse 
	     * @param experimentId. The Id of the experiment
	     * @return {timelapse}
	     */
        public string getTimelapse(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "SELECT timepoints,intervalSec FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                    MyComm.CommandText = strCommandText;
                    OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                    DataSet ds = new DataSet();
                    adapter.Fill(ds);

                    if (ds != null)
                    {
                        if (ds.Tables.Count > 0)
                        {
                            if (ds.Tables[0].Rows.Count > 0)
                            {
                                string timepoints = String.Empty, intervalSec = String.Empty;
                                if (ds.Tables[0].Rows[0].ItemArray[0] != DBNull.Value) timepoints = ds.Tables[0].Rows[0].ItemArray[0].ToString();
                                if (ds.Tables[0].Rows[0].ItemArray[1] != DBNull.Value) intervalSec = ds.Tables[0].Rows[0].ItemArray[1].ToString();
                                sMessage = sMessage + timepoints + "," + intervalSec;
                            }
                        }
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method sets the time lapse information 
	     * @param timepoints. Time points for time lapse
	     * @param intervalSec. Interval in seconds for time lapse
	     * @param experimentId. The Id of the experiment
	     * @return {status}
	     */
        public string setTimelapse(double timepoints,
                                   double intervalSec,
                                   long experimentId)
        {
            string sMessage = "Success:Experiment Timelapse is set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "UPDATE thorlabs.experiment SET timepoints = '" + timepoints + "', intervalSec = '" + intervalSec + "' WHERE experimentId = '" + experimentId + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the details of the sample 
	     * @param experimentId. The Id of the experiment
	     * @return {sample}
	     */
        public string getSample(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT sampleId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        int sampleId = Convert.ToInt32(experiment);
                        strCommandText = "SELECT type,offsetXMM,offsetYMM FROM thorlabs.sample WHERE sampleId = '" + sampleId + "'";
                        MyComm.CommandText = strCommandText;
                        OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                        DataSet ds = new DataSet();
                        adapter.Fill(ds);

                        if (ds != null)
                        {
                            if (ds.Tables.Count > 0)
                            {
                                if (ds.Tables[0].Rows.Count > 0)
                                {
                                    string type = String.Empty, offsetXMM = String.Empty, offsetYMM = String.Empty;
                                    if (ds.Tables[0].Rows[0].ItemArray[0] != DBNull.Value) type = ds.Tables[0].Rows[0].ItemArray[0].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[1] != DBNull.Value) offsetXMM = ds.Tables[0].Rows[0].ItemArray[1].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[2] != DBNull.Value) offsetYMM = ds.Tables[0].Rows[0].ItemArray[2].ToString();
                                    sMessage = sMessage + type + "," + offsetXMM + "," + offsetYMM;
                                }
                                else
                                {
                                    sMessage = "Error:Sample is not available";
                                }
                            }
                        }
                    }
                    else
                    {
                        sMessage = "Error:Sample is not set";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }


        /**
         * This method sets the details of the sample 
         * @param type. The type of sample
         * @param offsetX. The X offset for the sample
         * @param offsetY. The Y offset for the sample
         * @param experimentId. The Id of the experiment
         * @return {status}
         */
        public string setSample(int type,
                                double offsetX,
                                double offsetY,
                                long experimentId)
        {
            string sMessage = "Success:Experiment sample is set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT sampleId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        int sampleId = Convert.ToInt32(experiment);
                        strCommandText = "SELECT sampleId FROM thorlabs.sample WHERE sampleId = '" + sampleId + "'";
                        MyComm.CommandText = strCommandText;
                        object sample = MyComm.ExecuteScalar();
                        if (sample != null)
                        {
                            strCommandText = "UPDATE thorlabs.sample SET type = '" + type + "', offsetXMM = '" + offsetX + "', offsetYMM = '" + offsetY + "' WHERE sampleId = '" + sampleId + "'";
                            MyComm.CommandText = strCommandText;
                            MyComm.ExecuteNonQuery();
                        }
                        else
                        {
                            sMessage = "Error:Sample is not available";
                        }
                    }
                    else
                    {
                        sMessage = "Error:Sample is not set for the experiment";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the details of the wells 
	     * @param experimentId. The Id of the experiment
	     * @return {wells info}
	     */
        public string getWells(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT sampleId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        int sampleId = Convert.ToInt32(experiment);
                        strCommandText = "SELECT startRow,startColumn,rows,columns,wellOffsetXMM,wellOffsetYMM FROM thorlabs.sample WHERE sampleId = '" + sampleId + "'";
                        MyComm.CommandText = strCommandText;
                        OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                        DataSet ds = new DataSet();
                        adapter.Fill(ds);

                        if (ds != null)
                        {
                            if (ds.Tables.Count > 0)
                            {
                                if (ds.Tables[0].Rows.Count > 0)
                                {
                                    string startRow = String.Empty, startColumn = String.Empty, rows = String.Empty, columns = String.Empty, wellOffsetXMM = String.Empty, wellOffsetYMM = String.Empty;
                                    if (ds.Tables[0].Rows[0].ItemArray[0] != DBNull.Value) startRow = ds.Tables[0].Rows[0].ItemArray[0].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[1] != DBNull.Value) startColumn = ds.Tables[0].Rows[0].ItemArray[1].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[2] != DBNull.Value) rows = ds.Tables[0].Rows[0].ItemArray[2].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[3] != DBNull.Value) columns = ds.Tables[0].Rows[0].ItemArray[3].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[4] != DBNull.Value) wellOffsetXMM = ds.Tables[0].Rows[0].ItemArray[4].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[5] != DBNull.Value) wellOffsetYMM = ds.Tables[0].Rows[0].ItemArray[5].ToString();
                                    sMessage = sMessage + startRow + "," + startColumn + "," + rows + "," + columns + "," + wellOffsetXMM + "," + wellOffsetYMM;
                                }
                                else
                                {
                                    sMessage = "Error:Sample is not available";
                                }
                            }
                        }
                    }
                    else
                    {
                        sMessage = "Error:Wells are not set";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method sets the wells details 
	     * @param startRow. The start row of the plate
	     * @param startColumn. The start column of the plate
	     * @param rows. The number of rows
	     * @param columns. The number of columns
	     * @param wellOffsetXMM. The X offset for the well
	     * @param wellOffsetYMM. The Y offset for the well
	     * @param experimentId. The Id of the experiment
	     * @return {status}
	     */
        public string setWells(int startRow,
                               int startColumn,
                               int rows,
                               int columns,
                               double wellOffsetXMM,
                               double wellOffsetYMM,
                               long experimentId)
        {
            string sMessage = "Success:Experiment wells are set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT sampleId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        int sampleId = Convert.ToInt32(experiment);
                        strCommandText = "SELECT sampleId FROM thorlabs.sample WHERE sampleId = '" + sampleId + "'";
                        MyComm.CommandText = strCommandText;
                        object sample = MyComm.ExecuteScalar();
                        if (sample != null)
                        {
                            strCommandText = "UPDATE thorlabs.sample SET startRow = '" + startRow + "', startColumn = '" + startColumn + "', rows = '" + rows + "', columns = '" + columns + "', wellOffsetXMM = '" + wellOffsetXMM + "', wellOffsetYMM = '" + wellOffsetYMM + "' WHERE sampleId = '" + sampleId + "'";
                            MyComm.CommandText = strCommandText;
                            MyComm.ExecuteNonQuery();
                        }
                        else
                        {
                            sMessage = "Error:Sample is not available";
                        }
                    }
                    else
                    {
                        sMessage = "Error:Sample is not set for the experiment";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the details of the Sub Images 
	     * @param experimentId. The Id of the experiment
	     * @return {sub images info}
	     */
        public string getSubImages(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT sampleId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        int sampleId = Convert.ToInt32(experiment);
                        strCommandText = "SELECT subRows,subColumns,subOffsetXMM,subOffsetYMM,transOffsetXMM,transOffsetYMM FROM thorlabs.sample WHERE sampleId = '" + sampleId + "'";
                        MyComm.CommandText = strCommandText;
                        OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                        DataSet ds = new DataSet();
                        adapter.Fill(ds);

                        if (ds != null)
                        {
                            if (ds.Tables.Count > 0)
                            {
                                if (ds.Tables[0].Rows.Count > 0)
                                {
                                    string subRows = String.Empty, subColumns = String.Empty, subOffsetXMM = String.Empty, subOffsetYMM = String.Empty, transOffsetXMM = String.Empty, transOffsetYMM = String.Empty;
                                    if (ds.Tables[0].Rows[0].ItemArray[0] != DBNull.Value) subRows = ds.Tables[0].Rows[0].ItemArray[0].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[1] != DBNull.Value) subColumns = ds.Tables[0].Rows[0].ItemArray[1].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[2] != DBNull.Value) subOffsetXMM = ds.Tables[0].Rows[0].ItemArray[2].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[3] != DBNull.Value) subOffsetYMM = ds.Tables[0].Rows[0].ItemArray[3].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[4] != DBNull.Value) transOffsetXMM = ds.Tables[0].Rows[0].ItemArray[4].ToString();
                                    if (ds.Tables[0].Rows[0].ItemArray[5] != DBNull.Value) transOffsetYMM = ds.Tables[0].Rows[0].ItemArray[5].ToString();
                                    sMessage = sMessage + subRows + "," + subColumns + "," + subOffsetXMM + "," + subOffsetYMM + "," + transOffsetXMM + "," + transOffsetYMM;
                                }
                                else
                                {
                                    sMessage = "Error:Sub images are not available";
                                }
                            }
                        }
                    }
                    else
                    {
                        sMessage = "Error:Sample is not set";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method sets the details of Sub Images 
	     * @param subRows. The sub row of the Sub Image
	     * @param subColumns. The sub column of the Sub Image
	     * @param subOffsetXMM. The X sub offset for Sub Image
	     * @param subOffsetYMM. The Y sub offset for Sub Image
	     * @param transOffsetXMM. The X trans offset for Sub Image
	     * @param transOffsetYMM. The Y trans offset for Sub Image
	     * @param experimentId. The Id of the experiment
	     * @return {status}
	     */
        public string setSubImages(int subRows,
                                   int subColumns,
                                   double subOffsetXMM,
                                   double subOffsetYMM,
                                   double transOffsetXMM,
                                   double transOffsetYMM,
                                   long experimentId)
        {
            string sMessage = "Success:Experiment images are set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT sampleId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        int sampleId = Convert.ToInt32(experiment);
                        strCommandText = "SELECT sampleId FROM thorlabs.sample WHERE sampleId = '" + sampleId + "'";
                        MyComm.CommandText = strCommandText;
                        object sample = MyComm.ExecuteScalar();
                        if (sample != null)
                        {
                            strCommandText = "UPDATE thorlabs.sample SET subRows = '" + subRows + "', subColumns = '" + subColumns + "', subOffsetXMM = '" + subOffsetXMM + "', subOffsetYMM = '" + subOffsetYMM + "', transOffsetXMM = '" + transOffsetXMM + "', transOffsetYMM = '" + transOffsetYMM + "' WHERE sampleId = '" + sampleId + "'";
                            MyComm.CommandText = strCommandText;
                            MyComm.ExecuteNonQuery();
                        }
                        else
                        {
                            sMessage = "Error:Sample is not available";
                        }
                    }
                    else
                    {
                        sMessage = "Error:Sample is not set for the experimrnt";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the comments of the experiment 
	     * @param experimentId. The Id of the experiment
	     * @return {comments}
	     */
        public string getComments(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT comments FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (experiment != DBNull.Value)
                    {
                        sMessage = sMessage + experiment.ToString();
                    }
                    else
                    {
                        sMessage = "Error:Comments are not set";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method sets the comments for the experiment 
	     * @param comment. The comments to be added
	     * @param experimentId. The Id of the experiment
	     * @return {status}
	     */
        public string setComments(string comment, long experimentId)
        {
            string sMessage = "Success:Comments are set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "UPDATE thorlabs.experiment SET comments = '" + comment + "' WHERE experimentId = '" + experimentId + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This methods adds the category to the selected experiment 
	     * @param categoryName. The name of the category
	     * @param experimentId. The Id of the experiment
	     * @return {status}
	     */
        public string addCategoryForExperiment(long experimentId, string categoryName)
        {
            string sMessage = "Success:Experiment category added successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (categoryName.Trim() != string.Empty)
                    {
                        strCommandText = "SELECT categoryId from category where categoryName = '" + categoryName + "'";
                        MyComm.CommandText = strCommandText;
                        object category = MyComm.ExecuteScalar();
                        if (category == null)
                        {
                            strCommandText = "INSERT INTO thorlabs.category (categoryName) VALUES ('" + categoryName + "')";
                            MyComm.CommandText = strCommandText;
                            MyComm.ExecuteNonQuery();
                            strCommandText = "SELECT Max(id) FROM thorlabs.category";
                            MyComm.CommandText = strCommandText;
                            category = MyComm.ExecuteScalar();
                        }
                        long categoryId = Convert.ToInt32(category);

                        strCommandText = "SELECT experimentId FROM thorlabs.experimentcategory WHERE experimentId = '" + experimentId + "' AND categoryId = '" + categoryId + "'";
                        MyComm.CommandText = strCommandText;
                        object expcat = MyComm.ExecuteScalar();
                        if (expcat == null)
                        {
                            strCommandText = "INSERT INTO thorlabs.experimentcategory (experimentId,categoryId) VALUES ('" + experimentId + "','" + categoryId + "')";
                            MyComm.CommandText = strCommandText;
                            MyComm.ExecuteNonQuery();
                        }
                        else
                        {
                            sMessage = "Error:Category is already available for given experiment";
                        }
                    }
                    else
                    {
                        sMessage = "Error:Null is not allowed as category name";
                    }

                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method removes the category from the experiment 
	     * @param categoryName. The name of the category
	     * @param experimentId. The Id of the experiment
	     * @return {status}
	     */
        public string removeCategoryForExperiment(long experimentId, string categoryName)
        {
            string sMessage = "Success:Experiment category is removed successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    if (categoryName.Trim() != string.Empty)
                    {
                        strCommandText = "SELECT categoryId from category where categoryName = '" + categoryName + "'";
                        MyComm.CommandText = strCommandText;
                        object category = MyComm.ExecuteScalar();
                        if (category != null)
                        {
                            long categoryID = Convert.ToInt32(category);
                            strCommandText = "DELETE FROM thorlabs.experimentcategory WHERE experimentId = '" + experimentId + "' AND categoryId = '" + categoryID + "'";
                            MyComm.CommandText = strCommandText;
                            MyComm.ExecuteNonQuery();
                        }
                        else
                        {
                            sMessage = "Error:Category is not available";
                        }
                    }
                    else
                    {
                        sMessage = "Error:Null is not allowed as category name";
                    }

                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
         * This method returns all Categories of a Experiment 
         * @param pValidExperimentID
         * @return {list of categories}
         */
        public List<Category> getCategories(long validExperimentID)
        {
            List<Category> categories = new List<Category>();
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + validExperimentID + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "SELECT categoryId FROM thorlabs.experimentcategory WHERE experimentId = '" + validExperimentID + "'";
                    MyComm.CommandText = strCommandText;
                    OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                    DataSet ds = new DataSet();
                    adapter.Fill(ds);

                    if (ds != null)
                    {
                        if (ds.Tables.Count > 0)
                        {
                            if (ds.Tables[0].Rows.Count > 0)
                            {
                                long categoryId;
                                foreach (DataRow dr in ds.Tables[0].Rows)
                                {
                                    categoryId = Convert.ToInt32(dr.ItemArray[0]);
                                    strCommandText = "SELECT categoryName FROM thorlabs.category WHERE categoryId = '" + categoryId + "'";
                                    MyComm.CommandText = strCommandText;
                                    object cat = MyComm.ExecuteScalar();
                                    if (cat != null)
                                    {
                                        Category category = new Category();
                                        category.categoryId = categoryId;
                                        category.categoryName = Convert.ToString(cat);
                                        categories.Add(category);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                string str = ex.Message;

            }

            return categories;
        }

        /**
	     * This method searches the experiment based on the experiment name.
	     * @param experimentName. The experiment name to be searched.
	     * @return {Experiment List}
	     */
        public List<Experiment> getExperimentByExperimentName(string experimentName)
        {
            List<Experiment> experiments = new List<Experiment>();
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId,name,date,userName,computerName,softwareVersion,magnificationMag,comments,cameraId,timepoints,intervalSec,sampleId,zStagename,zStageSteps,zStageStepSize,ftpPath,imageFTPLocationID FROM thorlabs.experiment WHERE name = '" + experimentName + "'";
                MyComm.CommandText = strCommandText;
                OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                DataSet ds = new DataSet();
                adapter.Fill(ds);

                if (ds != null)
                {
                    if (ds.Tables.Count > 0)
                    {
                        if (ds.Tables[0].Rows.Count > 0)
                        {
                            foreach (DataRow dr in ds.Tables[0].Rows)
                            {
                                Experiment exp = new Experiment();
                                exp.experimentId = Convert.ToInt64(dr.ItemArray[0]);
                                exp.name = Convert.ToString(dr.ItemArray[1]);
                                if (dr.ItemArray[2] != DBNull.Value) exp.date = Convert.ToString(dr.ItemArray[2]);
                                if (dr.ItemArray[3] != DBNull.Value) exp.userName = Convert.ToString(dr.ItemArray[3]);
                                if (dr.ItemArray[4] != DBNull.Value) exp.computerName = Convert.ToString(dr.ItemArray[4]);
                                if (dr.ItemArray[5] != DBNull.Value) exp.softwareVersion = Convert.ToDouble(dr.ItemArray[5]);
                                if (dr.ItemArray[6] != DBNull.Value) exp.magnificationMag = Convert.ToDouble(dr.ItemArray[6]);
                                if (dr.ItemArray[7] != DBNull.Value) exp.comments = Convert.ToString(dr.ItemArray[7]);
                                if (dr.ItemArray[8] != DBNull.Value) exp.cameraId = Convert.ToInt32(dr.ItemArray[8]);
                                if (dr.ItemArray[9] != DBNull.Value) exp.timepoints = Convert.ToDouble(dr.ItemArray[9]);
                                if (dr.ItemArray[10] != DBNull.Value) exp.intervalSec = Convert.ToDouble(dr.ItemArray[10]);
                                if (dr.ItemArray[11] != DBNull.Value) exp.sampleId = Convert.ToInt32(dr.ItemArray[11]);
                                if (dr.ItemArray[12] != DBNull.Value) exp.zStagename = Convert.ToString(dr.ItemArray[12]);
                                if (dr.ItemArray[13] != DBNull.Value) exp.zStageSteps = Convert.ToDouble(dr.ItemArray[13]);
                                if (dr.ItemArray[14] != DBNull.Value) exp.zStageStepSize = Convert.ToDouble(dr.ItemArray[14]);
                                if (dr.ItemArray[15] != DBNull.Value) exp.ftpPath = Convert.ToString(dr.ItemArray[15]);
                                if (dr.ItemArray[16] != DBNull.Value) exp.imageFTPLocationID = Convert.ToInt32(dr.ItemArray[16]);

                                experiments.Add(exp);
                            }
                        }
                    }
                }

            }
            catch (Exception ex)
            {
                string str = ex.Message;

            }

            return experiments;
        }

        /**
	     * This method searches the experiment based on the user name.
	     * @param userName. The user name to be searched
	     * @return {Experiment List}
	     */
        private static List<Experiment> getExperimentByUserName(string userName)
        {
            List<Experiment> experiments = new List<Experiment>();
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId,name,date,userName,computerName,softwareVersion,magnificationMag,comments,cameraId,timepoints,intervalSec,sampleId,zStagename,zStageSteps,zStageStepSize,ftpPath,imageFTPLocationID FROM thorlabs.experiment WHERE userName = '" + userName + "'";
                MyComm.CommandText = strCommandText;
                OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                DataSet ds = new DataSet();
                adapter.Fill(ds);

                if (ds != null)
                {
                    if (ds.Tables.Count > 0)
                    {
                        if (ds.Tables[0].Rows.Count > 0)
                        {
                            foreach (DataRow dr in ds.Tables[0].Rows)
                            {
                                Experiment exp = new Experiment();
                                exp.experimentId = Convert.ToInt64(dr.ItemArray[0]);
                                exp.name = Convert.ToString(dr.ItemArray[1]);
                                if (dr.ItemArray[2] != DBNull.Value) exp.date = Convert.ToString(dr.ItemArray[2]);
                                if (dr.ItemArray[3] != DBNull.Value) exp.userName = Convert.ToString(dr.ItemArray[3]);
                                if (dr.ItemArray[4] != DBNull.Value) exp.computerName = Convert.ToString(dr.ItemArray[4]);
                                if (dr.ItemArray[5] != DBNull.Value) exp.softwareVersion = Convert.ToDouble(dr.ItemArray[5]);
                                if (dr.ItemArray[6] != DBNull.Value) exp.magnificationMag = Convert.ToDouble(dr.ItemArray[6]);
                                if (dr.ItemArray[7] != DBNull.Value) exp.comments = Convert.ToString(dr.ItemArray[7]);
                                if (dr.ItemArray[8] != DBNull.Value) exp.cameraId = Convert.ToInt32(dr.ItemArray[8]);
                                if (dr.ItemArray[9] != DBNull.Value) exp.timepoints = Convert.ToDouble(dr.ItemArray[9]);
                                if (dr.ItemArray[10] != DBNull.Value) exp.intervalSec = Convert.ToDouble(dr.ItemArray[10]);
                                if (dr.ItemArray[11] != DBNull.Value) exp.sampleId = Convert.ToInt32(dr.ItemArray[11]);
                                if (dr.ItemArray[12] != DBNull.Value) exp.zStagename = Convert.ToString(dr.ItemArray[12]);
                                if (dr.ItemArray[13] != DBNull.Value) exp.zStageSteps = Convert.ToDouble(dr.ItemArray[13]);
                                if (dr.ItemArray[14] != DBNull.Value) exp.zStageStepSize = Convert.ToDouble(dr.ItemArray[14]);
                                if (dr.ItemArray[15] != DBNull.Value) exp.ftpPath = Convert.ToString(dr.ItemArray[15]);
                                if (dr.ItemArray[16] != DBNull.Value) exp.imageFTPLocationID = Convert.ToInt32(dr.ItemArray[16]);

                                experiments.Add(exp);
                            }
                        }
                    }
                }

            }
            catch (Exception ex)
            {
                string str = ex.Message;

            }

            return experiments;
        }

        public static List<string> GetExperimentNamesByUserName(string userName)
        {
            List<Experiment> getList;

            getList = getExperimentByUserName(userName);

            List<string> expListNames = new List<string>();

            for(int i=0;i<getList.Count; i++)
            {
                expListNames.Add(getList[i].name);
            }

            return expListNames;
        }


        public static long GetExperimentIdByExpName(string expName)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();
            MyCmd.Connection = MyConn;
            string strCommand = "SELECT * FROM `thorlabs`.`experiment` WHERE name = '" + expName + "'";
            OdbcDataAdapter adp = new OdbcDataAdapter(strCommand, MyConn);
            DataSet dsExp = new DataSet();

            adp.Fill(dsExp, "Exp");

            if (dsExp.Tables[0].Rows.Count > 0)
            {
                const int EXP_INDEX = 0;

                return Convert.ToInt32(dsExp.Tables[0].Rows[0].ItemArray[EXP_INDEX].ToString());
            }

            return 0;
        }

        public static long GetAlgorithmIdByAlgName(string algName)
        {
            long algId = 0;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT AlgorithmId FROM thorlabs.algorithms WHERE AlgorithmName = '" + algName + "'";
            MyComm.CommandText = strCommandText;
            object algorithm = MyComm.ExecuteScalar();
            if (algorithm != null)
            {
                algId = Convert.ToInt64(algorithm);
            }

            return algId;
        }
        /**
	     * This method searches the experiment based on the comments.
	     * @param comments. The comments to be searched
	     * @return {Experiment List}
	     */
        public List<Experiment> getExperimentByComments(string comments)
        {
            List<Experiment> experiments = new List<Experiment>();
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId,name,date,userName,computerName,softwareVersion,magnificationMag,comments,cameraId,timepoints,intervalSec,sampleId,zStagename,zStageSteps,zStageStepSize,ftpPath,imageFTPLocationID FROM thorlabs.experiment WHERE comments = '" + comments + "'";
                MyComm.CommandText = strCommandText;
                OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                DataSet ds = new DataSet();
                adapter.Fill(ds);

                if (ds != null)
                {
                    if (ds.Tables.Count > 0)
                    {
                        if (ds.Tables[0].Rows.Count > 0)
                        {
                            foreach (DataRow dr in ds.Tables[0].Rows)
                            {
                                Experiment exp = new Experiment();
                                exp.experimentId = Convert.ToInt64(dr.ItemArray[0]);
                                exp.name = Convert.ToString(dr.ItemArray[1]);
                                if (dr.ItemArray[2] != DBNull.Value) exp.date = Convert.ToString(dr.ItemArray[2]);
                                if (dr.ItemArray[3] != DBNull.Value) exp.userName = Convert.ToString(dr.ItemArray[3]);
                                if (dr.ItemArray[4] != DBNull.Value) exp.computerName = Convert.ToString(dr.ItemArray[4]);
                                if (dr.ItemArray[5] != DBNull.Value) exp.softwareVersion = Convert.ToDouble(dr.ItemArray[5]);
                                if (dr.ItemArray[6] != DBNull.Value) exp.magnificationMag = Convert.ToDouble(dr.ItemArray[6]);
                                if (dr.ItemArray[7] != DBNull.Value) exp.comments = Convert.ToString(dr.ItemArray[7]);
                                if (dr.ItemArray[8] != DBNull.Value) exp.cameraId = Convert.ToInt32(dr.ItemArray[8]);
                                if (dr.ItemArray[9] != DBNull.Value) exp.timepoints = Convert.ToDouble(dr.ItemArray[9]);
                                if (dr.ItemArray[10] != DBNull.Value) exp.intervalSec = Convert.ToDouble(dr.ItemArray[10]);
                                if (dr.ItemArray[11] != DBNull.Value) exp.sampleId = Convert.ToInt32(dr.ItemArray[11]);
                                if (dr.ItemArray[12] != DBNull.Value) exp.zStagename = Convert.ToString(dr.ItemArray[12]);
                                if (dr.ItemArray[13] != DBNull.Value) exp.zStageSteps = Convert.ToDouble(dr.ItemArray[13]);
                                if (dr.ItemArray[14] != DBNull.Value) exp.zStageStepSize = Convert.ToDouble(dr.ItemArray[14]);
                                if (dr.ItemArray[15] != DBNull.Value) exp.ftpPath = Convert.ToString(dr.ItemArray[15]);
                                if (dr.ItemArray[16] != DBNull.Value) exp.imageFTPLocationID = Convert.ToInt32(dr.ItemArray[16]);

                                experiments.Add(exp);
                            }
                        }
                    }
                }

            }
            catch (Exception ex)
            {
                string str = ex.Message;

            }

            return experiments;
        }

        /**
	     * This method searches the experiment based on the category name.
	     * @param categoryName. The category to be searched
	     * @return {Experiment List}
	     */
        public List<Experiment> getExperimentByCategoryName(string categoryName)
        {
            List<Experiment> experiments = new List<Experiment>();
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT categoryId FROM thorlabs.category WHERE categoryName = '" + categoryName + "'";
                MyComm.CommandText = strCommandText;
                object category = MyComm.ExecuteScalar();
                if (category != null)
                {
                    int categoryId = Convert.ToInt32(category);
                    strCommandText = "SELECT experimentId,name,date,userName,computerName,softwareVersion,magnificationMag,comments,cameraId,timepoints,intervalSec,sampleId,zStagename,zStageSteps,zStageStepSize,ftpPath,imageFTPLocationID FROM thorlabs.experiment WHERE experimentId in (SELECT experimentId from thorlabs.experimentcategory WHERE categoryId = '" + categoryId + "')";
                    MyComm.CommandText = strCommandText;
                    OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                    DataSet ds = new DataSet();
                    adapter.Fill(ds);

                    if (ds != null)
                    {
                        if (ds.Tables.Count > 0)
                        {
                            if (ds.Tables[0].Rows.Count > 0)
                            {
                                foreach (DataRow dr in ds.Tables[0].Rows)
                                {
                                    Experiment exp = new Experiment();
                                    exp.experimentId = Convert.ToInt64(dr.ItemArray[0]);
                                    exp.name = Convert.ToString(dr.ItemArray[1]);
                                    if (dr.ItemArray[2] != DBNull.Value) exp.date = Convert.ToString(dr.ItemArray[2]);
                                    if (dr.ItemArray[3] != DBNull.Value) exp.userName = Convert.ToString(dr.ItemArray[3]);
                                    if (dr.ItemArray[4] != DBNull.Value) exp.computerName = Convert.ToString(dr.ItemArray[4]);
                                    if (dr.ItemArray[5] != DBNull.Value) exp.softwareVersion = Convert.ToDouble(dr.ItemArray[5]);
                                    if (dr.ItemArray[6] != DBNull.Value) exp.magnificationMag = Convert.ToDouble(dr.ItemArray[6]);
                                    if (dr.ItemArray[7] != DBNull.Value) exp.comments = Convert.ToString(dr.ItemArray[7]);
                                    if (dr.ItemArray[8] != DBNull.Value) exp.cameraId = Convert.ToInt32(dr.ItemArray[8]);
                                    if (dr.ItemArray[9] != DBNull.Value) exp.timepoints = Convert.ToDouble(dr.ItemArray[9]);
                                    if (dr.ItemArray[10] != DBNull.Value) exp.intervalSec = Convert.ToDouble(dr.ItemArray[10]);
                                    if (dr.ItemArray[11] != DBNull.Value) exp.sampleId = Convert.ToInt32(dr.ItemArray[11]);
                                    if (dr.ItemArray[12] != DBNull.Value) exp.zStagename = Convert.ToString(dr.ItemArray[12]);
                                    if (dr.ItemArray[13] != DBNull.Value) exp.zStageSteps = Convert.ToDouble(dr.ItemArray[13]);
                                    if (dr.ItemArray[14] != DBNull.Value) exp.zStageStepSize = Convert.ToDouble(dr.ItemArray[14]);
                                    if (dr.ItemArray[15] != DBNull.Value) exp.ftpPath = Convert.ToString(dr.ItemArray[15]);
                                    if (dr.ItemArray[16] != DBNull.Value) exp.imageFTPLocationID = Convert.ToInt32(dr.ItemArray[16]);

                                    experiments.Add(exp);
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                string str = ex.Message;

            }

            return experiments;
        }

        /**
	     * This method searches the experiment list for the given date range.
	     * @param pStartdate Start Date
	     * @param pEndDate End Date
	     * @return {Experiment List}
	     */
        public List<Experiment> getExperimentByDate(string startDate, string endDate)
        {
            List<Experiment> experiments = new List<Experiment>();
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId,name,date,userName,computerName,softwareVersion,magnificationMag,comments,cameraId,timepoints,intervalSec,sampleId,zStagename,zStageSteps,zStageStepSize,ftpPath,imageFTPLocationID FROM thorlabs.experiment WHERE date BETWEEN '" + startDate + "' AND '" + endDate + "'";
                MyComm.CommandText = strCommandText;
                OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                DataSet ds = new DataSet();
                adapter.Fill(ds);

                if (ds != null)
                {
                    if (ds.Tables.Count > 0)
                    {
                        if (ds.Tables[0].Rows.Count > 0)
                        {
                            foreach (DataRow dr in ds.Tables[0].Rows)
                            {
                                Experiment exp = new Experiment();
                                exp.experimentId = Convert.ToInt64(dr.ItemArray[0]);
                                exp.name = Convert.ToString(dr.ItemArray[1]);
                                if (dr.ItemArray[2] != DBNull.Value) exp.date = Convert.ToString(dr.ItemArray[2]);
                                if (dr.ItemArray[3] != DBNull.Value) exp.userName = Convert.ToString(dr.ItemArray[3]);
                                if (dr.ItemArray[4] != DBNull.Value) exp.computerName = Convert.ToString(dr.ItemArray[4]);
                                if (dr.ItemArray[5] != DBNull.Value) exp.softwareVersion = Convert.ToDouble(dr.ItemArray[5]);
                                if (dr.ItemArray[6] != DBNull.Value) exp.magnificationMag = Convert.ToDouble(dr.ItemArray[6]);
                                if (dr.ItemArray[7] != DBNull.Value) exp.comments = Convert.ToString(dr.ItemArray[7]);
                                if (dr.ItemArray[8] != DBNull.Value) exp.cameraId = Convert.ToInt32(dr.ItemArray[8]);
                                if (dr.ItemArray[9] != DBNull.Value) exp.timepoints = Convert.ToDouble(dr.ItemArray[9]);
                                if (dr.ItemArray[10] != DBNull.Value) exp.intervalSec = Convert.ToDouble(dr.ItemArray[10]);
                                if (dr.ItemArray[11] != DBNull.Value) exp.sampleId = Convert.ToInt32(dr.ItemArray[11]);
                                if (dr.ItemArray[12] != DBNull.Value) exp.zStagename = Convert.ToString(dr.ItemArray[12]);
                                if (dr.ItemArray[13] != DBNull.Value) exp.zStageSteps = Convert.ToDouble(dr.ItemArray[13]);
                                if (dr.ItemArray[14] != DBNull.Value) exp.zStageStepSize = Convert.ToDouble(dr.ItemArray[14]);
                                if (dr.ItemArray[15] != DBNull.Value) exp.ftpPath = Convert.ToString(dr.ItemArray[15]);
                                if (dr.ItemArray[16] != DBNull.Value) exp.imageFTPLocationID = Convert.ToInt32(dr.ItemArray[16]);
                                experiments.Add(exp);
                            }
                        }
                    }
                }

            }
            catch (Exception ex)
            {
                string str = ex.Message;

            }

            return experiments;
        }

        /**
         * This method sets the category name 
         * @param categoryName. The name of the category
         * @return {status}
         */
        public string createCategoty(string categoryName)
        {
            string sMessage = "Success:Category created successfully";

            if (categoryName.Trim() == String.Empty)
            {
                sMessage = "Error:Null is not allowed as category name";
            }
            else
            {
                try
                {
                    OdbcConnection MyConn = Connection.GetConnection();
                    OdbcCommand MyComm = new OdbcCommand();
                    MyComm.Connection = MyConn;
                    string strCommandText = "SELECT categoryId FROM thorlabs.category WHERE categoryName = '" + categoryName + "'";
                    MyComm.CommandText = strCommandText;
                    object category = MyComm.ExecuteScalar();
                    if (category == null)
                    {
                        strCommandText = "INSERT INTO thorlabs.category (categoryName) VALUES('" + categoryName + "')";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                    else
                    {
                        sMessage = "Error:Category already exists";
                    }
                }
                catch (Exception ex)
                {
                    sMessage = "Error:" + ex.Message;
                }
            }

            return sMessage;
        }

        /**
         * This method gives the count of the wavelength 
         * @param experimentId. The Id of the experiment
         * @return {number of wavelenths}
         */
        public string getNumberOfWavelengths(long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();                
                if (experiment != null)
                {
                    strCommandText = "SELECT COUNT(waveLengthId) FROM thorlabs.wavelength WHERE experimentId = '" + experimentId + "'";
                    MyComm.CommandText = strCommandText;
                    experiment = MyComm.ExecuteScalar();
                    sMessage = sMessage + experiment.ToString();
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
         * This method fetches the list of the wavelengths
         * @param pExperimentId The id of the experiment
         * @return {list of wavelenths}
         */
        public List<Wavelength> getWavelengths(long experimentId)
        {
            List<Wavelength> wavelengths = new List<Wavelength>();
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT waveLengthId,index1,name,exposureTimeMS,experimentId FROM thorlabs.wavelength WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                DataSet ds = new DataSet();
                adapter.Fill(ds);

                if (ds != null)
                {
                    if (ds.Tables.Count > 0)
                    {
                        if (ds.Tables[0].Rows.Count > 0)
                        {
                            foreach (DataRow dr in ds.Tables[0].Rows)
                            {
                                Wavelength wave = new Wavelength();
                                wave.waveLengthId = Convert.ToInt64(dr.ItemArray[0]);
                                if (dr.ItemArray[1] != DBNull.Value) wave.index1 = Convert.ToInt32(dr.ItemArray[1]);
                                if (dr.ItemArray[2] != DBNull.Value) wave.name = Convert.ToString(dr.ItemArray[2]);
                                if (dr.ItemArray[3] != DBNull.Value) wave.exposureTimeMS = Convert.ToDouble(dr.ItemArray[3]);
                                if (dr.ItemArray[4] != DBNull.Value) wave.experimentId = Convert.ToInt64(dr.ItemArray[4]);

                                wavelengths.Add(wave);
                            }
                        }
                    }
                }

            }
            catch (Exception ex)
            {

                string str = ex.Message;
            }

            return wavelengths;
        }

        /**
	     * This method fetches the details of the wavelength for particular index.
	     * @param index. The search index
	     * @param experimentId. The Id of the experiment
	     * @return {@wavelength}
	     */
        public string getWavelength(int index, long experimentId)
        {
            string sMessage = "Value:";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT waveLengthId,index1,name,exposureTimeMS,experimentId FROM thorlabs.wavelength WHERE experimentId = '" + experimentId + "' AND index1 = '" + index + "'";
                MyComm.CommandText = strCommandText;
                OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                DataSet ds = new DataSet();
                adapter.Fill(ds);

                if (ds != null)
                {
                    if (ds.Tables.Count > 0)
                    {
                        if (ds.Tables[0].Rows.Count > 0)
                        {
                            string waveLengthId = String.Empty, index1 = String.Empty, name = String.Empty, exposureTimeMS = String.Empty, sexperimentId = String.Empty;
                            if (ds.Tables[0].Rows[0].ItemArray[0] != DBNull.Value) waveLengthId = ds.Tables[0].Rows[0].ItemArray[0].ToString();
                            if (ds.Tables[0].Rows[0].ItemArray[1] != DBNull.Value) index1 = ds.Tables[0].Rows[0].ItemArray[1].ToString();
                            if (ds.Tables[0].Rows[0].ItemArray[2] != DBNull.Value) name = ds.Tables[0].Rows[0].ItemArray[2].ToString();
                            if (ds.Tables[0].Rows[0].ItemArray[3] != DBNull.Value) exposureTimeMS = ds.Tables[0].Rows[0].ItemArray[3].ToString();
                            if (ds.Tables[0].Rows[0].ItemArray[4] != DBNull.Value) sexperimentId = ds.Tables[0].Rows[0].ItemArray[4].ToString();
                            sMessage = sMessage + waveLengthId + "," + index1 + "," + name + "," + exposureTimeMS + "," + sexperimentId;
                        }
                        else
                        {
                            sMessage = "Error:Wavelength is not available";
                        }
                    }
                }

            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method updates the wavelength. 
	     * @param index. The index of the wavelength
	     * @param name. The name of the wavelength
	     * @param exposureTimeMS. The wavelength exposure time
	     * @param experimentId. The Id of the experiment
	     * @return {status}
	     */
        public string setWavelength(int index,
                                    string name,
                                    double exposureTimeMS,
                                    long experimentId)
        {
            string sMessage = "Success:Wavelength is set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "SELECT index1 FROM thorlabs.wavelength WHERE experimentId = '" + experimentId + "' AND index1 = '" + index + "'";
                    MyComm.CommandText = strCommandText;
                    object wave = MyComm.ExecuteScalar();
                    if (wave != null)
                    {
                        strCommandText = "UPDATE thorlabs.wavelength SET name = '" + name + "', exposureTimeMS = '" + exposureTimeMS + "' WHERE experimentId = '" + experimentId + "' AND index1 = '" + index + "'";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                    else
                    {
                        sMessage = "Error:Wavelength is not available";
                    }
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }
            return sMessage;
        }

        /**
	     * This method adds the wavelength 
	     * @param name. The name of the experiment
	     * @param exposureTimeMS. The wavelength exposure time
	     * @param experimentId. The Id of the experiment
	     * @return {@status}
	     */
        public string addWavelength(string name,
                                    double exposureTimeMS,
                                    long experimentId)
        {
            string sMessage = "Success:Wavelength added successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "INSERT INTO thorlabs.wavelength (experimentId,name,exposureTimeMS)" +
                                     "VALUES ('" + experimentId + "','" + name + "','" + exposureTimeMS + "')";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();
                }
                else
                {
                    sMessage = "Error:Experiment is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }
            return sMessage;
        }

        /**
         * This method removes the wavelength 
         * @param name. The name of the wavelength
         * @param experimentId. The Id of the experiment
         * @return {status}
         */
        public string removeWavelength(string name, long experimentId)
        {
            string sMessage = "Success:Wavelength is removed successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT waveLengthId FROM thorlabs.wavelength WHERE name = '" + name + "' AND experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object wavelength = MyComm.ExecuteScalar();
                if (wavelength != null)
                {
                    strCommandText = "DELETE FROM thorlabs.wavelength WHERE name = '" + name + "' AND experimentId = '" + experimentId + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();
                }
                else
                {
                    sMessage = "Error:Wavelength is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
         * This method authenticates the user
         * @param pLoginId. The login ID of the user
         * @param password. The password of the user
         * @return {Login Status}
         */
        public string loginUser(string LoginId, string Password)
        {
            string sMessage = "Success:";
            try
            {
                if (LoginId.Trim() != String.Empty && Password.Trim() != String.Empty)
                {
                    object lastName;
                    OdbcConnection MyConn = Connection.GetConnection();
                    OdbcCommand MyComm = new OdbcCommand();
                    MyComm.Connection = MyConn;
                    string strCommandText = "SELECT LastName FROM thorlabs.user WHERE LoginID = '" + LoginId + "' AND Password = '" + Password + "'";
                    MyComm.CommandText = strCommandText;
                    lastName = MyComm.ExecuteScalar();
                    if (lastName != null)
                    {
                        if (lastName != DBNull.Value)
                        {
                            sMessage = sMessage + lastName.ToString() + " login successful";
                        }
                        else
                        {
                            sMessage = sMessage + " login successful";
                        }
                        strCommandText = "Update thorlabs.user SET Status = '0' WHERE LoginID = '" + LoginId + "' AND Password = '" + Password + "'";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                    else
                    {
                        sMessage = "Error:Invalid login";
                    }
                }
                else
                {
                    sMessage = "Error:Invalid login";
                }

            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
         * This method logs out the user
         * @param pLoginId Login id of the user	
         * @return {Logout Status}
         */
        public string logOut(string LoginId)
        {
            string sMessage = "Success:Logout successful";

            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT userId FROM thorlabs.user WHERE LoginID = '" + LoginId + "'";
                MyComm.CommandText = strCommandText;
                object user = MyComm.ExecuteScalar();
                if (user != null)
                {
                    strCommandText = "Update thorlabs.user SET Status = '1' WHERE LoginID = '" + LoginId + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();
                }
                else
                {
                    sMessage = "Error:Logout failed";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
         * This method creates new user
         * @param firstName. The first name of the user
         * @param lastName. The last name of the user
         * @param emailId. The mail id of the user
         * @param password. The password of the user
         * @return {Create Status}
         */
        public string createUser(string firstName,
                                 string lastName,
                                 string emailId,
                                 string password)
        {
            string sMessage = "Success:User created successfully";

            if (firstName == string.Empty)
            {
                sMessage = "Error:First name cannot be null";
                return sMessage;
            }
            if (lastName == string.Empty)
            {
                sMessage = "Error:Last name cannot be null";
                return sMessage;
            }
            if (emailId == string.Empty)
            {
                sMessage = "Error:Email Id cannot be null";
                return sMessage;
            }
            if (password == string.Empty)
            {
                sMessage = "Error:Password cannot be null";
                return sMessage;
            }

            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;

                string strCommandText = "SELECT EmailID FROM thorlabs.user WHERE EmailID = '" + emailId + "'";
                MyComm.CommandText = strCommandText;
                object email = MyComm.ExecuteScalar();
                if (email != null)
                {
                    sMessage = "Error:Email Id already in use";
                    return sMessage;
                }

                strCommandText = "INSERT INTO thorlabs.user (FirstName,LastName,EmailID,LoginID,Password,Status,Role) VALUES ('" + firstName + "','" + lastName + "','" + emailId + "','" + emailId + "','" + password + "','1','0')";
                MyComm.CommandText = strCommandText;
                MyComm.ExecuteNonQuery();
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method deletes particular user
	     * @param userId. The UserID of the user to be deleted
	     * @return {Delete Status}
	     */
        public string deleteUser(string LoginId)
        {
            string strCommandText = string.Empty;
            string sMessage = "Success:User deleted successfully";

            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                strCommandText = "SELECT UserID FROM thorlabs.user WHERE LoginID = '" + LoginId + "'";
                MyComm.CommandText = strCommandText;
                object userID = MyComm.ExecuteScalar();
                if (userID == null)
                {
                    sMessage = "Error:User does not exist";
                }
                else
                {
                    strCommandText = "DELETE FROM thorlabs.user WHERE LoginID = '" + LoginId + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches all the users
	     * @return {List of users}
	     */
        public List<User> getAllUsers()
        {
            List<User> users = new List<User>();
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT UserID,FirstName,LastName,EmailID,LoginID,Status,Role FROM user";
                MyComm.CommandText = strCommandText;
                OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                DataSet ds = new DataSet();
                adapter.Fill(ds);

                if (ds != null)
                {
                    if (ds.Tables.Count > 0)
                    {
                        foreach (DataRow dr in ds.Tables[0].Rows)
                        {
                            User user = new User();
                            user.UserID = Convert.ToInt64(dr.ItemArray[0]);
                            if (dr.ItemArray[1] != DBNull.Value) user.FirstName = Convert.ToString(dr.ItemArray[1]);
                            if (dr.ItemArray[2] != DBNull.Value) user.LastName = Convert.ToString(dr.ItemArray[2]);
                            if (dr.ItemArray[3] != DBNull.Value) user.EmailID = Convert.ToString(dr.ItemArray[3]);
                            if (dr.ItemArray[4] != DBNull.Value) user.LoginID = Convert.ToString(dr.ItemArray[4]);
                            if (dr.ItemArray[5] != DBNull.Value) user.Status = Convert.ToString(dr.ItemArray[5]);
                            if (dr.ItemArray[6] != DBNull.Value) user.Role = Convert.ToString(dr.ItemArray[6]);
                            users.Add(user);
                        }
                    }
                }
            }
            catch (Exception ex)
            {

                string str = ex.Message;
            }
            return users;
        }

        /**
         * This method fetches all the users
         * @return {List of users}
         */
        public static List<Category> GetAllCategories()
        {
            List<Category> categories = new List<Category>();
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT categoryId,categoryName FROM category";
                MyComm.CommandText = strCommandText;
                OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                DataSet ds = new DataSet();
                adapter.Fill(ds);

                if (ds != null)
                {
                    if (ds.Tables.Count > 0)
                    {
                        foreach (DataRow dr in ds.Tables[0].Rows)
                        {
                            Category cat = new Category();
                            cat.categoryId = Convert.ToInt64(dr.ItemArray[0]);
                            if (dr.ItemArray[1] != DBNull.Value) cat.categoryName = Convert.ToString(dr.ItemArray[1]);
                            categories.Add(cat);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                string str = ex.Message;

            }
            return categories;
        }
        /**
	     * This method associates all the images in the given directory to the
	     * selected experiment 
	     * @param imagePath. The directory where images are located
	     * @param experimentId. The Id of the experiment
	     * @return {status}
	     */
        public string associateImages(string imagePath, long experimentID)
        {
            string sMessage = "Error:Obsolete method";

            return sMessage;
        }

        /**
        * This method adds notes to the selected image 
        * @param imageID. The id of the image
        * @param experimentId. The Id of the experiment
        * @param notes. The noted to be added
        * @return {status}
        */
        public string addNotesToImage(long imageId,
                                      long experimentId,
                                      string notes)
        {
            string sMessage = "Success:Notes added successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT imageID FROM thorlabs.image WHERE imageID = '" + imageId + "' AND experimentId = '" + experimentId + "'";
                MyComm.CommandText = strCommandText;
                object image = MyComm.ExecuteScalar();
                if (image != null)
                {
                    strCommandText = "UPDATE thorlabs.image SET notes = '" + notes + "' WHERE imageID = '" + imageId + "' AND experimentId = '" + experimentId + "'";
                    MyComm.CommandText = strCommandText;
                    MyComm.ExecuteNonQuery();
                }
                else
                {
                    sMessage = "Error:Image is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches all the images associated with the experiment 
	     * @param pValidExperimentID. The valid of the experiment ID
	     * @return {Image List}
	     */
        public List<Image> getAllImages(long validExperimentID)
        {
            List<Image> images = new List<Image>();
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE experimentId = '" + validExperimentID + "'";
                MyComm.CommandText = strCommandText;
                object experiment = MyComm.ExecuteScalar();
                if (experiment != null)
                {
                    strCommandText = "SELECT imageID,name,experimentID,notes FROM thorlabs.image WHERE experimentId = '" + validExperimentID + "'";
                    MyComm.CommandText = strCommandText;
                    OdbcDataAdapter adapter = new OdbcDataAdapter(strCommandText, MyConn);
                    DataSet ds = new DataSet();
                    adapter.Fill(ds);

                    if (ds != null)
                    {
                        if (ds.Tables.Count > 0)
                        {
                            if (ds.Tables[0].Rows.Count > 0)
                            {
                                foreach (DataRow dr in ds.Tables[0].Rows)
                                {
                                    Image image = new Image();
                                    image.imageID = Convert.ToInt64(dr.ItemArray[0]);
                                    image.name = Convert.ToString(dr.ItemArray[1]);
                                    if (dr.ItemArray[2] != DBNull.Value) image.experimentID = Convert.ToInt64(dr.ItemArray[2]);
                                    if (dr.ItemArray[3] != DBNull.Value) image.notes = Convert.ToString(dr.ItemArray[3]);
                                    images.Add(image);
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                string str = ex.Message;
                
            }

            return images;
        }

        /**
         * This method sets the ftp location
         * @param pUserName Ftp User Name
         * @param pPassword Ftp Password
         * @param pLocation Ftp location
         * @return {status}
         */
        public static string SetFtpDetails(string UserName, string Password, string Location)
        {
            string sMessage = "Success:FTP details are set successfully";
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT MAX(imageFTPLocationID) FROM thorlabs.imageftplocation";
                MyComm.CommandText = strCommandText;
                object location = MyComm.ExecuteScalar();                
                if (location != null)
                {
                    int locationId = Convert.ToInt32(location);
                    strCommandText = "UPDATE thorlabs.imageftplocation SET userName = '" + UserName + "', password = '" + Password + "', location = '" + Location + "' WHERE imageFTPLocationID = '" + locationId + "'";
                }
                else
                {
                    strCommandText = "INSERT INTO thorlabs.imageftplocation (userName,password,location) VALUES ('" + UserName + "'," + "'" + Password + "'," + "'" + Location + "')";
                }
                MyComm.CommandText = strCommandText;
                MyComm.ExecuteNonQuery();
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the ftp user name 
	     * @return {user name}
	     */
        public string getFtpUserName()
        {
            string sMessage = "Value:";
            try
            {
                object userName;
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT userName FROM thorlabs.imageftplocation";
                MyComm.CommandText = strCommandText;
                userName = MyComm.ExecuteScalar();
                if (userName != null)
                {
                    if (userName != DBNull.Value)
                    {
                        sMessage = sMessage + userName.ToString();
                    }
                    else
                    {
                        sMessage = "Error:FTP user name is not set";
                    }
                }
                else
                {
                    sMessage = "Error:FTP user name is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the ftp location 
	     * @return {ftp location}
	     */
        public string getFtpLocation()
        {
            string sMessage = "Value:";
            try
            {
                object location;
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT location FROM thorlabs.imageftplocation";
                MyComm.CommandText = strCommandText;
                location = MyComm.ExecuteScalar();
                if (location != null)
                {
                    if (location != DBNull.Value)
                    {
                        sMessage = sMessage + location.ToString();
                    }
                    else
                    {
                        sMessage = "Error:FTP location is not set";
                    }
                }
                else
                {
                    sMessage = "Error:FTP location is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method fetches the ftp password 
	     * @return {ftp password}
	     */
        public string getFtpPassword()
        {
            string sMessage = "Value:";
            try
            {
                object password;
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT password FROM thorlabs.imageftplocation";
                MyComm.CommandText = strCommandText;
                password = MyComm.ExecuteScalar();
                if (password != null)
                {
                    if (password != DBNull.Value)
                    {
                        sMessage = sMessage + password.ToString();
                    }
                    else
                    {
                        sMessage = "Error:FTP passwod is not set";
                    }
                }
                else
                {
                    sMessage = "Error:FTP passwod is not available";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method adds an algorithm 
	     * @return {status}
	     */
        public string addAlgorithm(string name)
        {
            string sMessage = "Success:Algorithm added successfully";

            if (name == string.Empty)
            {
                sMessage = "Error:Name cannot be null";
                return sMessage;
            }

            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;

                string strCommandText = "SELECT AlgorithmId FROM thorlabs.algorithms WHERE AlgorithmName = '" + name + "'";
                MyComm.CommandText = strCommandText;
                object algorithm = MyComm.ExecuteScalar();
                if (algorithm != null)
                {
                    sMessage = "Error:Algorithm already added";
                    return sMessage;
                }

                strCommandText = "INSERT INTO thorlabs.algorithms (AlgorithmName) VALUES ('" + name + "')";
                MyComm.CommandText = strCommandText;
                MyComm.ExecuteNonQuery();
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        /**
	     * This method deletes an algorithm 
	     * @return {status}
	     */
        public string deleteAlgorithm(string name)
        {
            string sMessage = "Success:Algorithm deleted successfully";         

            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;

                string strCommandText = "SELECT AlgorithmId FROM thorlabs.algorithms WHERE AlgorithmName = '" + name + "'";
                MyComm.CommandText = strCommandText;
                object algorithm = MyComm.ExecuteScalar();
                if (algorithm == null)
                {
                    sMessage = "Error:Algorithm not found";
                    return sMessage;
                }
                long AlgorithmId = Convert.ToInt64(algorithm);
                strCommandText = "DELETE FROM thorlabs.algorithms WHERE AlgorithmId = '" + AlgorithmId + "'";
                MyComm.CommandText = strCommandText;
                MyComm.ExecuteNonQuery();
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }
    



        #endregion Methods
    }
}
