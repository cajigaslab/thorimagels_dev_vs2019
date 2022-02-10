namespace MyBehaviorAssembly
{
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;
    using System.Windows.Input;

    using Microsoft.Xaml.Behaviors;

    //Class for setting behavior of a slider to update value when a slider drag is completed or when the mouse is unpressed after being held down
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
            RoutedEventHandler handler = AssociatedObject_ValueChanged;
            AssociatedObject.AddHandler(Thumb.DragCompletedEvent, handler);
            AssociatedObject.AddHandler(Mouse.PreviewMouseUpEvent, handler);
        }

        protected override void OnDetaching()
        {
            RoutedEventHandler handler = AssociatedObject_ValueChanged;
            AssociatedObject.RemoveHandler(Thumb.DragCompletedEvent, handler);
            AssociatedObject.RemoveHandler(Mouse.PreviewMouseUpEvent, handler);
        }

        private void AssociatedObject_ValueChanged(object sender, RoutedEventArgs e)
        {
            Value = (float)AssociatedObject.Value;
        }

        #endregion Methods
    }
}