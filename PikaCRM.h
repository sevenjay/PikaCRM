#ifndef _PikaCRM_PikaCRM_h
#define _PikaCRM_PikaCRM_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <PikaCRM/PikaCRM.lay>
#include <CtrlCore/lay.h>

//useful library
#include <SystemLog/SystemLog.h>
#include <SplashSV/splash-sv.h>						//this is   for Splash
#include <plugin/sqlite3/Sqlite3.h>
#include "boost/smart_ptr.hpp"


#define SOFTWARE_NAME					"PikaCRM"
#define SOFTWARE_VERSION				"0.0.1"
#define DATABASE_VERSION				"1"
#define BUILD_DATE						Date(2010, 7, 25)

//define file path and name------------------------------------------------------
#define APP_CONFIG_DIR					".PikaCRM"//need mkdir ".MobileConnect/"

#define FILE_CONFIG						"PikaCRM.xml"//in PATH_USER_HOME
#define FILE_LOG						"PikaCRM.log"//in PATH_USER_HOME
#define FILE_DATABASE					"PikaCRM.sqlite"//in PATH_USER_HOME
//end define file path and name--------------------------------------------------

struct Config {
	bool	IsDBEncrypt;
	String	Password;

	void Xmlize(XmlIO xml){	//necessary for StoreAsXMLFile(), LoadFromXMLFile()
		xml
			("Encrypted", IsDBEncrypt)
			("Password", Password)
		;
	}
	
	bool Load(String const &fileName){
		return LoadFromXMLFile(*this, fileName);
	}

	bool Save(String const &fileName){
		return StoreAsXMLFile(*this, "Config", fileName);
	}

};


class PikaCRM
{
	typedef PikaCRM CLASSNAME;
	
private :
	WithPikaCRMLayout<TopWindow> MainFrom;
	
	//must initial in PikaCRM(), OpenMainFrom()	------------------------------------
	int	mLanguage;

	SplashSV mSplash;
	
	Config mConfig;
	
	Sqlite3Session mSqlite3Session;
	boost::shared_ptr<Sql> mSql;
	//Sql * mSql;
	String mPassword;
	//-------------------------------------------------------------------------------
	//private utility-------------------------------------------------------------------
	String  getConfigDirPath();
	String	getLang4Char();
	Image	getLangLogo();
	

	//application control--------------------------------------------------------------
	void CloseMainFrom();
	//used in initial====================================================
	bool IsHaveDBFile();
	void CreateOrOpenDB();
	void InitialDB();
	void SetupDB();
		static void OnOptPWPush(WithInitialDBLayout<TopWindow> * d);
	
	void LoadConfig();
	void SetConfig();
	void SaveConfig();
	//end used in initial================================================
	
public:
	PikaCRM();
	~PikaCRM();
	
	//main control--------------------------------------------------------------
	String GetLogPath();
	void   OpenMainFrom();
	
	//interactive with GUI==============================================================
};

#endif