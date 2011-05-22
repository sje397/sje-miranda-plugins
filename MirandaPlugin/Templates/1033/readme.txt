========================================================================
    Miranda Plugin : [!output PROJECT_NAME] Project Overview
========================================================================

Miranda Plugin has created this [!output PROJECT_NAME] project for you as a starting point.

This file contains a summary of what you will find in each of the files that make up your project.

[!output PROJECT_NAME].vcproj
    This is the main project file for projects generated using an Application Wizard. 
    It contains information about the version of the product that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

[!output PROJECT_NAME].cpp
	Main plugin file. Contains the functions necessary for the plugin to load.
	
[!output PROJECT_NAME].rc
	Project resources - this is the the only resource file that should require editing on your part!
[!if OPTIONS_PAGE]
	This file contains for example the options dialog page.
[!endif]
	
version.h
	Version information

version.rc
	Version resource - do not edit!
	
common.h
	Common includes - also acts as the procompiled header file for the project.

common.cpp
	Just includes the common.h header file.
	
resource.h
	Resource defines
	
resource.rc
	Binds together the version and plugin resources	- do not edit!

[!if OPTIONS_PAGE]
options.cpp
options.h
	These files contain the dialog procedure for the options page, and placeholders for options including save and load routines.
[!endif]

/////////////////////////////////////////////////////////////////////////////
Other notes:

/////////////////////////////////////////////////////////////////////////////
