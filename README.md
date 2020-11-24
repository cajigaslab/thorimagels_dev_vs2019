# Introduction 
This project contains most of the source code of ThorImageLS. See Thorlabs Open Source License.pdf for licensing info.

# Getting Started
1.	Installation process
    - Download an clone the repository in your local machine.
2.	Software dependencies
    - Visual Studio 2015 and up recommended, with compiler version 10.0.19041.0
    - Required .Net 4.8 developer pack
    - Required C++ redistributables for 2019
    - For National intruments cards, we recommend NI Max version 18.5
    - For PMT2100, VISA drivers are required
    - 
3.	Latest releases
    - Current branch: ThorImageLS 4.3
4.	API references
    - See Thorlabs Open Source License.pdf 

# Build and Test
Building ThorImageLS
1.	From VS2012 x64 Native Tools command prompt navigate to .\Tools\BuildAllSolutions2012
2.	Run BuildAll64 and/or BuildAll64-Release batch file(s).
3.	Once complete you can check if any projects failed to build by viewing the build.txt or build-release.txt file(s).  
    - If any projects failed try running the BuildAll64.bat again.
    - Note: At the current status there are some C# projects that fail randomly, we recommend running BuilAll64.bat 2 or 3 times until there are no  failed projects. 
4.	Navigate to .\GUI\Applications\ThorImageLS
5.	Open the ThorImage.sln in VS2012
6.	Open the Solution Properties window set the configuration to desired Configuration
7.	If you want to build ThorImageLS in Release follow the next steps, otherwise skip 10 to step.
8.	Open the ThorImage project properties window. In the Build Events window you will see various Pre-build event command Line entries.
	if $(SolutionName) == ThorImage (
    cd ..\..\
    if $(ConfigurationName) == Debug (
    CopyDependenciesx64.bat
    )
    if $(ConfigurationName) == Release (
    rem CopyDependenciesx64-Release.bat
    )
    )
9.	Comment out (remove the “rem”) from the line 'rem CopyDependenciesx64-Release.bat'.
10. If this is the first time you run it, skip to step 14.  
12. Open CopyDependenciesx64.bat or CopyDependenciesx64-Release.bat depending on the configuration that you choose.
13. Uncomment the copy commands of the devices you want to include in the run. We recommend not including any the first time you run it.
14. Buld the ThorImage project.
15.	Navigate to .\GUI\Applications\ThorImageLS\ThorImage\bin\debug (or Release)
16.	To start the application select ThorImageLS.exe