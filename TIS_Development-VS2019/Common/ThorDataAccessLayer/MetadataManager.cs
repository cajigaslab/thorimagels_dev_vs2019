using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.Odbc;

namespace ThorDataAccessLayer
{
    public class MetadataManager
    {

       #region Public Methods

        #region Metadata 

        public string addMetadata(Metadata.MetaDataType metaDatatype, string metaDataName)
        {
           string sMessage = "Success:Metadata added successfully";
           
           if(string.IsNullOrEmpty(metaDataName))
            {
                sMessage = "Error:MetadataName invalid";
                return sMessage;
            }

           try
            {
               // Check if Matadata name already exists.
                 double MetaDataId = getMetaDataId(metaDataName);
                 if (MetaDataId > 0)
                 {
                     sMessage = "Error:MetadataName already exists";
                     return sMessage;
                 }
                 else
                 {
                     OdbcConnection MyConn = Connection.GetConnection();
                     OdbcCommand MyCmd = new OdbcCommand();
                     MyCmd.Connection = MyConn;
                     string strCommand = "INSERT INTO`thorlabs`.`metadata`(MetaDataType,MetaDataName)" + 
                                          "Values (" + (int)metaDatatype + ",'"+ metaDataName.Trim()+ "')";
                     MyCmd.CommandText = strCommand;
                     MyCmd.ExecuteNonQuery();
                     long metadataId = getMetaDataId(metaDataName.Trim());
                     return "Value: " + metadataId.ToString();                     
                 }                 
            }
            catch(Exception ex)
            {
                return "Error:" + ex.Message;
            }
        }

        public string deleteMetadata(string metaDataName)
        {
            string message;
            if(string.IsNullOrEmpty(metaDataName))
            {
                message = "Error: Invalid MetadataName";
                return message;
            }

            try
            {
                //Check If MetadataID present
                long MetaDataId = getMetaDataId(metaDataName);
                if (MetaDataId > 0)
                {
                    //Delete Record From Child Table
                    deleteFromWellMetadata(MetaDataId);

                    OdbcConnection MyConn = Connection.GetConnection();
                    OdbcCommand MyCmd = new OdbcCommand();
                    MyCmd.Connection = MyConn;
                    string strCommand = "DELETE FROM `thorlabs`.`metadata` where MetaDataName like ('"
                                        + metaDataName.Trim() + "')";
                    MyCmd.CommandText = strCommand;
                    MyCmd.ExecuteNonQuery();
                    message = "Success : " + metaDataName + " deleted !";
                    return message;
                }
                else
                {
                    message = "Error : " + metaDataName + " does not exist!";
                    return message;
                }
            }
            catch (Exception ex)
            {
                message = "Error : " + ex.Message;
                return message;
            }
        }

        public List<Metadata> getAllMetadata()
        {
            string strCommandText = "SELECT * FROM `thorlabs`.`metadata` ORDER BY MetaDataId";
            OdbcDataReader dataReader = null;
            List<Metadata> metadataList = null;
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComd = new OdbcCommand();
                MyComd.Connection = MyConn;
                MyComd.CommandText = strCommandText;
                dataReader = MyComd.ExecuteReader();
                if (dataReader != null)
                {
                    metadataList = new List<Metadata>();
                    while (dataReader.Read())
                    {
                        Metadata data = new Metadata();
                        data.metadataId = Convert.ToInt64(dataReader["MetaDataId"]);
                        data.metadataName = Convert.ToString(dataReader["MetaDataName"]);
                        data.metadataType = Convert.ToInt32(dataReader["MetaDataType"]);
                        metadataList.Add(data);
                    }
                }
                dataReader.Close();
            }
            catch
            {
                metadataList = null;
            }
            return metadataList;
        }

        public List<Metadata> getMetadataForType(ThorDataAccessLayer.Metadata.MetaDataType type)
        {
            string strCommandText = "SELECT * FROM `thorlabs`.`metadata` WHERE MetaDataType = "
                                    + Convert.ToInt32(type) + " ORDER BY MetaDataId ";
            OdbcDataReader dataReader = null;
            List<Metadata> metadataList = null;
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComd = new OdbcCommand();
                MyComd.Connection = MyConn;
                MyComd.CommandText = strCommandText;
                dataReader = MyComd.ExecuteReader();
                if (dataReader != null)
                {
                    metadataList = new List<Metadata>();
                    while (dataReader.Read())
                    {
                        Metadata data = new Metadata();
                        data.metadataId = Convert.ToInt64(dataReader["MetaDataId"]);
                        data.metadataName = Convert.ToString(dataReader["MetaDataName"]);
                        data.metadataType = Convert.ToInt32(dataReader["MetaDataType"]);
                        metadataList.Add(data);
                    }
                }
                dataReader.Close();
            }
            catch
            {
                metadataList = null;
            }
            return metadataList;
        }

        public long getMetaDataId(string metaDataName)
        {
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyCmd = new OdbcCommand();
                MyCmd.Connection = MyConn;
                string strCommand = "SELECT MetaDataId FROM `thorlabs`.`metadata` " +
                                    "WHERE MetaDataName like('" + metaDataName + "')";
                MyCmd.CommandText = strCommand;
                object Id = MyCmd.ExecuteScalar();
                if (Id != null)
                {
                    long MetadataId = Convert.ToInt64(Id);
                    return MetadataId;
                }
                else
                {
                    return 0;
                }
            }
            catch
            {
                return 0;
            }
        }

        #endregion

        #region WellMetadata

        public string addWellMetadata(long metadataId, string value, long[] ExperimentAlgorithmWellIds)
        {
            string message = "Success : Well metadata inserted successfully for wells:  ";
            string invalidExperimentAlgorithmWellId = "Invalid ExperimentAlgorithmWellIds: ";
            string skippedRecords = "Metadata already present for ExperimentAlgorithmWellIds: ";

            #region Validations
            if (string.IsNullOrEmpty(value))
            {
                message = "Error : Invalid Parameter MetadataValue";
                return message;
            }

            //Check Parameter For Not Null And Empty            
            if (ExperimentAlgorithmWellIds == null || ExperimentAlgorithmWellIds.Length == 0)
            {
                message = "Error : Invalid Parameter ExperimentAlgoritmWellIds";
                return message;
            }                                        

            //Check IF records Exists in Parent/Master Table
            bool isMatadataIdValid = ValidateMatadataId(metadataId);
            if (!isMatadataIdValid)
            {
                message = "Error : Invalid Matadata ID";
                return message;
            }

            #endregion

            foreach (long Id in ExperimentAlgorithmWellIds)
            {
                //Check If ExperimentAlgorithmWellId Present in experimentalgorithmwell Table
                bool isWellIdValid = ValidateExperimentAlgorithmWellId(Id);
                string strCommand= null;
                if (isWellIdValid)
                {
                    try
                    {
                        OdbcConnection MyConn = Connection.GetConnection();
                        OdbcCommand MyCmd = new OdbcCommand();
                        MyCmd.Connection = MyConn;
                        OdbcDataReader reader = null;
                        //check if record already exists
                        string IsRecordPresent = "SELECT * FROM `thorlabs`.`wellmetadata` WHERE MetaDataId = " 
                                                + metadataId  + " AND ExperimentAlgorithmWellId = " + Id;
                        MyCmd.CommandText = IsRecordPresent;
                        reader = MyCmd.ExecuteReader();

                        //if record not present insert
                        if (!reader.HasRows)
                        {
                           reader.Close();
                           strCommand = "INSERT INTO`thorlabs`.`wellmetadata`(MetaDataId," +
                                                "ExperimentAlgorithmWellId,MetaDataValue) Values (" + metadataId +
                                                "," + Id + ",'" + value + "')";
                            MyCmd.CommandText = strCommand;
                            MyCmd.ExecuteNonQuery();
                            message += Id.ToString() + ", ";
                        }
                        else
                        {
                            skippedRecords +=  Id.ToString() + ", ";
                        }
                    }
                    catch (Exception ex)
                    {
                      
                        message = "Error :" + ex.Message;
                    }
                }
                else
                {
                    invalidExperimentAlgorithmWellId += Id.ToString() + ", ";
                }
            }

            message = message.Remove(message.Length - 2, 2); message = message + "; ";
            invalidExperimentAlgorithmWellId = invalidExperimentAlgorithmWellId.Remove(invalidExperimentAlgorithmWellId.Length - 2, 2);
            invalidExperimentAlgorithmWellId = invalidExperimentAlgorithmWellId + "; ";
            skippedRecords = skippedRecords.Remove(skippedRecords.Length - 2, 2); skippedRecords = skippedRecords + "; ";
            
            return message + invalidExperimentAlgorithmWellId + skippedRecords;
                              
        }

        public string updateWellMetadata(long metadataId, string value, long[] 
                                         ExperimentAlgorithmWellIds)
        {
            string message = " Success : Record Updated";
            string invalidExperimentAlgorithmWellId = "Invalid Id: ";

            #region Validation
            if (string.IsNullOrEmpty(value))
            {
                message = "Error : Invalid MetadataName";
                return message;
            }

            if (ExperimentAlgorithmWellIds == null || ExperimentAlgorithmWellIds.Length == 0)
            {
                message = "Error : Invalid ExperimentAlgoritmWellIds";
                return message;
            }
            
            bool isMatadataIdValid = ValidateMatadataId(metadataId);
            if (!isMatadataIdValid)
            {
                message = "Error : Incorrect Matadata ID";
                return message;
            }
            #endregion

            foreach (long Id in ExperimentAlgorithmWellIds)
            {
                bool isWellIdValid = ValidateExperimentAlgorithmWellId(Id);
                if (isWellIdValid)
                {
                    try
                    {
                        OdbcConnection MyConn = Connection.GetConnection();
                        OdbcCommand MyCmd = new OdbcCommand();
                        MyCmd.Connection = MyConn;
                        string strCommand = "UPDATE `thorlabs`.`wellmetadata` SET MetaDataValue = '"
                                            + value + "' where MetaDataId = " + metadataId +
                                            "&& ExperimentAlgorithmWellId = " + Id;
                        MyCmd.CommandText = strCommand;
                        MyCmd.ExecuteNonQuery();
                        message += Id.ToString() + ", ";
                    }
                    catch (Exception ex)
                    {
                        message = "Error :" + ex.Message;
                    }
                }
                else
                {
                    invalidExperimentAlgorithmWellId += Id.ToString() + ", ";
                }
            }

            return message + invalidExperimentAlgorithmWellId;
       }                             

        public string deleteWellMetaData(long ExperimentAlgorithmWellId, long[] MetadataIds)
        {
            string message= String.Empty;
            string strCommand = String.Empty;
            string metadataId = string.Empty;
            bool isMatadataIdValid = false;            
            int rowsAffected=0;


            //Validation of arguments
            #region Validation
            bool isExperimentAlgorithmWellIdValid = isWellIdPresentInWellMetadata(ExperimentAlgorithmWellId);
            if (!isExperimentAlgorithmWellIdValid)
            {
                message = "Error : Invalid ExperimentAlgoritmWellId";
                return message;
            }

            if (MetadataIds != null && MetadataIds.Length != 0)
            {
                isMatadataIdValid = true;
                foreach (long ID in MetadataIds)
                {
                    metadataId += Convert.ToString(ID) + ",";
                }
                metadataId = metadataId.Substring(0, metadataId.Length - 1);
            }
            else
            {
                isMatadataIdValid = false;
            }
            #endregion

            //QueryFromation
            #region QueryFormation
            if (isMatadataIdValid == true && isExperimentAlgorithmWellIdValid)
            {
                strCommand = "DELETE FROM `thorlabs`.`wellmetadata` WHERE ExperimentAlgorithmWellId = "
                                + ExperimentAlgorithmWellId + " AND MetaDataId IN  (" + metadataId + " )";
            }
            else 
            {
                strCommand = "DELETE FROM `thorlabs`.`wellmetadata` WHERE ExperimentAlgorithmWellId = "
                              + ExperimentAlgorithmWellId;
            }
            #endregion

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();
            MyCmd.Connection = MyConn;
            MyCmd.CommandText = strCommand;
            rowsAffected = MyCmd.ExecuteNonQuery();
            message = "Success : " + rowsAffected + " Rows Deleted !";                
            return message;

        }

        public string deleteWellMetaData(long metadataId, 
                                        long[] ExperimentAlgorithmWellIds, 
                                        string[] values)
        {
            string status = string.Empty;
            string AlgorithmWellId = string.Empty;
            string value = string.Empty;
            string strCommand = string.Empty;
            int rowsAffected = 0;

            #region Validations

            bool isMatadataIdValid = ValidateMatadataIdInWellMetadataTable(metadataId);
            if (!isMatadataIdValid)
                return "Error : Invalid Parameter MetadataId";

            bool isExperimentAlgorithmWellIdsValid;
            bool isValueArrayValid;

            if (ExperimentAlgorithmWellIds != null && ExperimentAlgorithmWellIds.Length != 0)
            {
                isExperimentAlgorithmWellIdsValid = true;
                foreach (long id in ExperimentAlgorithmWellIds)
                {
                    AlgorithmWellId += Convert.ToString(id) + ",";
                }
                AlgorithmWellId = AlgorithmWellId.Substring(0, AlgorithmWellId.Length - 1);
            }
            else
            {
                isExperimentAlgorithmWellIdsValid = false;
            }

            if (values != null && values.Length != 0)
            {
                isValueArrayValid = true;
                foreach (string val in values)
                {
                    value += "'" + val + "',";
                }
                value = value.Substring(0, value.Length - 1);
            }
            else
            {
                isValueArrayValid = false;
            }

            #endregion

            #region QueryFormation

            if (isValueArrayValid && isExperimentAlgorithmWellIdsValid)
            {
                strCommand = "DELETE FROM `thorlabs`.`wellmetadata` where MetaDataId ="
                              + metadataId + " AND ExperimentAlgorithmWellId IN("
                              + AlgorithmWellId + ") AND MetaDataValue IN (" + value + " ) ";
            }
            else if (isValueArrayValid == false && isExperimentAlgorithmWellIdsValid == false)
            {
                strCommand = "DELETE FROM `thorlabs`.`wellmetadata` where MetaDataId = "
                            + metadataId;
            }
            else if (isExperimentAlgorithmWellIdsValid)
            {
                strCommand = "DELETE FROM `thorlabs`.`wellmetadata` where MetaDataId ="
                             + metadataId + " AND ExperimentAlgorithmWellId IN("
                             + AlgorithmWellId + ")";
            }
            else if (isValueArrayValid)
            {
                strCommand = "DELETE FROM `thorlabs`.`wellmetadata` where MetaDataId = "
                             + metadataId + " AND MetaDataValue IN (" + value + " ) ";
            }

            #endregion

            OdbcConnection MyConn = Connection.GetConnection();
            OdbcCommand MyCmd = new OdbcCommand();
            MyCmd.Connection = MyConn;
            MyCmd.CommandText = strCommand;
            rowsAffected = MyCmd.ExecuteNonQuery();
            status = "Success : " + rowsAffected + " Rows Deleted !";
            return status;
        }
         
        public List<WellMatadata> getMetadataForWell(long[] experimentAlgorithmWellIds)
        {
            string ExperimentAlgorithmWellIds = string.Empty;
            OdbcDataReader dataReader = null;
            List<WellMatadata> wellMetadataList = null;

            //validating experimentAlgorithmWellIds array
            if (experimentAlgorithmWellIds == null || experimentAlgorithmWellIds.Length == 0)
            {
                return wellMetadataList;
            }

            foreach (long id in experimentAlgorithmWellIds)
            {
                ExperimentAlgorithmWellIds += Convert.ToString(id) + ",";
            }
            ExperimentAlgorithmWellIds = ExperimentAlgorithmWellIds.Substring(0, ExperimentAlgorithmWellIds.Length - 1);
            string strCommandText = "SELECT * FROM `thorlabs`.`wellmetadata` " + 
                                    "WHERE  ExperimentAlgorithmWellId IN (" + ExperimentAlgorithmWellIds 
                                    + ") Order By ExperimentAlgorithmWellId ";

            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComd = new OdbcCommand();
                MyComd.Connection = MyConn;
                MyComd.CommandText = strCommandText;
                dataReader = MyComd.ExecuteReader();
                if (dataReader != null)
                {
                    wellMetadataList = new List<WellMatadata>();
                    while (dataReader.Read())
                    {
                        WellMatadata data = new WellMatadata();
                        data.MetaDataId = Convert.ToInt64(dataReader["MetaDataId"]);
                        data.ExperimentAlgorithmWellId = Convert.ToInt64(dataReader["ExperimentAlgorithmWellId"]);
                        data.MetaDataValue = Convert.ToString(dataReader["MetaDataValue"]);
                        wellMetadataList.Add(data);
                    }
                }
                dataReader.Close();
            }
            catch 
            {                            
                wellMetadataList = null;
            }

        return wellMetadataList;  
        }

        public List<WellMatadata> getWellsForMetadata(long MetadataId, string[] values)
        {
            string valueForQuery = string.Empty;
            string strCommandText= string.Empty;
            List<WellMatadata> wellMetadataList = null;
            
            #region Validation
            bool isMetadataIdValid = ValidateMatadataId(MetadataId);
            bool isArrayValid; 

            if (values == null || values.Length == 0)
            {
                isArrayValid = false;
            }
            else
            {
                isArrayValid = true;
                foreach (string vals in values)
                {
                    valueForQuery += "'" + vals + "' ,";
                }
                valueForQuery = valueForQuery.Substring(0, valueForQuery.Length - 1);
            }
            #endregion

            #region Query Formation
            if (isMetadataIdValid == true && isArrayValid == true)
            {                                  
               strCommandText = "SELECT * FROM `thorlabs`.`wellmetadata` WHERE  MetaDataId = " 
                                + MetadataId + " AND MetaDataValue IN (" + valueForQuery
                                + ") Order By ExperimentAlgorithmWellId ";
            }
            else if(isMetadataIdValid == false && isArrayValid == false)
            {
                return wellMetadataList;
            }
            else if (isArrayValid == true)
            {
                valueForQuery = valueForQuery.Substring(0, valueForQuery.Length - 1);
                strCommandText = "SELECT * FROM `thorlabs`.`wellmetadata` WHERE  MetaDataValue IN("
                                 + valueForQuery + ") Order By ExperimentAlgorithmWellId ";
            }
            else if (isMetadataIdValid == true)
            {

                strCommandText = "SELECT * FROM `thorlabs`.`wellmetadata` WHERE  MetaDataId = "
                                + MetadataId + " Order By ExperimentAlgorithmWellId ";
            }

            #endregion
            // Query Execution 
            
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyComd = new OdbcCommand();
                OdbcDataReader dataReader = null;
                MyComd.Connection = MyConn;
                MyComd.CommandText = strCommandText;
                dataReader = MyComd.ExecuteReader();
                if (dataReader != null)
                {
                    wellMetadataList = new List<WellMatadata>();
                    while (dataReader.Read())
                    {
                        WellMatadata data = new WellMatadata();
                        data.MetaDataId = Convert.ToInt64(dataReader["MetaDataId"]);
                        data.ExperimentAlgorithmWellId = Convert.ToInt64(dataReader["ExperimentAlgorithmWellId"]);
                        data.MetaDataValue = Convert.ToString(dataReader["MetaDataValue"]);
                        wellMetadataList.Add(data);
                    }
                }
                dataReader.Close();               
            }
            catch
            {
                wellMetadataList = null;
            }
         return wellMetadataList;
        }
        #endregion

        #endregion

       #region PrivateMethods

        #region GeneralValidations

        //private bool validateString(string value)
        //{
        //    if (string.IsNullOrEmpty(value.Trim()))
        //        return false;
        //    else
        //        return true;
        //}

        //private bool validateLongArray(long[] array)
        //{
        //    bool isArrayValueValid = true;

        //    if (array != null)
        //    {
        //        if (array.Length == 0)
        //        {
        //            isArrayValueValid = false;
        //        }
        //    }
        //    else
        //    {
        //        isArrayValueValid = false;
        //    }

        //    return isArrayValueValid;
        //}

        //private bool validateStringArray(string[] array)
        //{
        //    bool isArrayValueValid = true;

        //    if (array != null)
        //    {
        //        if (array.Length == 0)
        //        {
        //            isArrayValueValid = false;
        //        }
        //    }
        //    else
        //    {
        //        isArrayValueValid = false;
        //    }

        //    return isArrayValueValid;
        //}

        #endregion

        #region PrivateFunctionsForMetadata                

        private int deleteFromWellMetadata(double MetaDataId)
        {
            int rowsAffected = 0;
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyCmd = new OdbcCommand();
                MyCmd.Connection = MyConn;
                string strCommand = "DELETE FROM `thorlabs`.`wellmetadata` where MetaDataId ="
                                    + MetaDataId;
                MyCmd.CommandText = strCommand;
                rowsAffected = MyCmd.ExecuteNonQuery();
            }
            catch
            {
                rowsAffected = 0;
            }
            return rowsAffected;
        }

        private bool ValidateMatadataId(long metadataId)
        {
            OdbcDataReader reader;
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyCmd = new OdbcCommand();
                MyCmd.Connection = MyConn;
                string strCommand = "SELECT * FROM `thorlabs`.`metadata` WHERE MetaDataId ="
                                    + metadataId ;
                MyCmd.CommandText = strCommand;
                reader = MyCmd.ExecuteReader();
                if (reader.HasRows)
                {
                    reader.Close();
                    return true;
                }
                else
                {
                    reader.Close();
                    return false;
                }
            }
            catch
            {
                return false;
            }

        }

        #endregion

        #region PrivateFunctionsForWellMetadata

        private bool ValidateExperimentAlgorithmWellId(long Id)
        {
            OdbcDataReader reader;
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyCmd = new OdbcCommand();
                MyCmd.Connection = MyConn;
                string strCommand = "SELECT * FROM `thorlabs`.`experimentalgorithmwell` "
                                    + "WHERE ExperimentAlgorithmWellId =" + Id ;
                MyCmd.CommandText = strCommand;
                reader = MyCmd.ExecuteReader();
                if (reader.HasRows)
                {
                    reader.Close();
                    return true;
                }
                else
                {
                    reader.Close();
                    return false;
                }
            }
            catch
            {
                return false;
            }
        }           

        private bool isWellIdPresentInWellMetadata(long Id)
        {
            OdbcDataReader reader;
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyCmd = new OdbcCommand();
                MyCmd.Connection = MyConn;
                string strCommand = "SELECT * FROM `thorlabs`.`wellmetadata` "
                                    + "WHERE ExperimentAlgorithmWellId =" + Id;
                MyCmd.CommandText = strCommand;
                reader = MyCmd.ExecuteReader();
                if (reader.HasRows)
                {
                    reader.Close();
                    return true;
                }
                else
                {
                    reader.Close();
                    return false;
                }
            }
            catch
            {
                return false;
            }
        }

        private bool ValidateMatadataIdInWellMetadataTable(long metadataId)
        {
            OdbcDataReader reader;
            try
            {
                OdbcConnection MyConn = Connection.GetConnection();
                OdbcCommand MyCmd = new OdbcCommand();
                MyCmd.Connection = MyConn;
                string strCommand = "SELECT * FROM `thorlabs`.`wellmetadata` WHERE MetaDataId =("
                                    + metadataId + ")";
                MyCmd.CommandText = strCommand;
                reader = MyCmd.ExecuteReader();
                if (reader.HasRows)
                {
                    reader.Close();
                    return true;
                }
                else
                {
                    reader.Close();
                    return false;
                }
            }
            catch
            {
                return false;
            }
        }

        #endregion

        #endregion
    }
}
