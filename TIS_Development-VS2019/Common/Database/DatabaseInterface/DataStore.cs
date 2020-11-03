namespace DatabaseInterface
{
    using System;
    using System.Collections.Generic;
    using System.Data;
    using System.Data.SQLite;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    public class DataStore
    {
        #region Fields

        private static System.Data.SQLite.SQLiteConnection _connection = null;
        private static string _connectionString;
        private static DataStore _instance;

        private DataSet _batchesDataSet = new DataSet();
        private DataSet _episodesDataSset = new DataSet();
        private DataSet _experimentsDataSet = new DataSet();

        #endregion Fields

        #region Constructors

        private DataStore()
        {
        }

        #endregion Constructors

        #region Properties

        public static DataStore Instance
        {
            get
            {
                if (null == _instance)
                {
                    _instance = new DataStore();

                }
                return _instance;
            }
        }

        public DataSet BatchesDataSet
        {
            get
            {
                _batchesDataSet.Clear();

                string sql = "SELECT * FROM Batches";

                SQLiteDataAdapter da = new SQLiteDataAdapter(sql, _connection);
                da.Fill(_batchesDataSet);

                return _batchesDataSet;
            }
        }

        public string ConnectionString
        {
            get { return _connectionString; }
            set { _connectionString = value; }
        }

        public DataSet EpisodesDataSet
        {
            get
            {
                _experimentsDataSet.Clear();

                string sql = "SELECT * FROM Episodes";

                SQLiteDataAdapter da = new SQLiteDataAdapter(sql, _connection);
                da.Fill(_episodesDataSset);

                return _episodesDataSset;
            }
        }

        public DataSet ExperimentsDataSet
        {
            get
            {
                _experimentsDataSet.Clear();

                string sql = "SELECT * FROM Experiments";

                SQLiteDataAdapter da = new SQLiteDataAdapter(sql, _connection);
                da.Fill(_experimentsDataSet);

                return _experimentsDataSet;
            }
        }

        #endregion Properties

        #region Methods

        public void AddEpisode(string episodeName, string path, string batchName)
        {
            string result = this.ExecuteScalar(string.Format("select Id from Batches where Name=\"{0}\"", batchName));

            //if batch does not exist. Create a new batch
            if (result.Length <= 0)
            {
                DataTable table = new DataTable("Batches");
                DataRow row;
                DataColumn column;

                column = new DataColumn();
                column.DataType = System.Type.GetType("System.String");
                column.ColumnName = "Name";
                table.Columns.Add(column);

                row = table.NewRow();
                row["Name"] = batchName;
                table.Rows.Add(row);

                string sql = "SELECT * FROM Batches";

                SQLiteDataAdapter da = new SQLiteDataAdapter(sql, _connection);

                SQLiteCommandBuilder cb = new SQLiteCommandBuilder(da);

                da.Fill(table);
                da.Update(table);
            }

            //get the id of the batch
            result = this.ExecuteScalar(string.Format("select Id from Batches where Name=\"{0}\"", batchName));

            //Create a new experiment
            if (result.Length >= 0)
            {
                DataTable table = new DataTable("Episodes");
                DataRow row;
                DataColumn column;

                column = new DataColumn();
                column.DataType = System.Type.GetType("System.String");
                column.ColumnName = "Name";
                table.Columns.Add(column);

                column = new DataColumn();
                column.DataType = System.Type.GetType("System.Int32");
                column.ColumnName = "BatchId";
                table.Columns.Add(column);

                column = new DataColumn();
                column.DataType = System.Type.GetType("System.String");
                column.ColumnName = "Path";
                table.Columns.Add(column);

                row = table.NewRow();
                row["Name"] = episodeName;
                row["BatchId"] = result;
                row["Path"] = path;
                table.Rows.Add(row);

                string sql = "SELECT * FROM Episodes";

                SQLiteDataAdapter da = new SQLiteDataAdapter(sql, _connection);

                SQLiteCommandBuilder cb = new SQLiteCommandBuilder(da);

                da.Fill(table);
                da.Update(table);
            }
        }

        public void AddExperiment(string expName, string path, string batchName)
        {
            string result = this.ExecuteScalar(string.Format("select Id from Batches where Name=\"{0}\"", batchName));

            //if batch does not exist. Create a new batch
            if (result.Length <= 0)
            {
                DataTable table = new DataTable("Batches");
                DataRow row;
                DataColumn column;

                column = new DataColumn();
                column.DataType = System.Type.GetType("System.String");
                column.ColumnName = "Name";
                table.Columns.Add(column);

                row = table.NewRow();
                row["Name"] = batchName;
                table.Rows.Add(row);

                string sql = "SELECT * FROM Batches";

                SQLiteDataAdapter da = new SQLiteDataAdapter(sql, _connection);

                SQLiteCommandBuilder cb = new SQLiteCommandBuilder(da);

                da.Fill(table);
                da.Update(table);
            }

            //get the id of the batch
            result = this.ExecuteScalar(string.Format("select Id from Batches where Name=\"{0}\"", batchName));

            //Create a new experiment
            if (result.Length >= 0)
            {
                DataTable table = new DataTable("Experiments");
                DataRow row;
                DataColumn column;

                column = new DataColumn();
                column.DataType = System.Type.GetType("System.String");
                column.ColumnName = "Name";
                table.Columns.Add(column);

                column = new DataColumn();
                column.DataType = System.Type.GetType("System.Int32");
                column.ColumnName = "BatchId";
                table.Columns.Add(column);

                column = new DataColumn();
                column.DataType = System.Type.GetType("System.String");
                column.ColumnName = "Path";
                table.Columns.Add(column);

                row = table.NewRow();
                row["Name"] = expName;
                row["BatchId"] = result;
                row["Path"] = path;
                table.Rows.Add(row);

                string sql = "SELECT * FROM Experiments";

                SQLiteDataAdapter da = new SQLiteDataAdapter(sql, _connection);

                SQLiteCommandBuilder cb = new SQLiteCommandBuilder(da);

                da.Fill(table);
                da.Update(table);
            }
        }

        public void Close()
        {
            _connection.Close();
        }

        public void DeleteBatch(string batchName)
        {
            try
            {
               string result = this.ExecuteScalar(string.Format("select Id from Batches where Name=\"{0}\"",batchName));

               if (result.Length <= 0)
               {
                   return;
               }
               this.ExecuteNonQuery(String.Format("delete from {0} where BatchId =\"{2}\"", "Experiments", "BatchId", Convert.ToInt32(result)));
               this.ExecuteNonQuery(String.Format("delete from {0} where BatchId =\"{2}\"", "Episodes", "BatchId", Convert.ToInt32(result)));
               this.ExecuteNonQuery(String.Format("delete from {0} where Name = \"{1}\"", "Batches", batchName));
            }
            catch (Exception fail)
            {
                string str = fail.Message;
            }
        }

        public void ExecuteNonQuery(string sql)
        {
            SQLiteCommand mycommand = new SQLiteCommand(_connection);

            mycommand.CommandText = sql;

            mycommand.ExecuteNonQuery();

            return;
        }

        public string ExecuteScalar(string sql)
        {
            SQLiteCommand mycommand = new SQLiteCommand(_connection);

             mycommand.CommandText = sql;

             object value = mycommand.ExecuteScalar();

             if (value != null)

             {

             return value.ToString();

             }

             return "";
        }

        public string GetBatchName()
        {
            return "None";
        }

        public void Open()
        {
            string str = _connectionString;
            //in the event that the database is on a network drive
            //additional slashes must be added to the connection string
            if (str.StartsWith("URI=file:\\\\"))
            {
                str = str.Replace("URI=file:\\\\", "URI=file:\\\\\\\\");
            }

            _connection = new System.Data.SQLite.SQLiteConnection(str);

               _connection.Open();
        }

        #endregion Methods
    }
}