using TilesDisplayTest;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Markup;
using System.Threading;
using IcuTesting.WPF;

namespace TestProject1
{
    
    
    /// <summary>
    ///This is a test class for Window1Test and is intended
    ///to contain all Window1Test Unit Tests
    ///</summary>
    [TestClass()]
    public class Window1Test : MSTestIcuBase
    {
        #region Additional test attributes
        // 
        //You can use the following additional attributes as you write your tests:
        //
        //Use ClassInitialize to run code before running the first test in the class
        //[ClassInitialize()]
        //public static void MyClassInitialize(TestContext testContext)
        //{
        //}
        //
        //Use ClassCleanup to run code after all tests in a class have run
        //[ClassCleanup()]
        //public static void MyClassCleanup()
        //{
        //}
        //
        //Use TestInitialize to run code before running each test
        //[TestInitialize()]
        //public void MyTestInitialize()
        //{
        //}
        //
        //Use TestCleanup to run code after each test has run
        //[TestCleanup()]
        //public void MyTestCleanup()
        //{
        //}
        //
        #endregion

        static GuiHelper guiHelper = new GuiHelper();

        [TestMethod]
        public void Defaut_Config_Test()
        {
            ICU.Invoke(() =>
            {
                var w = new Window1();
             
                w.Show();
                Thread.Sleep(2000);
                ICU.CheckView(w, "Defaut_Config_Test", "Initial Window.");
                Thread.Sleep(2000);
                w.Close();
            });
        }
        [TestMethod]
        public void change_rows_and_columns()
        {
            var context = new WindowScenario<Window1>();
            ICU.Given(context)
                .AsA("Window1 Test")
                .When(() =>
                {
                    var w = context.Window;
                    var txtRows = guiHelper.Find<TextBox>(w, "tbRows");
                    var txtColumns = guiHelper.Find<TextBox>(w, "tbColumns");
                    txtRows.Text = "4";
                    txtColumns.Text = "4";
                    
                    var btnSelectFirst = guiHelper.Find<Button>(w, "btnSetFirst");
                    guiHelper.Click(btnSelectFirst);
                })
                .Then(() =>
                {
                    ICU.CheckView(context.Window, "change_rows_and_columns", "there should be 4 rows and 4 columns");
                })
                .Test();
        }
    }

}
