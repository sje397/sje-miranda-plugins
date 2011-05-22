
function OnFinish(selProj, selObj)
{
	try
	{
		var strProjectPath = wizard.FindSymbol('PROJECT_PATH');
		var strProjectName = wizard.FindSymbol('PROJECT_NAME');

        var strSafeProjectName = CreateASCIIName(strProjectName).toUpperCase();
        wizard.AddSymbol("SAFE_PROJECT_NAME_CAPS", strSafeProjectName);

        var strPluginGuid = wizard.CreateGuid();
        wizard.AddSymbol("GUID_PLUGIN", wizard.FormatGuid(strPluginGuid, 2));
        var strInterfacecGuid = wizard.CreateGuid();
        wizard.AddSymbol("GUID_INTERFACE", wizard.FormatGuid(strInterfacecGuid, 2));

		selProj = CreateCustomProject(strProjectName, strProjectPath);
		AddConfig(selProj, strProjectName);
		AddFilters(selProj);

		var InfFile = CreateCustomInfFile();
		AddFilesToCustomProj(selProj, strProjectName, strProjectPath, InfFile);
		PchSettings(selProj);
		InfFile.Delete();

		ExcludeResources(selProj, strProjectName);

		selProj.Object.Save();
	}
	catch(e)
	{
		if (e.description.length != 0)
			SetErrorInfo(e);
		return e.number
	}
}

function CreateCustomProject(strProjectName, strProjectPath)
{
	try
	{
		var strProjTemplatePath = wizard.FindSymbol('START_PATH'); //wizard.FindSymbol('PROJECT_TEMPLATE_PATH');
		var strProjTemplate = '';
		strProjTemplate = strProjTemplatePath + '\\default.vcproj';

		var Solution = dte.Solution;
		var strSolutionName = "";
		if (wizard.FindSymbol("CLOSE_SOLUTION"))
		{
			Solution.Close();
			strSolutionName = wizard.FindSymbol("VS_SOLUTION_NAME");
			if (strSolutionName.length)
			{
				var strSolutionPath = strProjectPath.substr(0, strProjectPath.length - strProjectName.length);
				Solution.Create(strSolutionPath, strSolutionName);
			}
		}

		var strProjectNameWithExt = '';
		strProjectNameWithExt = strProjectName + '.vcproj';

		var oTarget = wizard.FindSymbol("TARGET");
		var prj;
		if (wizard.FindSymbol("WIZARD_TYPE") == vsWizardAddSubProject)  // vsWizardAddSubProject
		{
			var prjItem = oTarget.AddFromTemplate(strProjTemplate, strProjectNameWithExt);
			prj = prjItem.SubProject;
		}
		else
		{
			prj = oTarget.AddFromTemplate(strProjTemplate, strProjectPath, strProjectNameWithExt);
		}
		return prj;
	}
	catch(e)
	{
		throw e;
	}
}

function AddFilters(proj)
{
    try {
        SetupFilters(proj);
    }
	catch(e)
	{
		throw e;
	}
}

function AddConfig(proj, strProjectName)
{
	try
	{	
		var config = proj.Object.Configurations('Debug');
		config.IntermediateDirectory = '$(ConfigurationName)';
		config.OutputDirectory = '$(ConfigurationName)';
		config.ConfigurationType = typeDynamicLibrary;

		var CLTool = config.Tools('VCCLCompilerTool');
        CLTool.AdditionalIncludeDirectories = wizard.FindSymbol("API_INCLUDE_DIR");
        CLTool.PrecompiledHeaderThrough = "common.h";
        CLTool.DisableSpecificWarnings = "4996";
        CLTool.PreprocessorDefinitions = "_DEBUG;_WINDOWS;_USERDLL";
   		CLTool.RuntimeLibrary = rtMultiThreadedDebug;
		CLTool.Optimization = optimizeDisabled;

		var LinkTool = config.Tools('VCLinkerTool');
		LinkTool.SubSystem = subSystemWindows;

		var config = proj.Object.Configurations('Release');
		config.IntermediateDirectory = '$(ConfigurationName)';
		config.OutputDirectory = '$(ConfigurationName)';
		config.ConfigurationType = typeDynamicLibrary;

		var CLTool = config.Tools('VCCLCompilerTool');
        CLTool.AdditionalIncludeDirectories = wizard.FindSymbol("API_INCLUDE_DIR");
        CLTool.PrecompiledHeaderThrough = "common.h";
        CLTool.DisableSpecificWarnings = "4996";
        CLTool.PreprocessorDefinitions = "NDEBUG;_WINDOWS;_USERDLL";
   		CLTool.RuntimeLibrary = rtMultiThreaded;

		var LinkTool = config.Tools('VCLinkerTool');
		LinkTool.SubSystem = subSystemWindows;
		
		var config = proj.Object.Configurations('Debug (Unicode)');
		config.IntermediateDirectory = '$(ConfigurationName)';
		config.OutputDirectory = '$(ConfigurationName)';
		config.ConfigurationType = typeDynamicLibrary;
		config.CharacterSet = charSetUNICODE;

		var CLTool = config.Tools('VCCLCompilerTool');
        CLTool.AdditionalIncludeDirectories = wizard.FindSymbol("API_INCLUDE_DIR");
        CLTool.PrecompiledHeaderThrough = "common.h";
        CLTool.DisableSpecificWarnings = "4996";
        CLTool.PreprocessorDefinitions = "_DEBUG;_WINDOWS;_USERDLL;UNICODE;_UNICODE";
   		CLTool.RuntimeLibrary = rtMultiThreadedDebug;
		CLTool.Optimization = optimizeDisabled;

		var LinkTool = config.Tools('VCLinkerTool');
		LinkTool.SubSystem = subSystemWindows;

		var config = proj.Object.Configurations('Release (Unicode)');
		config.IntermediateDirectory = '$(ConfigurationName)';
		config.OutputDirectory = '$(ConfigurationName)';
		config.ConfigurationType = typeDynamicLibrary;
		config.CharacterSet = charSetUNICODE;

		var CLTool = config.Tools('VCCLCompilerTool');
        CLTool.AdditionalIncludeDirectories = wizard.FindSymbol("API_INCLUDE_DIR");
        CLTool.PrecompiledHeaderThrough = "common.h";
        CLTool.DisableSpecificWarnings = "4996";
        CLTool.PreprocessorDefinitions = "NDEBUG;_WINDOWS;_USERDLL;UNICODE;_UNICODE";
   		CLTool.RuntimeLibrary = rtMultiThreaded;

		var LinkTool = config.Tools('VCLinkerTool');
		LinkTool.SubSystem = subSystemWindows;
		
	}
	catch(e)
	{
		throw e;
	}
}

function ExcludeResources(proj, strProjectName) {
	try
	{
	    for (var fileIdx = 1; fileIdx <= proj.Object.Files.Count; fileIdx++)
	    {
		    var file = proj.Object.Files.Item(fileIdx);
		    if (file.Extension.toLowerCase() == ".rc")
		    {
				if(file.Name == strProjectName + ".rc" || file.Name == "version.rc") {
				    for(var cfgIdx = 1; cfgIdx <= file.FileConfigurations.Count; cfgIdx++)
					    file.FileConfigurations.Item(cfgIdx).ExcludedFromBuild = true;
				}
			}
		}
    }
    catch(e) 
    {
        throw e;
    }
}

function PchSettings(proj)
{
	try
	{
		var files = proj.Object.Files;
		file = files("common.cpp");
		if(!file)
			return;

		var configs = proj.Object.Configurations;

		for (var i = 1; i <= configs.Count; i++)
		{
			// setup /Yu (using precompiled headers)
			config = configs.Item(i);
			var CLTool = config.Tools("VCCLCompilerTool");
			CLTool.UsePrecompiledHeader = pchUseUsingSpecific;
			// setup /Yc (create precompiled header)
			fileConfig = file.FileConfigurations(config.Name);
			fileConfig.Tool.UsePrecompiledHeader = pchCreateUsingSpecific;
		}
	}
	catch(e)
	{
		throw e;
	}
}

function DelFile(fso, strWizTempFile)
{
	try
	{
		if (fso.FileExists(strWizTempFile))
		{
			var tmpFile = fso.GetFile(strWizTempFile);
			tmpFile.Delete();
		}
	}
	catch(e)
	{
		throw e;
	}
}

function CreateCustomInfFile()
{
	try
	{
		var fso, TemplatesFolder, TemplateFiles, strTemplate;
		fso = new ActiveXObject('Scripting.FileSystemObject');

		var TemporaryFolder = 2;
		var tfolder = fso.GetSpecialFolder(TemporaryFolder);
		var strTempFolder = tfolder.Drive + '\\' + tfolder.Name;

		var strWizTempFile = strTempFolder + "\\" + fso.GetTempName();

		var strTemplatePath = wizard.FindSymbol('TEMPLATES_PATH');
		var strInfFile = strTemplatePath + '\\Templates.inf';
		wizard.RenderTemplate(strInfFile, strWizTempFile);

		var WizTempFile = fso.GetFile(strWizTempFile);
		return WizTempFile;
	}
	catch(e)
	{
		throw e;
	}
}

function GetTargetName(strName, strProjectName, strResPath, strHelpPath)
{
	try
	{
		var strTarget = strName;

		if (strName == "readme.txt")
			strTarget = "readme.txt";
		if (strName == "resource.h")
			strTarget = "resource.h";

		if (strName.substr(0, 4) == "base")
		{
			strTarget = strProjectName + strName.substr(4);

		}
		return strTarget; 
	}
	catch(e)
	{
		throw e;
	}
}

function AddFilesToCustomProj(proj, strProjectName, strProjectPath, InfFile)
{
	try
	{
		var projItems = proj.ProjectItems

		var strTemplatePath = wizard.FindSymbol('TEMPLATES_PATH');

		var strTpl = '';
		var strName = '';

		var strTextStream = InfFile.OpenAsTextStream(1, -2);
		while (!strTextStream.AtEndOfStream)
		{
			strTpl = strTextStream.ReadLine();
			if (strTpl != '')
			{
				strName = strTpl;
				var strTarget = GetTargetName(strName, strProjectName);
				var strTemplate = strTemplatePath + '\\' + strTpl;
				var strFile = strProjectPath + '\\' + strTarget;

				var bCopyOnly = false;  //"true" will only copy the file from strTemplate to strTarget without rendering/adding to the project
				var strExt = strName.substr(strName.lastIndexOf("."));
				if(strExt==".bmp" || strExt==".ico" || strExt==".gif" || strExt==".rtf" || strExt==".css")
					bCopyOnly = true;
				wizard.RenderTemplate(strTemplate, strFile, bCopyOnly);
				proj.Object.AddFile(strFile);
			}
		}
		strTextStream.Close();
	}
	catch(e)
	{
		throw e;
	}
}
