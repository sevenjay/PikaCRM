#ifndef _PikaCRM_PikaCRM_h
#define _PikaCRM_PikaCRM_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

//platform dependent---------------------------------------------------
#ifdef PLATFORM_POSIX

//#include <sys/ioctl.h>//for open
//#include <sys/fcntl.h>
//#include <linux/hdreg.h>

#elif defined(PLATFORM_WIN32)
/*
Windows 7						_WIN32_WINNT_WIN7 (0x0601)
Windows Server 2008				_WIN32_WINNT_WS08 (0x0600)
Windows Vista					_WIN32_WINNT_VISTA (0x0600)
Windows Server 2003 with SP1,
		Windows XP with SP2		_WIN32_WINNT_WS03 (0x0502)
Windows Server 2003, Windows XP	_WIN32_WINNT_WINXP (0x0501)
Windows 2000					_WIN32_WINNT_WIN2K (0x0500)
*/
//#define WINVER 0x0501 //<--this must define before include <windef.h> in Mingw
						//but it is included by <CtrlLib/CtrlLib.h>
#endif
//end platform dependent------------------------------------------------

//useful library------------------------------------------------------
#include <SystemLog/SystemLog.h>
#include <SplashSV/splash-sv.h>						//this is   for Splash
#include <plugin/sqlite3/Sqlite3.h>
#include <DropGrid/DropGrid.h>
#include <GridCtrl/GridCtrl.h> //must include before *.lay
#include <Report/Report.h>

#include <string>
#include <vector>
//#include "boost/smart_ptr.hpp"
//#include "boost/tokenizer.hpp"

#include "PikaCtrl.h" //must include before *.lay
//end useful library---------------------------------------------------

#define LAYOUTFILE <PikaCRM/PikaCRM.lay>
#include <CtrlCore/lay.h>


#define SOFTWARE_NAME					"PikaCRM"
#define SOFTWARE_VERSION				"1.0"
#define DATABASE_VERSION				"1"
#define BUILD_DATE						Date(2010, 11, 29)

#define PW_MAGIC_WORD					"sevenjay777"
#define PW_EMPTY						getMD5(PW_MAGIC_WORD) //avoid clear config file pw hack
//we let PW SETKEY ROLL by getSwap1st2ndChar

//define file path and name------------------------------------------------------
#define APP_CONFIG_DIR					".PikaCRM"//need mkdir ".MobileConnect/"

#define FILE_CONFIG						"PikaCRM.xml"//in PATH_USER_HOME
#define FILE_LOG						"PikaCRM.log"//in PATH_USER_HOME
#define FILE_DATABASE					"PikaCRM.sqlite"//in PATH_USER_HOME

#define WEB_UPDATE						"http://pika.sevenjay.tw/update"
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
	int		OrderFilter;

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
			("OrderFilter", OrderFilter)
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
			return ErrorValue(t_("Please select one of the list by pressing the button in the red column"));
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
	WithHelpLayout<ParentCtrl> Help;
	
	WithImportLayout<TopWindow> Import;

	WithContactInfoLayout<HidePanel> ContactInfo; 
	//Customer.Grid
	EditStringNotNull cesn;
	EditString ces1,ces2,ces3,ces4,ces5;
	MultiButton mCustomerGridContactBtn;
	Id CONTACTS_MAP;
	ToolBar customer_search_bar;

	//Contact.Grid
	EditStringNotNull coesn;
	EditString coes1,coes2,coes3;
	MultiButton mContactGridCustomerBtn;
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
	Config mConfig;
	
	Sqlite3Session mSqlite3Session;
	
	//RichTextCtrl * mImporWarning;
	
	//private data-------------------------------------------------------------------
	VectorMap< String, Vector<FieldId> > mFieldMap;
	Array<EditString> mFieldEditList;
	String mRevealedPW;
	bool mIsRegister;

	//private utility-------------------------------------------------------------------
	String  getConfigDirPath();
	String	getLang4Char(int language);
	Image	getLangLogo(int language);
	String	getMD5(const String & text);
	String	getSwap1st2ndChar(const String & text);
	
	Image	fitScale(const Image & i, int new_len){
		
		if(i.GetWidth()<=new_len || new_len<5) return i;
		
		ImageDraw iw(new_len, new_len);
		iw.Alpha().DrawRect(0, 0, new_len, new_len, GrayColor(0));
		iw.Alpha().DrawImage(0,0,new_len,new_len,i, GrayColor(255));		
		iw.DrawImage(0,0,new_len,new_len,i);
		return iw;
	};
	
	void	test(){PromptOK("test");};
	
	//database control------------------------------------------------------------
	bool OpenDB(Sqlite3Session & sqlsession, const String & database_file_path, const String & password, bool log=false);
	void ResetDBKey(Sqlite3Session & sqlsession, const String & password){
			SysLog.Info("reset database encrypted key.\n");
			String pwkey;
			if(password.IsEqual(PW_EMPTY))
				pwkey="";
			else
				pwkey=password;
				
			if(!sqlsession.ResetKey(getSwap1st2ndChar(pwkey)))
			{
				SysLog.Error("sqlite3 reset key error\n");
				///@note we dont know how to deal this error, undefine		
			}	
	}
	void LoadAllData();
	
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
	void StartEditCustomer() {Customer.btnDelete.Hide(); Customer.btnCancel.Show();}
	void EndEditCustomer() {Customer.btnDelete.Show(); Customer.btnCancel.Hide();}

	void LoadContact();
	void InsertContact();
	void UpdateContact();
	void RemoveContact();
	void StartEditContact() {Contact.btnDelete.Hide(); Contact.btnCancel.Show();}
	void EndEditContact() {Contact.btnDelete.Show(); Contact.btnCancel.Hide();}
	
	void LoadEvent();
	void InsertEvent();
	void UpdateEvent();
	void RemoveEvent();
	void StartEditEvent() {Event.btnDelete.Hide(); Event.btnCancel.Show();}
	void EndEditEvent() {Event.btnDelete.Show(); Event.btnCancel.Hide();}
	void UpdateEventDropStatus();
		
	void LoadMerchandise();
	void InsertMerchandise();
	void UpdateMerchandise();
	void RemoveMerchandise();
	void StartEditMerchandise() {Merchandise.btnDelete.Hide(); Merchandise.btnCancel.Show();}
	void EndEditMerchandise() {Merchandise.btnDelete.Show(); Merchandise.btnCancel.Hide();}
			
	void LoadOrder();
	void LoadOrderCustomer();
	void InsertOrder();
	void UpdateOrder();
	void RemoveOrder();
	void StartEditOrder() {Order.btnDelete.Hide(); Order.btnCancel.Show();}
	void EndEditOrder() {Order.btnDelete.Show(); Order.btnCancel.Hide();}
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
	void CreateMainDB(const String & database_file_path);
	void OpenMainDB(const String & database_file_path);
	int  GetDBVersion();
	void InitialDB();
	bool IsSetupDB(const String config_file_path);
		void OnOptPWAction(WithInitialDBLayout<TopWindow> * d);
		void OnOptRevealPWAction(WithInitialDBLayout<TopWindow> * d);
		void CheckPWSame(WithInitialDBLayout<TopWindow> * d);
	bool IsDBWork(Sqlite3Session & sqlsession){
		Sql sql(sqlsession);
		return sql.Execute("PRAGMA database_list;");
	}
	
	bool IsInputPWCheck();
		void CheckPWRight(WithInputPWLayout<TopWindow> * d, const String & pw);
	String GetSystemKey();
	String CombineKey(const String & key1, const String & key2);
	
	void LoadConfig(const String & config_file_path);
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
	void	FirstWelcome();
			void CheckAgree(Option * agree);
	void	CustomerGridContactBtnClick();
	
	void	BtnSearchClearClick(){ Customer.Grid.ClearFound(); };
	void	BtnSearchGoClick(){ Customer.Grid.DoFind(); };

	void	ContactGridCustomerBtnClick();
			bool IsSelectCustomerDialog(int original_id, int & costomer_id, String & title);
			
	void	EventGridCustomerBtnClick();
	void	EventNewStatusClick();
		
	void	OrderGridCustomerBtnClick();
	void	OrderFilterSet();
	
	void	BuyItemGridMerchBtnClick();
	
	void	ExportFile(GridCtrl * grid, String name);
			void	SelectExportDir(EditString * path, String & name);	
	void	ImportFile(GridCtrl * grid, String name);
			void	SelectImportDir(GridCtrl * grid, Vector< Vector<String> > * griddata, VectorMap<Id, int> * match_map);
			void	ImportChangMatch(GridCtrl * grid, Vector< Vector<String> > * griddata, VectorMap<Id, int> * match_map);
			void	ImportChangEncode(GridCtrl * grid, Vector< Vector<String> > * griddata, VectorMap<Id, int> * match_map);
			void	SetCsvGridData(GridCtrl * grid, Vector< Vector<String> > * griddata, VectorMap<Id, int> * match_map);
			String	EncodeToUtf8(String & src);	
	void	Print(GridCtrl * grid, String name);
	//File Operation------------------------------------------------------
	void	ExportCSV(GridCtrl * grid, const String & path, const String & name, bool is_bom);
	void 	ParserCSVFile(Stream & content, Vector< Vector<String> > & data);
	void 	ImportCSV(GridCtrl * datagrid, const String & name);
	
	//Preference Tab------------------------------------------------------
	void	ConfigDB();
	void	SavePreference();
	void	BackupDB();
	void	RestoreDB();
			void SelectRestoreDB(EditString * path);
	void	DoDBUpdate(WithDBRestoreLayout<TopWindow> * d);
			bool InputDBPW(String & pw);
			
	enum	UP_STATUS{UP_NONE, UP_OK, UP_NOT_WORK, UP_OPEN_FAIL, UP_COPY_FAIL, UP_OLDER};
	UP_STATUS	DBUpdate(const String & path, const String & key);
	//Help Tab------------------------------------------------------------
	void	ShowLicense();
};

//some function-----------------------------------------------
String Replace(String str, String find, String replace);
std::vector<std::string> ParserCsvLine(const char * line);
Font StdFontS(int scale); //return scaled StdFont, like 12, scale=-1 return 11
#endif