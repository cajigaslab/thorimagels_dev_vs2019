namespace ScriptManagerDll.ViewModel
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;

    using GongSolutions.Wpf.DragDrop;

    using DragDrop = GongSolutions.Wpf.DragDrop.DragDrop;

    internal class ScriptBuilderLists :  INotifyPropertyChanged
    {
        #region Fields

        //private ObservableCollection<ScriptItem> _script;
        private int _selectedLine;

        #endregion Fields

        #region Constructors

        public ScriptBuilderLists()
        {
        //    _script = new ObservableCollection<ScriptItem>();
        //    this.CollectionCmds = new ObservableCollection<ScriptItem>();
        }

        #endregion Constructors

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion Events

        #region Properties

        public bool CanAcceptChildren
        {
            get; set;
        }

        //public ObservableCollection<ScriptItem> CollectionCmds
        //{
        //    get;
        //    private set;
        //}

        //public ObservableCollection<ScriptItem> CollectionScript
        //{
        //    get
        //    {
        //        return _script;
        //    }

        //    set
        //    {
        //        _script = value;
        //        OnPropertyChanged("CollectionScript");
        //    }
        //}

        public int SelectedLine
        {
            get
            {
                return _selectedLine;
            }
            set
            {
                _selectedLine = value;

                OnPropertyChanged("SelectedLine");
            }
        }

        #endregion Properties

        #region Methods

        //public void DoNotifyCollectionChanged()
        //{
        //}

        //void IDropTarget.DragOver(IDropInfo dropInfo)
        //{
        //    DragDrop.DefaultDropHandler.DragOver(dropInfo);
        //}

        //void IDropTarget.Drop(IDropInfo dropInfo)
        //{
        //    if (dropInfo.DragInfo.SourceCollection.Equals(_script))
        //    {
        //        DragDrop.DefaultDropHandler.Drop(dropInfo);
        //    }
        //    else
        //    {
        //        ScriptItem si = new ScriptItem();
        //        ScriptItem data = (ScriptItem)dropInfo.Data;
        //        si.Name = data.Name;
        //        si.LineNumber = dropInfo.InsertIndex;
        //        si.Id = data.Id;
        //        si.Icon = data.Icon;
        //        si.Notes = data.Notes;
        //        si.Paramters = data.Paramters;
        //        si.Description = data.Description;

        //        _script.Insert(dropInfo.InsertIndex, si);

        //    }
        //        for (int i = 0; i < _script.Count; i++)
        //        {
        //            _script[i].LineNumber = i + 1;
        //        }

        //        this.SelectedLine = dropInfo.InsertIndex;
        //}

        public void OnPropertyChanged(string name)
        {
            if (PropertyChanged != null) PropertyChanged(this, new PropertyChangedEventArgs(name));
        }

        #endregion Methods
    }
}