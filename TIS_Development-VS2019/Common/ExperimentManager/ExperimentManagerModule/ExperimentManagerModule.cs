using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace ExperimentManagerModule
{
    public class ExperimentManagerModule
    {
        public bool Login(string username,string password)
        {
            return true;
        }

        public void Logout()
        {
        }
        
        public void AddUser(string firstname, string lastname, string emailid, string password)
        {
        }

        public void DeleteUser(string username)
        {
        }

        public int LoadExperiment(string ftpshare, string xmlfilpath)
        {
            int id=0;

            return id;
        }

        public string GetUser(int experimentID)
        {
            return "";
        }

        public void SetUser(int experimentID, string name)
        {
        }

        public string GetExperimentName(int experimentID)
        {
            return "";
        }

        public string GetDate(int experimentID)
        {
            return "";
        }

        public void SetDate(int experimentID,string date)
        {
            return;
        }

        public string GetComputer(int experimentID)
        {
            return "";
        }

        public void SetComputer(int experimentID,string computer)
        {
            return;
        }

        public double GetSoftwareVersion(int experimentID)
        {
            return 1.0;
        }

        public void SetSoftwareVersion(int experimentID,double version)
        {
            return;
        }

        public string GetCamera(int experimentID,string name,int width,int height,double pixelSizeUM )
        {
            return "";
        }

        public void SetCamera(int experimentID, string name, int width, int height, double pixelSizeUM)
        {
            return;
        }

        public string GetMagnification(int experimentID, double mag)
        {
            return "";
        }

        public void SetMagnification(int experimentID, double mag)
        {
            return;
        }

        public ArrayList GetWavelengths(int experimentID)
        {
            ArrayList array = new ArrayList();
            return array;
        }

        public void SetWavelength(int experimentID, int index , string name, double exposureTimeMS)
        {
            return;
        }

        public void GetZStage(int experimentID, ref string name,  ref int steps, ref double stepSizeUM, ref int zStreamFrames,ref int zStreamMode)
        {
            return;
        }

        public void SetZStage(int experimentID, string name, int steps,double stepSizeUM,int zStreamFrames, int zStreamMode)
        {
            return;
        }

        public void GetTimelapse(int experimentID, int timepoints,double intervalSec)
        {
            return;
        }

        public void SetTimelapse(int experimentID, ref int timepoints,ref double intervalSec)
        {
            return;
        }

        public void GetSample(int experimentID,ref int type,ref double offsetXMM,ref double offsetYMM)
        {
            return;
        }

        public void SetSample(int experimentID,int type,double offsetXMM,double offsetYMM)
        {
            return;
        }

        public void GetWells(int experimentID, ref int subRows,ref int subColumns,ref double subOffsetXMM,ref double subOffsetYMM,ref double transOffsetXMM,ref double transOffsetYMM)
        {
            return;
        }

        public void SetWells(int experimentID, int subRows,int subColumns,double subOffsetXMM,double subOffsetYMM,double transOffsetXMM,double transOffsetYMM)
        {
            return;
        }

        public void GetSubImage(int experimentID, ref int subRows, ref int subColumns, ref double subOffsetXMM, ref double subOffsetYMM,ref double transOffsetXMM, double transOffsetYMM)
        {
            return;
        }

        public void SetSubImage(int experimentID, int subRows, int subColumns, double subOffsetXMM, double subOffsetYMM, double transOffsetXMM, double transOffsetYMM)
        {
            return;
        }

        public string GetComments(int experimentID, string name)
        {
            return "";
        }

        public void SetComments(int experimentID, string name)
        {
            return;
        }

        public ArrayList GetCategories(int experimentID)
        {
            ArrayList array = new ArrayList();
            return array;
        }

        public void AddCategory(int experimentID, string name)
        {
            return;
        }

        public void DeleteCategory(int experimentID, string category)
        {
            return;
        }

    }
}
