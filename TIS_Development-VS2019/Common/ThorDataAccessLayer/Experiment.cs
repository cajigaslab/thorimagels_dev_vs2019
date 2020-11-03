using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.Odbc;

namespace ThorDataAccessLayer
{
    public class Experiment
    {
        internal long experimentId;
        internal string name;
        internal string date;
        internal string userName;
        internal string computerName;
        internal double softwareVersion;
        internal double magnificationMag;
        internal string comments;
        internal int cameraId;
        internal double timepoints;
        internal double intervalSec;
        internal int sampleId;
        internal string zStagename;
        internal double zStageSteps;
        internal double zStageStepSize;
        internal string ftpPath;
        internal int imageFTPLocationID;
    }
}
