@echo off

rem *** correct this if your VC install path is different!
set VCPATH="C:\Program Files\Microsoft Visual Studio 8\VC"

rem *** copy to different path if using Express
if exist %VCPATH%\Express\VCProjects goto exp_inst

copy MirandaPlugin.vsz %VCPATH%\vcprojects
copy MirandaPlugin.vsdir %VCPATH%\vcprojects
copy MirandaPlugin.ico %VCPATH%\vcprojects
goto copy_files

exp_inst:
copy MirandaPlugin.vsz %VCPATH%\Express\VCProjects
copy MirandaPlugin.vsdir %VCPATH%\Express\VCProjects
copy MirandaPlugin.ico %VCPATH%\Express\VCProjects

:copy_files
xcopy /y /e /i 1033\*.* %VCPATH%\VCWizards\MirandaPlugin\1033
xcopy /y /e /i HTML\*.* %VCPATH%\VCWizards\MirandaPlugin\HTML
xcopy /y /e /i Images\*.* %VCPATH%\VCWizards\MirandaPlugin\Images
xcopy /y /e /i Scripts\*.* %VCPATH%\VCWizards\MirandaPlugin\Scripts
xcopy /y /e /i Templates\*.* %VCPATH%\VCWizards\MirandaPlugin\Templates

copy default.vcproj %VCPATH%\VCWizards\MirandaPlugin