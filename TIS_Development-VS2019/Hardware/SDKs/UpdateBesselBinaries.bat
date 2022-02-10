set BesselBeam-folder="70-0123 BesselBeam_App"

::Copy Files from Source:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
copy ..\..\GUI\Controls\ThemeControl\ThemeControl\bin\Release\ThemeControl.dll .\%BesselBeam-folder%
copy ..\..\Common\ThorLogging\ThorLogging\bin\Release\ThorLogging.dll .\%BesselBeam-folder%
copy ..\..\GUI\Controls\Validations\Validations\bin\Release\Validations.dll .\%BesselBeam-folder%
copy ..\..\Common\ThorSharedTypes\ThorSharedTypes\bin\Release\ThorSharedTypes.dll .\%BesselBeam-folder%
copy ..\..\Tools\Mini-Circuits\"RF switch controller"\mcl_RF_Switch_Controller_NET45.dll .\%BesselBeam-folder%
copy ..\..\GUI\Applications\BesselBeam\BesselBeamGen\bin\Release\BesselBeamGen.exe .\%BesselBeam-folder%