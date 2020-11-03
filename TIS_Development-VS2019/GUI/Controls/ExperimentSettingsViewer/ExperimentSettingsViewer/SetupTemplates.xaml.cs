namespace ExperimentSettingsViewer
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Shapes;

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class SetupTemplates : Window
    {
        #region Fields

        string _strTemplates;

        #endregion Fields

        #region Constructors

        public SetupTemplates()
        {
            InitializeComponent();
            this.Loaded += new RoutedEventHandler(SetupTemplates_Loaded);
            this.Unloaded += new RoutedEventHandler(SetupTemplates_Unloaded);
        }

        #endregion Constructors

        #region Methods

        private void btnAdd_Click(object sender, RoutedEventArgs e)
        {
            Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

            dlg.FileName = "Experiment";
            dlg.DefaultExt = ".xml";
            dlg.Filter = "ThorImageExperiment (.xml)|*.xml";

            Nullable<bool> result = dlg.ShowDialog();

            if (true == result)
            {
                string strOriginalName = dlg.FileName;
                string strOriginalNameNoExt = System.IO.Path.GetFileNameWithoutExtension(dlg.FileName);

                do
                {
                    TemplateName dlgTemplate = new TemplateName();

                    dlgTemplate.NewName = strOriginalNameNoExt;

                    if (false == dlgTemplate.ShowDialog())
                    {
                        break;   //cancel button hits this
                    }

                    string strResult =  _strTemplates + "\\" + dlgTemplate.NewName + ".xml";

                    if (false == File.Exists(strResult))
                    {
                        //make a copy of the file with the new name into the template directory
                        File.Copy(strOriginalName, strResult);
                        break;
                    }
                    else
                    {
                        MessageBox.Show("Template name already exists! The new template will require a unique name.");
                    }
                }
                while (true);

                BuildTemplateList();
            }
        }

        private void btnDelete_Click(object sender, RoutedEventArgs e)
        {
            if (lbTemplates.SelectedIndex >= 0)
            {
                if (lbTemplates.Items[lbTemplates.SelectedIndex].Equals("Active"))
                {
                    MessageBox.Show("This template cannot be deleted");
                    return;
                }

                string strResult = _strTemplates + "\\" + lbTemplates.Items[lbTemplates.SelectedIndex] + ".xml";

                try
                {
                    if (MessageBoxResult.Yes == MessageBox.Show("Are you sure you want to delete the template?", "", MessageBoxButton.YesNo))
                    {
                        File.Delete(strResult);
                        BuildTemplateList();
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }
            }
        }

        private void btnOK_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void BuildTemplateList()
        {
            lbTemplates.Items.Clear();

            _strTemplates = Application.Current.Resources["TemplatesFolder"].ToString();

            string[] strList = Directory.GetFiles(_strTemplates, "*.xml");

            for (int i = 0; i < strList.Length; i++)
            {
                string str = System.IO.Path.GetFileNameWithoutExtension(strList[i]);

                //do not populate the list with the active and default templates
                if (str.Equals("Active") || str.Equals("Default"))
                {
                    continue;
                }

                lbTemplates.Items.Add(str);
            }
        }

        void SetupTemplates_Loaded(object sender, RoutedEventArgs e)
        {
            BuildTemplateList();
        }

        void SetupTemplates_Unloaded(object sender, RoutedEventArgs e)
        {
        }

        #endregion Methods
    }
}