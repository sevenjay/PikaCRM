#include "PikaCRM.h"

#define TFILE <PikaCRM/PikaCRM.t>
#include <Core/t.h>

#define IMAGECLASS SrcImages					// Adding Graphic
#define	IMAGEFILE <PikaCRM/SrcImages.iml>		//
#include <Draw/iml.h>							//

#define TOPICFILE <PikaCRM/srcdoc.tpp/all.i>	// Adding QTF for splash (and for other aims)
#include <Core/topic_group.h>					//

#include <PikaCRM/sql/sql.ids>	//for convenient use tables/columns name

//#include <string>
//#include <vector>
//#include "boost/smart_ptr.hpp"
#include "boost/tokenizer.hpp"
#include "DBCharset/DBCharset.h"

struct ConvContactNames : Convert
{
	Value Format(const Value &q) const
	{
		const VectorMap<int, String> & contact_map= ValueTo< VectorMap<int, String> >(q);
		String all_name;
		for(int i = 0; i < contact_map.GetCount(); i++)//add already select contact to costomer
		{
			int contact_id=contact_map.GetKey(i);
			String one_name(contact_map.Get(contact_id));
			all_name+=one_name+"\n";
		}
		
		if(all_name.GetLength()>0)
		{
			all_name.Remove(all_name.GetLength()-1,1);//remove last "\n"
		}
		
		return all_name;
	}
};
class DisplayColorNotNull : public Display
{
public:
	virtual void PaintBackground(Draw& w, const Rect& r, const Value& q,
	                             Color ink, Color paper, dword style) const
	{
    	if( IsNull(q) )	paper = Color(255, 223, 223);
    	Display::PaintBackground(w, r, q, ink, paper, style);
	};
};
class GDisplayNullRedBack : public GridDisplay
{
public:
	void Paint(Draw &w, int x, int y, int cx, int cy, const Value &val, dword style,
						Color &fg, Color &bg, Font &fnt, bool found, int fs, int fe)
	{
    	//Color new_bg = bg;
    	if( IsNull(val) ) bg = Color(255, 223, 223);
    	GridDisplay::Paint(w, x, y, cx, cy, val, style, fg, bg, fnt, found, fs, fe);
	};
};
class GDisplayNewUnsaved : public GridDisplay
{
public:
	void Paint(Draw &w, int x, int y, int cx, int cy, const Value &val, dword style,
						Color &fg, Color &bg, Font &fnt, bool found, int fs, int fe)
	{
    	Value show=val;
    	if( -1==val ) show=t_("Unsaved");
    	GridDisplay::Paint(w, x, y, cx, cy, show, style, fg, bg, fnt, found, fs, fe);
	};
};

PikaCRM::PikaCRM()
{
	SetAllFieldMap();
}

PikaCRM::~PikaCRM()
{
}
void PikaCRM::Initial()
{
	String config_file_path = getConfigDirPath()+FILE_CONFIG;
	String database_file_path = getConfigDirPath()+FILE_DATABASE;
	
	SysLog.Info("Loading Settings...\n");
	LoadConfig(config_file_path);
		
	SetLanguage( SetLNGCharset( mConfig.Language, CHARSET_UTF8 ) );
	
	int QtfHigh=20;
	mSplash.SplashInit("PikaCRM/srcdoc/Splash",QtfHigh,getLangLogo(mConfig.Language),SrcImages::Logo(),mConfig.Language);	
	mSplash.ShowSplash();

	SetupUI();

	if(mConfig.IsDBEncrypt)
	{
		mSplash.HideSplash();
		if(mConfig.IsRememberPW)
		{
			SysLog.Info("config: Remeber the PW\n");
			String syskey=GetSystemKey();
			if( syskey.IsEmpty() )
			{
				String msg = t_("The function of \"Remeber the password\" is not work.");
			#ifdef PLATFORM_POSIX
				PromptOK( msg + "&" +
					t_("To make it active, please excute \"hdsn`_permit.sh\" for PikaCRM to get reading hard disk serial number permission."));
			#elif defined(PLATFORM_WIN32)
				PromptOK( msg + "&" +
					t_("To make it active, please give PikaCRM the permission to reading hard disk serial number."));			
			#endif
			}
			String key=CombineKey(syskey, mConfig.Password);
			SysLog.Debug("systemPWKey:"+key+"\n");
			if(mConfig.SystemPWKey.IsEmpty() || key!=mConfig.SystemPWKey)//use different PC
			{
				SysLog.Info("config: application is running on different PC\n");
				if(!IsInputPWCheck()) throw ApExc("user cancel Input PW").SetHandle(ApExc::NONE);
			}
			//else
			//	;//just using mConfig.Password;
		}
		else//not Remember PW 
		{
			SysLog.Info("config: Not Remeber the PW\n");
			if(!IsInputPWCheck()) throw ApExc("user cancel Input PW").SetHandle(ApExc::NONE);
		}
		mSplash.ShowSplash();
	}

	
	mSplash.ShowSplashStatus(t_("Checking Database..."));
	SysLog.Info(t_("Checking Database..."))<<"\n";
	if(IsHaveDBFile(database_file_path))
	{
		mSplash.ShowSplashStatus(t_("Loading Database..."));
		SysLog.Info(t_("Loading Database..."))<<"\n";
		CreateOrOpenDB(database_file_path);//OpenDB

	}
	else
	{
		SysLog.Info("setup the database file\n");
		mSplash.HideSplash();
		FirstWelcome();
		if(!IsSetupDB(config_file_path)) throw ApExc("user cancel").SetHandle(ApExc::NONE);
		mSplash.ShowSplash();
		mSplash.ShowSplashStatus(t_("Creating the database..."));
		SysLog.Info(t_("Creating the database..."))<<"\n";;
		CreateOrOpenDB(database_file_path);//CreateDB
	}
	
	//test if database OK-----------------------------------------------------
	bool is_sql_ok=SQL.Execute("PRAGMA database_list;");
	
	if(is_sql_ok)
		;//donothing
	else //pw error or not the file?
	{
		String msg = t_("Failed to load database! Maybe file is encrypted.\n"
						"Last error: ")	+ SQL.GetLastError();
		throw ApExc(msg).SetHandle(ApExc::SYS_FAIL);
		///@remark setkey(password), wrong password may cause this
		///if make multi database, must do reset pw and forget pw  
	}
	//end test if database OK-----------------------------------------------------
	
	if(0==GetDBVersion()) InitialDB();
	/*
	int past_db_ver=GetDBVersion();
	if(past_db_ver<DATABASE_VERSION)
	{
		for(int i=past_db_ver;i<DATABASE_VERSION;++i)
		{
				UpdateToDB(i+1);
		}	
	}
	else if(past_db_ver>DATABASE_VERSION)
	{
		show can not up compatibility, please use the Latest version
	}	
	*/		
}
void PikaCRM::SetupUI()
{
	CtrlLayout(MainFrom);		
	MainFrom.WhenClose=THISBACK(CloseMainFrom);
	MainFrom.Sizeable().Zoomable();
	MainFrom.Title(t_("Pika Customer Relationship Management"));
	MainFrom.Icon(SrcImages::Icon16());
	MainFrom.LargeIcon(SrcImages::Icon32());
	
	
	//TabCtrl----------------------------------------------------------------------------
	//MainFrom.tabMain.WhenSet=THISBACK1(TabChange,MainFrom.tabMain.Get());
	CtrlLayout(Customer);
	MainFrom.tabMain.Add(Customer.SizePos(), t_("Customers"));
	CtrlLayout(Contact);
	MainFrom.tabMain.Add(Contact.SizePos(), t_("Contacts"));
	CtrlLayout(Merchandise);
	MainFrom.tabMain.Add(Merchandise.SizePos(), t_("Merchandises"));
	CtrlLayout(Order);
	MainFrom.tabMain.Add(Order.SizePos(), t_("Orders"));
	CtrlLayout(Event);
	MainFrom.tabMain.Add(Event.SizePos(), t_("Events"));
	CtrlLayout(Preference);
	MainFrom.tabMain.Add(Preference.SizePos(), t_("Preferences"));	
	CtrlLayout(Help);
	MainFrom.tabMain.Add(Help.SizePos(), t_("Help"));	
	//end TabCtrl------------------------------------------------------------------------
	//set icon---------------------------------------------------------------------------
	int imageh=SrcImages::CustomerAdd().GetHeight();
	
	int fonth= GetStdFont().GetHeight();
	
	int btnRectH=Customer.btnCreate.GetRect().Height()-Ctrl::VertLayoutZoom(6);//6 for 2 x sapce(3), define in Button::Paint
	int scale=imageh;
	if(imageh+fonth>btnRectH) //scale image
		scale=btnRectH-fonth;
		//fonth should be txtsz.cy
		//Size txtsz = *text ? GetSmartTextSize(text, font, txtcx) : paintrect.GetStdSize();
		//in LabelBase.cpp, but we can't, so whatever let it go.
	
	Customer.btnCreate.SetImage(fitScale(SrcImages::CustomerAdd(),scale)).SetFont(StdFontS(-1));
	Customer.btnModify.SetImage(fitScale(SrcImages::CustomerEdit(),scale)).SetFont(StdFontS(-1));
	Customer.btnDelete.SetImage(fitScale(SrcImages::CustomerRemove(),scale)).SetFont(StdFontS(-1));
	Customer.btnCreateF.SetImage(fitScale(SrcImages::CustomAdd(),scale)).SetFont(StdFontS(-1));
	Customer.btnModifyF.SetImage(fitScale(SrcImages::CustomEdit(),scale)).SetFont(StdFontS(-1));
	Customer.btnDeleteF.SetImage(fitScale(SrcImages::CustomRemove(),scale)).SetFont(StdFontS(-1));
	Customer.btnImport.SetImage(fitScale(SrcImages::Import(),scale)).SetFont(StdFontS(-1));
	Customer.btnExport.SetImage(fitScale(SrcImages::Export(),scale)).SetFont(StdFontS(-1));
	Customer.btnPrint.SetImage(fitScale(SrcImages::Print(),scale)).SetFont(StdFontS(-1));
	
	Contact.btnCreate.SetImage(fitScale(SrcImages::ContactAdd(),scale)).SetFont(StdFontS(-1));
	Contact.btnModify.SetImage(fitScale(SrcImages::ContactEdit(),scale)).SetFont(StdFontS(-1));
	Contact.btnDelete.SetImage(fitScale(SrcImages::ContactRemove(),scale)).SetFont(StdFontS(-1));
	Contact.btnCreateF.SetImage(fitScale(SrcImages::CustomAdd(),scale)).SetFont(StdFontS(-1));
	Contact.btnModifyF.SetImage(fitScale(SrcImages::CustomEdit(),scale)).SetFont(StdFontS(-1));
	Contact.btnDeleteF.SetImage(fitScale(SrcImages::CustomRemove(),scale)).SetFont(StdFontS(-1));
	Contact.btnImport.SetImage(fitScale(SrcImages::Import(),scale)).SetFont(StdFontS(-1));
	Contact.btnExport.SetImage(fitScale(SrcImages::Export(),scale)).SetFont(StdFontS(-1));
	Contact.btnPrint.SetImage(fitScale(SrcImages::Print(),scale)).SetFont(StdFontS(-1));
				
	Event.btnCreate.SetImage(fitScale(SrcImages::EventAdd(),scale)).SetFont(StdFontS(-1));
	Event.btnModify.SetImage(fitScale(SrcImages::EventEdit(),scale)).SetFont(StdFontS(-1));
	Event.btnDelete.SetImage(fitScale(SrcImages::EventRemove(),scale)).SetFont(StdFontS(-1));
	Event.btnExport.SetImage(fitScale(SrcImages::Export(),scale)).SetFont(StdFontS(-1));
	Event.btnPrint.SetImage(fitScale(SrcImages::Print(),scale)).SetFont(StdFontS(-1));
				
	Order.btnCreate.SetImage(fitScale(SrcImages::OrderAdd(),scale)).SetFont(StdFontS(-1));
	Order.btnModify.SetImage(fitScale(SrcImages::OrderEdit(),scale)).SetFont(StdFontS(-1));
	Order.btnDelete.SetImage(fitScale(SrcImages::OrderRemove(),scale)).SetFont(StdFontS(-1));
	Order.btnExport.SetImage(fitScale(SrcImages::Export(),scale)).SetFont(StdFontS(-1));
	Order.btnPrint.SetImage(fitScale(SrcImages::Print(),scale)).SetFont(StdFontS(-1));
		
	Merchandise.btnCreate.SetImage(fitScale(SrcImages::MerchandiseAdd(),scale)).SetFont(StdFontS(-1));
	Merchandise.btnModify.SetImage(fitScale(SrcImages::MerchandiseEdit(),scale)).SetFont(StdFontS(-1));
	Merchandise.btnDelete.SetImage(fitScale(SrcImages::MerchandiseRemove(),scale)).SetFont(StdFontS(-1));
	Merchandise.btnCreateF.SetImage(fitScale(SrcImages::CustomAdd(),scale)).SetFont(StdFontS(-1));
	Merchandise.btnModifyF.SetImage(fitScale(SrcImages::CustomEdit(),scale)).SetFont(StdFontS(-1));
	Merchandise.btnDeleteF.SetImage(fitScale(SrcImages::CustomRemove(),scale)).SetFont(StdFontS(-1));
	Merchandise.btnImport.SetImage(fitScale(SrcImages::Import(),scale)).SetFont(StdFontS(-1));
	Merchandise.btnExport.SetImage(fitScale(SrcImages::Export(),scale)).SetFont(StdFontS(-1));
	Merchandise.btnPrint.SetImage(fitScale(SrcImages::Print(),scale)).SetFont(StdFontS(-1));
	
	//Customer Tab-----------------------------------------------------------------------
	Customer.btnCreate <<= callback(&(Customer.Grid),&GridCtrl::DoAppend);
	Customer.btnModify <<= callback(&(Customer.Grid),&GridCtrl::DoEdit);
	Customer.btnDelete <<= callback(&(Customer.Grid),&GridCtrl::DoRemove);
	Customer.btnCreateF <<= THISBACK2(CreateField, &(Customer.Grid), "c");
	Customer.btnModifyF <<= THISBACK2(ModifyField, &(Customer.Grid), "c");
	Customer.btnDeleteF.Disable();
	//Customer.btnDeleteF <<= callback(&(Customer.Grid),&GridCtrl::DoRemove);
	Customer.btnImport <<= THISBACK2(ImportFile, &(Customer.Grid), "Customers");
	Customer.btnExport <<= THISBACK2(ExportFile, &(Customer.Grid), "Customers");
	Customer.btnPrint <<= THISBACK2(Print, &(Customer.Grid), "Customers");
	
	Customer.Grid.Absolute();
	Customer.Grid.AddIndex(C_ID).Default(-1);//for when create row before insert row
	Customer.Grid.AddColumn(C_TITLE,t_("Title")).Edit(cesn).Width(mConfig.CWidth.Get(~C_TITLE));
	Customer.Grid.AddColumn(C_PHONE,t_("Phone")).Edit(ces1).Width(mConfig.CWidth.Get(~C_PHONE));
	Customer.Grid.AddColumn(C_ADDRESS,t_("Address")).Edit(ces2).Width(mConfig.CWidth.Get(~C_ADDRESS));
	Customer.Grid.AddColumn(C_EMAIL,t_("Email")).Edit(ces3).Width(mConfig.CWidth.Get(~C_EMAIL));
	Customer.Grid.AddColumn(C_WEBSITE,t_("Web site")).Edit(ces4).Width(mConfig.CWidth.Get(~C_WEBSITE));
	///@important when SetConvert(), it will Convert when you add, so must add the type like RawToValue(temp) in LoadCustomer()
	Customer.Grid.AddColumn(CO_NAME,t_("Contact")).Edit(mCustomerGridContactBtn).Width(mConfig.CWidth.Get(~CO_NAME));//.SetConvert(Single<ConvContactNames>());
		mCustomerGridContactBtn.AddButton().SetLabel("...").WhenPush=THISBACK(CustomerGridContactBtnClick);
	Customer.Grid.AddIndex(CONTACTS_MAP);
	Customer.Grid.AddColumn(C_0).Hidden().Width(mConfig.CWidth.Get(~C_0));
	Customer.Grid.AddColumn(C_1).Hidden().Width(mConfig.CWidth.Get(~C_1));
	Customer.Grid.AddColumn(C_2).Hidden().Width(mConfig.CWidth.Get(~C_2));
	Customer.Grid.AddColumn(C_3).Hidden().Width(mConfig.CWidth.Get(~C_3));
	Customer.Grid.AddColumn(C_4).Hidden().Width(mConfig.CWidth.Get(~C_4));
	Customer.Grid.AddColumn(C_5).Hidden().Width(mConfig.CWidth.Get(~C_5));
	Customer.Grid.AddColumn(C_6).Hidden().Width(mConfig.CWidth.Get(~C_6));
	Customer.Grid.AddColumn(C_7).Hidden().Width(mConfig.CWidth.Get(~C_7));
	Customer.Grid.AddColumn(C_8).Hidden().Width(mConfig.CWidth.Get(~C_8));
	Customer.Grid.AddColumn(C_9).Hidden().Width(mConfig.CWidth.Get(~C_9));
	Customer.Grid.Appending().Removing().AskRemove().Editing().Canceling().ColorRows();//.Searching();
	//Customer.Grid.RejectNullRow();.Duplicating().Accepting().Clipboard()//.Absolute() for horizontal scroll
	//Customer.Grid.GetDisplay().SetTheme(2);
	//Customer.Grid.WhenCreateRow = THISBACK(test);
	Customer.Grid.WhenInsertRow = THISBACK(InsertCustomer);
	Customer.Grid.WhenNewRow = THISBACK(NewCustomer);
	Customer.Grid.WhenDuplicateRow=THISBACK(DuplicateCustomer);
	Customer.Grid.WhenUpdateRow = THISBACK(UpdateCustomer);
	Customer.Grid.WhenRemoveRow = THISBACK(RemoveCustomer);
	//Customer.Grid.SetToolBar();
	//Customer Search------------------------------------------
	Customer.Add(customer_search_bar.LeftPosZ(286, 84).TopPosZ(4, 20));
		Customer.Grid.FindBar(customer_search_bar, Ctrl::HorzLayoutZoom(80));
	Customer.btnSearchClear <<= THISBACK(BtnSearchClearClick);
	Customer.btnSearchGo <<= THISBACK(BtnSearchGoClick);
	
	//Contact Tab-----------------------------------------------------------------------
	Contact.btnCreate <<= callback(&(Contact.Grid),&GridCtrl::DoAppend);
	Contact.btnModify <<= callback(&(Contact.Grid),&GridCtrl::DoEdit);
	Contact.btnDelete <<= callback(&(Contact.Grid),&GridCtrl::DoRemove);
	Contact.btnCreateF <<= THISBACK2(CreateField, &(Contact.Grid), "co");
	Contact.btnModifyF <<= THISBACK2(ModifyField, &(Contact.Grid), "co");
	Contact.btnDeleteF.Disable();
	Contact.btnImport <<= THISBACK2(ImportFile, &(Contact.Grid), "Contacts");
	Contact.btnExport <<= THISBACK2(ExportFile, &(Contact.Grid), "Contacts");
	Contact.btnPrint <<= THISBACK2(Print, &(Contact.Grid), "Contacts");
	
	Contact.Grid.Absolute();
	Contact.Grid.AddIndex(CO_ID);
	Contact.Grid.AddColumn(CO_NAME,t_("Name_")).Edit(coesn).Width(mConfig.COWidth.Get(~CO_NAME));
	Contact.Grid.AddIndex(C_ID).Default(-1);
	Contact.Grid.AddColumn(C_TITLE,t_("Customer")).Width(mConfig.COWidth.Get(~C_TITLE));
	Contact.Grid.AddColumn(CO_PHONE,t_("Phone")).Edit(coes1).Width(mConfig.COWidth.Get(~CO_PHONE));
	Contact.Grid.AddColumn(CO_ADDRESS,t_("Address")).Edit(coes2).Width(mConfig.COWidth.Get(~CO_ADDRESS));
	Contact.Grid.AddColumn(CO_EMAIL,t_("Email")).Edit(coes3).Width(mConfig.COWidth.Get(~CO_EMAIL));
	Contact.Grid.AddColumn(CO_0).Hidden().Width(mConfig.COWidth.Get(~CO_0));
	Contact.Grid.AddColumn(CO_1).Hidden().Width(mConfig.COWidth.Get(~CO_1));
	Contact.Grid.AddColumn(CO_2).Hidden().Width(mConfig.COWidth.Get(~CO_2));
	Contact.Grid.AddColumn(CO_3).Hidden().Width(mConfig.COWidth.Get(~CO_3));
	Contact.Grid.AddColumn(CO_4).Hidden().Width(mConfig.COWidth.Get(~CO_4));
	Contact.Grid.AddColumn(CO_5).Hidden().Width(mConfig.COWidth.Get(~CO_5));
	Contact.Grid.AddColumn(CO_6).Hidden().Width(mConfig.COWidth.Get(~CO_6));
	Contact.Grid.AddColumn(CO_7).Hidden().Width(mConfig.COWidth.Get(~CO_7));
	Contact.Grid.AddColumn(CO_8).Hidden().Width(mConfig.COWidth.Get(~CO_8));
	Contact.Grid.AddColumn(CO_9).Hidden().Width(mConfig.COWidth.Get(~CO_9));
	Contact.Grid.Appending().Removing().AskRemove().Editing().Canceling().ColorRows();
	//.Searching() will take the partent of Grid.FindBar then take away GridFind, so don't use
	Contact.Grid.WhenInsertRow = THISBACK(InsertContact);
	Contact.Grid.WhenUpdateRow = THISBACK(UpdateContact);
	Contact.Grid.WhenRemoveRow = THISBACK(RemoveContact);
	//Contact Search------------------------------------------
	Contact.Add(contact_search_bar.LeftPosZ(286, 84).TopPosZ(4, 20));
		Contact.Grid.FindBar(contact_search_bar, Ctrl::HorzLayoutZoom(80));
	Contact.btnSearchClear <<= callback2(&(Contact.Grid),&GridCtrl::ClearFound,true,true);
	Contact.btnSearchGo <<= callback(&(Contact.Grid),&GridCtrl::DoFind);
	
	//Event Tab-----------------------------------------------------------------------
	Event.btnCreate <<= callback(&(Event.Grid),&GridCtrl::DoAppend);
	Event.btnModify <<= callback(&(Event.Grid),&GridCtrl::DoEdit);
	Event.btnDelete <<= callback(&(Event.Grid),&GridCtrl::DoRemove);
	Event.btnExport <<= THISBACK2(ExportFile, &(Event.Grid), "Events");
	Event.btnPrint <<= THISBACK2(Print, &(Event.Grid), "Events");
	
	Event.Grid.AddIndex(E_ID).Default(-1);//for when create row before insert row;
	Event.Grid.AddIndex(C_ID);
	Event.Grid.AddColumn(C_TITLE,t_("Customer")).Edit(mEventGridCustomerBtn);
		mEventGridCustomerBtn.SetDisplay(Single<DisplayColorNotNull>());
		mEventGridCustomerBtn.AddButton().SetLabel("...").WhenPush=THISBACK(EventGridCustomerBtnClick);
	Event.Grid.AddColumn(E_ASK,t_("Request")).Edit(eesn);
	//content
	Event.Grid.AddColumn(E_STATUS,t_("Status")).Edit(mEventDropStatus);
		mEventDropStatus.AddPlus(THISBACK(EventNewStatusClick));
	Event.Grid.AddColumn(E_RTIME,t_("Request Time")).Edit(edt);
	Event.Grid.AddColumn(E_CTIME,t_("Create Time"));
	Event.Grid.AddColumn(E_NOTE,t_("Note")).Edit(ees1);
	Event.Grid.Appending().Removing().AskRemove().Editing().Canceling().ColorRows();
	Event.Grid.WhenInsertRow = THISBACK(InsertEvent);
	Event.Grid.WhenUpdateRow = THISBACK(UpdateEvent);
	Event.Grid.WhenRemoveRow = THISBACK(RemoveEvent);
	//Event Search------------------------------------------
	Event.Add(event_search_bar.LeftPosZ(147, 84).TopPosZ(4, 20));
		Event.Grid.FindBar(event_search_bar, Ctrl::HorzLayoutZoom(80));
	Event.btnSearchClear <<= callback2(&(Event.Grid),&GridCtrl::ClearFound,true,true);
	Event.btnSearchGo <<= callback(&(Event.Grid),&GridCtrl::DoFind);
	
	//Merchandise Tab-----------------------------------------------------------------------
	Merchandise.btnCreate <<= callback(&(Merchandise.Grid),&GridCtrl::DoAppend);
	Merchandise.btnModify <<= callback(&(Merchandise.Grid),&GridCtrl::DoEdit);
	Merchandise.btnDelete <<= callback(&(Merchandise.Grid),&GridCtrl::DoRemove);
	Merchandise.btnCreateF <<= THISBACK2(CreateField, &(Merchandise.Grid), "m");
	Merchandise.btnModifyF <<= THISBACK2(ModifyField, &(Merchandise.Grid), "m");
	Merchandise.btnDeleteF.Disable();
	Merchandise.btnImport <<= THISBACK2(ImportFile, &(Merchandise.Grid), "Merchandises");
	Merchandise.btnExport <<= THISBACK2(ExportFile, &(Merchandise.Grid), "Merchandises");
	Merchandise.btnPrint <<= THISBACK2(Print, &(Merchandise.Grid), "Merchandises");
	
	Merchandise.Grid.Absolute();
	Merchandise.Grid.AddIndex(M_ID).Default(-1);//for when create row before insert row;
	Merchandise.Grid.AddColumn(M_NAME,t_("Product Name")).Edit(mesn).Width(mConfig.MWidth.Get(~M_NAME));
	Merchandise.Grid.AddColumn(M_MODEL,t_("Product Model")).Edit(mes1).Width(mConfig.MWidth.Get(~M_MODEL));
	Merchandise.Grid.AddColumn(M_PRICE,t_("Price")).Edit(med).Width(mConfig.MWidth.Get(~M_PRICE));
	Merchandise.Grid.AddColumn(M_0).Hidden().Width(mConfig.MWidth.Get(~M_0));
	Merchandise.Grid.AddColumn(M_1).Hidden().Width(mConfig.MWidth.Get(~M_1));
	Merchandise.Grid.AddColumn(M_2).Hidden().Width(mConfig.MWidth.Get(~M_2));
	Merchandise.Grid.AddColumn(M_3).Hidden().Width(mConfig.MWidth.Get(~M_3));
	Merchandise.Grid.AddColumn(M_4).Hidden().Width(mConfig.MWidth.Get(~M_4));
	Merchandise.Grid.AddColumn(M_5).Hidden().Width(mConfig.MWidth.Get(~M_5));
	Merchandise.Grid.AddColumn(M_6).Hidden().Width(mConfig.MWidth.Get(~M_6));
	Merchandise.Grid.AddColumn(M_7).Hidden().Width(mConfig.MWidth.Get(~M_7));
	Merchandise.Grid.AddColumn(M_8).Hidden().Width(mConfig.MWidth.Get(~M_8));
	Merchandise.Grid.AddColumn(M_9).Hidden().Width(mConfig.MWidth.Get(~M_9));
	Merchandise.Grid.Appending().Removing().AskRemove().Editing().Canceling().ColorRows();
	Merchandise.Grid.WhenInsertRow = THISBACK(InsertMerchandise);
	Merchandise.Grid.WhenUpdateRow = THISBACK(UpdateMerchandise);
	Merchandise.Grid.WhenRemoveRow = THISBACK(RemoveMerchandise);
	//Merchandise Search------------------------------------------
	Merchandise.Add(merchandise_search_bar.LeftPosZ(286, 84).TopPosZ(4, 20));
		Merchandise.Grid.FindBar(merchandise_search_bar, Ctrl::HorzLayoutZoom(80));
	Merchandise.btnSearchClear <<= callback2(&(Merchandise.Grid),&GridCtrl::ClearFound,true,true);
	Merchandise.btnSearchGo <<= callback(&(Merchandise.Grid),&GridCtrl::DoFind);
	
	//Order Tab-----------------------------------------------------------------------
	Order.btnCreate <<= callback(&(Order.Grid),&GridCtrl::DoAppend);
	Order.btnModify <<= callback(&(Order.Grid),&GridCtrl::DoEdit);
	Order.btnDelete <<= callback(&(Order.Grid),&GridCtrl::DoRemove);
	Order.btnExport <<= THISBACK2(ExportFile, &(Order.Grid), "Orders");
	Order.btnPrint <<= THISBACK2(Print, &(Order.Grid), "Orders");
	
	Order.Grid.AddColumn(O_ID,t_("Order ID")).Default(-1).SetDisplay(Single<GDisplayNewUnsaved>());//-1 for when create row before insert row;
	Order.Grid.AddIndex(C_ID);
	Order.Grid.AddColumn(C_TITLE,t_("Customer")).Edit(mOrderGridCustomerBtn);
		mOrderGridCustomerBtn.SetDisplay(Single<DisplayColorNotNull>());
		mOrderGridCustomerBtn.AddButton().SetLabel("...").WhenPush=THISBACK(OrderGridCustomerBtnClick);
	Order.Grid.AddColumn(O_SHIP_ADD,t_("Ship Add.")).Edit(oes1);
	Order.Grid.AddColumn(O_BILL_ADD,t_("Bill Add.")).Edit(oes2);
	Order.Grid.AddColumn(O_ORDER_DATE,t_("Order Date")).Edit(odd1);
	Order.Grid.AddColumn(O_SHIP_DATE,t_("Ship Date")).Edit(odd2);
	Order.Grid.AddColumn(O_STATUS,t_("Status")).Edit(oes3);
	Order.Grid.AddColumn(O_NOTE,t_("Note")).Edit(oes4);
	Order.Grid.Appending().Removing().AskRemove().Editing().Canceling().ColorRows();
	Order.Grid.WhenInsertRow = THISBACK(InsertOrder);
	Order.Grid.WhenUpdateRow = THISBACK(UpdateOrder);
	Order.Grid.WhenRemoveRow = THISBACK(RemoveOrder);
	Order.Grid.WhenChangeRow = THISBACK(ChangeOrder);
	//Order Filter------------------------------------------
	Order.dlFilter.Add(t_("All"));//0
	Order.dlFilter.Add(t_("Past year"));//1
	Order.dlFilter.Add(t_("Past half year"));//2
	Order.dlFilter.Add(t_("Past 2 months"));//3
	Order.dlFilter.Add(t_("Past month"));//4
	Order.dlFilter.SetIndex(mConfig.OrderFilter);
	Order.btnFilterSet <<= THISBACK(OrderFilterSet);
	//Order Search------------------------------------------
	Order.Add(order_search_bar.LeftPosZ(239, 84).TopPosZ(4, 20));
		Order.Grid.FindBar(order_search_bar, Ctrl::HorzLayoutZoom(80));
	Order.btnSearchClear <<= callback2(&(Order.Grid),&GridCtrl::ClearFound,true,true);
	Order.btnSearchGo <<= callback(&(Order.Grid),&GridCtrl::DoFind);
	//Order.ContactDrop-------------------------------------
	Order.ContactDrop.AddColumn(t_("Name"));
	Order.ContactDrop.AddColumn(t_("Phone"));
	Order.ContactDrop.AddColumn(t_("Email"));
	Order.ContactDrop.Width(200);
	//Order.ContactDrop.SetValueColumn(0);
	Order.ContactDrop.AddValueColumn(0).AddValueColumn(1);
	Order.ContactDrop.Tip(t_("Contact"));
	//Order.BuyItemGrid-------------------------------------
	Order.BuyItemGrid.AddIndex(O_ID);
	Order.BuyItemGrid.AddIndex(B_ID).Default(-1);//for when create row before insert row;
	Order.BuyItemGrid.AddIndex(M_ID);
	Order.BuyItemGrid.AddColumn(M_NAME,t_("Product Name / Model")).Edit(mBuyItemGridMerchBtn);
		mBuyItemGridMerchBtn.SetDisplay(Single<DisplayColorNotNull>());
		mBuyItemGridMerchBtn.AddButton().SetLabel("...").WhenPush=THISBACK(BuyItemGridMerchBtnClick);
	//Order.BuyItemGrid.AddColumn(M_MODEL,t_("Product Model"));
	Order.BuyItemGrid.AddColumn(M_PRICE,t_("Price"));
	
	Order.BuyItemGrid.AddColumn(B_PRICE,t_("Purchase price")).Edit(bed);
	Order.BuyItemGrid.AddColumn(B_NUMBER,t_("Quantity_")).Edit(beis).Default(0);
		beis.NotNull();
	Order.BuyItemGrid.Appending().Removing().AskRemove().Editing().Canceling().ColorRows();
	Order.BuyItemGrid.SetToolBar();
	Order.BuyItemGrid.WhenNewRow = THISBACK(NewBuyItem);
	Order.BuyItemGrid.WhenInsertRow = THISBACK(InsertBuyItem);
	Order.BuyItemGrid.WhenUpdateRow = THISBACK(UpdateBuyItem);
	Order.BuyItemGrid.WhenRemoveRow = THISBACK(RemoveBuyItem);
	
	//Preference Tab-----------------------------------------------------------------------
	Preference.dlLang.Add( SetLNGCharset(LNG_('E','N','U','S'),CHARSET_UTF8) , "English" );
	Preference.dlLang.Add( SetLNGCharset(LNG_('Z','H','T','W'),CHARSET_UTF8) , "繁體中文" );
	//Preference.dlLang.Add( SetLNGCharset(LNG_('J','A','J','P'),CHARSET_UTF8) , "日本語" );
	
	int index=Preference.dlLang.FindKey(mConfig.Language);
	if(-1==index) Preference.dlLang.SetIndex(0);
	else Preference.dlLang.SetIndex(index);
	
	Preference.btnSave <<= THISBACK(SavePreference);
	Preference.btnDatabase <<= THISBACK(ConfigDB);
	
	//Help Tab-----------------------------------------------------------------------
	Help.btnLicense << THISBACK(ShowLicense);
	String lan = ToLower(LNGAsText(mConfig.Language & 0xfffff));
	Topic about = GetTopic("PikaCRM/srcdoc/About$"+lan);
	if (about.text.IsEmpty()) {
		about = GetTopic("PikaCRM/srcdoc/About$en-us");
	}	
	about.text=Replace(about.text,"##SoftwareVersion",SOFTWARE_VERSION);
	about.text=Replace(about.text,"##DatabaseVersion",DATABASE_VERSION);
	about.text=Replace(about.text,"##BuildDate",Format(BUILD_DATE));
	Help.About.SetQTF(about);
	
	Topic link = GetTopic("PikaCRM/srcdoc/Link$"+lan);
	if (link.text.IsEmpty()) {
		link = GetTopic("PikaCRM/srcdoc/Link$en-us");
	}	
	Help.Link.SetQTF(link);
	
	//WithImportLayout<TopWindow> Import;------------------------------------------------------------
	CtrlLayoutOKCancel(Import,t_("Import File"));
	
	Import.swFormat <<= 0;
	
	Import.swFormat.DisableCase(1);
	Import.swFormat.DisableCase(2);
	
	Import.dlEncode.Add("Utf8");
	Import.dlEncode.Add("Big5");
	AddCodePage("Big5","CodePage/CP950.TXT");
	Import.dlEncode.SetIndex(0);
	
}
//database control------------------------------------------------------------
void PikaCRM::LoadSetAllField()
{
	SysLog.Info("Load and Set All Fields\n");
	SQL.ExecuteX("select * from Field;");

	while(SQL.Fetch())
	{
		mFieldEditList.Add(new EditString());
		if(SQL[F_TABLE]=="c")
		{
			FieldId & field=mFieldMap.Get("c")[SQL[F_ROWID]];//"c" [0] is FieldId with C_0
			int c_index=Customer.Grid.FindCol(field.Id);
			if(-1!=c_index)
			{
				Customer.Grid.GetColumn(c_index).Edit(mFieldEditList.Top()).Name(SQL[F_NAME].ToString()).Hidden(false);
				field.IsUsed=true;
			}
		}	
		else if(SQL[F_TABLE]=="co")
		{
			FieldId & field=mFieldMap.Get("co")[SQL[F_ROWID]];
			int c_index=Contact.Grid.FindCol(field.Id);
			if(-1!=c_index)
			{
				Contact.Grid.GetColumn(c_index).Edit(mFieldEditList.Top()).Name(SQL[F_NAME].ToString()).Hidden(false);
				field.IsUsed=true;
			}
		}	
		else if(SQL[F_TABLE]=="m")
		{
			FieldId & field=mFieldMap.Get("m")[SQL[F_ROWID]];
			int c_index=Merchandise.Grid.FindCol(field.Id);
			if(-1!=c_index)
			{
				Merchandise.Grid.GetColumn(c_index).Edit(mFieldEditList.Top()).Name(SQL[F_NAME].ToString()).Hidden(false);
				field.IsUsed=true;
			}
		}
	}
}
void PikaCRM::CreateField(GridCtrl * grid, String f_table)
{
	SysLog.Info("Create a custom field\n");	
	//UI--------------------------------------------
	TopWindow d;
	Button ok, cancel;

	d.Title(t_("Create a custom field")).SetRect(0, 0, Ctrl::HorzLayoutZoom(180), Ctrl::VertLayoutZoom(80));
	d.Add(ok.SetLabel(t_("OK")).LeftPosZ(20, 45).TopPosZ(50, 16));
	d.Add(cancel.SetLabel(t_("Cancel")).LeftPosZ(100, 45).TopPosZ(50, 16));
	ok.Ok() <<= d.Acceptor(IDOK);
	cancel.Cancel() <<= d.Rejector(IDCANCEL);
	
	EditStringNotNull edit;
	Label title;
	title.SetLabel(t_("Field title: "));
	d.Add(title.LeftPosZ(15, 75).TopPosZ(20, 16));
	d.Add(edit.LeftPosZ(70, 75).TopPosZ(20, 16));
	//end UI--------------------------------------------
	bool is_no_field;
	if(d.Run()==IDOK) {
		grid->Ready(false);
		
		is_no_field=true;
		for(int i=0; i<mFieldMap.Get(f_table).GetCount(); ++i){
			FieldId & field=mFieldMap.Get(f_table)[i];//"c" [0] is FieldId with C_0
			if(false==field.IsUsed){
				is_no_field=false;
				mFieldEditList.Add(new EditString());
				int c_index=grid->FindCol(field.Id);
				if(-1!=c_index)
				{
					grid->GetColumn(c_index).Edit(mFieldEditList.Top()).Name(edit.GetData().ToString()).Hidden(false).Width(50);
					field.IsUsed=true;
				
				//INSERT INTO "main"."Field" ("f_table","f_rowid","f_name") VALUES ('c','3','asdf')
					try
					{
						SQL & Insert(FIELD)
							(F_TABLE,  f_table)
							(F_ROWID,  i)
							(F_NAME,   edit.GetData().ToString());
					}
					catch(SqlExc &e)
					{
						SysLog.Error(e+"\n");
						Exclamation("[* " + DeQtfLf(e) + "]");
					}
					
					break;
				}
			}			
		}	
			
		grid->Ready(true);
		
		if(is_no_field) Exclamation(t_("There is no more column for custom field."));
	}
}
void PikaCRM::ModifyField(GridCtrl * grid, String f_table)
{
	SysLog.Info("Modify Fields\n");
	//UI--------------------------------------------
	WithModifyFieldsLayout<TopWindow> d;
	CtrlLayoutOKCancel(d,t_("Modify Fields"));

	
	EditString edit;
	Label title;
	title.SetLabel(t_("Field title: "));
	ArrayMap<int,StaticText> stList;
	ArrayMap<int,EditString> esList;
	int y_level=24;
	int y_start=32;
	//end UI--------------------------------------------
try
{
		
	SQL.ExecuteX("select * from Field where f_table==?;",f_table);

	while(SQL.Fetch())
	{
		stList.Add(SQL[F_ROWID],new StaticText());
		esList.Add(SQL[F_ROWID],new EditString());
		d.Add(stList.Top());
		d.Add(esList.Top());
		
		stList.Top().SetText(SQL[F_NAME].ToString()).LeftPosZ(16, 60).TopPosZ(y_start, 16);
		esList.Top().LeftPosZ(104, 72).TopPosZ(y_start, 16);

		y_start+=y_level;
	}
	
	
	if(d.Run()==IDOK) {
		for(int i=0;i<esList.GetCount();++i)
		{
			if(""==(esList[i].GetData().ToString())) continue;
			
			SQL & ::Update(FIELD) (F_NAME,  ~(esList[i]))
					.Where(F_TABLE == f_table && F_ROWID==esList.GetKey(i));

			//update grid
			FieldId & field=mFieldMap.Get(f_table)[esList.GetKey(i)];//"c" [0] is FieldId with C_0
			int c_index=grid->FindCol(field.Id);
			if(-1!=c_index)
			{
				grid->GetColumn(c_index).Name(esList[i].GetData().ToString());
			}
		}
	}
	
}
catch(SqlExc &e)
{
	SysLog.Error(e+"\n");
	Exclamation("[* " + DeQtfLf(e) + "]");
}

}

void PikaCRM::LoadCustomer()
{
	SysLog.Info("Load Customers\n");
	Customer.Grid.Clear();
	SQL.ExecuteX("select * from Customer;");
	while(SQL.Fetch())
	{
		VectorMap<int, String> temp_contact_map;
		Sql sql2;
		sql2 & Select(CO_ID, CO_NAME).From(CONTACT).Where(C_ID == SQL[C_ID]);
		while(sql2.Fetch())
		{
			temp_contact_map.Add(sql2[CO_ID], sql2[CO_NAME]);
		}
		const Value & raw_map = RawToValue(temp_contact_map);
			
		Customer.Grid.Add();
		Customer.Grid(C_ID) = SQL[C_ID];
		Customer.Grid(C_TITLE) = SQL[C_TITLE];
		Customer.Grid(C_PHONE) = SQL[C_PHONE];
		Customer.Grid(C_ADDRESS) = SQL[C_ADDRESS];
		Customer.Grid(C_EMAIL) = SQL[C_EMAIL];
		Customer.Grid(C_WEBSITE) = SQL[C_WEBSITE];
		Customer.Grid(CONTACTS_MAP) = raw_map;//this is must, "=" will set the same typeid for Value of GridCtrl with RawDeepToValue
		Customer.Grid(CO_NAME) = ConvContactNames().Format(Customer.Grid(CONTACTS_MAP));
		Customer.Grid(C_0) = SQL[C_0];
		Customer.Grid(C_1) = SQL[C_1];
		Customer.Grid(C_2) = SQL[C_2];
		Customer.Grid(C_3) = SQL[C_3];
		Customer.Grid(C_4) = SQL[C_4];
		Customer.Grid(C_5) = SQL[C_5];
		Customer.Grid(C_6) = SQL[C_6];
		Customer.Grid(C_7) = SQL[C_7];
		Customer.Grid(C_8) = SQL[C_8];
		Customer.Grid(C_9) = SQL[C_9];
	}
}
void PikaCRM::NewCustomer()
{
	int costomer_id = Customer.Grid.Get(C_ID);//get C_ID value of the current row
	
	if(-1==costomer_id)//no use in Customer.Grid.AddIndex(CONTACTS_MAP).Default(RawDeepToValue(temp_contact_map));
	{					//so this is set the same typeid for Value of GridCtrl with RawDeepToValue
						//to avoid "Invalid value conversion: "
		VectorMap<int, String> temp_contact_map;
		Customer.Grid(CONTACTS_MAP)=RawToValue(temp_contact_map);
	}
}
void PikaCRM::InsertCustomer()
{
	SysLog.Debug("Insert Customer\n");
	try
	{
		SQL & Insert(CUSTOMER)
			(C_TITLE,  Customer.Grid(C_TITLE))
			(C_PHONE,  Customer.Grid(C_PHONE))
			(C_ADDRESS,Customer.Grid(C_ADDRESS))
			(C_EMAIL,  Customer.Grid(C_EMAIL))
			(C_WEBSITE,Customer.Grid(C_WEBSITE))
			(C_0,  Customer.Grid(C_0))
			(C_1,  Customer.Grid(C_1))
			(C_2,  Customer.Grid(C_2))
			(C_3,  Customer.Grid(C_3))
			(C_4,  Customer.Grid(C_4))
			(C_5,  Customer.Grid(C_5))
			(C_6,  Customer.Grid(C_6))
			(C_7,  Customer.Grid(C_7))
			(C_8,  Customer.Grid(C_8))
			(C_9,  Customer.Grid(C_9));

		Customer.Grid(C_ID) = SQL.GetInsertedId();//it will return only one int primary key
		
		//database set C_ID of CONTACTS_MAP's Contact to now
		const VectorMap<int, String> & contact_map= ValueTo< VectorMap<int, String> >(Customer.Grid(CONTACTS_MAP));
		for(int i = 0; i < contact_map.GetCount(); i++)//add already select contact to customer
		{
			int contact_id=contact_map.GetKey(i);

			SQL & Update(CONTACT) (C_ID, Customer.Grid(C_ID)).Where(CO_ID == contact_id);				
			//update Contact.Grid(C_TITLE);
			int contact_row=Contact.Grid.Find(contact_id,CO_ID);
			Contact.Grid.Set(contact_row,C_TITLE,Customer.Grid(C_TITLE));
			Contact.Grid.Set(contact_row,C_ID,Customer.Grid(C_ID));
		}			
	}
	catch(SqlExc &e)
	{
		Customer.Grid.CancelInsert();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::DuplicateCustomer()///@note not use, because not support multiselect duplicate
{
	Customer.Grid(CO_NAME)="";
	InsertCustomer();
}
void PikaCRM::UpdateCustomer()
{
	SysLog.Debug("Update Customer\n");
	try
	{
		SQL & ::Update(CUSTOMER)
			(C_TITLE,  Customer.Grid(C_TITLE))
			(C_PHONE,  Customer.Grid(C_PHONE))
			(C_ADDRESS,Customer.Grid(C_ADDRESS))
			(C_EMAIL,  Customer.Grid(C_EMAIL))
			(C_WEBSITE,Customer.Grid(C_WEBSITE))
			(C_0,  Customer.Grid(C_0))
			(C_1,  Customer.Grid(C_1))
			(C_2,  Customer.Grid(C_2))
			(C_3,  Customer.Grid(C_3))
			(C_4,  Customer.Grid(C_4))
			(C_5,  Customer.Grid(C_5))
			(C_6,  Customer.Grid(C_6))
			(C_7,  Customer.Grid(C_7))
			(C_8,  Customer.Grid(C_8))
			(C_9,  Customer.Grid(C_9))
			.Where(C_ID == Customer.Grid(C_ID));
		//update Contact.Grid(C_TITLE)	
		const VectorMap<int, String> & contact_map= ValueTo< VectorMap<int, String> >(Customer.Grid(CONTACTS_MAP));
		for(int i = 0; i < contact_map.GetCount(); i++)
		{
			int contact_id=contact_map.GetKey(i);
			int contact_row=Contact.Grid.Find(contact_id,CO_ID);
			Contact.Grid.Set(contact_row,C_TITLE,Customer.Grid(C_TITLE));
		}
	}
	catch(SqlExc &e)
	{
		Customer.Grid.CancelUpdate();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveCustomer()
{
	SysLog.Debug("Remove Customer\n");
	const VectorMap<int, String> & contact_map= ValueTo< VectorMap<int, String> >(Customer.Grid(CONTACTS_MAP));
	try
	{
		SQL & Delete(CUSTOMER).Where(C_ID == Customer.Grid(C_ID));
		///@remark just clear customer in contact, this will be a performance issue
		for(int i = 0; i < contact_map.GetCount(); i++)
		{
			int contact_id=contact_map.GetKey(i);
			
			SQL.ExecuteX("UPDATE main.Contact SET c_id = -1 WHERE co_id = ?;", contact_id);
			//clear Contact.Grid(C_TITLE);
			int contact_row=Contact.Grid.Find(contact_id,CO_ID);
			Contact.Grid.Set(contact_row,C_TITLE,"");
			Contact.Grid.Set(contact_row,C_ID,-1);//there is no use for Contact.Grid.Set(C_ID,NULL) with ever set some data	
		}
	}
	catch(SqlExc &e)
	{
		Customer.Grid.CancelRemove();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}

void PikaCRM::LoadContact()
{
	SysLog.Info("Load Contacts\n");
	Contact.Grid.Clear();
	SQL.ExecuteX("select co_id, Contact.c_id, c_title, co_name, co_phone, co_address, co_email from Contact left outer join Customer on Contact.c_id = Customer.c_id;");
	while(SQL.Fetch())
	{
		Contact.Grid.Add(SQL[CO_ID],SQL[CO_NAME],SQL[C_ID],SQL[C_TITLE],SQL[CO_PHONE],SQL[CO_ADDRESS],SQL[CO_EMAIL],SQL[CO_0],SQL[CO_1],SQL[CO_2],SQL[CO_3],SQL[CO_4],SQL[CO_5],SQL[CO_6],SQL[CO_7],SQL[CO_8],SQL[CO_9]);
	}
}
void PikaCRM::InsertContact()
{
	SysLog.Debug("Insert Contact\n");
	try
	{
		SQL & Insert(CONTACT)
			(CO_NAME,  Contact.Grid(CO_NAME))
			(C_ID,  	Contact.Grid(C_ID))
			(CO_PHONE,  Contact.Grid(CO_PHONE))
			(CO_ADDRESS,Contact.Grid(CO_ADDRESS))
			(CO_EMAIL,  Contact.Grid(CO_EMAIL))
			(CO_0,  Contact.Grid(CO_0))
			(CO_1,  Contact.Grid(CO_1))
			(CO_2,  Contact.Grid(CO_2))
			(CO_3,  Contact.Grid(CO_3))
			(CO_4,  Contact.Grid(CO_4))
			(CO_5,  Contact.Grid(CO_5))
			(CO_6,  Contact.Grid(CO_6))
			(CO_7,  Contact.Grid(CO_7))
			(CO_8,  Contact.Grid(CO_8))
			(CO_9,  Contact.Grid(CO_9));

		Contact.Grid(CO_ID) = SQL.GetInsertedId();//it will return only one int primary key
	}
	catch(SqlExc &e)
	{
		Contact.Grid.CancelInsert();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::UpdateContact()
{
	SysLog.Debug("Update Contact\n");
	try
	{
		SQL & ::Update(CONTACT)
			(CO_NAME,  Contact.Grid(CO_NAME))
			(CO_PHONE,  Contact.Grid(CO_PHONE))
			(CO_ADDRESS,Contact.Grid(CO_ADDRESS))
			(CO_EMAIL,  Contact.Grid(CO_EMAIL))
			(CO_0,  Contact.Grid(CO_0))
			(CO_1,  Contact.Grid(CO_1))
			(CO_2,  Contact.Grid(CO_2))
			(CO_3,  Contact.Grid(CO_3))
			(CO_4,  Contact.Grid(CO_4))
			(CO_5,  Contact.Grid(CO_5))
			(CO_6,  Contact.Grid(CO_6))
			(CO_7,  Contact.Grid(CO_7))
			(CO_8,  Contact.Grid(CO_8))
			(CO_9,  Contact.Grid(CO_9))
			.Where(CO_ID == Contact.Grid(CO_ID));
			
		//UpdateCustomerContact2(Contact.Grid(C_ID));----------------------------
		if(-1==Contact.Grid(C_ID)) return;
		
		int customer_id=Contact.Grid(C_ID);
		int customer_row=Customer.Grid.Find(customer_id,C_ID);
		const VectorMap<int, String> & contact_map = ValueTo< VectorMap<int, String> >(Customer.Grid.Get(customer_row, CONTACTS_MAP));
		VectorMap<int, String> new_contact_map = contact_map;
		new_contact_map.UnlinkKey(Contact.Grid(CO_ID));
		new_contact_map.Put(Contact.Grid(CO_ID), Contact.Grid(CO_NAME));
		Customer.Grid.Set(customer_row,CONTACTS_MAP,RawToValue(new_contact_map));

		String all_name;
		all_name = ConvContactNames().Format(Customer.Grid.Get(customer_row, CONTACTS_MAP));
		Customer.Grid.Set(customer_row,CO_NAME,all_name);
	}
	catch(SqlExc &e)
	{
		Contact.Grid.CancelUpdate();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveContact()
{
	SysLog.Debug("Remove Contact\n");
	try
	{
		SQL & Delete(CONTACT).Where(CO_ID == Contact.Grid(CO_ID));
	
		//UpdateCustomerContact1(Contact.Grid(C_ID));----------------------------
		if( -1==Contact.Grid(C_ID) ) return;
		
		int customer_id=Contact.Grid(C_ID);
		int customer_row=Customer.Grid.Find(customer_id,C_ID);
		if(-1==customer_row) return; //there is no use for Contact.Grid.Set(C_ID,NULL) with ever set some data,
									 //Contact.Grid(C_ID) will be 0, not null. So we check one more time;
		const VectorMap<int, String> & contact_map = ValueTo< VectorMap<int, String> >(Customer.Grid.Get(customer_row, CONTACTS_MAP));
		VectorMap<int, String> new_contact_map = contact_map;
		new_contact_map.RemoveKey(Contact.Grid(CO_ID));
		Customer.Grid.Set(customer_row,CONTACTS_MAP,RawToValue(new_contact_map));

		String all_name;
		all_name = ConvContactNames().Format(Customer.Grid.Get(customer_row, CONTACTS_MAP));
		Customer.Grid.Set(customer_row,CO_NAME,all_name);
	}
	catch(SqlExc &e)
	{
		Contact.Grid.CancelRemove();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}

void PikaCRM::LoadEvent()
{
	SysLog.Info("Load Events\n");
	Event.Grid.Clear();
	String sql ="select e_id, Event.c_id, Event.c_title, e_ask, e_status, "
				"strftime('%m/%d/%Y %H:%M',e_rtime) as e_rtime, "
				"strftime('%m/%d/%Y %H:%M',e_ctime) as e_ctime, "
				"e_note from Event left outer join Customer on Event.c_id = Customer.c_id;";
	SQL.ExecuteX(sql);
	while(SQL.Fetch())
	{
		Event.Grid.Add(SQL[E_ID],SQL[C_ID],SQL[C_TITLE],SQL[E_ASK],SQL[E_STATUS],SQL[E_RTIME],SQL[E_CTIME],SQL[E_NOTE]);
	}
	UpdateEventDropStatus();
}
void PikaCRM::InsertEvent()
{
	SysLog.Debug("Insert Event\n");
	try
	{
		SQL & Insert(EVENT)
			(C_ID,		Event.Grid(C_ID))
			(C_TITLE,	Event.Grid(C_TITLE))
			(E_ASK,		Event.Grid(E_ASK))
			(E_STATUS,  Event.Grid(E_STATUS))
			(E_RTIME,	Event.Grid(E_RTIME))
			(E_NOTE,	Event.Grid(E_NOTE));

		Event.Grid(E_ID) = SQL.GetInsertedId();//it will return only one int primary key
		Event.Grid(E_CTIME) = GetSysTime();
	}
	catch(SqlExc &e)
	{
		Event.Grid.CancelInsert();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::UpdateEvent()
{
	SysLog.Debug("Update Event\n");
	try
	{
		SQL & ::Update(EVENT)
			(C_ID,		Event.Grid(C_ID))
			(C_TITLE,	Event.Grid(C_TITLE))
			(E_ASK,		Event.Grid(E_ASK))
			(E_STATUS,  Event.Grid(E_STATUS))
			(E_RTIME,	Event.Grid(E_RTIME))
			(E_NOTE,	Event.Grid(E_NOTE))
			.Where(E_ID == Event.Grid(E_ID));
	}
	catch(SqlExc &e)
	{
		Event.Grid.CancelUpdate();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveEvent()
{
	SysLog.Debug("Remove Event\n");
	try
	{
		SQL & Delete(EVENT).Where(E_ID == Event.Grid(E_ID));
	}
	catch(SqlExc &e)
	{
		Event.Grid.CancelRemove();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::UpdateEventDropStatus()
{
	SysLog.Debug("Update Event Drop Status\n");
	try
	{
		mEventDropStatus.Clear();
		SQL & Select(E_STATUS).From(EVENT).GroupBy(E_STATUS);
	
		while(SQL.Fetch())
			mEventDropStatus.Add(SQL[0]);
	}
	catch(SqlExc &e)
	{
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}

void PikaCRM::LoadMerchandise()
{
	SysLog.Info("Load Merchandises\n");
	Merchandise.Grid.Clear();
	SQL.ExecuteX("select * from Merchandise;");
	while(SQL.Fetch())
	{
		Merchandise.Grid.Add(SQL[M_ID],SQL[M_NAME],SQL[M_MODEL],SQL[M_PRICE],SQL[M_0],SQL[M_1],SQL[M_2],SQL[M_3],SQL[M_4],SQL[M_5],SQL[M_6],SQL[M_7],SQL[M_8],SQL[M_9]);
	}
}
void PikaCRM::InsertMerchandise()
{
	SysLog.Debug("Insert Merchandise\n");
	try
	{
		SQL & Insert(MERCHANDISE)
			(M_NAME,	Merchandise.Grid(M_NAME))
			(M_MODEL,	Merchandise.Grid(M_MODEL))
			(M_PRICE,	Merchandise.Grid(M_PRICE))
			(M_0,	Merchandise.Grid(M_0))
			(M_1,	Merchandise.Grid(M_1))
			(M_2,	Merchandise.Grid(M_2))
			(M_3,	Merchandise.Grid(M_3))
			(M_4,	Merchandise.Grid(M_4))
			(M_5,	Merchandise.Grid(M_5))
			(M_6,	Merchandise.Grid(M_6))
			(M_7,	Merchandise.Grid(M_7))
			(M_8,	Merchandise.Grid(M_8))
			(M_9,	Merchandise.Grid(M_9));

		Merchandise.Grid(M_ID) = SQL.GetInsertedId();//it will return only one int primary key
	}
	catch(SqlExc &e)
	{
		Merchandise.Grid.CancelInsert();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::UpdateMerchandise()
{
	SysLog.Debug("Update Merchandise\n");
	try
	{
		SQL & ::Update(MERCHANDISE)
			(M_NAME,	Merchandise.Grid(M_NAME))
			(M_MODEL,	Merchandise.Grid(M_MODEL))
			(M_PRICE,	Merchandise.Grid(M_PRICE))
			(M_0,	Merchandise.Grid(M_0))
			(M_1,	Merchandise.Grid(M_1))
			(M_2,	Merchandise.Grid(M_2))
			(M_3,	Merchandise.Grid(M_3))
			(M_4,	Merchandise.Grid(M_4))
			(M_5,	Merchandise.Grid(M_5))
			(M_6,	Merchandise.Grid(M_6))
			(M_7,	Merchandise.Grid(M_7))
			(M_8,	Merchandise.Grid(M_8))
			(M_9,	Merchandise.Grid(M_9))
			.Where(M_ID == Merchandise.Grid(M_ID));
	}
	catch(SqlExc &e)
	{
		Merchandise.Grid.CancelUpdate();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveMerchandise()
{
	SysLog.Debug("Remove Merchandise\n");
	try
	{
		SQL & Delete(MERCHANDISE).Where(M_ID == Merchandise.Grid(M_ID));
	}
	catch(SqlExc &e)
	{
		Merchandise.Grid.CancelRemove();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}

void PikaCRM::LoadOrder()
{
	SysLog.Info("Load Orders\n");
	Order.Grid.Clear();
	String sql ="select o_id, Orders.c_id, Orders.c_title, o_ship_add, o_bill_add, "
				"strftime('%m/%d/%Y',o_order_date) as o_order_date, "
				"strftime('%m/%d/%Y',o_ship_date) as o_ship_date, "
				"o_status, o_note from Orders left outer join Customer on Orders.c_id = Customer.c_id";
	switch(Order.dlFilter.GetIndex())
	{
		default:
		case 0:	
			sql<<";";
			break;
		
		case 1:	
			sql<<" where o_order_date > date('now', '-1 year');";
			break;
		
		case 2:	
			sql<<" where o_order_date > date('now', '-183 day');";
			break;
			
		case 3:	
			sql<<" where o_order_date > date('now', '-62 day');";
			break;

		case 4:	
			sql<<" where o_order_date > date('now', '-31 day');";
			break;		
	}
	
	SQL.ExecuteX(sql);
	while(SQL.Fetch())
	{
		Order.Grid.Add(SQL[O_ID],SQL[C_ID],SQL[C_TITLE],SQL[O_SHIP_ADD],SQL[O_BILL_ADD],SQL[O_ORDER_DATE],SQL[O_SHIP_DATE],SQL[O_STATUS],SQL[O_NOTE]);
	}
}
void PikaCRM::LoadOrderCustomer()
{
	SysLog.Info("Load the Customer of the Order\n");
	Order.ContactDrop.GetList().Clear();//DropGrid.Clear() will set focus, use DropGrid.list.Clear()
	bool is_sql_ok=SQL.Execute("select * from Customer where c_id = ?;", Order.Grid(C_ID));
	if(is_sql_ok)
	{
		while(SQL.Fetch())
		{
			Order.CustomerTitle	=SQL[C_TITLE].ToString();
			Order.CustomerPhone	=SQL[C_PHONE].ToString();
			Order.CustomerADD	=SQL[C_ADDRESS].ToString();
			Order.CustomerEmail	=SQL[C_EMAIL].ToString();
			Order.CustomerWeb	=SQL[C_WEBSITE].ToString();
			Sql sql2;
			sql2 & Select(CO_ID, CO_NAME, CO_PHONE, CO_EMAIL).From(CONTACT).Where(C_ID == SQL[C_ID]);
			while(sql2.Fetch())
			{
				Order.ContactDrop.Add(sql2[CO_NAME], sql2[CO_PHONE], sql2[CO_EMAIL]);
			}
			if(!Order.ContactDrop.IsEmpty()) Order.ContactDrop.SetIndex(0);
		}
	}
	else
	{
		SysLog.Error(SQL.GetLastError()+"\n");
		Exclamation(SQL.GetLastError());///@lazy to use try catch
	}
}
void PikaCRM::InsertOrder()
{
	SysLog.Debug("Insert Order\n");
	try
	{
		SQL & Insert(ORDERS)
			(C_ID,	Order.Grid(C_ID))
			(C_TITLE,	Order.Grid(C_TITLE))
			(O_SHIP_ADD,	Order.Grid(O_SHIP_ADD))
			(O_BILL_ADD,	Order.Grid(O_BILL_ADD))
			(O_ORDER_DATE,	Order.Grid(O_ORDER_DATE))
			(O_SHIP_DATE,	Order.Grid(O_SHIP_DATE))
			(O_STATUS,		Order.Grid(O_STATUS))
			(O_NOTE,		Order.Grid(O_NOTE));

		Order.Grid(O_ID) = SQL.GetInsertedId();//it will return only one int primary key
	}
	catch(SqlExc &e)
	{
		Order.Grid.CancelInsert();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::UpdateOrder()
{
	SysLog.Debug("Update Order\n");
	String now_time="CURRENT_TIMESTAMP";
	try
	{
		SQL & ::Update(ORDERS)
			(C_ID,	Order.Grid(C_ID))
			(C_TITLE,	Order.Grid(C_TITLE))
			(O_SHIP_ADD,	Order.Grid(O_SHIP_ADD))
			(O_BILL_ADD,	Order.Grid(O_BILL_ADD))
			(O_ORDER_DATE,	Order.Grid(O_ORDER_DATE))
			(O_SHIP_DATE,	Order.Grid(O_SHIP_DATE))
			(O_STATUS,		Order.Grid(O_STATUS))
			(O_MTIME,		GetSysTime())
			(O_NOTE,		Order.Grid(O_NOTE))
			.Where(O_ID == Order.Grid(O_ID));
	}
	catch(SqlExc &e)
	{
		Order.Grid.CancelUpdate();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveOrder()
{
	SysLog.Debug("Remove Order\n");
	try
	{
		SQL & Delete(ORDERS).Where(O_ID == Order.Grid(O_ID));
		RemoveOrderBuyItem();
		Order.BuyItemGrid.Clear();
	}
	catch(SqlExc &e)
	{
		Order.Grid.CancelRemove();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::ChangeOrder()
{
	SysLog.Debug("Change Order\n");
	LoadOrderCustomer();
	LoadBuyItem(Order.Grid(O_ID));
}

void PikaCRM::LoadBuyItem(int o_id)
{
	SysLog.Info("Load Buy Items\n");
	Order.BuyItemGrid.Clear();
	bool is_sql_ok=SQL.Execute("select * from BuyItem where o_id = ?;",o_id);
	if(is_sql_ok)
	{
		while(SQL.Fetch())
		{
			Order.BuyItemGrid.Add(SQL[O_ID],SQL[B_ID],SQL[M_ID],SQL[M_NAME]/*,SQL[M_MODEL]*/,SQL[M_PRICE],SQL[B_PRICE],SQL[B_NUMBER]);
		}
	}
	else
	{
		SysLog.Error(SQL.GetLastError()+"\n");
		Exclamation(SQL.GetLastError());///@lazy to use try catch
	}
}
void PikaCRM::NewBuyItem()
{
	if(Order.Grid.IsEmpty())
	{
		Exclamation(t_("Please create an order first."));
		Order.BuyItemGrid.DoCancelEdit();
		return;
	}
	
	if(!Order.Grid.IsSelected())
	{
		Exclamation(t_("Please select an order first."));
		Order.BuyItemGrid.DoCancelEdit();
		return;
	}
	
	Order.BuyItemGrid(O_ID)=Order.Grid(O_ID);
}
void PikaCRM::InsertBuyItem()
{
	SysLog.Debug("Insert Buy Item\n");
	try
	{
		SQL & Insert(BUYITEM)
			(O_ID,		Order.BuyItemGrid(O_ID))
			(M_ID,		Order.BuyItemGrid(M_ID))
			(M_NAME,	Order.BuyItemGrid(M_NAME))
			//(M_MODEL,	Order.BuyItemGrid(M_MODEL))
			(M_PRICE,	Order.BuyItemGrid(M_PRICE))
			(B_PRICE,	Order.BuyItemGrid(B_PRICE))
			(B_NUMBER,	Order.BuyItemGrid(B_NUMBER));
			
		Order.BuyItemGrid(B_ID) = SQL.GetInsertedId();//it will return only one int primary key
	}
	catch(SqlExc &e)
	{
		Order.BuyItemGrid.CancelInsert();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::UpdateBuyItem()
{
	SysLog.Debug("Update Buy Item\n");
	try
	{
		SQL & ::Update(BUYITEM)
			(O_ID,		Order.BuyItemGrid(O_ID))
			(M_ID,		Order.BuyItemGrid(M_ID))
			(M_NAME,	Order.BuyItemGrid(M_NAME))
			//(M_MODEL,	Order.BuyItemGrid(M_MODEL))
			(M_PRICE,	Order.BuyItemGrid(M_PRICE))
			(B_PRICE,	Order.BuyItemGrid(B_PRICE))
			(B_NUMBER,	Order.BuyItemGrid(B_NUMBER))
			.Where(B_ID == Order.BuyItemGrid(B_ID));
	}
	catch(SqlExc &e)
	{
		Order.BuyItemGrid.CancelUpdate();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveBuyItem()
{
	SysLog.Debug("Remove Buy Item\n");
	try
	{
		SQL & Delete(BUYITEM).Where(B_ID == Order.BuyItemGrid(B_ID));
	}
	catch(SqlExc &e)
	{
		Order.BuyItemGrid.CancelRemove();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveOrderBuyItem()
{
	SysLog.Debug("Remove Order Buy Item\n");
	try
	{
		SQL & Delete(BUYITEM).Where(O_ID == Order.Grid(O_ID));
	}
	catch(SqlExc &e)
	{
		Order.BuyItemGrid.CancelRemove();
		SysLog.Error(e+"\n");
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
//application control-----------------------------------------------------------
String PikaCRM::GetLogPath()
{
	return getConfigDirPath()+FILE_LOG;	
}
void PikaCRM::OpenMainFrom()
{	
	try
	{
		//Load and set customer field(UI+data)
		LoadSetAllField();
		//Load all tab data
		LoadCustomer();
		LoadContact();
		LoadEvent();
		LoadMerchandise();
		LoadOrder();	
	}
	catch(SqlExc &e)
	{
		mSplash.HideSplash();
		SysLog.Error(e+"\n");
		Exclamation( t_("There is a database operation error.\n"
						"If data is not correct, please report to our web site (Help->Report bugs) with the log and last error: \n")
						+ SQL.GetLastError());
	}
	
	if(mConfig.IsMaximized) MainFrom.Maximize();
	MainFrom.OpenMain();
	
	mSplash.ShowSplashStatus(t_("Normal Running..."));
	SysLog.Info(t_("Normal Running..."))<<"\n";
	
	mSplash.SetSplashTimer(500);
}
void PikaCRM::CloseMainFrom()//MainFrom.WhenClose call back
{
	SysLog.Info("close application\n");
	String config_file_path = getConfigDirPath()+FILE_CONFIG;
	double base=Ctrl::HorzLayoutZoom(1000);
	double fac=1000/base;
	//save all width of each column
		mConfig.CWidth.Get(~C_TITLE)=round(Customer.Grid.FindColWidth(C_TITLE)*fac);
		mConfig.CWidth.Get(~C_PHONE)=round(Customer.Grid.FindColWidth(C_PHONE)*fac);
		mConfig.CWidth.Get(~C_ADDRESS)=round(Customer.Grid.FindColWidth(C_ADDRESS)*fac);
		mConfig.CWidth.Get(~C_EMAIL)=round(Customer.Grid.FindColWidth(C_EMAIL)*fac);
		mConfig.CWidth.Get(~C_WEBSITE)=round(Customer.Grid.FindColWidth(C_WEBSITE)*fac);
		mConfig.CWidth.Get(~CO_NAME)=round(Customer.Grid.FindColWidth(CO_NAME)*fac);
		mConfig.CWidth.Get(~C_0)=round(Customer.Grid.FindColWidth(C_0)*fac);
		mConfig.CWidth.Get(~C_1)=round(Customer.Grid.FindColWidth(C_1)*fac);
		mConfig.CWidth.Get(~C_2)=round(Customer.Grid.FindColWidth(C_2)*fac);
		mConfig.CWidth.Get(~C_3)=round(Customer.Grid.FindColWidth(C_3)*fac);
		mConfig.CWidth.Get(~C_4)=round(Customer.Grid.FindColWidth(C_4)*fac);
		mConfig.CWidth.Get(~C_5)=round(Customer.Grid.FindColWidth(C_5)*fac);
		mConfig.CWidth.Get(~C_6)=round(Customer.Grid.FindColWidth(C_6)*fac);
		mConfig.CWidth.Get(~C_7)=round(Customer.Grid.FindColWidth(C_7)*fac);
		mConfig.CWidth.Get(~C_8)=round(Customer.Grid.FindColWidth(C_8)*fac);
		mConfig.CWidth.Get(~C_9)=round(Customer.Grid.FindColWidth(C_9)*fac);
	
		mConfig.COWidth.Get(~CO_NAME)=round(Contact.Grid.FindColWidth(CO_NAME)*fac);
		mConfig.COWidth.Get(~C_TITLE)=round(Contact.Grid.FindColWidth(C_TITLE)*fac);
		mConfig.COWidth.Get(~CO_PHONE)=round(Contact.Grid.FindColWidth(CO_PHONE)*fac);
		mConfig.COWidth.Get(~CO_ADDRESS)=round(Contact.Grid.FindColWidth(CO_ADDRESS)*fac);
		mConfig.COWidth.Get(~CO_EMAIL)=round(Contact.Grid.FindColWidth(CO_EMAIL)*fac);
		mConfig.COWidth.Get(~CO_0)=round(Contact.Grid.FindColWidth(CO_0)*fac);
		mConfig.COWidth.Get(~CO_1)=round(Contact.Grid.FindColWidth(CO_1)*fac);
		mConfig.COWidth.Get(~CO_2)=round(Contact.Grid.FindColWidth(CO_2)*fac);
		mConfig.COWidth.Get(~CO_3)=round(Contact.Grid.FindColWidth(CO_3)*fac);	
		mConfig.COWidth.Get(~CO_4)=round(Contact.Grid.FindColWidth(CO_4)*fac);
		mConfig.COWidth.Get(~CO_5)=round(Contact.Grid.FindColWidth(CO_5)*fac);
		mConfig.COWidth.Get(~CO_6)=round(Contact.Grid.FindColWidth(CO_6)*fac);
		mConfig.COWidth.Get(~CO_7)=round(Contact.Grid.FindColWidth(CO_7)*fac);
		mConfig.COWidth.Get(~CO_8)=round(Contact.Grid.FindColWidth(CO_8)*fac);
		mConfig.COWidth.Get(~CO_9)=round(Contact.Grid.FindColWidth(CO_9)*fac);
		
				
		mConfig.MWidth.Get(~M_NAME)=round(Merchandise.Grid.FindColWidth(M_NAME)*fac);
		mConfig.MWidth.Get(~M_MODEL)=round(Merchandise.Grid.FindColWidth(M_MODEL)*fac);
		mConfig.MWidth.Get(~M_PRICE)=round(Merchandise.Grid.FindColWidth(M_PRICE)*fac);
		mConfig.MWidth.Get(~M_0)=round(Merchandise.Grid.FindColWidth(M_0)*fac);
		mConfig.MWidth.Get(~M_1)=round(Merchandise.Grid.FindColWidth(M_1)*fac);
		mConfig.MWidth.Get(~M_2)=round(Merchandise.Grid.FindColWidth(M_2)*fac);
		mConfig.MWidth.Get(~M_3)=round(Merchandise.Grid.FindColWidth(M_3)*fac);
		mConfig.MWidth.Get(~M_4)=round(Merchandise.Grid.FindColWidth(M_4)*fac);
		mConfig.MWidth.Get(~M_5)=round(Merchandise.Grid.FindColWidth(M_5)*fac);
		mConfig.MWidth.Get(~M_6)=round(Merchandise.Grid.FindColWidth(M_6)*fac);
		mConfig.MWidth.Get(~M_7)=round(Merchandise.Grid.FindColWidth(M_7)*fac);
		mConfig.MWidth.Get(~M_8)=round(Merchandise.Grid.FindColWidth(M_8)*fac);
		mConfig.MWidth.Get(~M_9)=round(Merchandise.Grid.FindColWidth(M_9)*fac);
	
	mConfig.IsMaximized=MainFrom.IsMaximized();
	
	SaveConfig(config_file_path);
	
	mSplash.~SplashSV();
	MainFrom.Close();
}

bool PikaCRM::IsHaveDBFile(const String & database_file_path)
{	
	return FileExists(database_file_path);
}

void PikaCRM::CreateOrOpenDB(const String & database_file_path)
{
	mSqlite3Session.Close();
	if(!mSqlite3Session.Open(database_file_path))
	{
		String msg = t_("Can't create or open database file: ")	+ database_file_path;
		throw ApExc(msg).SetHandle(ApExc::SYS_FAIL);
	}
	SysLog.Debug("created or opened database file: "+database_file_path+"\n");
	
#ifdef _DEBUG
	mSqlite3Session.SetTrace();
#endif
	
	SQL = mSqlite3Session;//this is Upp default globe 
	
	if(!mConfig.Password.IsEqual(PW_EMPTY))
	{
		SysLog.Info("set database encrypted key.\n");
		if(!mSqlite3Session.SetKey(getSwap1st2ndChar(mConfig.Password)))
		{
			SysLog.Error("sqlite3 set key error\n");
			///@note we dont know how to deal this error, undefine		
		}
	}
}
int PikaCRM::GetDBVersion()
{
	int ver;
	int is_sql_ok=SQL.Execute("select * from System;");
	///@remark "PRAGMA user_version = integer ;" is useful?
	if(is_sql_ok)
	{
		while(SQL.Fetch())//user,ap_ver,sqlite_ver,db_ver
		{
			SysLog.Debug("") << SQL[USER]<<", "<<SQL[CTIME]<<", "<<
								SQL[AP_VER]<<", "<<SQL[SQLITE_VER]<<", "<<SQL[DB_VER]<<"\n";
			ver=SQL[DB_VER];
		}
		return ver;
	}
	else
	{
		return 0;
	}
}
void PikaCRM::InitialDB()
{
	SysLog.Info("initial the database file\n");
	FileIn initial("initial.sql");
	if(!initial.IsOpen())
	{
		throw ApExc("Can't open database initial file: ./initial.sql").SetHandle(ApExc::SYS_FAIL);
	}
	if(!SqlPerformScript(mSqlite3Session,initial))
	{
		throw ApExc("Perform Script ./initial.sql Fail!").SetHandle(ApExc::SYS_FAIL);
	}
	
	int is_sql_ok=SQL.Execute("INSERT INTO System (user,ap_ver,sqlite_ver,db_ver) VALUES (?,?,?,?);",
							"System",SOFTWARE_VERSION,mSqlite3Session.VersionInfo(),DATABASE_VERSION);
							///@todo set user name
							
	if(!is_sql_ok) throw ApExc("initial the database fail").SetHandle(ApExc::SYS_FAIL);					
#ifdef _DEBUG
	//in debug
	String out=ExportSch(mSqlite3Session, "main");						
	SaveFile("sql.sch", out);	
	SaveFile("sql.ids", ExportIds(mSqlite3Session, "main"));

#endif
}
bool PikaCRM::IsSetupDB(const String config_file_path)
{
	SysLog.Info("setup the database\n");
	WithInitialDBLayout<TopWindow> d;
	CtrlLayoutOKCancel(d, t_("Setup your database"));
	d.TopMost();
	d.esPassword.Password();
	d.esCheckPassword.Password();
	d.optRequire.SetEditable(false);
	d.optPW.WhenAction=THISBACK1(OnOptPWAction, &d);
	d.optRevealPW.WhenAction=THISBACK1(OnOptRevealPWAction, &d);
	d.ok.WhenPush = THISBACK1(CheckPWSame, &d);
	
	String note,note2;
	Font ff= GetStdFont();
	SysLog.Debug(ff.GetFaceName()+"\n");
	SysLog.Debug("h: ")<<ff.GetHeight();
	SysLog.Debug(" fh: ")<<ff.Info().GetFontHeight();
	SysLog.Debug(" 1zh: ")<<StdFontZ(1);
	SysLog.Debug(" 10zh: ")<<StdFontZ(10);
	SysLog.Debug(" 11zh: ")<<StdFontZ(11);
	SysLog.Debug(" 12zh: ")<<StdFontZ(12);
	
	note<<"[+75G "<<t_("Encrypted database can't be read even if someone has the database file.")<<" ]";
	d.rtNoteEncrypted.SetQTF(note);
	note2<<"[+75G "<<t_("Important: if you forgot the password, there is no way to access your database.")<<" ]";	
	d.rtNoteEncrypted2.SetQTF(note2);
	
	//Load from Config
	d.optPW=mConfig.IsDBEncrypt;
	OnOptPWAction(&d);
	d.optRequire=!mConfig.IsRememberPW;
	//Get from IsInputPWCheck
	d.esPassword=mRevealedPW;
	d.esCheckPassword=mRevealedPW;
	if(d.Run() == IDOK) {
		if(d.optPW)
		{
			String tempPW=d.esPassword.GetData();
			mConfig.IsDBEncrypt=true;
			mConfig.Password=getMD5(tempPW<<PW_MAGIC_WORD);
			mConfig.IsRememberPW=!(bool)d.optRequire;	
			mConfig.SystemPWKey=CombineKey(GetSystemKey(), mConfig.Password);	
		}
		else
		{
			//String tempPW=d.esPassword.GetData();
			mConfig.IsDBEncrypt=false;
			mConfig.Password=PW_EMPTY;
			mConfig.IsRememberPW=false;	
			mConfig.SystemPWKey="";			
		}

		SaveConfig(config_file_path);
		return true;
	}
	else
	{
		SysLog.Info("Cancel setup the database\n");
		return false;
	}
}
void PikaCRM::OnOptPWAction(WithInitialDBLayout<TopWindow> * d)
{
	if(d->optPW)
	{
		d->esPassword.SetEditable(true);
		d->esPassword.NotNull(true);
		d->esPassword.WantFocus(true);
		d->esCheckPassword.SetEditable(true);
		d->esCheckPassword.NotNull(true);
		d->esCheckPassword.WantFocus(true);
		d->optRequire.SetEditable(true);
	}
	else 
	{
		d->esPassword.SetEditable(false);
		d->esPassword.NotNull(false);
		d->esPassword.WantFocus(false);
		d->esCheckPassword.SetEditable(false);
		d->esCheckPassword.NotNull(false);
		d->esCheckPassword.WantFocus(false);
		d->optRequire.SetEditable(false);
	}
}
void PikaCRM::OnOptRevealPWAction(WithInitialDBLayout<TopWindow> * d)
{
	if(d->optRevealPW)
	{
		d->esPassword.Password(false);
		d->esCheckPassword.Password(false);
	}
	else 
	{
		d->esPassword.Password();
		d->esCheckPassword.Password();
	}
}
void PikaCRM::CheckPWSame(WithInitialDBLayout<TopWindow> * d)
{
	String p1=d->esPassword;
	String p2=d->esCheckPassword;
	if(d->optPW && !p1.IsEqual(p2))
		PromptOK(t_("Re-Enter Password does correspond to the new Password."));
}

void PikaCRM::LoadConfig(const String & config_file_path)
{
	if(FileExists(config_file_path))
	{	
		FileIn in(config_file_path);
		if(in.IsOpen())
			;
		else
		{
			String msg= t_("The config file can not be open!\n"
						"Please check you have the privilege to open:\n")
						+ config_file_path;
			throw ApExc(msg).SetHandle(ApExc::SYS_FAIL);
		}
		
		if(mConfig.Load(config_file_path))
		{
			SysLog.Info("loaded the config file\n");
		}
		else
		{
			throw ApExc("load the config file fail\n").SetHandle(ApExc::SYS_FAIL);
		}
	}
	else
	{
		SysLog.Info("make a new config file\n");
		mConfig.Language = SetLNGCharset( GetSystemLNG(), CHARSET_UTF8 );
		mConfig.IsDBEncrypt=false;
		mConfig.Password=PW_EMPTY;
		mConfig.IsRememberPW=false;
		mConfig.SystemPWKey="";
	
		mConfig.CWidth.Add(~C_TITLE, 100);
		mConfig.CWidth.Add(~C_PHONE, 100);
		mConfig.CWidth.Add(~C_ADDRESS, 100);
		mConfig.CWidth.Add(~C_EMAIL, 100);
		mConfig.CWidth.Add(~C_WEBSITE, 100);
		mConfig.CWidth.Add(~CO_NAME, 100);
		mConfig.CWidth.Add(~C_0, 0);
		mConfig.CWidth.Add(~C_1, 0);
		mConfig.CWidth.Add(~C_2, 0);
		mConfig.CWidth.Add(~C_3, 0);
		mConfig.CWidth.Add(~C_4, 0);
		mConfig.CWidth.Add(~C_5, 0);
		mConfig.CWidth.Add(~C_6, 0);
		mConfig.CWidth.Add(~C_7, 0);
		mConfig.CWidth.Add(~C_8, 0);
		mConfig.CWidth.Add(~C_9, 0);
		
		mConfig.COWidth.Add(~CO_NAME, 100);
		mConfig.COWidth.Add(~C_TITLE, 100);
		mConfig.COWidth.Add(~CO_PHONE, 100);
		mConfig.COWidth.Add(~CO_ADDRESS, 100);
		mConfig.COWidth.Add(~CO_EMAIL, 100);
		mConfig.COWidth.Add(~CO_0, 0);
		mConfig.COWidth.Add(~CO_1, 0);
		mConfig.COWidth.Add(~CO_2, 0);
		mConfig.COWidth.Add(~CO_3, 0);
		mConfig.COWidth.Add(~CO_4, 0);
		mConfig.COWidth.Add(~CO_5, 0);
		mConfig.COWidth.Add(~CO_6, 0);
		mConfig.COWidth.Add(~CO_7, 0);
		mConfig.COWidth.Add(~CO_8, 0);
		mConfig.COWidth.Add(~CO_9, 0);
		
		mConfig.MWidth.Add(~M_NAME, 100);
		mConfig.MWidth.Add(~M_MODEL, 100);
		mConfig.MWidth.Add(~M_PRICE, 100);
		mConfig.MWidth.Add(~M_0, 0);
		mConfig.MWidth.Add(~M_1, 0);
		mConfig.MWidth.Add(~M_2, 0);
		mConfig.MWidth.Add(~M_3, 0);
		mConfig.MWidth.Add(~M_4, 0);
		mConfig.MWidth.Add(~M_5, 0);
		mConfig.MWidth.Add(~M_6, 0);
		mConfig.MWidth.Add(~M_7, 0);
		mConfig.MWidth.Add(~M_8, 0);
		mConfig.MWidth.Add(~M_9, 0);
		
		mConfig.IsMaximized=false;
		mConfig.OrderFilter=0;
		
		SaveConfig(config_file_path);
	}
}

void PikaCRM::SaveConfig(const String & config_file_path)
{
	if(mConfig.Save(config_file_path))
	{
		SysLog.Info("Saved the config file\n");
	}
	else
	{
		String msg = t_("Can't write the application config directory!\n"
						"Please check you have the privilege to write:\n")
						+ config_file_path;
		throw ApExc(msg).SetHandle(ApExc::NOTICE);
	}
}


bool PikaCRM::IsInputPWCheck()
{
	String config_file_path=getConfigDirPath()+FILE_CONFIG;
	WithInputPWLayout<TopWindow> d;
	CtrlLayoutOKCancel(d,t_("Pika Customer Relationship Management"));
	d.esPassword.Password();
	d.ok.WhenPush = THISBACK2(CheckPWRight, &d, mConfig.Password);
	d.optRememberPW = mConfig.IsRememberPW;
	if(d.Run() == IDOK) {
		mConfig.IsRememberPW=(bool)d.optRememberPW;
		if(d.optRememberPW) mConfig.SystemPWKey=CombineKey(GetSystemKey(), mConfig.Password);
		SaveConfig(config_file_path);
		return true;
	}
	else
	{
		SysLog.Info("Cancel input the password\n");
		return false;
	}
}

void PikaCRM::CheckPWRight(WithInputPWLayout<TopWindow> * d, const String & pw)
{
	String p1=d->esPassword;
	String pwMD5=getMD5(p1<<PW_MAGIC_WORD);
	if(pw.IsEqual(pwMD5))
		mRevealedPW=d->esPassword;
	else
		Exclamation(t_("The Password is incorrect!"));
}


#ifdef PLATFORM_WIN32
#include "Win32HDSN.h"
#endif
String PikaCRM::GetSystemKey()
{
	SysLog.Info("Get the System Key\n");	
	int		pos;
	String 	output;
	String	key;
	
#ifdef PLATFORM_POSIX
	FILE * 	p_process;
	char  	buf[101]={};
	const char	process_cmd[] = "./hdsn 2>&1";
		if ( NULL != (p_process=popen(process_cmd,"r")) ) 
		{
			if ( fread(buf,sizeof(char),100,p_process) > 0) 
			{
				output=buf;
			}
			pclose(p_process);
		}
		else 
		{
		    output="fail to execute hdsn";
		}

#elif defined(PLATFORM_WIN32)
	String path=GetExeFilePath();
	int driveID=gGetPhysicalDeviceID(path[0]);
	SysLog.Debug("driveID: ")<<driveID<<"\n";
	char buf[1024]={};
	char * sn = gGetHardDriveSerialNumber (driveID, buf);
	output=sn;
#endif

	if( -1 != (pos=output.Find("serial_no:")) )
		key=getMD5(output);
	else
	{
		SysLog.Error(output+"\n");
		///@remark throw error? or just disable remember PW?
		//throw ApExc(output+"\n").SetHandle(ApExc::SYS_FAIL);
	}
	
	SysLog.Debug("System key: "+key+"\n");
	return key;
}

String PikaCRM::CombineKey(const String & key1, const String & key2) //avoid hacker copy system key and run
{
	String ckey;
	if(!key1.IsEmpty() && !key2.IsEmpty())
	{
		String temp=key1+key2;
		ckey=getMD5(temp);
	}
	return ckey;
}

void PikaCRM::SetAllFieldMap()
{
	Vector<FieldId> c;
	c.Add().SetId(C_0);
	c.Add().SetId(C_1);
	c.Add().SetId(C_2);
	c.Add().SetId(C_3);
	c.Add().SetId(C_4);
	c.Add().SetId(C_5);
	c.Add().SetId(C_6);
	c.Add().SetId(C_7);
	c.Add().SetId(C_8);
	c.Add().SetId(C_9);
	mFieldMap.Add("c",c);
	
	Vector<FieldId> co;
	co.Add().SetId(CO_0);
	co.Add().SetId(CO_1);
	co.Add().SetId(CO_2);
	co.Add().SetId(CO_3);
	co.Add().SetId(CO_4);
	co.Add().SetId(CO_5);
	co.Add().SetId(CO_6);
	co.Add().SetId(CO_7);
	co.Add().SetId(CO_8);
	co.Add().SetId(CO_9);
	mFieldMap.Add("co",co);
		
	Vector<FieldId> m;
	m.Add().SetId(M_0);
	m.Add().SetId(M_1);
	m.Add().SetId(M_2);
	m.Add().SetId(M_3);
	m.Add().SetId(M_4);
	m.Add().SetId(M_5);
	m.Add().SetId(M_6);
	m.Add().SetId(M_7);
	m.Add().SetId(M_8);
	m.Add().SetId(M_9);
	mFieldMap.Add("m",m);
}
//end application control-----------------------------------------------------------
//interactive with GUI==============================================================
void PikaCRM::FirstWelcome()
{
	SysLog.Info("First Run PikaCRM. Welcome!\n");
	WithFirstWelcomeLayout<TopWindow> d;
	CtrlLayoutOKCancel(d, t_("Welcome to use PikaCRM"));

	d.ok.WhenPush = THISBACK1(CheckAgree, &(d.agree));
	d.cancel.Hide();
	String note;
	note<<"[2G "<<t_("Welcome to use PikaCRM")<<" &]";
	note<<"[+75G "<<t_("You must accept the license to use")<<" ]";
	d.Welcome.SetQTF(note);

	Topic t = GetTopic("PikaCRM/srcdoc/License$"+ ToLower(LNGAsText(mConfig.Language & 0xfffff)));
	if (t.text.IsEmpty()) {
		t = GetTopic("PikaCRM/srcdoc/License$en-us");
	}	
	d.License.SetQTF(t);
	
	if(d.Run() == IDOK) {
		;//do nothing		
	}
	else{
		throw ApExc("user cancel").SetHandle(ApExc::NONE);
	}
}
void PikaCRM::CheckAgree(Option * agree)
{
	if(true!=agree->Get()) Exclamation(t_("You must accept the license to use"));
}
void PikaCRM::CustomerGridContactBtnClick()
{
	if(Customer.Grid(C_TITLE).ToString().IsEmpty())
	{
		Exclamation(t_("You must input Customer Title first!"));
		return;
	}
	//UI--------------------------------------------
	TopWindow d;
	Button ok, cancel;

    d.Title(t_("Select contacts (Multiple select)")).SetRect(0, 0, 400, 400);
	d.Add(ok.SetLabel(t_("OK")).LeftPosZ(40, 64).BottomPosZ(12, 24));
	d.Add(cancel.SetLabel(t_("Cancel")).LeftPosZ(130, 64).BottomPosZ(12, 24));
	ok.Ok() <<= d.Acceptor(IDOK);
	cancel.Cancel() <<= d.Rejector(IDCANCEL);
	
	ColumnListAlwaysCTRL list;
	d.Add(list);
	list.SetRect(0, 0, 400, 325);
	list.Columns(3);
	list.MultiSelect();
	//end UI--------------------------------------------
	int costomer_id = Customer.Grid.Get(C_ID);//get C_ID value of the current row

	const VectorMap<int, String> & contact_map= ValueTo< VectorMap<int, String> >(Customer.Grid(CONTACTS_MAP));
	
	for(int i = 0; i < contact_map.GetCount(); i++)//add already select contact to costomer column list
	{
		int contact_id=contact_map.GetKey(i);
		list.Add(contact_id, contact_map.Get(contact_id), true);
		
		int list_index=list.Find(contact_id);//use key find index
		if(0==i) list.SetCursor(list_index);///@important must set cursor once and first, or it will clear all selected
		list.SelectOne(list_index,true);
	}
try
{
	//add no costomer contact to select
	SQL & Select(CO_ID, CO_NAME).From(CONTACT).Where(C_ID==-1);
	while(SQL.Fetch())
		list.Add(SQL[CO_ID], SQL[CO_NAME],true);
	

	if(d.Run()==IDOK) {
		
		///@remark just clear costomer in contact, this will be a performance issue
		for(int i = 0; i < contact_map.GetCount(); i++)
		{
			int contact_id=contact_map.GetKey(i);

			//SQL & ::Update(CONTACT) (C_ID, NULL).Where(CO_ID == contact_id);//fail, NULL will be 0
			SQL.ExecuteX("UPDATE main.Contact SET c_id = -1 WHERE co_id = ?;", contact_id);
				
			//clear Contact.Grid(C_TITLE);
			int contact_row=Contact.Grid.Find(contact_id,CO_ID);
			Contact.Grid.Set(contact_row,C_TITLE,"");
			Contact.Grid.Set(contact_row,C_ID,-1);		
		}
		
		VectorMap<int, String> new_contact_map;
		for(int i = 0; i < list.GetCount(); i++)
		{
			if(list.IsSel(i))
			{
				int contact_id=list.Get(i);
				//update in the database. if costomer_id ==-1, no need to do, it will do in InsertCustomer()

				if(-1 != costomer_id)
				{
					SQL & ::Update(CONTACT) (C_ID,  costomer_id).Where(CO_ID == contact_id);
					//update Contact.Grid(C_TITLE);
					int contact_row=Contact.Grid.Find(contact_id,CO_ID);
					Contact.Grid.Set(contact_row,C_TITLE,Customer.Grid(C_TITLE));
					Contact.Grid.Set(contact_row,C_ID,Customer.Grid(C_ID));
				}
								
				//record in the map
				new_contact_map.Add(contact_id,list.GetValue(i));///@note if list i has no key, it will assert fail
			}
		}
		Customer.Grid(CONTACTS_MAP)=RawToValue(new_contact_map);
		String all_name = ConvContactNames().Format(Customer.Grid(CONTACTS_MAP));
		
		Customer.Grid.Set(CO_NAME,all_name);//Customer.Grid(CO_NAME)=hh;//must use Set to refresh and show
    }
    
}
catch(SqlExc &e)
{
	SysLog.Error(e+"\n");
	Exclamation("[* " + DeQtfLf(e) + "]");
}

}

void PikaCRM::EventGridCustomerBtnClick()
{
	//UI--------------------------------------------
	TopWindow d;
	Button ok, cancel;

    d.Title(t_("Select one customer")).SetRect(0, 0, 400, 400);
	d.Add(ok.SetLabel(t_("OK")).LeftPosZ(40, 64).BottomPosZ(12, 24));
	d.Add(cancel.SetLabel(t_("Cancel")).LeftPosZ(130, 64).BottomPosZ(12, 24));
	ok.Ok() <<= d.Acceptor(IDOK);
	cancel.Cancel() <<= d.Rejector(IDCANCEL);
	
	ColumnList list;
	d.Add(list);
	list.SetRect(0, 0, 400, 325);
	list.Columns(3);
	//list.MultiSelect();
	//end UI--------------------------------------------
try
{
	//add costomer to select
	SQL & Select(C_ID, C_TITLE).From(CUSTOMER);
	while(SQL.Fetch())
	{
		list.Add(SQL[C_ID], SQL[C_TITLE],true);
	}
	
	if(list.GetCount()<=0)	return;//there is no any customer
	if(Event.Grid(C_ID).IsNull())
	{
		int list_index=list.Find(Customer.Grid(C_ID));
		list.SetCursor(list_index);
	}
	else
	{
		int list_index=list.Find(Event.Grid(C_ID));
		list.SetCursor(list_index);
	}
	
	int costomer_id;
	String title;
	if(d.Run()==IDOK) {
		for(int i = 0; i < list.GetCount(); i++)
		{
			if(list.IsSel(i))
			{
				costomer_id=list.Get(i);
								
				//show on the grid
				title=String(list.GetValue(i));
				
				//update in the database
				if(-1 != Event.Grid(E_ID))
				{
					SQL & ::Update(EVENT) (C_ID, costomer_id) (C_TITLE,title).Where(E_ID == Event.Grid(E_ID));
				}
			}
		}
		Event.Grid.Set(C_ID,costomer_id);		
		Event.Grid.Set(C_TITLE,title);
    }
    
}
catch(SqlExc &e)
{
	SysLog.Error(e+"\n");
	Exclamation("[* " + DeQtfLf(e) + "]");
}
}
void PikaCRM::EventNewStatusClick()
{
	//UI--------------------------------------------
	TopWindow d;
	Button ok, cancel;

	d.Title(t_("New status")).SetRect(0, 0, 300, 150);
	d.Add(ok.SetLabel(t_("OK")).LeftPosZ(20, 45).TopPosZ(50, 16));
	d.Add(cancel.SetLabel(t_("Cancel")).LeftPosZ(100, 45).TopPosZ(50, 16));
	ok.Ok() <<= d.Acceptor(IDOK);
	cancel.Cancel() <<= d.Rejector(IDCANCEL);
	
	EditStringNotNull edit;
	Label title;
	title.SetLabel(t_("Status: "));
	d.Add(title.LeftPosZ(18, 75).TopPosZ(20, 16));
	d.Add(edit.LeftPosZ(70, 75).TopPosZ(20, 16));
	//end UI--------------------------------------------
	if(d.Run()==IDOK) {
		mEventDropStatus.Add(edit.GetData());
		mEventDropStatus.SetData(edit.GetData());
	}
}

void PikaCRM::OrderGridCustomerBtnClick()
{
	//UI--------------------------------------------
	TopWindow d;
	Button ok, cancel;

    d.Title(t_("Select one customer")).SetRect(0, 0, 400, 400);
	d.Add(ok.SetLabel(t_("OK")).LeftPosZ(40, 64).BottomPosZ(12, 24));
	d.Add(cancel.SetLabel(t_("Cancel")).LeftPosZ(130, 64).BottomPosZ(12, 24));
	ok.Ok() <<= d.Acceptor(IDOK);
	cancel.Cancel() <<= d.Rejector(IDCANCEL);
	
	ColumnList list;
	d.Add(list);
	list.SetRect(0, 0, 400, 325);
	list.Columns(3);
	//end UI--------------------------------------------
try{
	//add costomer to select
	SQL & Select(C_ID, C_TITLE).From(CUSTOMER);
	while(SQL.Fetch())
	{
		list.Add(SQL[C_ID], SQL[C_TITLE],true);
	}
	
	if(list.GetCount()<=0)	return;//there is no any customer
	if(Order.Grid(C_ID).IsNull())
	{
		int list_index=list.Find(Customer.Grid(C_ID));
		list.SetCursor(list_index);
	}
	else
	{
		int list_index=list.Find(Order.Grid(C_ID));
		list.SetCursor(list_index);
	}
	
	int costomer_id;
	String title;
	if(d.Run()==IDOK) {
		for(int i = 0; i < list.GetCount(); i++)
		{
			if(list.IsSel(i))
			{
				costomer_id=list.Get(i);
								
				//show on the grid
				title=String(list.GetValue(i));
				
				//update in the database
				if(-1 != Order.Grid(O_ID))
				{
					SQL & ::Update(ORDERS) (C_ID, costomer_id) (C_TITLE,title).Where(O_ID == Order.Grid(O_ID));
				}
			}
		}
		Order.Grid.Set(C_ID,costomer_id);		
		Order.Grid.Set(C_TITLE,title);
		LoadOrderCustomer();
    }
}
catch(SqlExc &e)
{
	SysLog.Error(e+"\n");
	Exclamation("[* " + DeQtfLf(e) + "]");
}
}

void PikaCRM::OrderFilterSet()
{
	mConfig.OrderFilter=Order.dlFilter.GetIndex();
	String config_file_path=getConfigDirPath()+FILE_CONFIG;
	SaveConfig(config_file_path);
	
	LoadOrder();
}

void PikaCRM::BuyItemGridMerchBtnClick()
{
	//UI--------------------------------------------
	TopWindow d;
	Button ok, cancel;

    d.Title(t_("Select one product")).SetRect(0, 0, 400, 400);
	d.Add(ok.SetLabel(t_("OK")).LeftPosZ(40, 64).BottomPosZ(12, 24));
	d.Add(cancel.SetLabel(t_("Cancel")).LeftPosZ(130, 64).BottomPosZ(12, 24));
	ok.Ok() <<= d.Acceptor(IDOK);
	cancel.Cancel() <<= d.Rejector(IDCANCEL);
	
	ColumnList list;
	d.Add(list);
	list.SetRect(0, 0, 400, 325);
	list.Columns(3);
	//end UI--------------------------------------------
try{
	//add costomer to select
	Vector<double> price_list;
	SQL & Select(M_ID, M_NAME, M_MODEL, M_PRICE).From(MERCHANDISE);
	while(SQL.Fetch())
	{
		String item_value=SQL[M_NAME].ToString();
		if(!SQL[M_MODEL].IsNull()) item_value+=" - "+SQL[M_MODEL].ToString();
		list.Add(SQL[M_ID], item_value, true);
		price_list.Add(SQL[M_PRICE]);
	}
	
	if(list.GetCount()<=0)	return;//there is no any customer
	if(Order.BuyItemGrid(M_ID).IsNull())
	{
		
	}
	else
	{
		int list_index=list.Find(Order.BuyItemGrid(M_ID));
		list.SetCursor(list_index);
	}
	
	double price;
	int merch_id;
	String title;
	if(d.Run()==IDOK) {
		for(int i = 0; i < list.GetCount(); i++)
		{
			if(list.IsSel(i))
			{
				merch_id	= list.Get(i);
				title		= list.GetValue(i).ToString();
				price	= price_list[i];
				
				//update in the database
				if(-1 != Order.BuyItemGrid(B_ID))
				{
					SQL & ::Update(BUYITEM) 
							(M_ID,		merch_id)
							(M_NAME,	title)
							//(M_MODEL,	Order.BuyItemGrid(M_MODEL))
							(M_PRICE,	price)
							.Where(B_ID == Order.BuyItemGrid(B_ID));
				}
			}
		}
		Order.BuyItemGrid.Set(M_ID,merch_id);		
		Order.BuyItemGrid.Set(M_NAME,title);	
		Order.BuyItemGrid.Set(M_PRICE,price);	
    }
}
catch(SqlExc &e)
{
	SysLog.Error(e+"\n");
	Exclamation("[* " + DeQtfLf(e) + "]");
}
}


void PikaCRM::ExportFile(GridCtrl * grid, String name)
{
	SysLog.Info("Export File\n");	
	//UI--------------------------------------------
	WithExportLayout<TopWindow> d;
	CtrlLayoutOKCancel(d,t_("Export File"));
	d.swFormat <<= 0;
	d.btnBrowse <<= THISBACK2(SelectExportDir,&(d.esFilePath),name);

	d.swFormat.DisableCase(1);
	d.swFormat.DisableCase(2);
	d.swFormat.DisableCase(3);
	
	//end UI--------------------------------------------

		
	if(d.Run()==IDOK) {
		ExportCSV(grid, ~(d.esFilePath), name, d.opUTF8BOM.Get());
	}
}
void PikaCRM::SelectExportDir(EditString * path, String & name)
{
	static FileSel fileSelEx; //static for remember user selection in one session
	fileSelEx.Type("CSV file (*.csv)", "*.csv");
	fileSelEx.Type("Text file (*.txt)", "*.txt");
	fileSelEx.Set(name+".csv");
	if(fileSelEx.ExecuteSaveAs()){
		*path=~fileSelEx;
	}
}

void PikaCRM::ExportCSV(GridCtrl * grid, const String & path, const String & name, bool is_bom)
{
	SysLog.Info("Exporting CSV File\n");	
	FileOut out(path);//clear file
	if(!out.IsOpen() || out.IsError())
	{ 
		SysLog.Error("Export file: The file can not be opened\n");
		Exclamation("The file can not be opened.");
		return;
	}
	
	if(is_bom)
	{
		unsigned char bom[] = {0xEF, 0xBB, 0xBF};
		out.Put(bom, 3);
	}
	
	int cols=grid->GetColumnCount();
	int rows=grid->GetCount();//not include fix row title
	//outAppend.Put(name+",rows,"+AsString(rows)+",cols,"+AsString(cols)+"\r\n");	
	SysLog.Info(name+",rows,"+AsString(rows)+",cols,"+AsString(cols)+"\n");
	
	//test---------------------------------------------
	///String s="____";
	//String test="\""+s+"\","+"\""+s+"\","+"\""+s+"\","+"\""+s+"\","+"\""+s+"\","+"\""+s+"\"";
	//outAppend.Put(test+"\r\n");
	//outAppend.Put(test+"\r\n");
	//end test------------------------------------------
	
	//row title
	String 	line;
	bool 	isFirstOne=true;
	for(int i=0;i<cols;++i)
	{
		if(!grid->GetColumn(i).IsHidden())
		{
			if(isFirstOne==false) line<<",";
			line<<"\""<<grid->GetColumn(i).GetName()<<"\"";
			isFirstOne=false;
		}		
	}
	out.Put(line+"\r\n");
	line.Clear();
	
	//data start from row=0
	for(int i=0;i<rows;++i){
		isFirstOne=true;
		for(int j=0;j<cols;++j)
		{
			if(!grid->GetColumn(j).IsHidden())
			{
				if(isFirstOne==false) line<<",";
				
				String temp=grid->Get(i,j).ToString();
				String out=Replace(temp,"\"","\"\"");
				if(!out.IsEmpty())line<<"\""<<out<<"\"";
				isFirstOne=false;
			}		
		}
		out.Put(line+"\r\n");
		line.Clear();
	}
	
	out.Close();
}

void PikaCRM::ImportFile(GridCtrl * grid, String name)
{
	SysLog.Info("Import File\n");	
	Vector< Vector<String> > griddata;
	VectorMap<Id, int> match_map;
	//UI--------------------------------------------
	Import.btnBrowse <<= callback3(this, &PikaCRM::SelectImportDir,&(Import.Grid),&griddata,&match_map);//THISBACK2(SelectImportDir,&(d.esFilePath),&(d.Grid));
	Import.btnChangMatch <<= callback3(this, &PikaCRM::ImportChangMatch,&(Import.Grid),&griddata,&match_map);
	Import.dlEncode.WhenAction = callback3(this, &PikaCRM::ImportChangEncode,&(Import.Grid),&griddata,&match_map);
	
	Import.esFilePath.Clear();
	Import.rtWarning.Clear();
	Import.Grid.Reset();
	
	int cols=grid->GetColumnCount();
	for(int i=0;i<cols;++i)
	{
		if(!grid->GetColumn(i).IsHidden())
		{
			if(0==Import.Grid.GetColumnCount()) //every grid import first column is not null;
				Import.Grid.AddColumn(grid->GetColumnId(i), grid->GetColumn(i).GetName()).SetDisplay(Single<GDisplayNullRedBack>());
			else
				Import.Grid.AddColumn(grid->GetColumnId(i), grid->GetColumn(i).GetName());
			match_map.Add(grid->GetColumnId(i), Import.Grid.GetColumnCount()-1);
		}		
	}
	//end UI--------------------------------------------
	
	if(Import.Run()==IDOK) {
		ImportCSV(&(Import.Grid), name);
	}
	
	Import.Close();
}
void PikaCRM::SelectImportDir(GridCtrl * grid, Vector< Vector<String> > * griddata, VectorMap<Id, int> * match_map)
{	
	SysLog.Debug("Selecting Import Dir\n");
	static FileSel fileSelIm; //static for remember user selection in one session
	fileSelIm.Type("CSV file (*.csv)", "*.csv");
	fileSelIm.Type("Text file (*.txt)", "*.txt");
	if(fileSelIm.ExecuteOpen()){
		FileIn csv(~fileSelIm);
		if(csv.IsOpen())
		{
			Import.esFilePath=~fileSelIm;
			Import.rtWarning.Clear();
			StringStream content(LoadStreamBOM(csv));
			ParserCSVFile(content, *griddata);
			SetCsvGridData(grid, griddata, match_map);
		}
		else
		{
			SysLog.Error("Import file: The file can not be opened\n");
			Exclamation("The file can not be opened.");
		}
	}
}
void PikaCRM::ParserCSVFile(Stream & content, Vector< Vector<String> > & data)
{
	SysLog.Debug("Parsering CSV File\n");
	std::string temp;
	std::vector<std::string> std_csv_row;
	
	data.Clear();
	while(!content.IsEof()){
		Vector<String> csv_row;
		
		//(1)easy way, but not resolve "a","b,c","d"
		//vs2=Split(~file.GetLine(), ',', false);
		
		//(2)hard way, use boost::tokenizer
		String temp=content.GetLine();
		temp=EncodeToUtf8(temp);
		//for csv format work-around to fix the default behavior of the boost escaped_list_separator:
		//First replace all back-slash characters (\) with two back-slash characters (\\) so they are not stripped away.
		temp=Replace(temp,"\\","\\\\");
		//Secondly replace all double-quotes ("") with a single back-slash character and a quote (\")
		temp=Replace(temp,"\"\"","\\\"");
		std_csv_row=ParserCsvLine(~temp);
		for(int i=0;i<std_csv_row.size();++i)
		{
			csv_row.Add(std_csv_row[i]);//auto string->String
		}
		
		data.Add(csv_row);
	}
}
void PikaCRM::ImportChangMatch(GridCtrl * grid, Vector< Vector<String> > * griddata, VectorMap<Id, int> * match_map)
{
	SysLog.Info("Changing Import Match\n");	
	//UI--------------------------------------------
	WithImportMatchLayout<TopWindow> d;
	CtrlLayoutOKCancel(d,t_("Change match column"));
	
	Array<DropList> dlCsvId;
	int max_csv_col=0;
	Import.rtWarning.Clear();
	d.GridMatch.Tip("Double click data to change");
	d.GridMatch.SetToolBar();
	//GridCsv-----------------------------------------------
	for(int i=0;i<griddata->GetCount();++i)
	{
		d.GridCsv.Add();
		for(int j=0;j<(*griddata)[i].GetCount();++j)
		{
			if(0==i) d.GridCsv.AddColumn(AsString(j));
			d.GridCsv.Set(i,j,(*griddata)[i][j]);
			if(j>max_csv_col) max_csv_col=j;
		}
	}
	
	//GridMatch-----------------------------------------------	
	int cols=grid->GetColumnCount();
	for(int i=0;i<cols;++i)
	{
		dlCsvId.Add(new DropList());
		for(int j=0;j<max_csv_col+1;++j) dlCsvId.Top().Add(j);//DropList 0~
		d.GridMatch.AddColumn(match_map->GetKey(i), grid->GetColumn(i).GetName()).Edit(dlCsvId.Top());
		if(0==i) d.GridMatch.Add();
		d.GridMatch.Set(0,i,(*match_map)[i]);	
	}
	d.GridMatch.Editing();
	
	//end UI--------------------------------------------
		
	if(d.Run()==IDOK) {
		int cols=d.GridMatch.GetColumnCount();
		for(int i=0;i<cols;++i)
		{
			//save to match map
			(*match_map)[i]=d.GridMatch.Get(match_map->GetKey(i));		
		}
		
		SetCsvGridData(grid, griddata, match_map);
	}
}
void PikaCRM::ImportChangEncode(GridCtrl * grid, Vector< Vector<String> > * griddata, VectorMap<Id, int> * match_map)
{
	if(Import.esFilePath.GetData().IsNull()) return;
	FileIn csv( ~(Import.esFilePath.GetData().ToString()) );
		if(csv.IsOpen())
		{
			Import.rtWarning.Clear();
			StringStream content(LoadStreamBOM(csv));
			ParserCSVFile(content, *griddata);
			SetCsvGridData(grid, griddata, match_map);
		}
		else
		{
			SysLog.Error("Import file: The file can not be opened\n");
			Exclamation("The file can not be opened.");
		}
}
void PikaCRM::SetCsvGridData(GridCtrl * grid, Vector< Vector<String> > * griddata, VectorMap<Id, int> * match_map)
{
		grid->Clear();
		for(int i=0;i<griddata->GetCount();++i)
		{
			grid->Add();
			for(int j=0;( j<grid->GetColumnCount() ) && ( j<(*griddata)[i].GetCount() );++j)
			{
				grid->Set(i,j,(*griddata)[i][(*match_map)[j]]);
					if(0==j) //every grid import first column is not null;
					{
						if( IsNull( (*griddata)[i][(*match_map)[j]] ) )
						{
							String note;
							note<<"[+75G@3 "<<t_("There is a wrong data with red color and the data of the row can't be imported.")<<" ]";	
							Import.rtWarning.SetQTF(note);
							SysLog.Warning("The data of the row can't be imported. row:"+AsString(i)+"\n");
						}
					}
			}
		}		
}
String PikaCRM::EncodeToUtf8(String & src)
{
	String code=Import.dlEncode.GetData();	
	if("Utf8"==code) return src;
	
	return DBCSToUtf8(src, code);	
}

void PikaCRM::ImportCSV(GridCtrl * datagrid, const String & name)
{
	SysLog.Info("Importing CSV File\n");
	if("Customers"==name)
	{
		int c0=datagrid->FindCol(C_0);
		int c1=datagrid->FindCol(C_1);
		int c2=datagrid->FindCol(C_2);
		int c3=datagrid->FindCol(C_3);
		int c4=datagrid->FindCol(C_4);
		int c5=datagrid->FindCol(C_5);
		int c6=datagrid->FindCol(C_6);
		int c7=datagrid->FindCol(C_7);
		int c8=datagrid->FindCol(C_8);
		int c9=datagrid->FindCol(C_9);
		for(int i=0;i<datagrid->GetRowCount();++i)
		{
			if( IsNull(datagrid->Get(i,C_TITLE)) ) continue;
			Customer.Grid.Add();
			//int x=Customer.Grid(C_ID);
			Customer.Grid(C_TITLE) = datagrid->Get(i,C_TITLE);
			Customer.Grid(C_PHONE) = datagrid->Get(i,C_PHONE);
			Customer.Grid(C_ADDRESS) = datagrid->Get(i,C_ADDRESS);
			Customer.Grid(C_EMAIL) = datagrid->Get(i,C_EMAIL);
			Customer.Grid(C_WEBSITE) = datagrid->Get(i,C_WEBSITE);
			VectorMap<int, String> temp_contact_map;
			if(!datagrid->Get(i,CO_NAME).IsNull())
			{
				Contact.Grid.Add();
				Contact.Grid(CO_NAME)=datagrid->Get(i,CO_NAME);
				Contact.Grid(C_ID)=-1;
				InsertContact();
				temp_contact_map.Add(Contact.Grid(CO_ID), Contact.Grid(CO_NAME));
			}
			const Value & raw_map = RawToValue(temp_contact_map);
			Customer.Grid(CONTACTS_MAP) = raw_map;//this is must, "=" will set the same typeid for Value of GridCtrl with RawDeepToValue
			Customer.Grid(CO_NAME) = ConvContactNames().Format(Customer.Grid(CONTACTS_MAP));
			
			if(-1!=c0) Customer.Grid(C_0) = datagrid->Get(i,C_0);
			if(-1!=c1) Customer.Grid(C_1) = datagrid->Get(i,C_1);
			if(-1!=c2) Customer.Grid(C_2) = datagrid->Get(i,C_2);
			if(-1!=c3) Customer.Grid(C_3) = datagrid->Get(i,C_3);
			if(-1!=c4) Customer.Grid(C_4) = datagrid->Get(i,C_4);
			if(-1!=c5) Customer.Grid(C_5) = datagrid->Get(i,C_5);
			if(-1!=c6) Customer.Grid(C_6) = datagrid->Get(i,C_6);
			if(-1!=c7) Customer.Grid(C_7) = datagrid->Get(i,C_7);
			if(-1!=c8) Customer.Grid(C_8) = datagrid->Get(i,C_8);
			if(-1!=c9) Customer.Grid(C_9) = datagrid->Get(i,C_9);
			InsertCustomer();
		}
	}
	else if("Contacts"==name)
	{
		int co0=datagrid->FindCol(CO_0);
		int co1=datagrid->FindCol(CO_1);
		int co2=datagrid->FindCol(CO_2);
		int co3=datagrid->FindCol(CO_3);
		int co4=datagrid->FindCol(CO_4);
		int co5=datagrid->FindCol(CO_5);
		int co6=datagrid->FindCol(CO_6);
		int co7=datagrid->FindCol(CO_7);
		int co8=datagrid->FindCol(CO_8);
		int co9=datagrid->FindCol(CO_9);
		for(int i=0;i<datagrid->GetRowCount();++i)
		{
			if( IsNull(datagrid->Get(i,CO_NAME)) ) continue;
			Contact.Grid.Add();
			//int x=Customer.Grid(C_ID);
			Contact.Grid(CO_NAME) = datagrid->Get(i,CO_NAME);
			Contact.Grid(CO_PHONE) = datagrid->Get(i,CO_PHONE);
			Contact.Grid(CO_ADDRESS) = datagrid->Get(i,CO_ADDRESS);
			Contact.Grid(CO_EMAIL) = datagrid->Get(i,CO_EMAIL);
			if(-1!=co0) Contact.Grid(CO_0) = datagrid->Get(i,CO_0);
			if(-1!=co1) Contact.Grid(CO_1) = datagrid->Get(i,CO_1);
			if(-1!=co2) Contact.Grid(CO_2) = datagrid->Get(i,CO_2);
			if(-1!=co3) Contact.Grid(CO_3) = datagrid->Get(i,CO_3);
			if(-1!=co4) Contact.Grid(CO_4) = datagrid->Get(i,CO_4);
			if(-1!=co5) Contact.Grid(CO_5) = datagrid->Get(i,CO_5);
			if(-1!=co6) Contact.Grid(CO_6) = datagrid->Get(i,CO_6);
			if(-1!=co7) Contact.Grid(CO_7) = datagrid->Get(i,CO_7);
			if(-1!=co8) Contact.Grid(CO_8) = datagrid->Get(i,CO_8);
			if(-1!=co9) Contact.Grid(CO_9) = datagrid->Get(i,CO_9);
			
			if(datagrid->Get(i,C_TITLE).IsNull())
			{
				InsertContact();
			}
			else
			{
				//find database if there is the same C_TITLE, no need to add
				SQL * Select(C_ID).From(CUSTOMER).Where(C_TITLE == datagrid->Get(i,C_TITLE)); 
				if(SQL.Fetch())
				{
					Contact.Grid(C_ID)=SQL[C_ID];
					Contact.Grid(C_TITLE)=datagrid->Get(i,C_TITLE);
					InsertContact();
					
					//UpdateCustomerContactX(Contact.Grid(C_ID));----------------------------
					int customer_id=Contact.Grid(C_ID);
					int customer_row=Customer.Grid.Find(customer_id,C_ID);
					if(-1==customer_row) continue;

					const VectorMap<int, String> & contact_map = ValueTo< VectorMap<int, String> >(Customer.Grid.Get(customer_row, CONTACTS_MAP));
					VectorMap<int, String> new_contact_map = contact_map;
					new_contact_map.Add(Contact.Grid(CO_ID), Contact.Grid(CO_NAME));
					Customer.Grid.Set(customer_row,CONTACTS_MAP,RawToValue(new_contact_map));
			
					String all_name;
					all_name = ConvContactNames().Format(Customer.Grid.Get(customer_row, CONTACTS_MAP));
					Customer.Grid.Set(customer_row,CO_NAME,all_name);
				}
				else
				{
					InsertContact();
					
					//add new cusotmer--------------------------------------------
					VectorMap<int, String> temp_contact_map;
					
					Customer.Grid.Add();
					Customer.Grid(C_TITLE)=datagrid->Get(i,C_TITLE);
					
					temp_contact_map.Add(Contact.Grid(CO_ID), Contact.Grid(CO_NAME));
					const Value & raw_map = RawToValue(temp_contact_map);
					Customer.Grid(CONTACTS_MAP) = raw_map;//this is must, "=" will set the same typeid for Value of GridCtrl with RawDeepToValue
					Customer.Grid(CO_NAME) = ConvContactNames().Format(Customer.Grid(CONTACTS_MAP));
					
					InsertCustomer();
				}
			}
		}		
	}
	else if("Merchandises"==name)
	{
		int m0=datagrid->FindCol(M_0);
		int m1=datagrid->FindCol(M_1);
		int m2=datagrid->FindCol(M_2);
		int m3=datagrid->FindCol(M_3);
		int m4=datagrid->FindCol(M_4);
		int m5=datagrid->FindCol(M_5);
		int m6=datagrid->FindCol(M_6);
		int m7=datagrid->FindCol(M_7);
		int m8=datagrid->FindCol(M_8);
		int m9=datagrid->FindCol(M_9);
		for(int i=0;i<datagrid->GetRowCount();++i)
		{
			if( IsNull(datagrid->Get(i,M_NAME)) ) continue;
			Merchandise.Grid.Add();
			Merchandise.Grid(M_NAME) = datagrid->Get(i,M_NAME);
			Merchandise.Grid(M_MODEL) = datagrid->Get(i,M_MODEL);
			Merchandise.Grid(M_PRICE) = StrDbl(datagrid->Get(i,M_PRICE).ToString());
			
			if(-1!=m0) Merchandise.Grid(M_0) = datagrid->Get(i,M_0);
			if(-1!=m1) Merchandise.Grid(M_1) = datagrid->Get(i,M_1);
			if(-1!=m2) Merchandise.Grid(M_2) = datagrid->Get(i,M_2);
			if(-1!=m3) Merchandise.Grid(M_3) = datagrid->Get(i,M_3);
			if(-1!=m4) Merchandise.Grid(M_4) = datagrid->Get(i,M_4);
			if(-1!=m5) Merchandise.Grid(M_5) = datagrid->Get(i,M_5);
			if(-1!=m6) Merchandise.Grid(M_6) = datagrid->Get(i,M_6);
			if(-1!=m7) Merchandise.Grid(M_7) = datagrid->Get(i,M_7);
			if(-1!=m8) Merchandise.Grid(M_8) = datagrid->Get(i,M_8);
			if(-1!=m9) Merchandise.Grid(M_9) = datagrid->Get(i,M_9);
			InsertMerchandise();
		}
	}
	else
	{
		;//do nothing
	}
	
}


void PikaCRM::Print(GridCtrl * grid, String name)
{
	Report r;
	r.Header("[A2> Page $$P");
	r << "[* PikaCRM";
	
	int cols=grid->GetColumnCount();
	int rows=grid->GetCount();	
	String line = name+" data: count "+AsString(rows);
	SysLog.Info(line+"\n");
	r << line;	
	
	String table;
	table << "{{";	


	String 	row_title;
	bool 	isFirstOne=true;
	for(int i=0;i<cols;++i)
	{
		if(!grid->GetColumn(i).IsHidden())
		{
			if(isFirstOne==false)
			{
				table<<":";
				row_title<<":: ";
			}
			table<<"1";
			row_title<<grid->GetColumn(i).GetName();
			isFirstOne=false;
		}		
	}
	table<<" "<<row_title;
	
	//data start from row=0
	for(int i=0;i<rows;++i){
		for(int j=0;j<cols;++j)
		{
			if(!grid->GetColumn(j).IsHidden())
			{
				String temp=grid->Get(i,j).ToString();
				temp=Replace(temp,"\n","&");
				table << ":: " << temp;
			}		
		}
	}	
	
	r<<table;

	Perform(r);	
}

void PikaCRM::ConfigDB()
{
	SysLog.Info("configure the database\n");
	if(mConfig.Password.IsEqual(PW_EMPTY))
	{
		;
	}
	else //if no pw (PW_EMPTY) not need
	{
		if(!IsInputPWCheck()) return;
	}
	
	String config_file_path = getConfigDirPath()+FILE_CONFIG;	
	String database_file_path = getConfigDirPath()+FILE_DATABASE;
	CreateOrOpenDB(database_file_path);//must resetkey before any operation after open db, so we re-open
	
	if(IsSetupDB(config_file_path))
	{
			SysLog.Info("reset database encrypted key.\n");
			String pwkey;
			if(mConfig.Password.IsEqual(PW_EMPTY))
				pwkey="";
			else
				pwkey=mConfig.Password;
				
			if(!mSqlite3Session.ResetKey(getSwap1st2ndChar(pwkey)))
			{
				SysLog.Error("sqlite3 reset key error\n");
				///@note we dont know how to deal this error, undefine		
			}
	}
	
}

void PikaCRM::SavePreference()
{
	SysLog.Info("save the preference\n");
	int index=Preference.dlLang.GetIndex();
	//test-------------------------------------------------------
	int tt0=LNG_('Z','H','T','W');//860823
	int tt1=Preference.dlLang.GetKey(index);//860823
	
	int tt2=GetSystemLNG();//268247703
	int tt3=SetLNGCharset( GetSystemLNG(), CHARSET_UTF8 );//268247703
	int tt4=GetSystemLNG()& 0xfffff;//860823
	String langStr1 = LNGAsText(tt1);//ZH-TW
	String langStr2 = LNGAsText(tt2);//ZH-TW UTF-8 //these two are the same in linux
	String langStr3 = LNGAsText(tt3);//ZH-TW UTF-8 //but different in Windows
	//end test---------------------------------------------------

	if(-1!=index)
	{
		mConfig.Language=Preference.dlLang.GetKey(index);
	}

	String config_file_path = getConfigDirPath()+FILE_CONFIG;	
	SaveConfig(config_file_path);
	PromptOK(t_("The setting has been saved and will take effect the next time you start this application."));
}

void PikaCRM::ShowLicense()
{
	//UI--------------------------------------------
	TopWindow d;
	Button ok;
	RichTextView license;
	
	d.Title(t_("License")).SetRect( 0, 0, Ctrl::HorzLayoutZoom(600), Ctrl::VertLayoutZoom(550));
	d.Add(ok.SetLabel(t_("OK")).LeftPosZ(550, 45).TopPosZ(520, 24));///@todo fix layout
	ok.Ok() <<= d.Acceptor(IDOK);

	Topic t = GetTopic("PikaCRM/srcdoc/License$"+ ToLower(LNGAsText(mConfig.Language & 0xfffff)));
	if (t.text.IsEmpty()) {
		t = GetTopic("PikaCRM/srcdoc/License$en-us");
	}	
	license.SetQTF(t);

	d.Add(license.LeftPosZ(5, 590).TopPosZ(5, 510));
	//end UI--------------------------------------------
	if(d.Run()==IDOK) {
		;
	}	
}
//end interactive with GUI==========================================================
//private utility-------------------------------------------------------------------
String PikaCRM::getConfigDirPath()
{
	String directory_path(APP_CONFIG_DIR);
#ifdef PLATFORM_POSIX
	String full_directory_path = GetHomeDirFile(directory_path+"/");
#endif

#ifdef PLATFORM_WIN32
	//String full_directory_path = GetHomeDirFile("Application Data/"+directory_path+"/");
	String full_directory_path = "Data/"; //portable path
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
			//RLOG("can't create the application config directory!");//in ~/.upp/PikaCRM/PikaCRM.log
			String msg;
			msg =t_("Can't create the application config directory!\n"
					"Please check you have the privilege to create:\n")
					+ full_directory_path;
			throw ApExc(msg).SetHandle(ApExc::SYS_FAIL);
		}
	}

	return full_directory_path;
}
String PikaCRM::getLang4Char(int language)
{
	String lang4=LNGAsText(language & 0xfffff);//EN-US UTF8 & 0xfffff = EN-US
	if(!lang4.IsEmpty()) lang4.Remove(2,1); //remove "-"
	return lang4;
}
Image PikaCRM::getLangLogo(int language)
{
	String logo_name;
	logo_name="Logo"+getLang4Char(language);//LogoENUS, LogoZHTW, ...
	int id=SrcImages::Find(logo_name);
							
	Image image;							
	if(id>=0)
		image=SrcImages::Get(id);//SrcImages::LogoENUS()
	else
		image=SrcImages::Logo();
							
	return image;
}
String PikaCRM::getMD5(const String & text)
{
	unsigned char digest[16];
	MD5(digest, ~text, text.GetLength());
	String md5ed(digest, 16);
		
	String r;
	for(int i = 0; i < md5ed.GetCount(); i++)
		r << UPP::FormatIntHex((byte)md5ed[i], 2);
		
	return r;
}
String PikaCRM::getSwap1st2ndChar(const String & text)
{
	if(text.IsEmpty()) return "";
	
	String r(text);
	//String r2=text;
	r.Remove(0);
	r.Insert(1,text[0]);
	
	return r;
}
	    
//end private utility---------------------------------------------------------------

//some function-----------------------------------------------
String Replace(String str, String find, String replace) 
{ //from Functions4U.cpp
	String ret;
	
	int lenStr = str.GetCount();
	int lenFind = find.GetCount();
	int i = 0, j;
	while ((j = str.Find(find, i)) >= i) {
		ret += str.Mid(i, j-i) + replace;
		i = j + lenFind;
		if (i >= lenStr)
			break;
	}
	ret += str.Mid(i);
	return ret;
}
std::vector<std::string> ParserCsvLine(const char * line)
{
	typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;
	std::vector< std::string > vec;
	std::string sline(line);

	Tokenizer tok(sline);
	vec.assign(tok.begin(),tok.end());
	
	return vec;
}

Font StdFontS(int scale)
{
	return Font(Font::STDFONT, GetStdFont().GetHeight()+scale*GetStdFont().GetHeight()/10); 
}

//end some function-----------------------------------------------