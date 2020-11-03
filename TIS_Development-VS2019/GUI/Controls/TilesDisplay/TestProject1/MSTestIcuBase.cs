using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using IcuTesting.WPF;


namespace TestProject1
{
    [TestClass]
    public class MSTestIcuBase
    {
        protected IcuTest icu;

        public virtual IcuTest ICU
        {
            get
            {
                if (icu == null)
                {
                    icu = IcuTestStarter.Startup(@"C:\Users\mgao\Documents\Visual Studio 2008\Projects\TilesDisplayTest\Icu");
                }
                return icu;
            }
        }

        public TestContext TestContext { get; set; }

        [TestInitialize()]
        public void TestInitialize()
        {
            ICU.BeginTest(TestContext.TestName);
        }

        [TestCleanup()]
        public void TestCleanup()
        {
            ICU.EndTest();
        }

        [AssemblyCleanup()]
        public static void AssemblyCleanup()
        {
            IcuTestStarter.Shutdown();
        }
    }
}
