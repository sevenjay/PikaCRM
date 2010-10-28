#include "PikaCRM.h"





GUI_APP_MAIN
{	
	//RLOG("Start Application...");//in ~/.upp/PikaCRM/PikaCRM.log //this is must for log.old bug
try
{
	//SetLanguage( SetLNGCharset( GetSystemLNG(), CHARSET_UTF8 ) );
	
	PikaCRM pikaCRM;
	FirstSetVppLogName(pikaCRM.GetLogPath());
	SysLog.Open(GetExeTitle());	
#ifdef _DEBUG
	SysLog.SetLevels(SystemLog::LALLDEBUG);// enables all log levels
#else
	SysLog.SetLevels(SystemLog::LALLDEBUG);///@todo when 1.0 release change
#endif
	/*
			LEMERGENCY	= 0x01,
			LALERT		= 0x02,
			LCRITICAL	= 0x04, used for system fail
			LERROR		= 0x08, used for operator fail, software error
			LWARNING	= 0x10, used for operator mistake, warning user
			LNOTICE		= 0x20,
			LINFO		= 0x40, used for notice user, software information
			LDEBUG		= 0x80, used for debug only
			LALLERR		= 0x0f, //Used to enable all error-level logs, no LWARNING
			LALL		= 0x7f, //Used to enable all but not debug logs
			LALLDEBUG	= 0xff

	*/
	/*	default
	FCoutEnabled = false;
	FCerrEnabled = true;
	FUppLogEnabled = true;	
	FSysLogEnabled = false;
	FLastLevel = LERROR;
	*/
	SysLog.EnableCout();
	SysLog.EnableCerr();
	SysLog.EnableUppLog();
	SysLog.EnableSysLog(false);

	SysLog(SystemLog::LDEBUG) << "Start logging\n";
	//SysLog(SystemLog::LDEBUG) << _WIN32_WINNT;
	pikaCRM.Initial();
	pikaCRM.OpenMainFrom();
	
	Ctrl::EventLoop();
	SysLog(SystemLog::LDEBUG) << "End Application Main\n";
	
}
catch(ApExc & e)
{
	if(e.GetHandle()&ApExc::EXIT) SysLog.Critical(e+"\n");
	else SysLog.Error(e+"\n");
	
	
	if(e.GetHandle()&ApExc::REPORT) e+="\nAnd please report this issue to xxxweb.";
	
	if(e.GetHandle()&ApExc::NOTICE) Exclamation("[* " + DeQtfLf(e) + "]");
	
}
catch(...)
{	//can not do anything
	String what= t_("Sorry, there is an unknown error.\n"
					"If it always happens, you can report to our web site (Help->Report bugs) with the log and last error:\n")
				+GetLastErrorMessage();
	Exclamation("[* " + DeQtfLf(what) + "]");
}
}

