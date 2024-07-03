namespace SequentialControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Globalization;
    using System.IO;
    using System.Reflection;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Xml;

    using GongSolutions.Wpf.DragDrop;

    using SequentialControl.Model;

    using ThorLogging;

    using ThorSharedTypes;

    using DragDrop = GongSolutions.Wpf.DragDrop.DragDrop;

    public class SequentialControlViewModel : VMBase, IMVM, IDropTarget
    {
        #region Fields

        private const int MAX_CHANNELS = 4;

        private readonly SequentialControlModel _sequentialControlModel;

        private bool _betweenFrames = false;
        private bool _betweenStacks = true;
        private SequenceStep _draggedTemplateSequenceStep;
        private bool _isDraggingTemplateSequenceStep = false;
        private ICommand _previewSequentialCommand;
        private int _previousCaptureSequenceSelectedLine = 0;
        private int _previousCaptureTemplateSelectedLine = 0;
        private Dictionary<string, PropertyInfo> _properties = new Dictionary<string, PropertyInfo>();
        private ICommand _templateListAddCommand;

        #endregion Fields

        #region Constructors

        public SequentialControlViewModel()
        {
            _sequentialControlModel = new SequentialControlModel();
        }

        #endregion Constructors

        #region Properties

        public bool BetweenFrames
        {
            get
            {
                return _betweenFrames;
            }
            set
            {
                _betweenFrames = value;
                OnPropertyChanged("BetweenFrames");
            }
        }

        public bool BetweenStacks
        {
            get
            {
                return _betweenStacks;
            }
            set
            {
                _betweenStacks = value;
                OnPropertyChanged("BetweenStacks");
            }
        }

        public ObservableCollection<SequenceStep> CollectionCaptureSequence
        {
            get
            {
                return _sequentialControlModel.CollectionCaptureSequence;
            }

            set
            {
                _sequentialControlModel.CollectionCaptureSequence = value;
                OnPropertyChanged("CollectionCaptureSequence");
            }
        }

        public ObservableCollection<SequenceStep> CollectionSequences
        {
            get
            {
                return _sequentialControlModel.CollectionSequences;
            }

            set
            {
                _sequentialControlModel.CollectionSequences = value;
                OnPropertyChanged("CollectionSequences");
            }
        }

        public int EnableSequentialCapture
        {
            get
            {
                return _sequentialControlModel.EnableSequentialCapture;
            }
            set
            {
                _sequentialControlModel.EnableSequentialCapture = value;
                OnPropertyChanged("EnableSequentialCapture");
            }
        }

        public bool IsSequentialCapturing
        {
            get
            {
                return _sequentialControlModel.IsSequentialCapturing;
            }
            set
            {
                _sequentialControlModel.IsSequentialCapturing = value;
            }
        }

        public Visibility IsTabletModeEnabled
        {
            get
            {
                return ResourceManagerCS.Instance.TabletModeEnabled ? Visibility.Collapsed : Visibility.Visible;
            }
        }

        public ICommand PreviewSequentialCommand
        {
            get
            {
                if (_previewSequentialCommand == null)
                    _previewSequentialCommand = new RelayCommand(() => _sequentialControlModel.StartSequentialPreview());

                return _previewSequentialCommand;
            }
        }

        public int RemoveSequenceStep
        {
            get
            {
                return _sequentialControlModel.RemoveSequenceStep;
            }
            set
            {
                _sequentialControlModel.RemoveSequenceStep = value;
                OnPropertyChanged("CollectionCaptureSequence");
            }
        }

        public int RemoveStepTemplate
        {
            get
            {
                return _sequentialControlModel.RemoveStepTemplate;
            }
            set
            {
                _sequentialControlModel.RemoveStepTemplate = value;
                OnPropertyChanged("CollectionSequences");
            }
        }

        public bool SequentialStopped
        {
            get
            {
                return _sequentialControlModel.SequentialStopped;
            }
            set
            {
                _sequentialControlModel.SequentialStopped = value;
            }
        }

        public ICommand TemplateListAddCommand
        {
            get => _templateListAddCommand ?? (_templateListAddCommand = new RelayCommand(() => { _sequentialControlModel.TemplateListAdd(); OnPropertyChanged("CollectionSequences"); }));
        }

        public bool UpdateSequencesColors
        {
            get
            {
                return true;
            }
            set
            {
                if (value)
                {
                    LoadXMLSettings();
                }
            }
        }

        public int UpdateStepTemplate
        {
            get
            {
                return _sequentialControlModel.UpdateStepTemplate;
            }
            set
            {
                _sequentialControlModel.UpdateStepTemplate = value;
            }
        }

        public string[] UpdateTemplateName
        {
            set => _sequentialControlModel.UpdateTemplateName = value;
        }

        #endregion Properties

        #region Indexers

        public object this[string propertyName, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                return (null != myPropInfo) ? myPropInfo.GetValue(this) : null;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    myPropInfo.SetValue(this, value);
                }
            }
        }

        public object this[string propertyName, int index, object defaultObject = null]
        {
            get
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        return collection.GetType().GetProperty("Item").GetValue(collection, new object[] { index });
                    }
                    else
                    {
                        return myPropInfo.GetValue(this, null);
                    }
                }
                return defaultObject;
            }
            set
            {
                PropertyInfo myPropInfo = GetPropertyInfo(propertyName);
                if (null != myPropInfo)
                {
                    if (typeof(CustomCollection<>) == myPropInfo.PropertyType.GetGenericTypeDefinition())
                    {
                        var collection = myPropInfo.GetValue(this, null);
                        collection.GetType().GetProperty("Item").SetValue(collection, value, new object[] { index });
                    }
                    else
                    {
                        myPropInfo.SetValue(this, value, null);
                    }
                }
            }
        }

        #endregion Indexers

        #region Methods

        //If a sequence doesn't have the color or channelInder attributes we need to add default ones while loading. This is used for the case when loading a template with old sequential settings
        public void CreateColorAttributeForSequences(XmlNode sequenceNode, XmlDocument doc)
        {
            string str = string.Empty;
            if (null == sequenceNode || null == doc)
            {
                return;
            }

            int channels;
            if (ResourceManagerCS.GetCameraType() == (int)ICamera.CameraType.LSM)
            {
                XmlNode lsmNode = sequenceNode.SelectSingleNode("LSM");
                if (!(XmlManager.GetAttribute(lsmNode, doc, "channel", ref str) && Int32.TryParse(str, out channels)))
                {
                    return;
                }
            }
            else
            {
                XmlNode camNode = sequenceNode.SelectSingleNode("Camera");
                if (!(XmlManager.GetAttribute(camNode, doc, "channel", ref str) && Int32.TryParse(str, out channels)))
                {
                    return;
                }
            }

            Color[] colors = (Color[])MVMManager.Instance["ImageViewCaptureSetupVM", "DefaultChannelColors"];
            string[] colornames = (string[])MVMManager.Instance["ImageViewCaptureSetupVM", "DefaultChannelColorNames"];
            XmlNodeList wavNdList = sequenceNode.SelectNodes("Wavelengths/Wavelength");
            for (int i = 0, k = 0; i < MAX_CHANNELS && k < wavNdList.Count; i++)
            {
                if ((channels & (0x0001 << i)) > 0 && colornames[i] != null)
                {
                    if (!XmlManager.GetAttribute(wavNdList[k], doc, "channelIndex", ref str))
                    {
                        XmlManager.SetAttribute(wavNdList[k], doc, "channelIndex", i.ToString());
                    }
                    if (!XmlManager.GetAttribute(wavNdList[k], doc, "color", ref str))
                    {
                        XmlManager.SetAttribute(wavNdList[k], doc, "color", colors[i].ToString());
                    }
                    if (!XmlManager.GetAttribute(wavNdList[k], doc, "colorName", ref str))
                    {
                        XmlManager.SetAttribute(wavNdList[k], doc, "colorName", colornames[i]);
                    }
                    k++;
                }
            }
        }

        public PropertyInfo GetPropertyInfo(string propertyName)
        {
            PropertyInfo myPropInfo = null;
            if (!_properties.TryGetValue(propertyName, out myPropInfo))
            {
                myPropInfo = typeof(SequentialControlViewModel).GetProperty(propertyName);
                if (null != myPropInfo)
                {
                    _properties.Add(propertyName, myPropInfo);
                }
            }
            return myPropInfo;
        }

        void IDropTarget.DragOver(IDropInfo dropInfo)
        {
            DragDrop.DefaultDropHandler.DragOver(dropInfo);
            if (dropInfo.DragInfo.SourceCollection.Equals(CollectionCaptureSequence) &&
                dropInfo.TargetCollection.Equals(CollectionCaptureSequence))
            {
                dropInfo.Effects = DragDropEffects.Move;
                if (false == _isDraggingTemplateSequenceStep)
                {
                    //Persist the dragged script item
                    SequenceStep data = (SequenceStep)dropInfo.Data;
                    _previousCaptureSequenceSelectedLine = data.SequenceLineNumber - 1;
                    if (null != CollectionCaptureSequence[_previousCaptureSequenceSelectedLine])
                    {
                        _draggedTemplateSequenceStep = CollectionCaptureSequence[_previousCaptureSequenceSelectedLine];
                        _isDraggingTemplateSequenceStep = true;
                    }
                }
            }
            else if (dropInfo.DragInfo.SourceCollection.Equals(CollectionSequences) &&
                dropInfo.TargetCollection.Equals(CollectionSequences))
            {
                dropInfo.Effects = DragDropEffects.Move;
                if (false == _isDraggingTemplateSequenceStep)
                {
                    //Persist the dragged script item
                    SequenceStep data = (SequenceStep)dropInfo.Data;
                    _previousCaptureTemplateSelectedLine = data.TemplateLineNumber;
                    if (null != CollectionSequences[_previousCaptureTemplateSelectedLine])
                    {
                        _draggedTemplateSequenceStep = CollectionSequences[_previousCaptureTemplateSelectedLine];
                        _isDraggingTemplateSequenceStep = true;
                    }
                }
            }
            else if (dropInfo.DragInfo.SourceCollection.Equals(CollectionCaptureSequence) &&
                dropInfo.TargetCollection.Equals(CollectionSequences))
            {
                dropInfo.Effects = DragDropEffects.None;
                _isDraggingTemplateSequenceStep = false;
            }
        }

        void IDropTarget.Drop(IDropInfo dropInfo)
        {
            try
            {
                _isDraggingTemplateSequenceStep = false;
                if (dropInfo.DragInfo.SourceCollection.Equals(CollectionCaptureSequence) &&
                    dropInfo.TargetCollection.Equals(CollectionCaptureSequence))
                {
                    if (_draggedTemplateSequenceStep != null)
                    {
                        //Persist the dragged script item
                        CollectionCaptureSequence[_previousCaptureSequenceSelectedLine] = _draggedTemplateSequenceStep;
                    }
                    //moving an existing item in the list
                    DragDrop.DefaultDropHandler.Drop(dropInfo);
                    _sequentialControlModel.ReassignCaptureSequenceLineNumbers();
                }
                else if (dropInfo.DragInfo.SourceCollection.Equals(CollectionSequences) &&
                    dropInfo.TargetCollection.Equals(CollectionCaptureSequence))
                {
                    SequenceStep data = (SequenceStep)dropInfo.Data;
                    XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];
                    XmlNode importNode = doc.ImportNode(data.SequenceStepNode, true);
                    SequenceStep si = new SequenceStep(data.Name, importNode, dropInfo.InsertIndex, true);

                    CollectionCaptureSequence.Insert(dropInfo.InsertIndex, si);
                    _sequentialControlModel.ReassignCaptureSequenceLineNumbers();
                }
                else if (dropInfo.DragInfo.SourceCollection.Equals(CollectionSequences) &&
                    dropInfo.TargetCollection.Equals(CollectionSequences))
                {
                    if (_draggedTemplateSequenceStep != null)
                    {
                        //Persist the dragged script item
                        CollectionSequences[_previousCaptureTemplateSelectedLine] = _draggedTemplateSequenceStep;
                    }
                    //moving an existing item in the list
                    DragDrop.DefaultDropHandler.Drop(dropInfo);
                    _sequentialControlModel.ReassignTemplateListLineNumbers();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        public void LoadXMLSettings()
        {
            XmlDocument doc = MVMManager.Instance.SettingsDoc[(int)SettingsFileType.ACTIVE_EXPERIMENT_SETTINGS];

            XmlNodeList ndList = doc.SelectNodes("/ThorImageExperiment/CaptureSequence");
            string str = string.Empty;
            int iTmp;

            if (ndList.Count > 0)
            {
                EnableSequentialCapture = 0;
                if (XmlManager.GetAttribute(ndList[0], doc, "enable", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    EnableSequentialCapture = (1 == iTmp) ? 1 : 0;
                }
                if (XmlManager.GetAttribute(ndList[0], doc, "sequentialType", ref str) && Int32.TryParse(str, NumberStyles.Any, CultureInfo.InvariantCulture, out iTmp))
                {
                    BetweenFrames = (1 == iTmp);
                }
            }

            string sequenceListFolder = Application.Current.Resources["LightPathListFolder"].ToString();
            string sequenceListFile = sequenceListFolder + "\\LightPathList.xml";

            if (true == Directory.Exists(sequenceListFolder))
            {
                XmlDocument sequenceListDoc = new XmlDocument();
                sequenceListDoc.Load(sequenceListFile);
                XmlNodeList templatesNdList = sequenceListDoc.SelectNodes("/ThorImageLightPathList/LightPathSequenceStep");
                CollectionSequences = new ObservableCollection<SequenceStep>();

                for (int i = 0; i < templatesNdList.Count; i++)
                {
                    if (XmlManager.GetAttribute(templatesNdList[i], sequenceListDoc, "name", ref str))
                    {
                        CreateColorAttributeForSequences(templatesNdList[i], sequenceListDoc);
                        SequenceStep si = new SequenceStep(str, templatesNdList[i], i, false);
                        CollectionSequences.Add(si);
                    }
                }
                _sequentialControlModel.ReassignTemplateListLineNumbers();

                ndList = doc.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");
                CollectionCaptureSequence = new ObservableCollection<SequenceStep>();
                for (int i = 0; i < ndList.Count; i++)
                {
                    if (XmlManager.GetAttribute(ndList[i], doc, "name", ref str))
                    {
                        CreateColorAttributeForSequences(ndList[i], doc);
                        SequenceStep si = new SequenceStep(str, ndList[i], i, true);
                        CollectionCaptureSequence.Add(si);
                    }
                }

                _sequentialControlModel.ReassignCaptureSequenceLineNumbers();
            }
        }

        public void OnPropertyChange(string propertyName)
        {
            if (null != GetPropertyInfo(propertyName))
            {
                OnPropertyChanged(propertyName);
            }
        }

        public void UpdateExpXMLSettings(ref XmlDocument experimentFile)
        {
            //////Capture Sequence
            XmlNodeList sequenceStepNdList;
            XmlNodeList ndList = experimentFile.SelectNodes("/ThorImageExperiment/CaptureSequence");
            if (0 >= ndList.Count)
            {
                ndList = experimentFile.SelectNodes("ThorImageExperiment");
                XmlElement elem = experimentFile.CreateElement("CaptureSequence");
                ndList[0].AppendChild(elem);
                ndList = experimentFile.SelectNodes("/ThorImageExperiment/CaptureSequence");
            }

            if (ndList.Count > 0)
            {
                XmlManager.SetAttribute(ndList[0], experimentFile, "enable", EnableSequentialCapture.ToString());
                XmlManager.SetAttribute(ndList[0], experimentFile, "sequentialType", (BetweenFrames) ? "1" : "0");
            }

            sequenceStepNdList = experimentFile.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");

            if (0 >= sequenceStepNdList.Count)
            {
                XmlElement elem = experimentFile.CreateElement("LightPathSequenceStep");
                ndList[0].AppendChild(elem);
                sequenceStepNdList = experimentFile.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep");
            }

            //Remove all channel steps and add those that are in capture sequence collection
            for (int i = 0; i < sequenceStepNdList.Count; i++)
            {
                ndList[0].RemoveChild(sequenceStepNdList[i]);
            }
            for (int i = 0; i < CollectionCaptureSequence.Count; i++)
            {
                XmlNode importNode = ndList[0].OwnerDocument.ImportNode(CollectionCaptureSequence[i].SequenceStepNode, true);
                ndList[0].AppendChild(importNode);
            }
            //Add in default values for laser elements that did not exist in previous TILS versions (Carrying over from SettingsUpdater)
            XmlNodeList laserNodeList = experimentFile.SelectNodes("/ThorImageExperiment/CaptureSequence/LightPathSequenceStep/MCLS");
            XmlNode wavelengthNode = experimentFile.SelectSingleNode("/ThorImageExperiment/MCLS");

            foreach (XmlNode laserNode in laserNodeList)
            {
                //Only replace these for the MCLS, and if the node already exists, but is missing the new tags
                if (laserNode.Attributes != null && laserNode.Attributes["allttl"] == null)
                {
                    //Fill in the missing attributes with current settings
                    XmlAttribute wavelengthAttribute = wavelengthNode.Attributes["wavelength1"];
                    if (wavelengthAttribute != null)
                    {
                        XmlAttribute attr1 = experimentFile.CreateAttribute("allttl");
                        attr1.Value = wavelengthNode.Attributes["allttl"].Value;
                        laserNode.Attributes.Append(attr1);
                        XmlAttribute attr2 = experimentFile.CreateAttribute("allanalog");
                        attr2.Value = wavelengthNode.Attributes["allanalog"].Value;
                        laserNode.Attributes.Append(attr2);
                        XmlAttribute attr3 = experimentFile.CreateAttribute("wavelength1");
                        attr3.Value = wavelengthNode.Attributes["wavelength1"].Value;
                        laserNode.Attributes.Append(attr3);
                        XmlAttribute attr4 = experimentFile.CreateAttribute("wavelength2");
                        attr4.Value = wavelengthNode.Attributes["wavelength2"].Value;
                        laserNode.Attributes.Append(attr4);
                        XmlAttribute attr5 = experimentFile.CreateAttribute("wavelength3");
                        attr5.Value = wavelengthNode.Attributes["wavelength3"].Value;
                        laserNode.Attributes.Append(attr5);
                        XmlAttribute attr6 = experimentFile.CreateAttribute("wavelength4");
                        attr6.Value = wavelengthNode.Attributes["wavelength4"].Value;
                        laserNode.Attributes.Append(attr6);
                    }
                }
            }
            //Add Camera tag to the sequence step if it doesn't have one
            foreach (XmlNode node in sequenceStepNdList)
            {
                XmlNodeList cameraNodeList = node.SelectNodes("/Camera");
                if (0 >= cameraNodeList.Count)
                {
                    int cameraChannels = 0;
                    if (ResourceManagerCS.GetCameraType() == (int)ICamera.CameraType.CCD)
                    {
                        cameraChannels = (int)MVMManager.Instance["CameraControlViewModel", "ChannelNum"];
                    }
                    XmlElement elem = experimentFile.CreateElement("Camera");
                    node.AppendChild(elem);
                    cameraNodeList = node.SelectNodes("Camera");

                    //Save the channel property for Camera in binary format
                    XmlManager.SetAttribute(cameraNodeList[0], experimentFile, "channel", cameraChannels.ToString());
                }
            }
            ////End Capture Sequence

            //Update the channel step templates
            XmlDocument sequenceListDoc = new XmlDocument();
            string sequenceListFolder = Application.Current.Resources["LightPathListFolder"].ToString();
            //if the directory doesn't existe, create it.
            ResourceManagerCS.SafeCreateDirectory(sequenceListFolder);

            string sequenceListFile = sequenceListFolder + "\\LightPathList.xml";

            //if the file doesnt exist or is corrupted, create it.
            if (true == Directory.Exists(sequenceListFile))
            {
                sequenceListDoc.Load(sequenceListFile);
                if (null == sequenceListDoc)
                {
                    sequenceListDoc.LoadXml("<ThorImageLightPathList></ThorImageLightPathList>");
                }
            }
            else
            {
                sequenceListDoc.LoadXml("<ThorImageLightPathList></ThorImageLightPathList>");
            }

            ndList = sequenceListDoc.SelectNodes("/ThorImageLightPathList");
            sequenceStepNdList = sequenceListDoc.SelectNodes("/ThorImageLightPathList/LightPathSequenceStep");
            //remove all nodes before inserting the ones found in the lighpath sequence step collection
            for (int j = 0; j < sequenceStepNdList.Count; j++)
            {
                ndList[0].RemoveChild(sequenceStepNdList[j]);
            }
            for (int j = 0; j < CollectionSequences.Count; j++)
            {
                XmlNode importNode = ndList[0].OwnerDocument.ImportNode(CollectionSequences[j].SequenceStepNode, true);
                ndList[0].AppendChild(importNode);
            }
            sequenceListDoc.Save(sequenceListFile);
        }

        #endregion Methods
    }
}