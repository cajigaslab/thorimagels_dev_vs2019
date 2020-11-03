#region Header

/*
 * MathConverter and accompanying samples are copyright (c) 2011 by Ivan Krivyakov
 * ivan [at] ikriv.com
 * They are distributed under the Apache License http://www.apache.org/licenses/LICENSE-2.0.html
 */

#endregion Header

namespace IKriv.Wpf
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Text.RegularExpressions;
    using System.Windows;
    using System.Windows.Data;
    using System.Windows.Markup;

    /// <summary>
    /// Value converter that performs arithmetic calculations over its argument(s)
    /// </summary>
    /// <remarks>
    /// MathConverter can act as a value converter, or as a multivalue converter (WPF only).
    /// It is also a markup extension (WPF only) which allows to avoid declaring resources,
    /// ConverterParameter must contain an arithmetic expression over converter arguments. Operations supported are +, -, * and /
    /// Single argument of a value converter may referred as x, a, or {0}
    /// Arguments of multi value converter may be referred as x,y,z,t (first-fourth argument), or a,b,c,d, or {0}, {1}, {2}, {3}, {4}, ...
    /// The converter supports arithmetic expressions of arbitrary complexity, including nested subexpressions
    /// </remarks>
    public class MathConverter : MarkupExtension, IMultiValueConverter, IValueConverter
    {
        #region Fields

        Dictionary<string, IExpression> _storedExpressions = new Dictionary<string, IExpression>();

        #endregion Fields

        #region Nested Interfaces

        interface IExpression
        {
            #region Methods

            decimal Eval(object[] args);

            #endregion Methods
        }

        #endregion Nested Interfaces

        #region Methods

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return Convert(new object[] { value }, targetType, parameter, culture);
        }

        public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
        {
            try
            {
                // string format is not available with 1/(y-x), use rounding instead
                // supplied with parameter by separator '~'
                string roundValSeparator = "~";
                int roundVal = -1;
                if (((string)parameter).Contains(roundValSeparator))
                {
                    string[] split = Regex.Split((string)parameter, roundValSeparator);
                    Int32.TryParse(split[split.Length - 1], out roundVal);
                    parameter = split[0];
                }
                decimal result = Parse(parameter.ToString()).Eval(values);
                result = (0 < roundVal) ? Math.Round(result, roundVal) : result;

                if (targetType == typeof(decimal)) return result;
                if (targetType == typeof(string)) return result.ToString();
                if (targetType == typeof(int)) return (int)result;
                if (targetType == typeof(double)) return (double)result;
                if (targetType == typeof(long)) return (long)result;
                if (targetType == typeof(object)) return (object)result;
                throw new ArgumentException(String.Format("Unsupported target type {0}", targetType.FullName));
            }
            catch (Exception ex)
            {
                ProcessException(ex);
            }

            return DependencyProperty.UnsetValue;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }

        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            return this;
        }

        protected virtual void ProcessException(Exception ex)
        {
            Console.WriteLine(ex.Message);
        }

        private IExpression Parse(string s)
        {
            IExpression result = null;
            if (!_storedExpressions.TryGetValue(s, out result))
            {
                result = new Parser().Parse(s);
                _storedExpressions[s] = result;
            }

            return result;
        }

        #endregion Methods

        #region Nested Types

        class BinaryOperation : IExpression
        {
            #region Fields

            private IExpression _left;
            private Func<decimal, decimal, decimal> _operation;
            private IExpression _right;

            #endregion Fields

            #region Constructors

            public BinaryOperation(char operation, IExpression left, IExpression right)
            {
                _left = left;
                _right = right;
                switch (operation)
                {
                    case '+': _operation = (a, b) => (a + b); break;
                    case '-': _operation = (a, b) => (a - b); break;
                    case '*': _operation = (a, b) => (a * b); break;
                    case '/': _operation = (a, b) => (a / b); break;
                    default: throw new ArgumentException("Invalid operation " + operation);
                }
            }

            #endregion Constructors

            #region Methods

            public decimal Eval(object[] args)
            {
                return _operation(_left.Eval(args), _right.Eval(args));
            }

            #endregion Methods
        }

        class Constant : IExpression
        {
            #region Fields

            private decimal _value;

            #endregion Fields

            #region Constructors

            public Constant(string text)
            {
                if (!decimal.TryParse(text, out _value))
                {
                    throw new ArgumentException(String.Format("'{0}' is not a valid number", text));
                }
            }

            #endregion Constructors

            #region Methods

            public decimal Eval(object[] args)
            {
                return _value;
            }

            #endregion Methods
        }

        class Negate : IExpression
        {
            #region Fields

            private IExpression _param;

            #endregion Fields

            #region Constructors

            public Negate(IExpression param)
            {
                _param = param;
            }

            #endregion Constructors

            #region Methods

            public decimal Eval(object[] args)
            {
                return -_param.Eval(args);
            }

            #endregion Methods
        }

        class Parser
        {
            #region Fields

            private int pos;
            private string text;

            #endregion Fields

            #region Methods

            public IExpression Parse(string text)
            {
                try
                {
                    pos = 0;
                    this.text = text;
                    var result = ParseExpression();
                    RequireEndOfText();
                    return result;
                }
                catch (Exception ex)
                {
                    string msg =
                        String.Format("MathConverter: error parsing expression '{0}'. {1} at position {2}", text, ex.Message, pos);

                    throw new ArgumentException(msg, ex);
                }
            }

            private IExpression CreateVariable(int n)
            {
                ++pos;
                SkipWhiteSpace();
                return new Variable(n);
            }

            private IExpression ParseExpression()
            {
                IExpression left = ParseTerm();

                while (true)
                {
                    if (pos >= text.Length) return left;

                    var c = text[pos];

                    if (c == '+' || c == '-')
                    {
                        ++pos;
                        IExpression right = ParseTerm();
                        left = new BinaryOperation(c, left, right);
                    }
                    else
                    {
                        return left;
                    }
                }
            }

            private IExpression ParseFactor()
            {
                SkipWhiteSpace();
                if (pos >= text.Length) throw new ArgumentException("Unexpected end of text");

                var c = text[pos];

                if (c == '+')
                {
                    ++pos;
                    return ParseFactor();
                }

                if (c == '-')
                {
                    ++pos;
                    return new Negate(ParseFactor());
                }

                if (c == 'x' || c == 'a') return CreateVariable(0);
                if (c == 'y' || c == 'b') return CreateVariable(1);
                if (c == 'z' || c == 'c') return CreateVariable(2);
                if (c == 't' || c == 'd') return CreateVariable(3);

                if (c == '(')
                {
                    ++pos;
                    var expression = ParseExpression();
                    SkipWhiteSpace();
                    Require(')');
                    SkipWhiteSpace();
                    return expression;
                }

                if (c == '{')
                {
                    ++pos;
                    var end = text.IndexOf('}', pos);
                    if (end < 0) { --pos; throw new ArgumentException("Unmatched '{'"); }
                    if (end == pos) { throw new ArgumentException("Missing parameter index after '{'"); }
                    var result = new Variable(text.Substring(pos, end - pos).Trim());
                    pos = end + 1;
                    SkipWhiteSpace();
                    return result;
                }

                const string decimalRegEx = @"(\d+\.?\d*|\d*\.?\d+)";
                var match = Regex.Match(text.Substring(pos), decimalRegEx);
                if (match.Success)
                {
                    pos += match.Length;
                    SkipWhiteSpace();
                    return new Constant(match.Value);
                }
                else
                {
                    throw new ArgumentException(String.Format("Unexpeted character '{0}'", c));
                }
            }

            private IExpression ParseTerm()
            {
                IExpression left = ParseFactor();

                while (true)
                {
                    if (pos >= text.Length) return left;

                    var c = text[pos];

                    if (c == '*' || c == '/')
                    {
                        ++pos;
                        IExpression right = ParseFactor();
                        left = new BinaryOperation(c, left, right);
                    }
                    else
                    {
                        return left;
                    }
                }
            }

            private void Require(char c)
            {
                if (pos >= text.Length || text[pos] != c)
                {
                    throw new ArgumentException("Expected '" + c + "'");
                }

                ++pos;
            }

            private void RequireEndOfText()
            {
                if (pos != text.Length)
                {
                    throw new ArgumentException("Unexpected character '" + text[pos] + "'");
                }
            }

            private void SkipWhiteSpace()
            {
                while (pos < text.Length && Char.IsWhiteSpace((text[pos]))) ++pos;
            }

            #endregion Methods
        }

        class Variable : IExpression
        {
            #region Fields

            private int _index;

            #endregion Fields

            #region Constructors

            public Variable(string text)
            {
                if (!int.TryParse(text, out _index) || _index < 0)
                {
                    throw new ArgumentException(String.Format("'{0}' is not a valid parameter index", text));
                }
            }

            public Variable(int n)
            {
                _index = n;
            }

            #endregion Constructors

            #region Methods

            public decimal Eval(object[] args)
            {
                if (_index >= args.Length)
                {
                    throw new ArgumentException(String.Format("MathConverter: parameter index {0} is out of range. {1} parameter(s) supplied", _index, args.Length));
                }

                if (typeof(System.Xml.XmlAttribute) == args[_index].GetType())
                {
                    return System.Convert.ToDecimal(((System.Xml.XmlAttribute)args[_index]).Value);
                }
                return System.Convert.ToDecimal(args[_index]);
            }

            #endregion Methods
        }

        #endregion Nested Types
    }
}