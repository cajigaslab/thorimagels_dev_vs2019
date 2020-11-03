using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.ObjectModel;
using System.ComponentModel;

namespace ExperimentSettingsBrowser
{
    public class DatabaseItem
    {
       
        private string _expName;
        private string _expPath;
        private int _id;

        public string ExpName
        {
            get { return this._expName; }
            set { this._expName = value; }
        }
        public string ExpPath
        {
            get { return this._expPath; }
            set { this._expPath = value; }
        }
        public int ID
        {
            get { return this._id; }
            set { this._id = value; }

        }

    }

}