#include "PikaCRM.h"





GUI_APP_MAIN
{	
	PikaCRM pikaCRM;
try
{
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
	
	String msg="[G* " + DeQtfLf(e) + "]";
	
	if(e.GetHandle()&ApExc::REPORT) msg+=t_("[* &And please report this issue to [^http://pika.sevenjay.tw/node/add/forum/2^ Bugs Report].]");
	
	if(e.GetHandle()&ApExc::NOTICE) Exclamation(msg);
	
}
catch(...)
{	//can not do anything
	String what= t_("Sorry, there is an unknown error.&"
					"If it always happens, you can report to [^http://pika.sevenjay.tw/node/add/forum/2^ Bugs Report] with the log and last error:&")
				+ DeQtfLf(GetLastErrorMessage());
	Exclamation(what);
	
	SysLog.Critical("last error: "+GetLastErrorMessage()+"\n");
	SysLog.Critical("last sql error: "+SQL.GetLastError()+"\n");
}
}

