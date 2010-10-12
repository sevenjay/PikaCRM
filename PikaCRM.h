#ifndef _PikaCRM_PikaCRM_h
#define _PikaCRM_PikaCRM_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

//useful library------------------------------------------------------
#include <SystemLog/SystemLog.h>
#include <SplashSV/splash-sv.h>						//this is   for Splash
#include <plugin/sqlite3/Sqlite3.h>
#include <DropGrid/DropGrid.h>
#include <GridCtrl/GridCtrl.h> //must include before *.lay

#include <string>
#include <vector>
//#include "boost/smart_ptr.hpp"
//#include "boost/tokenizer.hpp"

//end useful library---------------------------------------------------

#define LAYOUTFILE <PikaCRM/PikaCRM.lay>
#include <CtrlCore/lay.h>

//platform dependent---------------------------------------------------
#ifdef PLATFORM_POSIX

//#include <sys/ioctl.h>//for open
//#include <sys/fcntl.h>
//#include <linux/hdreg.h>

#elif defined(PLATFORM_WIN32)

#endif
//end platform dependent------------------------------------------------


#define SOFTWARE_NAME					"PikaCRM"
#define SOFTWARE_VERSION				"0.8"
#define DATABASE_VERSION				"1"
#define BUILD_DATE						Date(2010, 9, 1)

#define PW_MAGIC_WORD					"sevenjay777"
#define PW_EMPTY						getMD5(PW_MAGIC_WORD) //avoid clear config file pw hack
//we let PW SETKEY ROLL by getSwap1st2ndChar

//define file path and name------------------------------------------------------
#define APP_CONFIG_DIR					".PikaCRM"//need mkdir ".MobileConnect/"

#define FILE_CONFIG						"PikaCRM.xml"//in PATH_USER_HOME
#define FILE_LOG						"PikaCRM.log"//in PATH_USER_HOME
#define FILE_DATABASE					"PikaCRM.sqlite"//in PATH_USER_HOME
//end define file path and name--------------------------------------------------
class ApExc : public String {
	byte mHandle;
	
public:
	enum Handle
	{
		NONE	= 0x00,
		RELEASE	= 0x01, //release some resource
		NOTICE	= 0x02, //notice user
		RECOVER	= 0x04, //recover from error
		REPORT	= 0x08, //need user to report
		EXIT	= 0x10, //exit ap
		//LNOTICE	= 0x20,
		//LINFO		= 0x40,
		//LDEBUG	= 0x80,
		//------------------
		SYS_FAIL= 0x12, //NOTICE, EXIT		
		AP_FAIL	= 0x0a, //NOTICE, REPORT
		FATAL	= 0x1a  //NOTICE, REPORT, EXIT
	};
	
	//Exc(); // throw exception according to GetLastError()
	ApExc() : String(GetLastErrorMessage()), mHandle(ApExc::NOTICE) {}
	ApExc(const String& desc) : String(desc), mHandle(ApExc::NOTICE) {}
	ApExc(const char * desc) : String(desc), mHandle(ApExc::NOTICE) {}
	
	ApExc & SetHandle(Handle h) {mHandle=h; return *this;}
	ApExc & AddHandle(Handle h) {mHandle&=h; return *this;}
	ApExc & RemoveHandle(Handle h) {mHandle&=~h; return *this;}
	byte GetHandle() {return mHandle;}
};

struct Config {
	int		Language;
	bool	IsDBEncrypt;
	String	Password;
	bool	IsRememberPW;
	String	SystemPWKey;
	
	VectorMap<String, int> CWidth;
	VectorMap<String, int> COWidth;
	VectorMap<String, int> MWidth;
	
	bool	IsMaximized;

	void Xmlize(XmlIO xml){	//necessary for StoreAsXMLFile(), LoadFromXMLFile()
		xml
			("Language", Language)
			("Encrypted", IsDBEncrypt)
			("Password", Password)
			("RememberPW", IsRememberPW)
			("SystemPWKey", SystemPWKey)
			("CustomerWidth", CWidth)
			("ContactWidth", COWidth)
			("MerchandiseWidth", MWidth)
			("IsMaximized", IsMaximized)
		;
	}
	
	bool Load(String const &fileName){
		return LoadFromXMLFile(*this, fileName);
	}

	bool Save(String const &fileName){
		return StoreAsXMLFile(*this, "Config", fileName);
	}

};
class ColumnListAlwaysCTRL : public ColumnList {
public:
	virtual void  LeftDown(Point p, dword keyflags) {
		keyflags = keyflags|K_CTRL;
		ColumnList::LeftDown(p, keyflags);
	};
	virtual void  LeftUp(Point p, dword keyflags) {
		keyflags = keyflags|K_CTRL;
		ColumnList::LeftUp(p, keyflags);
	};
	
	void GotFocus()	{
	//if(cursor < 0 && GetCount())
		//SetCursor(GetSbPos());
		Refresh();
		//SyncInfo();
	}
};
class MultiButtonNotNULL : public MultiButton {
	virtual Value GetData() const{
		if(IsNull(Get()))
			return ErrorValue(t_("Please select one customer."));///@todo pass msg for buyitem
		else
			return MultiButton::GetData();
	};
};

class FieldId : Moveable<FieldId> {
public:
	SqlId Id;	
	bool IsUsed;
	
	FieldId() : IsUsed(false)	{};
	void SetId(SqlId id){ Id=id;}
};

class PikaCRM
{
	typedef PikaCRM CLASSNAME;
	
private :
	//UI--------------------------------------------------------------------------
	WithPikaCRMLayout<TopWindow> MainFrom;
	WithCustomerLayout<ParentCtrl> Customer;
	WithContactLayout<ParentCtrl> Contact;
	WithEventLayout<ParentCtrl> Event;
	WithOrderLayout<ParentCtrl> Order;
	WithMerchandiseLayout<ParentCtrl> Merchandise;
	WithPreferenceLayout<ParentCtrl> Preference;

	//Customer.Grid
	EditStringNotNull cesn;
	EditString ces1,ces2,ces3,ces4,ces5;
	MultiButton mCustomerGridContactBtn;
	Id CONTACTS_MAP;
	ToolBar customer_search_bar;

	//Contact.Grid
	EditStringNotNull coesn;
	EditString coes1,coes2,coes3;
	ToolBar contact_search_bar;
	
	//Event.Grid
	EditStringNotNull eesn;
	EditString ees1,ees2,ees3,ees4;
	MultiButtonNotNULL mEventGridCustomerBtn;
	ToolBar event_search_bar;
	DropTime edt;
	DropGrid mEventDropStatus;
	
	//Merchandise.Grid
	EditStringNotNull mesn;
	EditString mes1,mes2,mes3,mes4;
	EditDouble med;
	ToolBar merchandise_search_bar;
	
	//Order.Grid
	EditString oes1,oes2,oes3,oes4;
	DropDate odd1,odd2;
	MultiButtonNotNULL mOrderGridCustomerBtn;
	ToolBar order_search_bar;
	
	//Order.BuyItemGrid
	EditDoubleNotNull	bed;
	EditIntSpin beis;
	MultiButtonNotNULL mBuyItemGridMerchBtn;
	
	
	//must initial in PikaCRM(), Initial(), OpenMainFrom()	------------------------------------
	SplashSV mSplash;
	
	Config mConfig;
	
	Sqlite3Session mSqlite3Session;
	
	//private data-------------------------------------------------------------------
	VectorMap< String, Vector<FieldId> > mFieldMap;
	Array<EditString> mFieldEditList;
	String mRevealedPW;

	//private utility-------------------------------------------------------------------
	String  getConfigDirPath();
	String	getLang4Char(int language);
	Image	getLangLogo(int language);
	String	getMD5(const String & text);
	String	getSwap1st2ndChar(const String & text);
	void	test(){PromptOK("test");};
	
	//database control------------------------------------------------------------
	void LoadSetAllField();
	void CreateField(GridCtrl * grid, String f_table);
	void ModifyField(GridCtrl * grid, String f_table);
	void RemoveField();
	
	void LoadCustomer();
	void NewCustomer();
	void InsertCustomer();
	void DuplicateCustomer();
	void UpdateCustomer();
	void RemoveCustomer();

	void LoadContact();
	void InsertContact();
	void UpdateContact();
	void RemoveContact();
	void Update_dg_contact();
	
	void LoadEvent();
	void InsertEvent();
	void UpdateEvent();
	void RemoveEvent();
	void UpdateEventDropStatus();
		
	void LoadMerchandise();
	void InsertMerchandise();
	void UpdateMerchandise();
	void RemoveMerchandise();
			
	void LoadOrder();
	void LoadOrderCustomer();
	void InsertOrder();
	void UpdateOrder();
	void RemoveOrder();
	void ChangeOrder();
	void RemoveOrderBuyItem();
				
	void LoadBuyItem(int o_id);
	void NewBuyItem();
	void InsertBuyItem();
	void UpdateBuyItem();
	void RemoveBuyItem();
	
	//application control--------------------------------------------------------------
	void CloseMainFrom();
	
	//used in initial====================================================
	void SetupUI();
	bool IsHaveDBFile(const String & database_file_path);
	void CreateOrOpenDB(const String & database_file_path);
	int  GetDBVersion();
	void InitialDB();
	bool IsSetupDB(const String config_file_path);
		void OnOptPWAction(WithInitialDBLayout<TopWindow> * d);
		void OnOptRevealPWAction(WithInitialDBLayout<TopWindow> * d);
		void CheckPWSame(WithInitialDBLayout<TopWindow> * d);
	bool IsDBWork();
	
	bool IsInputPWCheck();
		void CheckPWRight(WithInputPWLayout<TopWindow> * d, const String & pw);
	String GetSystemKey();
	String CombineKey(const String & key1, const String & key2);
	
	void LoadConfig(const String & config_file_path);
	void SetConfig();
	void SaveConfig(const String & config_file_path);
	
	void SetAllFieldMap();
	//end used in initial================================================
	
public:
	PikaCRM();
	~PikaCRM();
	
	//main control--------------------------------------------------------------
	String GetLogPath();
	void   Initial();
	void   OpenMainFrom();
	
	//interactive with GUI==============================================================
	void	CustomerGridContactBtnClick();
	
	void	BtnSearchClearClick(){ Customer.Grid.ClearFound(); };
	void	BtnSearchGoClick(){ Customer.Grid.DoFind(); };
			
	void	EventGridCustomerBtnClick();
	void	EventNewStatusClick();
		
	void	OrderGridCustomerBtnClick();
	
	void	BuyItemGridMerchBtnClick();
	
	void	ExportFile(GridCtrl * grid, String name);
			void	SelectExportDir(EditString * path, String & name);	
	void	ImportFile(GridCtrl * grid, String name);
			void	SelectImportDir(EditString * path, GridCtrl * grid, Vector< Vector<String> > * griddata);
			void	ImportChangMatch(GridCtrl * grid, Vector< Vector<String> > * griddata,  VectorMap<Id, int> * match_map);

	//File Operation------------------------------------------------------
	void	ExportCSV(GridCtrl * grid, const String & path, const String & name);
	void 	ParserCSVFile(FileIn & file, Vector< Vector<String> > & data);
	void 	ImportCSV(GridCtrl * datagrid, const String & name);
	
	//Preference Tab------------------------------------------------------
	void	ConfigDB();
	void	SavePreference();
};

//some function-----------------------------------------------
String Replace(String str, String find, String replace);
std::vector<std::string> ParserCsvLine(const char * line);
#endif