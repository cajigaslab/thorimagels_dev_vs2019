using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using ThorSharedTypes;

namespace FileNameTestClass
{
    
    [TestClass]
    public class UnitTest1
    {
        string directoryPath = @"C:\UnitTesting";


        FileName NoDots = new FileName("TestName1", false);

        FileName withExtenstion = new FileName("TestName1.txt");

        FileName dotInNameNoExt = new FileName("Test.Name", false);

        FileName dotInNameWExt = new FileName("Test.Name.txt");
        // Tests with _
        FileName underScoreTests = new FileName("Test.Name_0003.txt");

        FileName underScoreTestsNoExt = new FileName("Test.Name_0003", false);

        FileName nameWUnder = new FileName("Test_Name_0002", false);

        FileName nameWUnder2 = new FileName("Test_Two", false);

        FileName underScoreWExt = new FileName("Test_Ext_0002.txt");

        FileName underScoreWExt2 = new FileName("Test_ExtII.txt");


        [TestMethod]
        public void TestBasic()
        {
            Assert.AreEqual(NoDots.FullName, "TestName1");

            Assert.AreEqual(withExtenstion.FullName, "TestName1.txt");

            Assert.AreEqual(dotInNameNoExt.FullName, "Test.Name");

            Assert.AreEqual(dotInNameWExt.FullName, "Test.Name.txt");

            //test a name w/ extension and remove the extension through the class property
            Assert.AreEqual(dotInNameWExt.NameWithoutExtension, "Test.Name");

        }

        [TestMethod]
        public void TestGen()
        {

            // Check if the directory exists
            if (Directory.Exists(directoryPath))
            {
                // Get all files in the directory
                string[] files = Directory.GetFiles(directoryPath);

                // Loop through each file and delete it
                foreach (string file in files)
                {
                    File.Delete(file);
                    Console.WriteLine($"Deleted file: {file}");
                }

                Console.WriteLine("All files deleted successfully.");
            }
            else
            {
                Console.WriteLine("Directory does not exist.");
            }

            NoDots.MakeUnique(directoryPath);

            Assert.AreEqual(NoDots.FullName, "TestName1");


            createFileAndGen(directoryPath, NoDots);


            NoDots.MakeUnique(directoryPath);
            Assert.AreEqual(NoDots.FullName, "TestName1_0001");
            //////////////////////////////////////////////////////////////////
            createFileAndGen(directoryPath, withExtenstion);


            withExtenstion.MakeUnique(directoryPath);
            Assert.AreEqual(withExtenstion.FullName, "TestName1_0001.txt");
            
        }


        [TestMethod]
        public void currentlyFixing1()
        {
            //////////////////////////////////////////////////////////////////
            createFileAndGen(directoryPath, dotInNameNoExt);

            dotInNameNoExt.MakeUnique(directoryPath);
            Assert.AreEqual(dotInNameNoExt.NameWithoutExtension, "Test.Name_0001");

            Assert.AreEqual(dotInNameNoExt.FullName, "Test.Name_0001");
            ////////////////////////////////////////////////////////////////
            ///

        }


        [TestMethod]
        public void currentlyFixing2()
        {

            ////////////////////////////////////////////////////////////////
            createFileAndGen(directoryPath, dotInNameWExt);

            dotInNameWExt.MakeUnique(directoryPath);
            Assert.AreEqual(dotInNameWExt.FullName, "Test.Name_0001.txt");
            ////////////////////////////////////////////////////////////////
        }

        [TestMethod]
        public void underscoreTests()
        {

            ////////////////////////////////////////////////////////////////
            createFileAndGen(directoryPath, underScoreTests);
            Assert.AreEqual(underScoreTests.FullName, "Test.Name_0003.txt");


            underScoreTests.MakeUnique(directoryPath);
            Assert.AreEqual(underScoreTests.FullName, "Test.Name_0004.txt");
            ////////////////////////////////////////////////////////////////
            createFileAndGen(directoryPath, underScoreTestsNoExt);
            Assert.AreEqual(underScoreTestsNoExt.FullName, "Test.Name_0003");


            underScoreTestsNoExt.MakeUnique(directoryPath);
            Assert.AreEqual(underScoreTestsNoExt.FullName, "Test.Name_0004");
            ////////////////////////////////////////////////////////////////
        }

        [TestMethod]
        public void underScoreInName()
        {
            ////////////////////////////////////////////////////////////////// "Test_Name_0002"
            createFileAndGen(directoryPath, nameWUnder);
            Assert.AreEqual(nameWUnder.NameWithoutExtension, "Test_Name_0002");

            nameWUnder.MakeUnique(directoryPath);
            Assert.AreEqual(nameWUnder.NameWithoutExtension, "Test_Name_0003");

            Assert.AreEqual(nameWUnder.FullName, "Test_Name_0003");
            ////////////////////////////////////////////////////////////////
            ///

            createFileAndGen(directoryPath, nameWUnder2);
            Assert.AreEqual(nameWUnder2.NameWithoutExtension, "Test_Two");

            nameWUnder2.MakeUnique(directoryPath);
            Assert.AreEqual(nameWUnder2.NameWithoutExtension, "Test_Two_0001");

            Assert.AreEqual(nameWUnder2.FullName, "Test_Two_0001");

        }
        [TestMethod]
        public void underScoreAndExt()
        {
            //////////////////////////////////////////////////////////////////   "Test_Ext_0002.txt"
            createFileAndGen(directoryPath, underScoreWExt);
            Assert.AreEqual(underScoreWExt.NameWithoutExtension, "Test_Ext_0002");

            underScoreWExt.MakeUnique(directoryPath);
            Assert.AreEqual(underScoreWExt.NameWithoutExtension, "Test_Ext_0003");

            Assert.AreEqual(underScoreWExt.FullName, "Test_Ext_0003.txt");
            ////////////////////////////////////////////////////////////////
            ///
            createFileAndGen(directoryPath, underScoreWExt2);
            Assert.AreEqual(underScoreWExt2.NameWithoutExtension, "Test_ExtII");

            underScoreWExt2.MakeUnique(directoryPath);
            Assert.AreEqual(underScoreWExt2.NameWithoutExtension, "Test_ExtII_0001");

            Assert.AreEqual(underScoreWExt2.FullName, "Test_ExtII_0001.txt");
        }


            public void createFileAndGen(string dir, FileName name)
        {
            string newPath2 = Path.Combine(dir, name.FullName);
            File.WriteAllText(newPath2, "Testing File Contents");
        }
    }

}
