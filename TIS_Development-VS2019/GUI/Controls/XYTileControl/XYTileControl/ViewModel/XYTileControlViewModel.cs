namespace XYTileControl.ViewModel
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
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
    using System.Xml;

    using ThorSharedTypes;

    public interface XYtileViewModel
    {
        #region Methods

        int GetTotalNumberTiles();

        void MoveToNextTile();

        void MoveToPreviousTile();

        void ResetSample();

        void ResetSampleHomeLocation();

        void ResetSampleInScichart();

        void ResetSampleView();

        void RestScanAreaView();

        void ShowTilesGrid(bool? isEnabled);

        void XYTable_AddItem(ObservableCollection<XYPosition> xytableData, XYPosition xyPosition, int index = -1, bool singleTile = false);

        void XYTable_DeleteItem(ObservableCollection<XYPosition> xytableData, int tiledAreaIndex);

        void XYTable_EditItem(ObservableCollection<XYPosition> xytableData, int selectedRow, int selectedColumn, object newValue);

        void XYTable_SelectItem(ObservableCollection<XYPosition> xytableData, int index);

        void XYtileViewModel_Loaded();

        void XYtileViewModel_Unloaded();

        #endregion Methods
    }

    public class Creator
    {
        #region Fields

        private XYTileDisplay _xyTileControl;

        #endregion Fields

        #region Constructors

        public Creator(XYTileDisplay xyTileControl)
        {
            this._xyTileControl = xyTileControl;
        }

        #endregion Constructors

        #region Methods

        public XYtileViewModel FactoryMethod(TileDisplayMode mode)
        {
            XYtileViewModel xyTileViewModel;
            switch (mode)
            {
                case TileDisplayMode.Edit:
                    xyTileViewModel = new EditModeViewModel(_xyTileControl); ;
                    break;
                case TileDisplayMode.Capture:
                    xyTileViewModel = new CaptureModeViewModel(_xyTileControl);
                    break;
                case TileDisplayMode.View:
                    xyTileViewModel = new ViewModeViewModel(_xyTileControl);
                    break;
                case TileDisplayMode.DefaultView:
                    xyTileViewModel = new DefaultViewModeViewMode(_xyTileControl);
                    break;
                default:
                    xyTileViewModel = new EditModeViewModel(_xyTileControl);
                    break;
            }
            return xyTileViewModel;
        }

        #endregion Methods
    }
}