using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Data;
using DatabaseInterface;
using System.ComponentModel;

namespace SqlTest
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        public MainWindow()
        {
            InitializeComponent();
            this.Loaded += MainWindow_Loaded;
            this.Unloaded += MainWindow_Unloaded;
        }

        public event PropertyChangedEventHandler PropertyChanged;
       
        protected virtual void OnPropertyChanged(string propertyName)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        void MainWindow_Unloaded(object sender, RoutedEventArgs e)
        {
            DataStore.Instance.Close();
        }

        void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            DataStore.Instance.ConnectionString = "URI=file:C:\\Users\\Thorlabs\\Documents\\ThorImageLS\\Application Settings\\thordatabase.db";
            DataStore.Instance.Open();

            this.DataContext = this;
        }

        public DataView BatchesDataView
        {
            get
            {
                return new DataView(DataStore.Instance.BatchesDataSet.Tables[0]);
            }
        }

        public DataView ExperimentsDataView
        {
            get
            {
                return new DataView(DataStore.Instance.ExperimentsDataSet.Tables[0]);
            }
        }

        private void UpdateItemsSource()
        {
        }

        private int _experimentCount = 0;
        private int _batchCount = 0;
        private void btnAdd_Click(object sender, RoutedEventArgs e)
        {
            DataStore.Instance.AddExperiment(string.Format("MyExperiment{0}", _experimentCount), string.Format("C:/Test/MyData{0}", _experimentCount), string.Format("Batch{0}", _batchCount));
              _experimentCount++;
            DataStore.Instance.AddExperiment(string.Format("MyExperiment{0}", _experimentCount), string.Format("C:/Test/MyData{0}", _experimentCount), string.Format("Batch{0}", _batchCount));
            _experimentCount++;
            DataStore.Instance.AddExperiment(string.Format("MyExperiment{0}", _experimentCount), string.Format("C:/Test/MyData{0}", _experimentCount), string.Format("Batch{0}", _batchCount));
            _experimentCount++;
            _batchCount++;
            OnPropertyChanged("ExperimentsDataView");
            OnPropertyChanged("BatchesDataView");
 


        }

        private int _deleteCount = 0;

        private void btnDelete_Click(object sender, RoutedEventArgs e)
        {
            DataStore.Instance.DeleteBatch(string.Format("Batch{0}",_deleteCount));
            _deleteCount++;

            OnPropertyChanged("ExperimentsDataView");
            OnPropertyChanged("BatchesDataView");

        }
    }
}
