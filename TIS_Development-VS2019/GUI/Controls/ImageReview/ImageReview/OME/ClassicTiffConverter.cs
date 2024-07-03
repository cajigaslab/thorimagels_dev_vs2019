using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.IO;
using System.Runtime.InteropServices;
using ImageReviewModule.Model;
using System.Xml;

namespace ImageReviewDll.OME
{
    public class ClassicTiffConverter
    {
        public string experimentFileFullName;
        private List<string> channelNameList = new List<string>();

        string path = "";
        int dataType; // 0: Tiff, 1: Raw, 2: OME Tiff
        ushort zCount = 0;
        int tCount = 0;
        string channelArray;
        int onlySaveEnabledChannels = 1; // 1:true, 0:false // for raw
        bool[] enabledChannelsFlag; // for raw
        SampleInfo sample;
        int regionPixelX, regionPixelY, bitsPerPixel;
        float regionWidth, regionHeight; // for region/image
        int regionPositionPixelX, regionPositionPixelY;
        float regionPixelSizeUM, cameraPixelSizeUM;
        long widthPixel, heightPixel;
        double zStepSizeUM, intervalSec;
        string experimentName, date, userName, computerName, instrument = "ThorImage", softwareVersion, notes; // Additional info

        public ClassicTiffConverter(string folderPath)
        {
            path = folderPath;
            experimentFileFullName = folderPath + "\\Experiment.xml";
        }

        private int LoadExperimentXML()
        {
            int ret = 0;

            var xmlProcess = new XMLProcess(experimentFileFullName);
            channelNameList.Clear();

            // Read attributes from experiment.xml

            path = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Name", "path");
            if (string.IsNullOrEmpty(path))
            {
                MessageBox.Show("No folder path defined in experiment file");
                return -1;
            }
            var value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Streaming", "rawData");
            if (!int.TryParse(value, out dataType))
            {
                MessageBox.Show("Cannot read Stream rawData");
                ret = -1;
            }
            if (dataType != 0 && dataType != 1)
            {
                MessageBox.Show("Unsupported data type");
                ret = -1;
            }

            channelArray = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Wavelengths", "Wavelength", "name");
            if (string.IsNullOrEmpty(channelArray))
            {
                MessageBox.Show("No channels defined in experiment file");
                ret = -1;
            }
            channelNameList = channelArray.Split(',').ToList();
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/ZStage", "zStreamFrames");
            if (!ushort.TryParse(value, out zCount))
            {
                MessageBox.Show("Cannot read zStreamFrames");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Streaming", "frames");
            if (!int.TryParse(value, out tCount))
            {
                MessageBox.Show("Cannot read Stream frames");
                ret = -1;
            }

            // Attributes for sample start here
            sample.name = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "name");

            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "height");
            if (!float.TryParse(value, out sample.height))
            {
                MessageBox.Show("Cannot read Sample height");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "width");
            if (!float.TryParse(value, out sample.width))
            {
                MessageBox.Show("Cannot read Sample width");
                ret = -1;
            }
            sample.type = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "type");
            if (string.IsNullOrEmpty(sample.type))
            {
                MessageBox.Show("Cannot read Sample type");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "row");
            if (!ushort.TryParse(value, out sample.rowSize))
            {
                MessageBox.Show("Cannot read row size");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "column");
            if (!ushort.TryParse(value, out sample.columnSize))
            {
                MessageBox.Show("Cannot read column size");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "WellShape");
            if (value == "RectangleWell")
            {
                sample.wellShape = 1;

                value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "WellWidth");
                if (!string.IsNullOrEmpty(value) && !float.TryParse(value, out sample.wellWidth))
                {
                    MessageBox.Show("Cannot read WellWidth");
                    ret = -1;
                }
                value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "WellHeight");
                if (!string.IsNullOrEmpty(value) && !float.TryParse(value, out sample.wellHeight))
                {
                    MessageBox.Show("Cannot read WellHeight");
                    ret = -1;
                }
            }
            else if (value == "CircleWell")
            {
                sample.wellShape = 2;

                value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "diameter");
                if (!string.IsNullOrEmpty(value) && !float.TryParse(value, out sample.diameter))
                {
                    MessageBox.Show("Cannot read diameter");
                    ret = -1;
                }
            }
            else
            {
                sample.wellShape = 0;
                sample.diameter = 0;
                sample.wellWidth = 0;
                sample.wellHeight = 0;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "centerToCenterX");
            if (!float.TryParse(value, out sample.centerToCenterX))
            {
                MessageBox.Show("Cannot read centerToCenterX");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "centerToCenterY");
            if (!float.TryParse(value, out sample.centerToCenterY))
            {
                MessageBox.Show("Cannot read centerToCenterY");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "topLeftCenterOffsetX");
            if (!float.TryParse(value, out sample.topLeftCenterOffsetX))
            {
                MessageBox.Show("Cannot read topLeftCenterOffsetX");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Sample", "topLeftCenterOffsetY");
            if (!float.TryParse(value, out sample.topLeftCenterOffsetY))
            {
                MessageBox.Show("Cannot read topLeftCenterOffsetY");
                ret = -1;
            }
            // sample end here

            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/LSM", "widthUM");
            if (!float.TryParse(value, out regionWidth))
            {
                MessageBox.Show("Cannot read LSM widthUM");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/LSM", "heightUM");
            if (!float.TryParse(value, out regionHeight))
            {
                MessageBox.Show("Cannot read LSM heightUM");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/LSM", "pixelX");
            if (!int.TryParse(value, out regionPixelX))
            {
                MessageBox.Show("Cannot read LSM pixelX");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/LSM", "pixelY");
            if (!int.TryParse(value, out regionPixelY))
            {
                MessageBox.Show("Cannot read LSM pixelY");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/LSM", "offsetX");
            if (!int.TryParse(value, out regionPositionPixelX))
            {
                MessageBox.Show("Cannot read LSM offsetX");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/LSM", "offsetY");
            if (!int.TryParse(value, out regionPositionPixelY))
            {
                MessageBox.Show("Cannot read LSM offsetY");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/LSM", "pixelSizeUM");
            if (!float.TryParse(value, out regionPixelSizeUM))
            {
                MessageBox.Show("Cannot read LSM pixelSizeUM");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Camera", "pixelSizeUM");
            if (!float.TryParse(value, out cameraPixelSizeUM))
            {
                MessageBox.Show("Cannot read Camera pixelSizeUM");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Camera", "bitsPerPixel");
            if (!int.TryParse(value, out bitsPerPixel))
            {
                MessageBox.Show("Cannot read Camera bitsPerPixel");
                ret = -1;
            }

            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Camera", "width");
            if (!long.TryParse(value, out widthPixel))
            {
                MessageBox.Show("Cannot read Camera width");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Camera", "height");
            if (!long.TryParse(value, out heightPixel))
            {
                MessageBox.Show("Cannot read Camera height");
                ret = -1;
            }

            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/ZStage", "stepSizeUM");
            if (!double.TryParse(value, out zStepSizeUM))
            {
                MessageBox.Show("Cannot read ZStage stepSizeUM");
                ret = -1;
            }
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Timelapse", "intervalSec");
            if (!double.TryParse(value, out intervalSec))
            {
                MessageBox.Show("Cannot read Timelapse intervalSec");
                ret = -1;
            }

            // Attribute for raw data
            value = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/RawData", "onlyEnabledChannels");
            if (!int.TryParse(value, out onlySaveEnabledChannels))
            {
                MessageBox.Show("Cannot read RawData onlyEnabledChannels");
                //ret = -1;
            }

            // Attribute for additional information
            experimentName = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Name", "name");
            date = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Date", "date");
            userName = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/User", "name");
            computerName = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Computer", "name");
            softwareVersion = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/Software", "version");
            notes = XMLProcess.Read(experimentFileFullName, "ThorImageExperiment/ExperimentNotes", "text");
            return ret;
        }

        // Additional data will be shown in ThorAnalysis
        private string GenerateAdditionalData()
        {
            string data =
$@"<?xml version=""1.0"" encoding=""utf-16""?>
<Experiment>  
    <ExperimentInfo Name=""{experimentName}"" CreatedDate=""{date}"" ModifiedDate=""{date}"" UserName=""{userName}"" ComputerName=""{computerName}"" InstrumentType=""{instrument}"" SoftwareVersion=""{softwareVersion}"" Notes=""{notes}"" IntensityBits=""{bitsPerPixel}"">
        <ExperimentDataFile>{experimentName}</ExperimentDataFile>
        <ExperimentPath>{path}</ExperimentPath>
    </ExperimentInfo> 
        <PlateInfo Name=""Slide"" Width=""{sample.width}"" Height=""{sample.height}""/>
    <TemplateScans>
    </TemplateScans>
</Experiment>";

            return data;
        }

        public long DoConvert(out string omeTiffFileName)
        {
            long ret = 0;
            omeTiffFileName = "";

            // load experiment.xml to get the path, channel names, count of z and count of t and any necessary attributes
            ret = LoadExperimentXML();

            if (ret < 0)
                return ret;

            // Create subfolder for result
            var resultFolder = path + "\\" + "ConvertedOMETiff";
            if (!Directory.Exists(resultFolder))
                Directory.CreateDirectory(resultFolder);
            omeTiffFileName = resultFolder + "\\" + "Image.tif";
            if (!IsFileGoodForWriting(omeTiffFileName))
            {
                MessageBox.Show("Cannot create file " + omeTiffFileName + ", please check your write permission in the folder");
                return -1;
            }

            // configure OME header
            var omeHandle = CreateOMETiff(omeTiffFileName);
            ret = ConfigureOMETiff(omeHandle, sample, regionPixelX, regionPixelY, regionWidth, regionHeight, zCount, tCount, regionPositionPixelX, regionPositionPixelY, bitsPerPixel, regionPixelSizeUM, zStepSizeUM, intervalSec, channelNameList.Count, channelArray);

            if (ret < 0)
            {
                CloseOMETiff(omeHandle);
                return ret;
            }

            // load image data and write data for each image
            int imageHandle = 0;

            if (dataType == 0) // Tiff
            {
                ImageInfo imageInfo = new ImageInfo();
                uint imageCount = 0;

                for (int k = 0; k < channelNameList.Count; k++)
                {
                    for (int j = 1; j <= tCount; j++)
                    {
                        for (int i = 1; i <= zCount; i++)
                        {
                            // Get corresponding tiff file name
                            var prefix = channelNameList[k] + "_001_001";
                            var fileName = prefix + "_" + i.ToString("D3") + "_" + j.ToString("D3") + ".tif";
                            var tiffFileName = path + "\\" + fileName;
                            if (!File.Exists(tiffFileName))
                                break;

                            // Get imageHandle
                            GetImageCount(tiffFileName, ref imageHandle, ref imageCount);

                            // Read tiff image
                            GetImageInfo(imageHandle, 0, ref imageInfo);

                            var pixelCount = imageInfo.ImageType == ImageTypes.GRAY ? 1 : 3;
                            int pixelSize = 0;
                            switch (imageInfo.PixelType)
                            {
                                case PixelTypes.PixelType_INT8:
                                case PixelTypes.PixelType_UINT8:
                                    pixelSize = 1;
                                    break;
                                case PixelTypes.PixelType_INT16:
                                case PixelTypes.PixelType_UINT16:
                                    pixelSize = 2;
                                    break;
                                case PixelTypes.PixelType_FLOAT:
                                    pixelSize = 4;
                                    break;
                            }
                            var buffer = Marshal.AllocHGlobal((int)(imageInfo.Width * imageInfo.Height * pixelCount * pixelSize));

                            var result = GetImageData(imageHandle, 0, buffer);

                            // write ome tiff
                            if (result == 0)
                                SaveOMEData(omeHandle, k, i - 1, j - 1, buffer);

                            Marshal.FreeHGlobal(buffer);

                            CloseImage(imageHandle);
                        }
                    }
                }
            }
            else if (dataType == 1) // Raw
            {
                var rawFileName = path + "\\" + "Image_001_001.raw";

                if (!File.Exists(rawFileName))
                    return -1;

                LoadRawDataFile(rawFileName, ref imageHandle);

                //1 pixel = 2 bytes
                var imageSize = regionPixelX * regionPixelY * 2;
                var buffer = Marshal.AllocHGlobal(imageSize);

                // Read data
                var cCount = channelNameList.Count;
                var maxCount = 4;
                if (onlySaveEnabledChannels == 0)
                    InitEnabledChannelsFlag();

                for (int j = 1; j <= tCount; j++)
                {
                    for (int i = 1; i <= zCount; i++)
                    {
                        if (onlySaveEnabledChannels == 1)
                        {
                            for (int k = 0; k < cCount; k++)
                            {
                                // get corresponding data
                                var offset = ((j - 1) * cCount * zCount + (i - 1) * cCount + k) * imageSize;
                                var result = GetRawData(imageHandle, offset, imageSize, buffer);

                                // write ome tiff
                                if (result == 0)
                                    SaveOMEData(omeHandle, k, i - 1, j - 1, buffer);
                            }
                        }
                        else
                        {
                            for (int k = 0; k < maxCount; k++)
                            {
                                var offset = ((j - 1) * maxCount * zCount + (i - 1) * maxCount + k) * imageSize;
                                int channelIndexOfOME = 0;
                                
                                // Only save enabled channels
                                if (CalculateChannelIndexOfOME(k, ref channelIndexOfOME))
                                {
                                    // get corresponding data
                                    var result = GetRawData(imageHandle, offset, imageSize, buffer);

                                    // write ome tiff
                                    if (result == 0)
                                        SaveOMEData(omeHandle, channelIndexOfOME, i - 1, j - 1, buffer);
                                }
                            }
                        }
                    }
                }
                Marshal.FreeHGlobal(buffer);
                CloseRawImage(imageHandle);
            }

            // save additional data for displaying in ThorAnalysis
            var additionData = GenerateAdditionalData();
            if (!string.IsNullOrEmpty(additionData))
                SaveAdditionalData(omeHandle, additionData);

            CloseOMETiff(omeHandle);

            return 0;
        }

        public long DoConvertRawToTiff(ref string tiffExperimentFilePth)
        {
            long ret = 0;

            // load experiment.xml to get the path, channel names, count of z and count of t
            ret = LoadExperimentXML();

            if (ret < 0 || dataType != 1) // error or not raw type
                return -1;

            // Create subfolder for result
            var resultFolder = path + "\\" + path.Substring(path.LastIndexOf('\\') + 1) + "-tiff";
            if (!Directory.Exists(resultFolder))
                Directory.CreateDirectory(resultFolder);
            tiffExperimentFilePth = resultFolder;

            var rawFileName = path + "\\" + "Image_001_001.raw";

            if (!File.Exists(rawFileName))
                return -1;

            // Convert data
            ret = ConvertRawToTIFF(rawFileName, resultFolder, channelNameList.Count, tCount, zCount,intervalSec, channelArray, regionPixelX, regionPixelY, cameraPixelSizeUM, zStepSizeUM);

            if (ret == 0)
            {
                // change path and rawData, save Experiment.xml
                XmlDocument origDoc = new XmlDocument();
                origDoc.Load(experimentFileFullName);
                // Create a clone of the original document to make sure we don't modify Experiment.xml
                XmlDocument doc = new XmlDocument();
                doc.LoadXml(origDoc.OuterXml);
                origDoc = null;
                XmlNode xn = doc.SelectSingleNode("/ThorImageExperiment/Name");
                var attr = xn.Attributes.GetNamedItem("path");
                if (attr != null)
                    attr.Value = resultFolder;
                xn = doc.SelectSingleNode("/ThorImageExperiment/Streaming");
                attr = xn.Attributes.GetNamedItem("rawData");
                if (attr != null)
                    attr.Value = "0";
                var newExperimentFileFullName = resultFolder + experimentFileFullName.Substring(experimentFileFullName.LastIndexOf('\\'));
                doc.Save(newExperimentFileFullName);

                // copy other files
                var filenames = Directory.GetFiles(path);
                foreach (var item in filenames)
                {
                    var filename = item.Substring(item.LastIndexOf('\\'));
                    if (filename.Contains("ROIMask.raw") || filename.Contains("ROIs.xaml") || filename.Contains("_Preview.tif"))
                        File.Copy(path + filename, resultFolder + filename, true);
                }

            }
            return 0;
        }
        // Flags for enabled channels
        void InitEnabledChannelsFlag()
        {
            enabledChannelsFlag = new bool[4] { false, false, false, false };
            foreach (var item in channelNameList)
            {
                switch (item)
                {
                    case "ChanA":
                        enabledChannelsFlag[0] = true;
                        break;
                    case "ChanB":
                        enabledChannelsFlag[1] = true;
                        break;
                    case "ChanC":
                        enabledChannelsFlag[2] = true;
                        break;
                    case "ChanD":
                        enabledChannelsFlag[3] = true;
                        break;
                }
            }
        }

        // Calculate channel index for OME tiff
        // return false if not an enabled channel, otherwise return true
        // indexOfOME is new channel index based on all enabled channels
        bool CalculateChannelIndexOfOME(int indexOfRaw, ref int indexOfOME)
        {
            bool isSavedChannel = enabledChannelsFlag[indexOfRaw];
            if (isSavedChannel)
            {
                indexOfOME = 0;
                for (int i = 0; i < indexOfRaw; i++)
                    if (enabledChannelsFlag[i])
                        indexOfOME++;
            }
            return isSavedChannel;
        }

        int LoadRawDataFile(string fileName, ref int imageHandle)
        {
            int result = ClassicTiffConverterWrapper.LoadRawDataFile(fileName, ref imageHandle);
            return result;
        }

        int GetRawData(int imageHandle, int offset, int size, IntPtr image_data)
        {
            int result = ClassicTiffConverterWrapper.GetRawData(imageHandle, offset, size, image_data);
            return result;
        }

        void CloseRawImage(int imageHandle)
        {
            if (imageHandle < 0)
                return;
            ClassicTiffConverterWrapper.CloseRawImage(imageHandle);
        }

        int GetImageCount(string fileName, ref int imageHandle, ref uint imageCount)
        {
            int result = ClassicTiffConverterWrapper.GetImageCount(fileName, ref imageHandle, ref imageCount);
            return result;
        }

        int GetImageInfo(int imageHandle, uint imageNumber, ref ImageInfo info)
        {
            int result = ClassicTiffConverterWrapper.GetImageInfo(imageHandle, imageNumber, ref info);
            return result;
        }

        int GetImageData(int imageHandle, uint imageNumber, IntPtr image_data)
        {
            int result = ClassicTiffConverterWrapper.GetImageData(imageHandle, imageNumber, image_data);
            return result;
        }

        void CloseImage(int imageHandle)
        {
            if (imageHandle < 0)
                return;
            ClassicTiffConverterWrapper.CloseImage(imageHandle);
        }

        long CreateOMETiff(string fileName)
        {
            int result = ClassicTiffConverterWrapper.CreateOMETiff(fileName);
            if (result < 0)
            {
                throw new Exception($"Create ome tiff : '{fileName}' failed! Error code is {result}.");
            }
            return result;
        }

        long ConfigureOMETiff(long handle, SampleInfo sample, int regionPixelX, int regionPixelY, float regionW, float regionH, ushort zCount, int tCount,
    int regionPositionPixelX, int regionPositionPixelY, int bitsPerPixel, float regionPixelSizeUM, double zStepSizeUM, double intervalSec, int channelNumber, string channels)
        {
            int result = ClassicTiffConverterWrapper.ConfigOMEHeader(handle, sample, regionPixelX, regionPixelY, regionW, regionH, zCount, tCount,
                regionPositionPixelX, regionPositionPixelY, bitsPerPixel, regionPixelSizeUM, zStepSizeUM, intervalSec, channelNumber, channels);
            if (result < 0)
            {
                throw new Exception($"Configure ome tiff : '{handle}' failed! Error code is {result}.");
            }
            return result;
        }

        long SaveOMEData(long handle, int channelID, int zIndex, int tIndex, IntPtr data)
        {
            int result = ClassicTiffConverterWrapper.SaveOMEData(handle, channelID, zIndex, tIndex, data);
            if (result < 0)
            {
                throw new Exception($"save ome tiff : '{handle}' failed! Error code is {result}.");
            }
            return result;
        }
        long SaveAdditionalData(long handle, string data)
        {
            int result = ClassicTiffConverterWrapper.SaveAdditionalData(handle, data, data.Length);
            if (result < 0)
            {
                throw new Exception($"save addition data : '{handle}' failed! Error code is {result}.");
            }
            return result;
        }

        long CloseOMETiff(long handle)
        {
            int result = ClassicTiffConverterWrapper.CloseOMETiff(handle);
            if (result < 0)
            {
                throw new Exception($"Close ome tiff : '{handle}' failed! Error code is {result}.");
            }
            return result;
        }

        int ConvertRawToTIFF(string rawFileName, string tiffFolderName, int cCount, int tCount, int zCount, double intervalSec, string channelNameArray, long width, long height, double umPerPixel, double zStepSizeUM)
        {
            int result = ClassicTiffConverterWrapper.ConvertRawToTIFF(rawFileName, tiffFolderName, cCount, tCount, zCount, intervalSec, channelNameArray, width, height, umPerPixel, zStepSizeUM);
            return result;
        }
        // Check if file is Good for writing
        public static bool IsFileGoodForWriting(string filePath)
        {
            if (filePath == null)
                return false;

            FileStream stream = null;
            FileInfo file = new FileInfo(filePath);

            bool isFileExists = File.Exists(filePath);

            try
            {
                stream = file.Open(FileMode.OpenOrCreate, FileAccess.Read, FileShare.None);
            }
            catch (Exception)
            {
                //the file is unavailable because it is:
                //still being written to
                //or being processed by another thread
                //or does not exist (has already been processed)
                return false;
            }
            finally
            {
                if (stream != null)
                    stream.Close();
            }

            if (!isFileExists)
                File.Delete(filePath);

            //file is not locked
            return true;
        }
    }
}
