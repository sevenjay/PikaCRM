#include "PikaCRM.h"

#define TFILE <PikaCRM/PikaCRM.t>
#include <Core/t.h>

#define IMAGECLASS SrcImages					// Adding Graphic
#define	IMAGEFILE <PikaCRM/SrcImages.iml>		//
#include <Draw/iml.h>							//

#define TOPICFILE <PikaCRM/srcdoc.tpp/all.i>	// Adding QTF for splash (and for other aims)
#include <Core/topic_group.h>					//


PikaCRM::PikaCRM()
{
	mLanguage=LNG_('Z','H','T','W');
	//mLanguage=LNG_('E','N','U','S');
	Upp::SetLanguage(mLanguage);

	Upp::CtrlLayout(MainFrom, t_("Pika Customer Relationship Management"));
	
	int QtfHigh=20;
	mSplash.SplashInit("PikaCRM/srcdoc/Splash",QtfHigh,getLangLogo(),SrcImages::Logo(),mLanguage);
	
	//WithSplashLayout<TopWindow> splash;
	//CtrlLayout(splash, t_("Application Setup"));
	//splash.OpenMain();
	//Splash();
	
	//splash.Close();
	MainFrom.WhenClose=THISBACK(CloseMainFrom);
	//MainFrom.lbOne=t_("Setting");
}

PikaCRM::~PikaCRM()
{
}


//application control-----------------------------------------------------------
String PikaCRM::GetLogPath()
{
	return getConfigDirPath()+FILE_LOG;	
}
void PikaCRM::OpenMainFrom()
{
	mSplash.ShowSplash();
	mSplash.ShowSplashStatus(t_("Loading Settings..."));
	SysLog.Info(t_("Loading Settings..."))<<"\n";
	LoadConfig();///@todo see if encrypt?
	
	mSplash.ShowSplashStatus(t_("Loading Database..."));
	SysLog.Info(t_("Loading Database..."))<<"\n";
	if(IsHaveDBFile())
	{
		SetupDB();
		CreateOrOpenDB();//OpenDB
	}
	else
	{
		CreateOrOpenDB();//CreateDB
		InitialDB();
	}
	
	MainFrom.OpenMain();
	
	mSplash.ShowSplashStatus(t_("Normal Running..."));
	SysLog.Info(t_("Normal Running..."))<<"\n";
	
	mSplash.SetSplashTimer(1500);
}
void PikaCRM::CloseMainFrom()//MainFrom.WhenClose call back
{
	mSplash.~SplashSV();
	MainFrom.Close();
}

bool PikaCRM::IsHaveDBFile()
{		
	String database_file_path = getConfigDirPath()+FILE_DATABASE;
	return FileExists(database_file_path);
}

void PikaCRM::CreateOrOpenDB()
{
	String database_file_path = getConfigDirPath()+FILE_DATABASE;
	if(!mSqlite3Session.Open(database_file_path))
	{
		SysLog.Error("can't create or open database file\n"+database_file_path+"\n");
		///@todo thow 
	}
	SysLog.Debug("create or open database file: "+database_file_path+"\n");
	
	if(!mPassword.IsEmpty())
	{
		SysLog.Info("setting database encrypted\n");
		if(!mSqlite3Session.SetKey(mPassword))
		{
			SysLog.Error("sqlite3 set key error\n");
			///@note we dont know how to deal this error, undefine		
		}
	}
	
#ifdef _DEBUG
	mSqlite3Session.SetTrace();
#endif
	mSql.reset(new Sql(mSqlite3Session));//mSql=mSqlite3Session will error
}
void PikaCRM::InitialDB()
{
	FileIn initial("initial.sql");
	if(!initial)
	{
		SysLog.Error("can't open database initial file: initial.sql\n");
		///@todo thow 
	}
	bool is_sql_ok = SqlPerformScript(mSqlite3Session,initial);
	
	is_sql_ok=mSql->Execute("INSERT INTO System (user,ap_ver,sqlite_ver,db_ver) VALUES (?,?,?,?);",
							"System",SOFTWARE_VERSION,mSqlite3Session.VersionInfo(),DATABASE_VERSION);
							///@todo set user name
}
void PikaCRM::SetupDB()
{
	WithInitialDBLayout<TopWindow> d,e;
	CtrlLayoutOKCancel(d, t_("Setup your database"));
	d.esPassword.Password();
	d.optPW.WhenAction=callback1(OnOptPWPush, &d);
	/*CtrlRetriever r;
	Size sz = size;
	r
		(d.cx, size.cx)
		(d.cy, size.cy)
		(d.lang, lang)
	;*/
	

	String note,note2;
	note<<"[1 "<<t_("Encrypted database can't be read even if someone has the database file.")<<" ]";
	d.rtNoteEncrypted.SetQTF(note);
	note2<<"[1 "<<t_("Important: if you forgot the password, there is no way to access your database.")<<" ]";
	d.rtNoteEncrypted2.SetQTF(note2);
	if(d.Run() == IDOK) {
		/*r.Retrieve();
		Init();
		if(sz != size)
			Generate();*/
	}
	
	
}
void PikaCRM::OnOptPWPush(WithInitialDBLayout<TopWindow> * d)
{
	if(d->optPW)
	{
		d->esPassword.SetEditable(true);
		d->esPassword.NotNull(true);
		d->esPassword.WantFocus(true);
		d->esCheckPassword.SetEditable(true);
		d->esCheckPassword.NotNull(true);
		d->esCheckPassword.WantFocus(true);
	}
	else 
	{
		d->esPassword.SetEditable(false);
		d->esPassword.NotNull(false);
		d->esPassword.WantFocus(false);
		d->esCheckPassword.SetEditable(false);
		d->esCheckPassword.NotNull(false);
		d->esCheckPassword.WantFocus(false);
	}
}

void PikaCRM::LoadConfig()
{
	String config_file_path = getConfigDirPath()+FILE_CONFIG;
	if(FileExists(config_file_path))
	{
		if(mConfig.Load(config_file_path))
		{
			SysLog.Debug("loaded the config file\n");
		}
		else
		{
			SysLog.Error("load the config file fail\n");
			///@todo thow and renew outside
		}
	}
	else
	{
		SysLog.Info("make a new config file\n");
		mConfig.IsDBEncrypt=false;
		//mConfig.Password="lalala";
		if(mConfig.Save(config_file_path))
		{
			SysLog.Debug("make the config file\n");
		}
		else
		{
			SysLog.Error("make a new config file fail\n");
			///@todo thow can't save
		}
	}
}
void PikaCRM::SetConfig()
{
}
void PikaCRM::SaveConfig()
{
}

//end application control-----------------------------------------------------------
//interactive with GUI==============================================================



//end interactive with GUI==========================================================
//private utility-------------------------------------------------------------------
String PikaCRM::getConfigDirPath()
{
	String directory_path(APP_CONFIG_DIR);
#ifdef PLATFORM_POSIX
	String full_directory_path = GetHomeDirFile(directory_path+"/");
#endif

#ifdef PLATFORM_WIN32
	String full_directory_path = GetHomeDirFile("Application Data/"+directory_path+"/");
	//win_full_directory_path=WinPath(full_directory_path);///@todo test if we need do it 
#endif

	if(DirectoryExists(full_directory_path))
		;//do nothing
	else
	{
		if(RealizeDirectory(full_directory_path))
			;//do nothing
		else
		{
			RLOG("can't create the application config directory!");
			//throw this error///@todo throw "Can't create config directory!"
		}
	}

	return full_directory_path;
}
String PikaCRM::getLang4Char()
{
	String lang4=LNGAsText(mLanguage);//EN-US
	if(!lang4.IsEmpty()) lang4.Remove(2,1); //remove "-"
	return lang4;
}
Image PikaCRM::getLangLogo()
{
	String logo_name;
	logo_name="Logo"+getLang4Char();//LogoENUS, LogoZHTW, ...
	int id=SrcImages::Find(logo_name);
							
	Image image;							
	if(id>=0)
		image=SrcImages::Get(id);//SrcImages::LogoENUS()
	else
		image=SrcImages::Logo();
							
	return image;
}
	    
//end private utility---------------------------------------------------------------