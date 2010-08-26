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
	String	SystemPWKey;

	void Xmlize(XmlIO xml){	//necessary for StoreAsXMLFile(), LoadFromXMLFile()
		xml
			("Encrypted", IsDBEncrypt)
			("Password", Password)
			("RememberPW", IsRememberPW)
			("SystemPWKey", SystemPWKey)
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
	String	getMD5(const String & text);
	String	getSwap1st2ndChar(const String & text);
	

	//application control--------------------------------------------------------------
	void CloseMainFrom();
	//used in initial====================================================
	bool IsHaveDBFile(const String & database_file_path);
	void CreateOrOpenDB(const String & database_file_path);
	void InitialDB();
	void SetupDB(const String config_file_path);
		void OnOptPWAction(WithInitialDBLayout<TopWindow> * d);
		void CheckPWSame(WithInitialDBLayout<TopWindow> * d);
	bool IsDBWork();
	
	void InputPWCheck();
		void CheckPWRight(WithInputPWLayout<TopWindow> * d, const String & pw);
	String GetSystemKey();
	String CombineKey(const String & key1, const String & key2);
	
	void LoadConfig(const String & config_file_path);
	void SetConfig();
	void SaveConfig(const String & config_file_path);
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