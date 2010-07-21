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
	CtrlLayout(MainFrom, t_("Pika Customer Relationship Management"));
	
	mLanguage=LNG_('Z','H','T','W');
	//mLanguage=LNG_('E','N','U','S');
	
	
	SetLanguage(mLanguage);
	mSplash.SplashInit("PikaCRM/srcdoc/Splash",20,getLangLogo(),SrcImages::Logo(),mLanguage);
	
	//WithSplashLayout<TopWindow> splash;
	//CtrlLayout(splash, t_("Application Setup"));
	//splash.OpenMain();
	//Splash();
	
	//splash.Close();
	MainFrom.WhenClose=THISBACK(CloseMainFrom);
	MainFrom.lbOne=t_("Setting");
}

PikaCRM::~PikaCRM()
{
}

void PikaCRM::OpenMainFrom()
{
	mSplash.ShowSplash();
	mSplash.ShowSplashStatus(t_("Loading Settings..."));
	SysLog.Info(t_("Loading Settings..."))<<"\n";
	MainFrom.OpenMain();
	
	mSplash.ShowSplashStatus(t_("Normal Running..."));
	SysLog.Info(t_("Normal Running..."))<<"\n";
	
	mSplash.SetSplashTimer(500);
}
void PikaCRM::CloseMainFrom()//MainFrom.WhenClose call back
{
	mSplash.~SplashSV();
	MainFrom.Close();
}

bool PikaCRM::IsHaveDBFile()
{		
	
	
	
}

//private---------------------------------------------------------------
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
	    
//end private---------------------------------------------------------------