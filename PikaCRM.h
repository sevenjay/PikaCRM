#ifndef _PikaCRM_PikaCRM_h
#define _PikaCRM_PikaCRM_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <PikaCRM/PikaCRM.lay>
#include <CtrlCore/lay.h>

//useful library
#include <SystemLog/SystemLog.h>
#include <SplashSV/splash-sv.h>						//this is   for Splash


class PikaCRM
{
	typedef PikaCRM CLASSNAME;
	
private :
	WithPikaCRMLayout<TopWindow> MainFrom;
	
	//CallbackArgTarget<int> mCBLanguage;	
	int	mLanguage;

	SplashSV mSplash;
	
	String	getLang4Char();
	Image	getLangLogo();
	
public:
	PikaCRM();
	~PikaCRM();
	bool IsHaveDBFile();
	void OpenMainFrom();
	void CloseMainFrom();
	void InitialDB();
	
};

#endif

