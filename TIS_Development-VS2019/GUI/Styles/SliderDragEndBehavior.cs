﻿namespace MyBehaviorAssembly
{
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;

    using Microsoft.Xaml.Behaviors;

    public class SliderDragEndValueBehavior : Behavior<Slider>
    {
        #region Fields

        public static readonly DependencyProperty ValueProperty = DependencyProperty.Register(
            "Value", typeof(float), typeof(SliderDragEndValueBehavior), new PropertyMetadata(default(float)));

        #endregion Fields

        #region Properties

        public float Value
        {
            get { return (float)GetValue(ValueProperty); }
            set { SetValue(ValueProperty, value); }
        }

        #endregion Properties

        #region Methods

        protected override void OnAttached()
        {
            RoutedEventHandler handler = AssociatedObject_DragCompleted;
            AssociatedObject.AddHandler(Thumb.DragCompletedEvent, handler);
        }

        protected override void OnDetaching()
        {
            RoutedEventHandler handler = AssociatedObject_DragCompleted;
            AssociatedObject.RemoveHandler(Thumb.DragCompletedEvent, handler);
        }

        private void AssociatedObject_DragCompleted(object sender, RoutedEventArgs e)
        {
            Value = (float)AssociatedObject.Value;
        }

        #endregion Methods
    }
}