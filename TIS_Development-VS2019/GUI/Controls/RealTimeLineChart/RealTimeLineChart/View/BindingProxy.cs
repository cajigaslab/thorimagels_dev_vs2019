﻿namespace RealTimeLineChart.View
{
    using System.Windows;

    public class BindingProxy : Freezable
    {
        #region Fields

        // Using a DependencyProperty as the backing store for Data.
        // This enables animation, styling, binding, etc...
        public static readonly DependencyProperty DataProperty = 
            DependencyProperty.Register("Data", typeof(object),
            typeof(BindingProxy), new UIPropertyMetadata(null));

        #endregion Fields

        #region Properties

        public object Data
        {
            get { return (object)GetValue(DataProperty); }
            set { SetValue(DataProperty, value); }
        }

        #endregion Properties

        #region Methods

        protected override Freezable CreateInstanceCore()
        {
            return new BindingProxy();
        }

        #endregion Methods
    }
}