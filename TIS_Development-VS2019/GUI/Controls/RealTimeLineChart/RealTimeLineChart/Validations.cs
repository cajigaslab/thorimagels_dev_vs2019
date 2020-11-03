#region Header

//Inspired by the work of Tri Q Tran and Samuel Jack
//http://www.codeproject.com/Members/tri-q-tran
//http://stackoverflow.com/users/1727/samuel-jack

#endregion Header

namespace RealTimeLineChart.InputValidation
{
    using System;
    using System.Collections.ObjectModel;
    using System.Globalization;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Input;

    public static class InputBindingsManager
    {
        #region Fields

        public static readonly DependencyProperty UpdatePropertySourceWhenEnterPressedProperty = DependencyProperty.RegisterAttached(
            "UpdatePropertySourceWhenEnterPressed", typeof(DependencyProperty), typeof(InputBindingsManager), new PropertyMetadata(null, OnUpdatePropertySourceWhenEnterPressedPropertyChanged));

        #endregion Fields

        #region Constructors

        static InputBindingsManager()
        {
        }

        #endregion Constructors

        #region Methods

        public static DependencyProperty GetUpdatePropertySourceWhenEnterPressed(DependencyObject dp)
        {
            return (DependencyProperty)dp.GetValue(UpdatePropertySourceWhenEnterPressedProperty);
        }

        public static void SetUpdatePropertySourceWhenEnterPressed(DependencyObject dp, DependencyProperty value)
        {
            dp.SetValue(UpdatePropertySourceWhenEnterPressedProperty, value);
        }

        static void DoUpdateSource(object source)
        {
            DependencyProperty property =
                GetUpdatePropertySourceWhenEnterPressed(source as DependencyObject);

            if (property == null)
            {
                return;
            }

            UIElement elt = source as UIElement;

            if (elt == null)
            {
                return;
            }

            BindingExpression binding = BindingOperations.GetBindingExpression(elt, property);
            TraversalRequest trNext = new TraversalRequest(FocusNavigationDirection.Next);

            if (binding != null)
            {
                binding.UpdateSource();
                if ((elt.Focusable) && (elt.IsFocused))
                { elt.MoveFocus(trNext); }
            }
        }

        static void HandlePreviewKeyDown(object sender, KeyEventArgs e)
        {
            if ((e.Key == Key.Enter) || (e.Key == Key.Return))
            {
                DoUpdateSource(e.Source);
                e.Handled = true;
            }
        }

        private static void OnUpdatePropertySourceWhenEnterPressedPropertyChanged(DependencyObject dp, DependencyPropertyChangedEventArgs e)
        {
            UIElement element = dp as UIElement;

            if (element == null)
            {
                return;
            }

            if (e.OldValue != null)
            {
                element.PreviewKeyDown -= HandlePreviewKeyDown;
            }

            if (e.NewValue != null)
            {
                element.PreviewKeyDown += new KeyEventHandler(HandlePreviewKeyDown);
            }
        }

        #endregion Methods
    }

    public class Validation : ValidationRule
    {
        #region Fields

        public static readonly DependencyProperty ErrorOutProperty = 
            DependencyProperty.RegisterAttached("ErrorOut", typeof(Collection<ValidationError>), typeof(Validation), new UIPropertyMetadata(null));
        public static readonly DependencyProperty MaxValueProperty = 
            DependencyProperty.RegisterAttached("MaxValue", typeof(double), typeof(Validation), new UIPropertyMetadata(double.NaN));
        public static readonly DependencyProperty MinValueProperty = 
            DependencyProperty.RegisterAttached("MinValue", typeof(double), typeof(Validation), new UIPropertyMetadata(double.NaN));
        public static readonly DependencyProperty NoEmptyValueProperty = 
            DependencyProperty.RegisterAttached("NoEmptyValue", typeof(bool), typeof(Validation), new UIPropertyMetadata(false, OnValidationTypeChanged));
        public static readonly DependencyProperty ValidationTypeProperty = 
            DependencyProperty.RegisterAttached("ValidationType", typeof(Type), typeof(Validation), new UIPropertyMetadata(null, OnValidationTypeChanged));

        #endregion Fields

        #region Constructors

        public Validation()
        {
        }

        public Validation(Type validationType)
        {
            ValidationType = validationType;
        }

        public Validation(object owner,Type validationType, double maxValue, double minValue, bool noEmptyValue, Collection<ValidationError> errors)
        {
            Owner = owner;
            ValidationType = validationType;
            MaxValue = maxValue;
            MinValue = minValue;
            NoEmptyValue = noEmptyValue;
            ErrorOut = errors;
        }

        #endregion Constructors

        #region Properties

        public Collection<ValidationError> ErrorOut
        {
            get; set;
        }

        public double MaxValue
        {
            get; set;
        }

        public double MinValue
        {
            get; set;
        }

        public bool NoEmptyValue
        {
            get; set;
        }

        public object Owner
        {
            get; set;
        }

        public Type ValidationType
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        public static Collection<ValidationError> GetErrorOut(DependencyObject obj)
        {
            return (Collection<ValidationError>)obj.GetValue(ErrorOutProperty);
        }

        public static double GetMaxValue(DependencyObject obj)
        {
            return (double)obj.GetValue(MaxValueProperty);
        }

        public static double GetMinValue(DependencyObject obj)
        {
            return (double)obj.GetValue(MinValueProperty);
        }

        public static bool GetNoEmptyValue(DependencyObject obj)
        {
            return (bool) obj.GetValue(NoEmptyValueProperty);
        }

        public static Type GetValidationType(DependencyObject obj)
        {
            return (Type)obj.GetValue(ValidationTypeProperty);
        }

        public static void SetErrorOut(DependencyObject obj, Collection<object> value)
        {
            obj.SetValue(ErrorOutProperty, value);
        }

        public static void SetMaxValue(DependencyObject obj, double value)
        {
            obj.SetValue(MaxValueProperty, value);
        }

        public static void SetMinValue(DependencyObject obj, double value)
        {
            obj.SetValue(MinValueProperty, value);
        }

        public static void SetNoEmptyValue(DependencyObject obj, bool value)
        {
            obj.SetValue(NoEmptyValueProperty, value);
        }

        public static void SetValidationType(DependencyObject obj, Type value)
        {
            obj.SetValue(ValidationTypeProperty, value);
        }

        public override ValidationResult Validate(object value, CultureInfo cultureInfo)
        {
            bool ret = true;
            string error = string.Empty;

            try
            {
                //check if empty:
                if ((NoEmptyValue) && (string.IsNullOrEmpty(value.ToString().Trim())))
                {
                    ret = false;
                    error += string.Format("Value must not be blank; ");
                }
                // Try converting with the validation Type
                if (ValidationType != null)
                    Convert.ChangeType(value, ValidationType);
                // Check the Max and Min Values
                double numericValue;
                if (double.TryParse(value.ToString(), NumberStyles.Any, cultureInfo, out numericValue))
                {
                    if (MaxValue != double.NaN)
                    {
                        if (numericValue > MaxValue)
                        {
                            ret = false;
                            // return invalid number range error
                            error += string.Format("Value must be less than {0}. ", MaxValue);
                        }
                    }
                    if (MinValue != double.NaN)
                    {
                        if (numericValue < MinValue)
                        {
                            ret = false;
                            // return invalid number range error
                            error += string.Format("Value must be greater than {0}. ", MinValue);
                        }
                    }
                }

            }
            catch (Exception)
            {
                ret = false;
                // return invalid type error.
                if (ValidationType != null)
                {
                    error += string.Format("Value is not of type {0}.", ValidationType.Name);
                }
            }

            var validationResult = new ValidationResult(ret, error);
            var validationError = new ValidationError(Owner, validationResult);
            validationError.Update(ErrorOut);

            return validationResult;
        }

        private static void OnValidationTypeChanged(DependencyObject obj, DependencyPropertyChangedEventArgs args)
        {
            var element = obj as FrameworkElement;
            if (element == null) return;

            // When the element has loaded.
            element.Loaded += (s, e) =>
            {
                // Create a new validator
                var validation = new Validation(obj, GetValidationType(obj), GetMaxValue(obj), GetMinValue(obj), GetNoEmptyValue(obj), GetErrorOut(obj));
                // Get the binding expression for the textbox property
                Binding binding = BindingOperations.GetBinding(obj, TextBox.TextProperty);
                // If not null and doesn't already contain the validator, then add it.
                if (binding != null)
                    if (!binding.ValidationRules.Contains(validation))
                        binding.ValidationRules.Add(validation);
            };
        }

        #endregion Methods
    }

    public class ValidationError
    {
        #region Constructors

        public ValidationError(object owner, ValidationResult result)
        {
            Owner = owner;
            ValidationResult = result;
        }

        #endregion Constructors

        #region Properties

        public object Owner
        {
            get; set;
        }

        public ValidationResult ValidationResult
        {
            get; set;
        }

        #endregion Properties

        #region Methods

        public void Update(Collection<ValidationError> errors)
        {
            if (errors == null || ValidationResult == null) return;

            foreach (ValidationError error in errors)
            {
                if (error.Owner == Owner)
                {
                    errors.Remove(error);
                    break;
                }
            }

            if (!ValidationResult.IsValid) errors.Add(new ValidationError(Owner, ValidationResult));
        }

        #endregion Methods
    }
}