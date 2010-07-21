#ifndef _PikaCRM_PikaCRM_h
#define _PikaCRM_PikaCRM_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <PikaCRM/PikaCRM.lay>
#include <CtrlCore/lay.h>

//useful library
#include <SystemLog/SystemLog.h>
#include <SplashSV/splash-sv.h>						//this is   for Splash

#define SOFTWARE_NAME					"PikaCRM"
#define SOFTWARE_VERSION				"0.0.1"
#define BUILD_DATE						Date(2009, 7, 20)

//define file path and name------------------------------------------------------
#define APP_CONFIG_DIR					".PikaCRM"//need mkdir ".MobileConnect/"

#define FILE_CONFIG						"PikaCRM.ini"//in PATH_USER_HOME
#define FILE_LOG						"PikaCRM.log"//in PATH_USER_HOME
//end define file path and name--------------------------------------------------



class PikaCRM
{
	typedef PikaCRM CLASSNAME;
	
private :
	WithPikaCRMLayout<TopWindow> MainFrom;
	
	//CallbackArgTarget<int> mCBLanguage;	
	int	mLanguage;

	SplashSV mSplash;
	
	//private utility-------------------------------------------------------------------
	String  getConfigDirPath();
	String	getLang4Char();
	Image	getLangLogo();
	

	
public:
	PikaCRM();
	~PikaCRM();
	
	//application control--------------------------------------------------------------
	String GetLogPath();
	void OpenMainFrom();
	void CloseMainFrom();
	bool IsHaveDBFile();
	void InitialDB();
	
	void LoadConfig();
	void SetConfig();
	void SaveConfig();
	
	//interactive with GUI==============================================================
};

#endif