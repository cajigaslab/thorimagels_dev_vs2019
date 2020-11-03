using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.Odbc;

namespace ThorDataAccessLayer
{
    public class GatingParameterManager
    {
        #region Data manipulation functions

        public string addGate(string name, string description, List<GatingParameter> gatingParameters)
        {
            string sMessage = "Value:";
            if (name != null) name = name.Trim();

            try
            { 
                if (!String.IsNullOrEmpty(name))
                {
                    long gateId = 0;

                    OdbcConnection MyConn = Connection.GetConnection();
                    OdbcCommand MyComm = new OdbcCommand();
                    MyComm.Connection = MyConn;
                    string strCommandText = "SELECT GateId FROM thorlabs.gates " +
                                            "WHERE GateName = '" + name + "'";
                    MyComm.CommandText = strCommandText;
                    object gate = MyComm.ExecuteScalar();
                    if (gate == null)
                    {
                        strCommandText = "INSERT INTO thorlabs.gates " +
                                                "(GateName, Description) " +
                                                "VALUES ('" + name + "', " +
                                                "'" + description + "')";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();

                        strCommandText = "SELECT GateId FROM thorlabs.gates " +
                                                "WHERE GateName = '" + name + "'";
                        MyComm.CommandText = strCommandText;
                        gate = MyComm.ExecuteScalar();
                        if (gate != null)
                        {
                            gateId = Convert.ToInt64(gate);
                        }

                        //Add parameters
                        string status = AddGatingParameters(gateId, gatingParameters);
                        if (status.StartsWith("Success:"))
                            sMessage = "Value:" + gateId.ToString();
                        else
                            sMessage = status;
                    }
                    else
                    {
                        sMessage = "Error:Gate name already in use";
                    }
                }
                else
                {
                    sMessage = "Error:Gate name invalid";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        public string updateGate(long gateId, Gate gate)
        {
            string sMessage = "Success:Gate updated successfully";

            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComm = new OdbcCommand();
                MyComm.Connection = MyConn;
                string strCommandText = "SELECT GateId FROM thorlabs.gates " +
                                        "WHERE GateId = '" + gateId + "'";
                MyComm.CommandText = strCommandText;
                object gt = MyComm.ExecuteScalar();
                if (gt != null)
                {
                    if (!String.IsNullOrEmpty(gate.GateName.Trim()))
                    {
                        strCommandText = "SELECT GateId FROM thorlabs.gates " +
                                         "WHERE GateName = '" + gate.GateName.Trim() + "' AND " +
                                         "GateId = '" + gateId + "'";
                        MyComm.CommandText = strCommandText;
                        gt = MyComm.ExecuteScalar();
                        if (gt != null)
                        {
                            //Update gate
                            strCommandText = "UPDATE thorlabs.gates " +
                                             "SET GateName = '" + gate.GateName.Trim() + "', " +
                                             "Description = '" + gate.Description + "' " +
                                             "WHERE GateId = '" + gateId + "'";
                            MyComm.CommandText = strCommandText;
                            MyComm.ExecuteNonQuery();

                            //Delete previous parameters
                            strCommandText = "DELETE FROM thorlabs.gatingparameters " +
                                             "WHERE GateId = '" + gateId + "'";
                            MyComm.CommandText = strCommandText;
                            MyComm.ExecuteNonQuery();

                            //Add new parameters
                            string status = AddGatingParameters(gateId, gate.GatingParameters);
                            if (status.StartsWith("Success:"))
                                sMessage = "Success:Gate updated successfully";
                            else
                                sMessage = status;
                        }
                        else
                        {
                            sMessage = "Error:Gate name already in use";
                        }
                    }
                    else
                    {
                        sMessage = "Error:Gate name invalid";
                    }
                }
                else
                {
                    sMessage = "Error:GateId invalid";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        public string deleteGate(string name)
        {
            string sMessage = "Success:Gate deleted successfully";
            if (name != null) name = name.Trim();

            try
            {
                if (!String.IsNullOrEmpty(name))
                {
                    long gateId = 0;

                    OdbcConnection MyConn = Connection.GetConnection();
                    OdbcCommand MyComm = new OdbcCommand();
                    MyComm.Connection = MyConn;
                    string strCommandText = "SELECT GateId FROM thorlabs.gates " +
                                            "WHERE GateName = '" + name + "'";
                    MyComm.CommandText = strCommandText;
                    object gate = MyComm.ExecuteScalar();
                    if (gate != null)
                    {
                        gateId = Convert.ToInt64(gate);
                        strCommandText = "DELETE FROM thorlabs.gates " +
                                         "WHERE GateId = '" + gateId + "'";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                    else
                    {
                        sMessage = "Error:Gate does not exist";
                    }
                }
                else
                {
                    sMessage = "Error:Gate name invalid";
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }

        #endregion

        #region Query functions

        public List<Gate> getAllGates()
        {
            string strCommandText = "SELECT * FROM `thorlabs`.`gates` ORDER BY GateId";
            OdbcDataReader dataReader = null;
            List<Gate> gateList = null;
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComd = new OdbcCommand();
                MyComd.Connection = MyConn;
                MyComd.CommandText = strCommandText;
                dataReader = MyComd.ExecuteReader();
                
                if (dataReader.HasRows)
                {                    
                    gateList = new List<Gate>();
                    Gate gateData;
                    while (dataReader.Read())
                    {
                        gateData= new Gate();
                        List<GatingParameter> listGatingParameter = null;
                        gateData.GateId = Convert.ToInt64(dataReader["GateId"]);
                        gateData.GateName = Convert.ToString(dataReader["GateName"]);
                        gateData.Description = Convert.ToString(dataReader["Description"]);
                        //Function call to get list of GatingParameter Objects associated with current Gate.
                        listGatingParameter = getGateParametersList(gateData.GateId);
                        gateData.GatingParameters = listGatingParameter;
                        gateList.Add(gateData);
                    }
                }
                dataReader.Close();
            }
            catch
            {
                gateList = null;
            }
            return gateList;
        }
     
        public Gate getGate(string name)
        {
            Gate gateObject = null;

            if (string.IsNullOrEmpty(name))
            {
                return gateObject;
            }

            string strCommandText = "SELECT * FROM `thorlabs`.`gates` Where GateName like ('" + name + "')";
            OdbcDataReader dataReader = null;
        
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComd = new OdbcCommand();
                MyComd.Connection = MyConn;
                MyComd.CommandText = strCommandText;
                dataReader = MyComd.ExecuteReader();
                if (dataReader.HasRows)
                {
                    gateObject = new Gate();
                    List<GatingParameter> listGatingParameter = null;
                    gateObject.GateId = Convert.ToInt64(dataReader["GateId"]);
                    gateObject.GateName = Convert.ToString(dataReader["GateName"]);
                    gateObject.Description = Convert.ToString(dataReader["Description"]);
                    //Function call to get list of GatingParameter Objects associated with current Gate.
                    listGatingParameter = getGateParametersList(gateObject.GateId);
                    gateObject.GatingParameters = listGatingParameter; 
                }
                dataReader.Close();
            }
            catch
            {
                gateObject = null;
            }
            return gateObject;
        }

        public List<Gate> getGatesOfDescription(string description)
        {
            List<Gate> gateList = null;

            if (string.IsNullOrEmpty(description))
            {
                return gateList;
            }
            else
            {                
                if(string.IsNullOrEmpty(description.Trim()))
                    return gateList;
            }

           string strCommandText = "SELECT * FROM `thorlabs`.`gates` Where Description like ('%" +
                                    description + "%') ORDER BY GateId";
            OdbcDataReader dataReader = null;

            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComd = new OdbcCommand();
                MyComd.Connection = MyConn;
                MyComd.CommandText = strCommandText;
                dataReader = MyComd.ExecuteReader();
                if (dataReader.HasRows)
                {
                    gateList = new List<Gate>();
                    while (dataReader.Read())
                    {
                        Gate gateData = new Gate();
                        List<GatingParameter> listGatingParameter = null;
                        gateData.GateId = Convert.ToInt64(dataReader["GateId"]);
                        gateData.GateName = Convert.ToString(dataReader["GateName"]);
                        gateData.Description = Convert.ToString(dataReader["Description"]);
                        //Function call to get list of GatingParameter Objects associated with current Gate.
                        listGatingParameter = getGateParametersList(gateData.GateId);
                        gateData.GatingParameters = listGatingParameter;
                        gateList.Add(gateData);
                    }
                }
                dataReader.Close();
            }
            catch
            {
                gateList = null;
            }
            return gateList;
        }

        public long getGateId(string name)
        {
            long gateID = 0;

            if (string.IsNullOrEmpty(name))
            {
                return gateID;
            }

            string strCommandText = "SELECT * FROM `thorlabs`.`gates` Where GateName like ('" + name + "')";
           
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComd = new OdbcCommand();
                MyComd.Connection = MyConn;
                MyComd.CommandText = strCommandText;
                object objectForGateID  = MyComd.ExecuteScalar();
                if (objectForGateID != null)
                {
                   gateID = Convert.ToInt64(objectForGateID);
                }
            }    
            catch
            {
                gateID = 0;
            }
            return gateID;
        }

        public List<string> getAllParameters()
        {
            string strCommandText = "select column_Name from information_schema.columns where Table_Name='colocalization'";
            OdbcDataReader dataReader = null;
            List<string> gateList = null;
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComd = new OdbcCommand();
                MyComd.Connection = MyConn;
                MyComd.CommandText = strCommandText;
                dataReader = MyComd.ExecuteReader();

                if (dataReader.HasRows)
                {
                    gateList = new List<string>();
                    string gateData;
                    while (dataReader.Read())
                    {
                        gateData = Convert.ToString(dataReader["column_Name"]);
                        gateList.Add(gateData);
                    }
                }
                dataReader.Close();
            }
            catch
            {
                gateList = null;
            }
            return gateList;
        }

        public int[] getParamValues(string queryString)
        {
            string strCommandText = queryString;
            OdbcDataReader dataReader = null;
            List<int> gateResult = null;
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComd = new OdbcCommand();
                MyComd.Connection = MyConn;
                MyComd.CommandText = strCommandText;
                dataReader = MyComd.ExecuteReader();
                gateResult = new List<int>();
                if (dataReader.HasRows)
                {
                  
                    int gateData;
                    while (dataReader.Read())
                    {
                        try
                        {
                            gateData = Convert.ToInt32(dataReader[0]);
                        
                            gateResult.Add(gateData);
                        }
                        catch
                        {

                        }
                    }
                }
                dataReader.Close();
            }
            catch
            {
                gateResult = null;
            }
            return gateResult.ToArray();
        }

        #endregion

        #region Private functions

        private string AddGatingParameters(long gateId, List<GatingParameter> gatingParameters)
        {
            string sMessage = "Success:Parameters added successfully";

            try
            {
                if (gatingParameters != null && gatingParameters.Count > 0)
                {
                    OdbcConnection MyConn = Connection.GetConnection();
                    OdbcCommand MyComm = new OdbcCommand();
                    MyComm.Connection = MyConn;

                    string parameters = "Statistic, RangeType, RangeValueFrom,RangeValueTo,GateId";
                    foreach (GatingParameter gp in gatingParameters)
                    {
                        string parameterValues = "'" + gp.Statistic + "', " +
                                                 "'" + Convert.ToInt32(gp.RangeType) + "', " +
                                                 "'" + gp.RangeValueFrom + "', " +
                                                  "'" + gp.RangeValueTo + "', " +
                                                 "'" + gateId.ToString() + "'";
                        string strCommandText = "INSERT INTO thorlabs.gatingparameters " +
                                                "(" + parameters + ") " +
                                                "VALUES " +
                                                "(" + parameterValues + ")";
                        MyComm.CommandText = strCommandText;
                        MyComm.ExecuteNonQuery();
                    }
                }
            }
            catch (Exception ex)
            {
                sMessage = "Error:" + ex.Message;
            }

            return sMessage;
        }        

        private List<GatingParameter> getGateParametersList(long Id)
        {
            List<GatingParameter> listGatingParameter = null;
            try
            {
                OdbcCommand cmdGatingParameter = new OdbcCommand();
                OdbcDataReader readerForGatingParameter = null;

                string strGatingParameter = "SELECT * FROM `thorlabs`.`gatingparameters` WHERE GateId = "
                    + Id + " ORDER BY ParameterId";

                OdbcConnection MyConn = Connection.GetConnection();
                cmdGatingParameter.Connection = MyConn;
                cmdGatingParameter.CommandText = strGatingParameter;
                readerForGatingParameter = cmdGatingParameter.ExecuteReader();

                if (readerForGatingParameter.HasRows)
                {
                    listGatingParameter = new List<GatingParameter>();
                    GatingParameter GatingParameters = null;
                    while (readerForGatingParameter.Read())
                    {
                        GatingParameters = new GatingParameter();
                        GatingParameters.ParameterId = Convert.ToInt64(readerForGatingParameter["ParameterId"]);
                        GatingParameters.Statistic = Convert.ToString(readerForGatingParameter["Statistic"]);
                        GatingParameters.RangeType = (RangeTypes)(readerForGatingParameter["RangeType"]);
                        GatingParameters.RangeValueFrom = Convert.ToString(readerForGatingParameter["RangeValueFrom"]);
                        GatingParameters.RangeValueTo = Convert.ToString(readerForGatingParameter["RangeValueTo"]);
                        GatingParameters.GeteId = Convert.ToInt64(readerForGatingParameter["GateId"]);
                        listGatingParameter.Add(GatingParameters);
                    }
                }
                else
                {
                    return listGatingParameter;
                }
            }
            catch
            {
                listGatingParameter = null;
            }

            return listGatingParameter;
        }

        #endregion
    }
}
