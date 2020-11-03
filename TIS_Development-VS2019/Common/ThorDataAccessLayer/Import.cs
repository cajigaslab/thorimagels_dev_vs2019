namespace ThorDataAccessLayer
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Data;
    using System.Data.Odbc;
    using System.IO;
    using System.Linq;
    using System.Net;
    using System.Text;
    using System.Xml;

    public class Import
    {
        #region Fields

        private string _algorithm = String.Empty;
        private long _algorithmId = 0;
        private string[] _arrColumns;
        private char[] _csvDelim = new char[1] { ',' };
        private string _experiment = String.Empty;
        private long _experimentId = 0;
        private string _ftpLocation = String.Empty;
        private string _ftpPassword = String.Empty;
        private string _ftpUserName = String.Empty;
        private string _pathSep = @"/";
        private string _resultsLocation = String.Empty;
        private string _well = String.Empty;
        private long _wellId = 0;
        private long _experimentAlgorithmWellId = 0;
        private long _resultsId = 0;

        #endregion Fields

        #region Properties

        public long ExperimentAlgorithmWellId
        {
            get
            {
                return _experimentAlgorithmWellId;
            }
        }


        public long AlgorithmID
        {
            get
            {
                return _algorithmId;
            }
        }

        public long ExperimentID
        {
            get
            {
                return _experimentId;
            }
        }

        public string ResultsPath
        {
            get
            {
                return _resultsLocation;
            }
        }

        public long WellID
        {
            get
            {
                return _wellId;
            }
        }

        #endregion Properties

        #region Methods

        public string deleteExperimentAlgorithmResults(long resultsId)
        {
            string sMessage = "Success:Experiment-Algorithm results deleted sucessfully";
            string sExperimentAlgorithmWellIds = String.Empty;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;

            string strCommandText = "SELECT ResultsTable FROM thorlabs.algorithms WHERE AlgorithmId = " +
                                    "(SELECT algorithmId FROM thorlabs.experimentalgorithmresults WHERE " +
                                    "ResultsId = '" + resultsId + "')";
            MyComm.CommandText = strCommandText;
            object resTable = MyComm.ExecuteScalar();

            if (resTable != null)
            {
                if (resTable != DBNull.Value)
                {
                    string sResultsTable = Convert.ToString(resTable);
                    strCommandText = "SELECT ExperimentAlgorithmWellId FROM thorlabs.experimentalgorithmwell " + 
                                     "WHERE ResultsId = '" + resultsId + "'";
                    MyComm.CommandText = strCommandText;
                    OdbcDataReader rdr = MyComm.ExecuteReader();

                    if (rdr.HasRows)
                    {
                        while (rdr.Read())
                        {
                            sExperimentAlgorithmWellIds += "," + rdr.GetString(0);
                        }
                    }
                    rdr.Close();

                    if (sExperimentAlgorithmWellIds != String.Empty)
                    {
                        sExperimentAlgorithmWellIds = sExperimentAlgorithmWellIds.Remove(0, 1);

                        strCommandText = "DELETE FROM thorlabs." + sResultsTable + " WHERE " + 
                                         "ExperimentAlgorithmWellId IN (" + sExperimentAlgorithmWellIds + ")";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();

                        strCommandText = "DELETE FROM thorlabs.experimentalgorithmwell WHERE " + 
                                         "ExperimentAlgorithmWellId IN (" + sExperimentAlgorithmWellIds + ")";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();

                        strCommandText = "DELETE FROM thorlabs.experimentalgorithmresults WHERE " + 
                                         "ResultsId = '" + resultsId + "'";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                    else
                    {
                        sMessage = "Error:No data for deletion";
                    }
                }
                else
                {
                    sMessage = "Error:No data for deletion";
                }
            }
            else
            {
                sMessage = "Error:No data for deletion";
            }

            return sMessage;
        }
       
        public string deleteAllExperimentAlgorithmResults(string experimentName, string algorithmName)
        {
            string sMessage = "Success:All Experiment-Algorithm results deleted sucessfully";
            long expId = 0, algId = 0;
            string sResultsTable = String.Empty;
            string sResultsIds = String.Empty;
            string sExperimentAlgorithmWellIds = String.Empty;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE name = '" + experimentName + "'";
            MyComm.CommandText = strCommandText;
            object experiment = MyComm.ExecuteScalar();
            if (experiment != null)
            {
                expId = Convert.ToInt64(experiment);
                strCommandText = "SELECT AlgorithmId FROM thorlabs.algorithms WHERE AlgorithmName = '" + algorithmName + "'";
                MyComm.CommandText = strCommandText;
                object algorithm = MyComm.ExecuteScalar();
                if (algorithm != null)
                {
                    algId = Convert.ToInt64(algorithm);
                    strCommandText = "SELECT ResultsTable FROM thorlabs.algorithms WHERE AlgorithmId = '" + algId + "'";
                    MyComm.CommandText = strCommandText;
                    algorithm = MyComm.ExecuteScalar();
                    if (algorithm != DBNull.Value)
                    {
                        sResultsTable = Convert.ToString(algorithm);

                        strCommandText = "SELECT ResultsId FROM thorlabs.experimentalgorithmresults WHERE " +
                                         "AlgorithmId = '" + algId + "' AND ExperimentId = '" + expId + "'";
                        MyComm.CommandText = strCommandText;
                        OdbcDataReader rdr = MyComm.ExecuteReader();

                        if (rdr.HasRows)
                        {
                            while (rdr.Read())
                            {
                                sResultsIds += "," + rdr.GetString(0);
                            }
                        }
                        rdr.Close();

                        if (sResultsIds != String.Empty)
                        {
                            sResultsIds = sResultsIds.Remove(0, 1);
                            strCommandText = "SELECT ExperimentAlgorithmWellId FROM thorlabs.experimentalgorithmwell WHERE " +
                                             "ResultsId IN (" + sResultsIds + ")";
                            MyComm.CommandText = strCommandText;
                            rdr = MyComm.ExecuteReader();

                            if (rdr.HasRows)
                            {
                                while (rdr.Read())
                                {
                                    sExperimentAlgorithmWellIds += "," + rdr.GetString(0);
                                }
                            }
                            rdr.Close();

                            if (sExperimentAlgorithmWellIds != String.Empty)
                            {
                                sExperimentAlgorithmWellIds = sExperimentAlgorithmWellIds.Remove(0, 1);

                                strCommandText = "DELETE FROM thorlabs." + sResultsTable + " WHERE ExperimentAlgorithmWellId IN (" + sExperimentAlgorithmWellIds + ")";
                                MyComm.CommandText = strCommandText;
                                MyComm.ExecuteNonQuery();

                                strCommandText = "DELETE FROM thorlabs.experimentalgorithmwell WHERE ExperimentAlgorithmWellId IN (" + sExperimentAlgorithmWellIds + ")";
                                MyComm.CommandText = strCommandText;
                                MyComm.ExecuteNonQuery();

                                strCommandText = "DELETE FROM thorlabs.experimentalgorithmresults WHERE ResultsId IN (" + sResultsIds + ")";
                                MyComm.CommandText = strCommandText;
                                MyComm.ExecuteNonQuery();
                            }
                            else
                            {
                                sMessage = "Error:No data for deletion";
                            }
                        }
                        else
                        {
                            sMessage = "Error:No data for deletion";
                        }
                    }
                    else
                    {
                        sMessage = "Error:Results table not found";
                    }
                }
                else
                {
                    sMessage = "Error:Algorithm not found";
                }
            }
            else
            {
                sMessage = "Error:Experiment not found";
            }

            return sMessage;
        }

        public string importExperimentResults(string experimentName,
            string algorithmName,
            string wellName,
            string resultsLocation)
        {
            string sMessage = "Success:Experiment results imported successfully";
            FtpWebResponse response = null;
            StreamReader reader = null;

            try
            {
                //Set ExperimentId, AlgorithmId and WellId
                SetParams(experimentName, algorithmName, resultsLocation);

                //Set FTP details
                GetAndSetFtpDetails();

                //string sExperimentPath = ftpLocation + sPathSep + sResultsLocation + sPathSep + sExperiment + sPathSep + sAlgorithm;
                string sExperimentPath = _ftpLocation + _pathSep + _resultsLocation;

                //Check whether entry of this analysis run has been made into the database
                _resultsId = GetResultsId();

                if (_resultsId == 0) //Add an entry of this analysis run into the database
                {
                    _resultsId = AddExperimentAlgorithmResults();
                }

                FtpWebRequest request;
                Stream responseStream;

                List<string> dirs = new List<string>(); //List to hold well names

                if (wellName != String.Empty) //Specific well
                {
                    dirs.Add(wellName);
                }
                else //All wells
                {
                    request = (FtpWebRequest)WebRequest.Create(sExperimentPath);
                    request.Method = WebRequestMethods.Ftp.ListDirectory;
                    request.Credentials = new NetworkCredential(_ftpUserName, _ftpPassword);

                    response = (FtpWebResponse)request.GetResponse();
                    responseStream = response.GetResponseStream();
                    reader = new StreamReader(responseStream);

                    while (!reader.EndOfStream)
                    {
                        dirs.Add(reader.ReadLine());
                    }
                }

                foreach (string dirname in dirs) // for each well
                {
                    bool bIsWell = false;

                    string dirPath = sExperimentPath + _pathSep + dirname;
                    request = (FtpWebRequest)WebRequest.Create(dirPath);
                    request.Method = WebRequestMethods.Ftp.ListDirectory;
                    request.Credentials = new NetworkCredential(_ftpUserName, _ftpPassword);

                    response = (FtpWebResponse)request.GetResponse();
                    responseStream = response.GetResponseStream();
                    reader = new StreamReader(responseStream);

                    List<string> files = new List<string>();

                    while (!reader.EndOfStream)
                    {
                        files.Add(reader.ReadLine());
                    }

                    foreach (string filename in files) //For each file/mask folder in well
                    {
                        if (filename.Equals(dirname + "_DataTable.csv")) bIsWell = true;

                        if (filename.EndsWith(" Mask"))
                        {
                            AddAlgorithmMask(filename);                            
                        }
                    }

                    if (bIsWell)
                    {
                        _wellId = GetWellId(dirname);
                        //_experimentAlgorithmWellId = GetExperimentAlgorithmWellId(_experimentId, _algorithmId, _wellId, _resultsLocation);
                        _experimentAlgorithmWellId = GetExperimentAlgorithmWellId(_wellId);

                        //if lExperimentAlgorithmWellId > 0, experiment+algorithm+well data has already been imported. Skip the well
                        if (_experimentAlgorithmWellId == 0)//experiment+algorithm+well data not already imported
                        {
                            _experimentAlgorithmWellId = AddExperimentAlgorithmWellId(_wellId);
                            string sWellMessage = ImportWellData(dirname, _experimentAlgorithmWellId,resultsLocation);
                            if (sWellMessage.StartsWith("Error:")) return sWellMessage;
                        }
                        else
                        {
                            sMessage = sMessage + ", " + dirname + " skipped";
                        }
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

        private void AddAlgorithmMask(string maskName)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT AlgorithmMaskId FROM thorlabs.algorithmmasks WHERE " +
                                    "AlgorithmId = '" + _algorithmId + "' AND " +
                                    "MaskName = '" + maskName + "'";
            MyComm.CommandText = strCommandText;
            object mask = MyComm.ExecuteScalar();

            if (mask == null)
            {
                strCommandText = "INSERT INTO thorlabs.algorithmmasks (AlgorithmId, MaskName) VALUES " +
                                 "('" + _algorithmId + "','" + maskName + "')";
                MyComm.CommandText = strCommandText;
                MyComm.ExecuteNonQuery();
            }
        }

        private long AddExperimentAlgorithmWellId(long wellId)
        {
            long experimentAlgorithmWellId = 0;

            //            string resultsLoc = sResultsLocation + sPathSep + sExperiment + sPathSep + sAlgorithm + sPathSep + wellName;
            //string resultsLoc = _resultsLocation + _pathSep + wellName;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;

            string strCommandText = "INSERT INTO thorlabs.experimentalgorithmwell " +
                                 "(ResultsId,WellId) VALUES " +
                                 "('" + _resultsId + "','" + wellId + "')";
            MyComm.CommandText = strCommandText;
            MyComm.ExecuteNonQuery();

            strCommandText = "SELECT ExperimentAlgorithmWellId FROM thorlabs.experimentalgorithmwell WHERE " + 
                             "WellId = '" + wellId + "' AND " +
                             "ResultsId = '" + _resultsId + "'";
            MyComm.CommandText = strCommandText;
            object expAlgWell = MyComm.ExecuteScalar();
            experimentAlgorithmWellId = Convert.ToInt64(expAlgWell);

            return experimentAlgorithmWellId;
        }

        private long AddExperimentAlgorithmResults()
        {
            long resId = 0;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "INSERT INTO thorlabs.experimentalgorithmresults " +
                                    "(ExperimentId, AlgorithmId, ResultsLocation) " +
                                    "VALUES ('" + _experimentId + "', " +
                                    "'" + _algorithmId + "', " + 
                                    "'" + _resultsLocation + "')";
            MyComm.CommandText = strCommandText;
            MyComm.ExecuteNonQuery();

            strCommandText = "SELECT ResultsId FROM thorlabs.experimentalgorithmresults " +
                                    "WHERE ExperimentId = '" + _experimentId + "' AND " +
                                    "AlgorithmId = '" + _algorithmId + "' AND " +
                                    "ResultsLocation = '" + _resultsLocation + "'";
            MyComm.CommandText = strCommandText;
            object results = MyComm.ExecuteScalar();
            if (results != null)
            {
                resId = Convert.ToInt64(results);
            }           

            return resId;
        }

        private long GetResultsId()
        {
            long resId = 0;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT ResultsId FROM thorlabs.experimentalgorithmresults " +
                                    "WHERE ExperimentId = '" + _experimentId + "' AND " +
                                    "AlgorithmId = '" + _algorithmId + "' AND " +
                                    "ResultsLocation = '" + _resultsLocation + "'";
            MyComm.CommandText = strCommandText;
            object results = MyComm.ExecuteScalar();
            if (results != null)
            {
                resId = Convert.ToInt64(results);
            }

            return resId;
        }

        private long GetAlgorithmId(string algorithmName)
        {
            long algId = 0;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT AlgorithmId FROM thorlabs.algorithms WHERE AlgorithmName = '" + algorithmName + "'";
            MyComm.CommandText = strCommandText;
            object algorithm = MyComm.ExecuteScalar();
            if (algorithm != null)
            {
                algId = Convert.ToInt64(algorithm);
            }

            return algId;
        }

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
                        if (ds.Tables[0].Rows[0].ItemArray[0] != DBNull.Value) _ftpUserName = ds.Tables[0].Rows[0].ItemArray[0].ToString();
                        if (ds.Tables[0].Rows[0].ItemArray[1] != DBNull.Value) _ftpPassword = ds.Tables[0].Rows[0].ItemArray[1].ToString();
                        if (ds.Tables[0].Rows[0].ItemArray[2] != DBNull.Value) _ftpLocation = "ftp://" + ds.Tables[0].Rows[0].ItemArray[2].ToString();
                    }
                }
            }
        }

        private string GetColumnNames()
        {
            string sColumns = ",";

            foreach (string columnName in _arrColumns)
            {
                ////string clmName = columnName.Replace(" ", String.Empty);
                sColumns = sColumns + "," + columnName;
            }

            sColumns = sColumns.Replace(",,", String.Empty);

            return sColumns;
        }

        private string GetColumnType(string columnName)
        {
            string type = "VARCHAR(45)";

            if (columnName.Equals("Cell ID") ||
                columnName.Contains("Left") ||
                columnName.Contains("Top") ||
                columnName.Contains("Width") ||
                columnName.Contains("Height"))
            {
                type = "INTEGER";
            }
            else if (columnName.Contains("Area") ||
                    columnName.Contains("Centroid") ||
                    columnName.Contains("Ratio") ||
                    columnName.Contains("Droplet Count") ||
                    columnName.StartsWith("TII ") ||
                    columnName.StartsWith("API ") ||
                    columnName.StartsWith("MPI ") ||
                    columnName.StartsWith("SPI ") ||
                    columnName.StartsWith("Diff: ") ||
                    columnName.Contains(" Li Pi ") ||
                    columnName.Contains("Diameter"))
            {
                type = "DOUBLE";
            }

            return type;
        }

        private string GetDataColumns()
        {
            string sColumns = String.Empty;

            foreach(string columnName in _arrColumns)
            {
                string columnType = GetColumnType(columnName);
                string clmName = columnName.Replace(" ", String.Empty);
                sColumns = sColumns + " `" + clmName + "` " + columnType + " default NULL,";
            }

            return sColumns;
        }

        private string GetDBTableNameForAlgorithm(string algorithmName,string resultsLocation)
        {
            string sTable = String.Empty;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT ResultsTable FROM thorlabs.algorithms WHERE AlgorithmId = '" + _algorithmId + "'";
            MyComm.CommandText = strCommandText;
            object table = MyComm.ExecuteScalar();
            if (table != DBNull.Value)
            {
                sTable = Convert.ToString(table);
            }
            else //Create table
            {
                string location = resultsLocation.Replace("\\", String.Empty);
                location = location.Replace("/", String.Empty);

            //                sTable = "_" + algorithmName.Replace(" ", String.Empty) + "_" + location.GetHashCode().ToString();
                sTable = algorithmName.Replace(" ", String.Empty);

                string sDataColumns = GetDataColumns();

                strCommandText = "CREATE TABLE `thorlabs`.`" + sTable + "` (" +
                                 "`DataId` bigint(20) unsigned NOT NULL auto_increment," +
                                 "`ExperimentAlgorithmWellId` bigint(20) unsigned NOT NULL," +
                                 sDataColumns +
                                 "PRIMARY KEY  (`DataId`)," +
                                 "KEY `FK_" + sTable + "` (`ExperimentAlgorithmWellId`)," +
                                 "CONSTRAINT `FK_" + sTable + "` FOREIGN KEY (`ExperimentAlgorithmWellId`) REFERENCES `experimentalgorithmwell` (`ExperimentAlgorithmWellId`)" +
                                 ") ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;";
                MyComm.CommandText = strCommandText;
                MyComm.ExecuteNonQuery();

                strCommandText = "UPDATE `thorlabs`.`algorithms` " +
                                 "SET ResultsTable = '" + sTable +
                                 "' WHERE AlgorithmId = '" + _algorithmId + "'";
                MyComm.CommandText = strCommandText;
                MyComm.ExecuteNonQuery();
            }

            return sTable;
        }

        private long GetExperimentAlgorithmWellId(long wellId)
        {
            long experimentAlgorithmWellId = 0;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT ExperimentAlgorithmWellId FROM thorlabs.experimentalgorithmwell WHERE " +
                                    "ResultsId = '" + _resultsId + "' AND " +                                    
                                    "WellId = '" + wellId + "'";
            MyComm.CommandText = strCommandText;
            object expAlgWell = MyComm.ExecuteScalar();

            if (expAlgWell != null)
            {
                experimentAlgorithmWellId = Convert.ToInt64(expAlgWell);
            }

            return experimentAlgorithmWellId;
        }

        private long GetExperimentId(string experimentName)
        {
            long expId = 0;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT experimentId FROM thorlabs.experiment WHERE name = '" + experimentName + "'";
            MyComm.CommandText = strCommandText;
            object experiment = MyComm.ExecuteScalar();
            if (experiment != null)
            {
                expId = Convert.ToInt64(experiment);
            }

            return expId;
        }

        private int GetNumberOfImagesPerMask()
        {
            int iNoOfImages = -1; // actual value DBNull

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT ImagesPerMask FROM thorlabs.algorithms WHERE AlgorithmId = '" + _algorithmId + "'";
            MyComm.CommandText = strCommandText;
            object images = MyComm.ExecuteScalar();
            if (images != DBNull.Value)
            {
                iNoOfImages = Convert.ToInt32(images);
            }

            return iNoOfImages;
        }

        private long GetWellId(string wellName)
        {
            long wlId = 0;

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "SELECT WellId FROM thorlabs.wells WHERE WellName = '" + wellName + "'";
            MyComm.CommandText = strCommandText;
            object well = MyComm.ExecuteScalar();

            if (well != null)
            {
                wlId = Convert.ToInt64(well);
            }
            else //Add well
            {
                strCommandText = "INSERT INTO thorlabs.wells (WellName) VALUES ('" + wellName + "')";
                MyComm.CommandText = strCommandText;
                MyComm.ExecuteNonQuery();

                strCommandText = "SELECT WellId FROM thorlabs.wells WHERE WellName = '" + wellName + "'";
                MyComm.CommandText = strCommandText;
                well = MyComm.ExecuteScalar();
                wlId = Convert.ToInt64(well);
            }

            return wlId;
        }

        private string ImportWellData(string sWellName, long experimentAlgorithmWellId, string resultsLocation)
        {
            string sMessage = "Success:Well data imported successfully";

            //string sCSVDataPath = ftpLocation + sPathSep
            //                    + sResultsLocation + sPathSep
            //                    + sExperiment + sPathSep
            //                    + sAlgorithm + sPathSep
            //                    + sWellName + sPathSep
            //                    + sWellName + "_DataTable.csv";

            string sCSVDataPath = _ftpLocation + _pathSep
                                + _resultsLocation + _pathSep
                                + sWellName + _pathSep
                                + sWellName + "_DataTable.csv";

            FtpWebResponse response = null;
            StreamReader reader = null;
            bool bStartImport = false;
            string tableName = String.Empty;

            try
            {
                FtpWebRequest request = (FtpWebRequest)WebRequest.Create(sCSVDataPath);
                request.Method = WebRequestMethods.Ftp.DownloadFile;
                request.Credentials = new NetworkCredential(_ftpUserName, _ftpPassword);

                response = (FtpWebResponse)request.GetResponse();
                Stream responseStream = response.GetResponseStream();
                reader = new StreamReader(responseStream);

                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText;

                while (!reader.EndOfStream)
                {
                    string line = reader.ReadLine();

                    if (bStartImport)
                    {
                        string sDataTableFields = GetColumnNames();
                        sDataTableFields = "ExperimentAlgorithmWellId," + sDataTableFields;
                        string sDataTableValues = line.Replace(",", "','");
                        sDataTableValues = "'" + experimentAlgorithmWellId + "', '" + sDataTableValues + "'";
                        sDataTableValues = sDataTableValues.Replace("''", "null");
                        //strCommandText = "INSERT INTO `thorlabs`.`" + tableName + "` (" +
                        //                 sDataTableFields +
                        //                 ") VALUES (" +
                        //                 sDataTableValues +
                        //                 ")";
                        strCommandText = "INSERT INTO `thorlabs`.`" + tableName + "` VALUES ('0'," +
                                         sDataTableValues +
                                         ");";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }

                    if (line.StartsWith("Cell ID"))
                    {
                        _arrColumns = line.Split(_csvDelim);
                        tableName = GetDBTableNameForAlgorithm(_algorithm,resultsLocation);
                        bStartImport = true;
                    }

                    if (line.Contains("X-Centroid of "))
                    {
                        char[] csvDelim = new char[1] { ',' };
                        string[] columnInfo = line.Split(csvDelim);
                        string CentroidColumn = columnInfo[1];
                        string MaskName = columnInfo[2].Replace("X-Centroid of ", String.Empty);
                        SetCentroidColumnForMask(MaskName, CentroidColumn);
                    }
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        private void SetCentroidColumnForMask(string maskName, string centroidColumn)
        {
            centroidColumn = centroidColumn.Replace(" ", String.Empty);
            string ycentroidColumn = centroidColumn.Replace("X-", "Y-");

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "UPDATE thorlabs.algorithmmasks SET " +
                                    "XCentroidColumn = '" + centroidColumn + "', " +
                                    "YCentroidColumn = '" + ycentroidColumn + "' WHERE " +
                                    "AlgorithmId = '" + _algorithmId + "' AND " +
                                    "MaskName = '" + maskName + "'";
            MyComm.CommandText = strCommandText;
            MyComm.ExecuteNonQuery();
        }

        private string SetInputParamsFromXML(string xmlInputFile)
        {
            string sMessage = "Success:Input parameters set successfully";
            FtpWebResponse response = null;
            XmlTextReader reader = null;

            try
            {
                FtpWebRequest request = (FtpWebRequest)WebRequest.Create(xmlInputFile);
                request.Method = WebRequestMethods.Ftp.DownloadFile;

                request.Credentials = new NetworkCredential(_ftpUserName, _ftpPassword);

                response = (FtpWebResponse)request.GetResponse();

                Stream responseStream = response.GetResponseStream();
                reader = new XmlTextReader(responseStream);

                while (reader.Read())
                {
                    if (reader.NodeType == XmlNodeType.Element)
                    {
                        switch (reader.Name)
                        {
                            case "Experiment":
                                _experiment = reader.GetAttribute(0);
                                break;

                            case "ResultsLocation":
                                _resultsLocation = reader.GetAttribute(0);
                                break;

                            case "Well":
                                _well = reader.GetAttribute(0);
                                break;
                        }
                    }
                }

                if (_experiment.Trim() == string.Empty) sMessage = "Error:Experiment name not specified";
                if (_resultsLocation.Trim() == string.Empty) sMessage = "Error:Results location not specified";
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            if (reader != null) reader.Close();
            if (response != null) response.Close();

            return sMessage;
        }

        private string SetParams(string experimentName,
            string algorithmName,
            string resultsLocation)
        {
            string sMessage = "Success:Parameters set successfully";

            try
            {
                _experiment = experimentName;
                _algorithm = algorithmName;
                _resultsLocation = resultsLocation.Replace("\\","/");
                if (experimentName != String.Empty) _experimentId = GetExperimentId(experimentName);
                if (algorithmName != String.Empty) _algorithmId = GetAlgorithmId(algorithmName);
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        private void UpdateAlgorithmImagesPerMask(int imagesPerMask)
        {
            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyComm = new OdbcCommand();
            MyComm.Connection = MyConn;
            string strCommandText = "UPDATE thorlabs.algorithms SET " +
                                    "ImagesPerMask = '" + imagesPerMask + "' WHERE " +
                                    "AlgorithmId = '" + _algorithmId + "'";
            MyComm.CommandText = strCommandText;
            MyComm.ExecuteNonQuery();
        }

        #endregion Methods
    }
}