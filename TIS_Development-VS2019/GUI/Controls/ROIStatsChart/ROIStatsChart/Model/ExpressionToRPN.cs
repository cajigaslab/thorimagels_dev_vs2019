namespace ROIStatsChart.Model
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;

    class ExpressionToRPN
    {
        #region Fields

        private const string OPS = "-+/*^";

        private static Dictionary<string, char> _associativity = new Dictionary<string, char>()
            {
                {"^", 'R'},
                {"*", 'L'},
                {"/", 'L'},
                {"+", 'L'},
                {"-", 'L'}
            };
        private static Dictionary<string, int> _precedence = new Dictionary<string, int>()
            {
                {"^", 4},
                {"*", 3},
                {"/", 3},
                {"+", 2},
                {"-", 2}
            };

        #endregion Fields

        #region Methods

        //Shunting-yard algorithm to convert an Algebraic expression
        //into Reverse Polish Notation (RPN)
        public static string[] GetRPN(string[] expression)
        {
            Stack<string>  stack = new Stack<string>();
            List<string> rpnOutput = new List<string>();
            for (int i = 0; i < expression.Length; i++)
            {

                if (expression[i] == "+" || expression[i] == "-" || expression[i] == "*" || expression[i] == "/" || expression[i] == "^")
                {
                    string o1 = expression[i];
                    if (stack.Count > 0)
                    {
                        while (stack.Count > 0 && OPS.IndexOf(stack.Peek()) != -1 &&
                            (('L' == _associativity[o1] && _precedence[o1] <= _precedence[stack.Peek()]) ||
                            ('R' == _associativity[o1] && _precedence[o1] <= _precedence[stack.Peek()])))
                        {
                            rpnOutput.Add(stack.Pop());
                        }
                    }
                    stack.Push(o1);
                }
                else if (expression[i] == "(")
                {
                    stack.Push(expression[i]);
                }
                else if (expression[i] == ")")
                {
                    while (stack.Peek() != "(")
                    {
                        rpnOutput.Add(stack.Pop());
                    }
                    stack.Pop();
                }
                else
                {
                    rpnOutput.Add(expression[i]);
                }
            }
            while (stack.Count > 0)
            {
                rpnOutput.Add(stack.Pop());
            }

            return rpnOutput.ToArray();
        }

        #endregion Methods
    }
}