#ifndef _PikaCRM_PikaCRM_h
#define _PikaCRM_PikaCRM_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <PikaCRM/PikaCRM.lay>
#include <CtrlCore/lay.h>

//useful library------------------------------------------------------
#include <SystemLog/SystemLog.h>
#include <SplashSV/splash-sv.h>						//this is   for Splash
#include <plugin/sqlite3/Sqlite3.h>
#include "boost/smart_ptr.hpp"
//end useful library---------------------------------------------------

//platform dependent---------------------------------------------------
#ifdef PLATFORM_POSIX

#include <sys/ioctl.h>//for open
#include <sys/fcntl.h>
#include <linux/hdreg.h>

#elif defined(PLATFORM_WIN32)

#endif
//end platform dependent------------------------------------------------

//#include <string>
//#include <vector>



#define SOFTWARE_NAME					"PikaCRM"
#define SOFTWARE_VERSION				"0.0.1"
#define DATABASE_VERSION				"1"
#define BUILD_DATE						Date(2010, 7, 25)

#define PW_MAGIC_WORD					"sevenjay777"
//we let PW SETKEY ROLL by getSwap1st2ndChar

//define file path and name------------------------------------------------------
#define APP_CONFIG_DIR					".PikaCRM"//need mkdir ".MobileConnect/"

#define FILE_CONFIG						"PikaCRM.xml"//in PATH_USER_HOME
#define FILE_LOG						"PikaCRM.log"//in PATH_USER_HOME
#define FILE_DATABASE					"PikaCRM.sqlite"//in PATH_USER_HOME
//end define file path and name--------------------------------------------------

struct Config {
	bool	IsDBEncrypt;
	String	Password;
	bool	IsRememberPW;
	String	SystemKey;

	void Xmlize(XmlIO xml){	//necessary for StoreAsXMLFile(), LoadFromXMLFile()
		xml
			("Encrypted", IsDBEncrypt)
			("Password", Password)
			("RememberPW", IsRememberPW)
			("SystemKey", SystemKey)
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
	//-------------------------------------------------------------------------------
	//private utility-------------------------------------------------------------------
	String  getConfigDirPath();
	String	getLang4Char();
	Image	getLangLogo();
	String	getMD5(String & text);
	String	getSwap1st2ndChar(String & text);
	

	//application control--------------------------------------------------------------
	void CloseMainFrom();
	//used in initial====================================================
	bool IsHaveDBFile(String database_file_path);
	void CreateOrOpenDB(String database_file_path);
	void InitialDB();
	void SetupDB(String config_file_path);
		void OnOptPWAction(WithInitialDBLayout<TopWindow> * d);
		void CheckPWSame(WithInitialDBLayout<TopWindow> * d);
	bool IsDBWork();
	
	void InputPWCheck();
		void CheckPWRight(WithInputPWLayout<TopWindow> * d, String & pw);
	String GetSystemKey();
	
	void LoadConfig(String config_file_path);
	void SetConfig();
	void SaveConfig(String config_file_path);
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