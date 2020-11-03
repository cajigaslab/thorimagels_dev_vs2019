namespace XMLViewer
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Media;
    using System.Xml;
    using System.Text;
    

    using Microsoft.Win32;

    //}/// <summary>
    /// Exposes attached behaviors that can be
    /// applied to TreeViewItem objects.
    /// </summary>
    public static class TreeViewItemBehavior
    {
        #region Fields

        public static readonly DependencyProperty IsBroughtIntoViewWhenSelectedProperty = 
         DependencyProperty.RegisterAttached(
         "IsBroughtIntoViewWhenSelected",
         typeof(bool),
         typeof(TreeViewItemBehavior),
         new UIPropertyMetadata(false, OnIsBroughtIntoViewWhenSelectedChanged));

        #endregion Fields

        #region Methods

        public static bool GetIsBroughtIntoViewWhenSelected(TreeViewItem treeViewItem)
        {
            return (bool)treeViewItem.GetValue(IsBroughtIntoViewWhenSelectedProperty);
        }

        public static void SetIsBroughtIntoViewWhenSelected(TreeViewItem treeViewItem, bool value)
        {
            treeViewItem.SetValue(IsBroughtIntoViewWhenSelectedProperty, value);
        }

        static void OnIsBroughtIntoViewWhenSelectedChanged(DependencyObject depObj, DependencyPropertyChangedEventArgs e)
        {
            TreeViewItem item = depObj as TreeViewItem;
            if (item == null)
                return;

            if (e.NewValue is bool == false)
                return;

            if ((bool)e.NewValue)
                item.Selected += OnTreeViewItemSelected;
            else
                item.Selected -= OnTreeViewItemSelected;
        }

        static void OnTreeViewItemSelected(object sender, RoutedEventArgs e)
        {
            // Only react to the Selected event raised by the TreeViewItem
            // whose IsSelected property was modified. Ignore all ancestors
            // who are merely reporting that a descendant's Selected fired.
            if (!Object.ReferenceEquals(sender, e.OriginalSource))
                return;

            TreeViewItem item = e.OriginalSource as TreeViewItem;
            if (item != null)
                item.BringIntoView();
        }

        #endregion Methods
    }

    /// <summary>
    /// Interaction logic for Viewer.xaml
    /// </summary>
    public partial class Viewer : UserControl
    {
        #region Fields

        private List<TreeViewItem> CurrentNodeMatches = new List<TreeViewItem>();
        private string LastSearchText;
        private int NewNodeIndex = 0;
        private bool _collapseAllAttributes;
        private bool _collapseAllElements;
        private string _docPath = null;
        private bool _expandAllAttributes;
        private bool _expandAllElements;
        private bool _isSaved;
        private bool _isValueChanged = false;
        private int _validIndex = -1;
        private XmlDocument _xmldocument;

        #endregion Fields

        #region Constructors

        public Viewer()
        {
            InitializeComponent();
            SearchText = "Search Settings";
            tbSearch.Foreground = new SolidColorBrush(Colors.LightGray);
            
            this.DataContext = this;
        }

        #endregion Constructors

        #region Properties

        public bool CollapseAllAttributes
        {
            get
            {
                return _collapseAllAttributes;
            }
            set
            {
                try
                {
                    _collapseAllAttributes = true;
                    collapseTreeAttributes();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        public bool CollapseAllElements
        {
            get
            {
                return _collapseAllElements;
            }
            set
            {
                try
                {
                    _collapseAllElements = true;
                    collapseTreeElements();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        public bool ExpandAllAttributes
        {
            get { return _expandAllAttributes; }
            set
            {
                try
                {
                    _expandAllAttributes = true;
                    expandTreeAttributes();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        public bool ExpandAllElements
        {
            get
            {
                return _expandAllElements;
            }
            set
            {
                try
                {
                    _expandAllElements = true;
                    expandTreeElements();
                }
                catch(Exception ex)
                {
                    MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        public bool IsSaved
        {
            get
            {
                return _isSaved;
            }
            set
            {
                try
                {
                    _isSaved = true;
                    saveFile();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        public string SearchFoundText
        {
            get;
            set;
        }

        public string SearchText
        {
            get;
            set;
        }

        public XmlDocument XmlDocument
        {
            get { return _xmldocument; }
            set
            {
                try
                {
                    if (value != null)
                    {
                        _xmldocument = value;
                        _docPath = _xmldocument.BaseURI.Replace("file:///", "");
                        _docPath = _docPath.Replace("/","\\");
                        bindXMLDocument();
                        _isValueChanged = false;
                    }
                    else
                    {
                        if (_isValueChanged)
                        {
                            String str = String.Format("Do you want to save changes to {0}?",_docPath);
                            MessageBoxResult result = MessageBox.Show(str, "Save", MessageBoxButton.YesNo, MessageBoxImage.Question);
                            if (result == MessageBoxResult.Yes)
                            {
                                _isValueChanged = false;
                                saveFile();
                            }
                        }
                       _xmldocument = null;
                       bindXMLDocument();

                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        #endregion Properties

        #region Methods

        public void btnNext_Click(object sender, RoutedEventArgs e)
        {
            int direction = 1;
            Search(direction);
        }

        public void btnPrev_Click(object sender, RoutedEventArgs e)
        {
            int direction = -1;
            Search(direction);
        }

        public void tbSearch_MouseDown(object sender, EventArgs e)
        {
            if (SearchText == "Search Settings")
            {
                
                tbSearch.Clear();
                tbSearch.Foreground = new SolidColorBrush(Colors.White);
            }
        }

        public void tbSearch_LostFocus(object sender, EventArgs e)
        {
            if (SearchText == "Search Settings" || SearchText.Length==0)
            {
                
                
                tbSearch.Text = "Search Settings";

                tbSearch.Foreground = new SolidColorBrush(Colors.LightGray);
                
            }
        }



        private void bindXMLDocument()
        {
            try
            {
                if (_xmldocument == null)
                {
                    xmlTree.ItemsSource = null;
                    return;
                }
                XmlDataProvider provider = new XmlDataProvider();
                provider.Document = _xmldocument;
                Binding binding = new Binding();
                binding.Source = provider;
                binding.XPath = "child::node()";
                xmlTree.SetBinding(TreeView.ItemsSourceProperty, binding);

                _validIndex = -1;
                foreach(XmlNode node in _xmldocument.ChildNodes)
                {
                    if (node.GetType().ToString() != "System.Xml.XmlElement")
                        _validIndex++;
                    else
                        break;
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void collapseTreeAttributes()
        {
            try
            {
                if (xmlTree.ItemsSource == null)
                    return;

                TreeViewItem outerTreeItem = (TreeViewItem)(xmlTree.ItemContainerGenerator.ContainerFromIndex(_validIndex));
                ContentPresenter CP = FindVisualChild<ContentPresenter>(outerTreeItem as DependencyObject);
                HierarchicalDataTemplate HT = (HierarchicalDataTemplate)CP.ContentTemplate;
                TreeViewItem innerTreeItem = (TreeViewItem)HT.FindName("attributeTreeView", CP);
                innerTreeItem.IsExpanded = false;

                if (outerTreeItem.HasItems)
                {
                    collapseTreeAttributesRecursively(outerTreeItem);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void collapseTreeAttributesRecursively(TreeViewItem outerTreeItem)
        {
            try
            {
                TreeViewItem TreeItem;
                ContentPresenter CP;
                HierarchicalDataTemplate HT;
                TreeViewItem innerTreeItem;

                for (int i = 0; i < outerTreeItem.Items.Count; i++)
                {
                    TreeItem = (TreeViewItem)(outerTreeItem.ItemContainerGenerator.ContainerFromIndex(i));
                    CP = FindVisualChild<ContentPresenter>(TreeItem as DependencyObject);
                    HT = (HierarchicalDataTemplate)CP.ContentTemplate;
                    innerTreeItem = (TreeViewItem)HT.FindName("attributeTreeView", CP);
                    innerTreeItem.IsExpanded = false;
                    if (TreeItem.HasItems)
                    {
                        collapseTreeAttributesRecursively(TreeItem);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void collapseTreeElements()
        {
            try
            {
                if (xmlTree.ItemsSource == null)
                    return;

                TreeViewItem outerTreeItem = (TreeViewItem)(xmlTree.ItemContainerGenerator.ContainerFromIndex(_validIndex));
                outerTreeItem.IsExpanded = false;

                if (outerTreeItem.HasItems)
                {
                    collapseTreeElementsRecursively(outerTreeItem);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void collapseTreeElementsRecursively(TreeViewItem ParentTreeItem)
        {
            try
            {
                TreeViewItem TreeItem;

                for (int i = 0; i < ParentTreeItem.Items.Count; i++)
                {
                    TreeItem = (TreeViewItem)(ParentTreeItem.ItemContainerGenerator.ContainerFromIndex(i));

                    if (TreeItem != null)
                    {
                        TreeItem.IsExpanded = false;
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void expandTreeAttributes()
        {
            try
            {
                if (xmlTree.ItemsSource == null)
                    return;

                TreeViewItem outerTreeItem = (TreeViewItem)(xmlTree.ItemContainerGenerator.ContainerFromIndex(_validIndex));
                if (outerTreeItem != null)
                {
                    ContentPresenter CP = FindVisualChild<ContentPresenter>(outerTreeItem as DependencyObject);
                    HierarchicalDataTemplate HT = (HierarchicalDataTemplate)CP.ContentTemplate;
                    TreeViewItem innerTreeItem = (TreeViewItem)HT.FindName("attributeTreeView", CP);
                    innerTreeItem.IsExpanded = true;

                    if (outerTreeItem.HasItems)
                    {
                        expandTreeAttributesRecursively(outerTreeItem);
                    }
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void expandTreeAttributesRecursively(TreeViewItem ParentTreeItem)
        {
            try
            {
                TreeViewItem TreeItem;
                ContentPresenter CP;
                HierarchicalDataTemplate HT;
                TreeViewItem innerTreeItem;

                for(int i=0;i<ParentTreeItem.Items.Count;i++)
                    {
                        TreeItem = (TreeViewItem)(ParentTreeItem.ItemContainerGenerator.ContainerFromIndex(i));
                        if (TreeItem != null)
                        {
                            CP = FindVisualChild<ContentPresenter>(TreeItem as DependencyObject);
                            HT = (HierarchicalDataTemplate)CP.ContentTemplate;
                            innerTreeItem = (TreeViewItem)HT.FindName("attributeTreeView", CP);
                            innerTreeItem.IsExpanded = true;
                            if (TreeItem.HasItems)
                            {
                                expandTreeAttributesRecursively(TreeItem);
                            }
                        }

                    }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void expandTreeElements()
        {
            try
            {
                if (xmlTree.ItemsSource == null)
                    return;

                TreeViewItem outerTreeItem = (TreeViewItem)(xmlTree.ItemContainerGenerator.ContainerFromIndex(_validIndex));

                outerTreeItem.IsExpanded = true;

                if (outerTreeItem.HasItems)
                {
                    expandTreeElementsRecursively(outerTreeItem);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void expandTreeElementsRecursively(TreeViewItem ParentTreeItem)
        {
            try
            {
                TreeViewItem TreeItem;

                for (int i = 0; i < ParentTreeItem.Items.Count; i++)
                {
                    TreeItem = (TreeViewItem)(ParentTreeItem.ItemContainerGenerator.ContainerFromIndex(i));

                    if (TreeItem != null)
                    {
                        TreeItem.IsExpanded = true;
                    }

                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private childItem FindVisualChild<childItem>(DependencyObject obj)
            where childItem : DependencyObject
        {
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(obj); i++)
            {
                DependencyObject child = VisualTreeHelper.GetChild(obj, i);
                if (child != null && child is childItem)
                    return (childItem)child;
                else
                {
                    childItem childOfChild = FindVisualChild<childItem>(child);
                    if (childOfChild != null)
                        return childOfChild;
                }
            }
            return null;
        }

        private void saveFile()
        {
            try
            {
                if (_xmldocument != null)
                {
                    _xmldocument.Save(_docPath);
                    _isValueChanged = false;
                    /*
                    SaveFileDialog saveDialog = new SaveFileDialog();
                    saveDialog.AddExtension = true;
                    saveDialog.DefaultExt = ".xml";
                    saveDialog.Title = "Save As";
                    saveDialog.FileName = _docPath;
                    if (saveDialog.ShowDialog() == true)
                    {
                        string path = saveDialog.FileName;
                        _xmldocument.Save(path);
                        _isValueChanged = false;
                        MessageBox.Show("File saved.", "Save", MessageBoxButton.OK, MessageBoxImage.Information);
                    }
                    else
                    {
                        MessageBox.Show("File save canceled!", "Save", MessageBoxButton.OK, MessageBoxImage.Information);
                    }
                    */

                }
                else
                {
                    MessageBox.Show("No file to save!", "Save", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void Search(int direction)
        {
            string searchText = SearchText;

            if (String.IsNullOrEmpty(searchText))
            {
                return;
            };

            if (LastSearchText != searchText)
            {
                //It's a new Search
                CurrentNodeMatches.Clear();
                LastSearchText = searchText;
                NewNodeIndex = 0;

                SearchNodes(searchText, (ItemsControl)xmlTree.ItemContainerGenerator.ContainerFromIndex(0));
            }

            if (NewNodeIndex >= 0 && CurrentNodeMatches.Count > 0)
            {
                if (NewNodeIndex > 0)
                {
                    CurrentNodeMatches[NewNodeIndex].IsSelected = false;
                }
                if (1 == direction) { NewNodeIndex++; } else if(-1 == direction){ NewNodeIndex--; }

                if (NewNodeIndex == CurrentNodeMatches.Count)
                {
                    NewNodeIndex = 0;
                }

                if (NewNodeIndex < 0)
                {
                    NewNodeIndex = CurrentNodeMatches.Count - 1;
                }

                xmlTree.Items.MoveCurrentTo(NewNodeIndex);

                CurrentNodeMatches[NewNodeIndex].IsSelected = true;

                SearchFoundText = string.Format("{0} of {1}", NewNodeIndex + 1, CurrentNodeMatches.Count);

            }
            else
            {
                SearchFoundText = "0 of 0";
            }
            var bindingExpression = lbSearchFound.GetBindingExpression(Label.ContentProperty);
            bindingExpression.UpdateTarget();
        }

        private void SearchNodes(string searchText, ItemsControl itemsControl)
        {
            TreeViewItem tvi = (TreeViewItem)itemsControl;

            if (tvi != null)
            {
                string str = ((XmlLinkedNode)tvi.Header).Name;

                if (str.ToLower().Contains(searchText.ToLower()))
                {
                    CurrentNodeMatches.Add(tvi);
                    tvi.IsSelected = true;
                }
                else
                {
                    if (((XmlLinkedNode)tvi.Header).Attributes != null)
                    {
                        for (int i = 0; i < ((XmlLinkedNode)tvi.Header).Attributes.Count; i++)
                        {
                            if (((XmlLinkedNode)tvi.Header).Attributes[i].Name.ToLower().Contains(searchText.ToLower()) || ((XmlLinkedNode)tvi.Header).Attributes[i].Value.ToLower().Contains(searchText.ToLower()))
                            {
                                CurrentNodeMatches.Add(tvi);
                                tvi.IsSelected = true;
                                break;
                            }
                        }
                    }

                }

                ItemContainerGenerator itemContainerGenerator = itemsControl.ItemContainerGenerator;

                if (itemContainerGenerator.Status == System.Windows.Controls.Primitives.GeneratorStatus.ContainersGenerated)
                {
                    for(int i=0; i<itemsControl.Items.Count; i++)
                    {
                        ItemsControl childControl = itemContainerGenerator.ContainerFromIndex(i) as ItemsControl;
                        if (childControl != null)
                        {
                            SearchNodes(searchText, childControl);
                        }
                    }
                }

            }
        }

        private void tbSearch_TextChanged(object sender, TextChangedEventArgs e)
        {
            var bindingExpression = tbSearch.GetBindingExpression(TextBox.TextProperty);
            bindingExpression.UpdateSource();

            Search(0);
        }

        private void TextBox_GotFocus(object sender, RoutedEventArgs e)
        {
            _isValueChanged = true;
        }

        #endregion Methods
    }
}