#include "PikaCRM.h"

#define TFILE <PikaCRM/PikaCRM.t>
#include <Core/t.h>

#define IMAGECLASS SrcImages					// Adding Graphic
#define	IMAGEFILE <PikaCRM/SrcImages.iml>		//
#include <Draw/iml.h>							//

#define TOPICFILE <PikaCRM/srcdoc.tpp/all.i>	// Adding QTF for splash (and for other aims)
#include <Core/topic_group.h>					//

#include <PikaCRM/sql/sql.ids>	//for convenient use tables/columns name

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
class ColorNotNull : public Display
{
public:
	virtual void PaintBackground(Draw& w, const Rect& r, const Value& q,
	                             Color ink, Color paper, dword style) const
	{
    	if( IsNull(q) )	paper = Color(255, 223, 223);
    	Display::PaintBackground(w, r, q, ink, paper, style);
	};
};

PikaCRM::PikaCRM()
{
	mLanguage=LNG_('Z','H','T','W');
	//mLanguage=LNG_('E','N','U','S');
	Upp::SetLanguage(mLanguage);

	Upp::CtrlLayout(MainFrom, t_("Pika Customer Relationship Management"));
	
	int QtfHigh=20;
	mSplash.SplashInit("PikaCRM/srcdoc/Splash",QtfHigh,getLangLogo(),SrcImages::Logo(),mLanguage);
	
	SetupUI();

	SetAllFieldMap();

	MainFrom.WhenClose=THISBACK(CloseMainFrom);
	MainFrom.Sizeable().Zoomable();
}

PikaCRM::~PikaCRM()
{
}

void PikaCRM::SetupUI()
{
	//TabCtrl----------------------------------------------------------------------------
	//MainFrom.tabMain.WhenSet=THISBACK1(TabChange,MainFrom.tabMain.Get());
	CtrlLayout(Customer);
	MainFrom.tabMain.Add(Customer.SizePos(), t_("Customers"));
	CtrlLayout(Contact);
	MainFrom.tabMain.Add(Contact.SizePos(), t_("Contacts"));
	CtrlLayout(Event);
	MainFrom.tabMain.Add(Event.SizePos(), t_("Events"));
	CtrlLayout(Order);
	MainFrom.tabMain.Add(Order.SizePos(), t_("Orders"));
	CtrlLayout(Merchandise);
	MainFrom.tabMain.Add(Merchandise.SizePos(), t_("Merchandises"));
	CtrlLayout(Preference);
	MainFrom.tabMain.Add(Preference.SizePos(), t_("Preferences"));
	//end TabCtrl------------------------------------------------------------------------
	//Customer Tab-----------------------------------------------------------------------
	Customer.btnCreate <<= callback(&(Customer.Grid),&GridCtrl::DoAppend);
	Customer.btnModify <<= callback(&(Customer.Grid),&GridCtrl::DoEdit);
	Customer.btnDelete <<= callback(&(Customer.Grid),&GridCtrl::DoRemove);
	Customer.btnCreateF <<= THISBACK(CreateField);
	//Customer.btnModifyF <<= callback(&(Customer.Grid),&GridCtrl::DoEdit);
	//Customer.btnDeleteF <<= callback(&(Customer.Grid),&GridCtrl::DoRemove);

	Customer.Grid.AddIndex(C_ID).Default(-1);//for when create row before insert row
	Customer.Grid.AddColumn(C_TITLE,t_("Title")).Edit(cesn);
	Customer.Grid.AddColumn(C_PHONE,t_("Phone")).Edit(ces1);
	Customer.Grid.AddColumn(C_ADDRESS,t_("Address")).Edit(ces2);
	Customer.Grid.AddColumn(C_EMAIL,t_("Email")).Edit(ces3);
	Customer.Grid.AddColumn(C_WEBSITE,t_("Web site")).Edit(ces4);
	///@important when SetConvert(), it will Convert when you add, so must add the type like RawToValue(temp) in LoadCustomer()
	Customer.Grid.AddColumn(CO_NAME,t_("Contact")).Edit(mCustomerGridContactBtn);//.SetConvert(Single<ConvContactNames>());
		mCustomerGridContactBtn.AddButton().SetLabel("...").WhenPush=THISBACK(CustomerGridContactBtnClick);
	Customer.Grid.AddIndex(CONTACTS_MAP);
	Customer.Grid.AddColumn(C_0).Hidden();
	Customer.Grid.AddColumn(C_1).Hidden();
	Customer.Grid.AddColumn(C_2).Hidden();
	Customer.Grid.AddColumn(C_3).Hidden();
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
	Customer.Add(customer_search_bar.LeftPosZ(286, 82).TopPosZ(4, 19));
		Customer.Grid.FindBar(customer_search_bar, 140);
	Customer.btnSearchClear <<= THISBACK(BtnSearchClearClick);
	Customer.btnSearchGo <<= THISBACK(BtnSearchGoClick);
	
	//Contact Tab-----------------------------------------------------------------------
	Contact.btnCreate <<= callback(&(Contact.Grid),&GridCtrl::DoAppend);
	Contact.btnModify <<= callback(&(Contact.Grid),&GridCtrl::DoEdit);
	Contact.btnDelete <<= callback(&(Contact.Grid),&GridCtrl::DoRemove);
	
	Contact.Grid.AddIndex(CO_ID);
	Contact.Grid.AddColumn(CO_NAME,t_("Name_")).Edit(coesn);
	Contact.Grid.AddIndex(C_ID);
	Contact.Grid.AddColumn(C_TITLE,t_("Customer"));
	Contact.Grid.AddColumn(CO_PHONE,t_("Phone")).Edit(coes1);
	Contact.Grid.AddColumn(CO_ADDRESS,t_("Address")).Edit(coes2);
	Contact.Grid.AddColumn(CO_EMAIL,t_("Email")).Edit(coes3);
	Contact.Grid.Appending().Removing().AskRemove().Editing().Canceling().ColorRows();
	//.Searching() will take the partent of Grid.FindBar then take away GridFind, so don't use
	Contact.Grid.WhenInsertRow = THISBACK(InsertContact);
	Contact.Grid.WhenUpdateRow = THISBACK(UpdateContact);
	Contact.Grid.WhenRemoveRow = THISBACK(RemoveContact);
	//Contact Search------------------------------------------
	Contact.Add(contact_search_bar.LeftPosZ(286, 82).TopPosZ(4, 19));
		Contact.Grid.FindBar(contact_search_bar, 140);
	Contact.btnSearchClear <<= callback2(&(Contact.Grid),&GridCtrl::ClearFound,true,true);
	Contact.btnSearchGo <<= callback(&(Contact.Grid),&GridCtrl::DoFind);
	
	//Event Tab-----------------------------------------------------------------------
	Event.btnCreate <<= callback(&(Event.Grid),&GridCtrl::DoAppend);
	Event.btnModify <<= callback(&(Event.Grid),&GridCtrl::DoEdit);
	Event.btnDelete <<= callback(&(Event.Grid),&GridCtrl::DoRemove);
	
	Event.Grid.AddIndex(E_ID).Default(-1);//for when create row before insert row;
	Event.Grid.AddIndex(C_ID);
	Event.Grid.AddColumn(C_TITLE,t_("Customer")).Edit(mEventGridCustomerBtn);
		mEventGridCustomerBtn.SetDisplay(Single<ColorNotNull>());
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
	Event.Add(event_search_bar.LeftPosZ(286, 82).TopPosZ(4, 19));
		Event.Grid.FindBar(event_search_bar, 140);
	Event.btnSearchClear <<= callback2(&(Event.Grid),&GridCtrl::ClearFound,true,true);
	Event.btnSearchGo <<= callback(&(Event.Grid),&GridCtrl::DoFind);
	
	//Merchandise Tab-----------------------------------------------------------------------
	Merchandise.btnCreate <<= callback(&(Merchandise.Grid),&GridCtrl::DoAppend);
	Merchandise.btnModify <<= callback(&(Merchandise.Grid),&GridCtrl::DoEdit);
	Merchandise.btnDelete <<= callback(&(Merchandise.Grid),&GridCtrl::DoRemove);
	
	Merchandise.Grid.AddIndex(M_ID).Default(-1);//for when create row before insert row;
	Merchandise.Grid.AddColumn(M_NAME,t_("Product Name")).Edit(mesn);
	Merchandise.Grid.AddColumn(M_MODEL,t_("Product Model")).Edit(mes1);
	Merchandise.Grid.AddColumn(M_PRICE,t_("Price")).Edit(med);
	Merchandise.Grid.Appending().Removing().AskRemove().Editing().Canceling().ColorRows();
	Merchandise.Grid.WhenInsertRow = THISBACK(InsertMerchandise);
	Merchandise.Grid.WhenUpdateRow = THISBACK(UpdateMerchandise);
	Merchandise.Grid.WhenRemoveRow = THISBACK(RemoveMerchandise);
	//Merchandise Search------------------------------------------
	Merchandise.Add(merchandise_search_bar.LeftPosZ(286, 82).TopPosZ(4, 19));
		Merchandise.Grid.FindBar(merchandise_search_bar, 140);
	Merchandise.btnSearchClear <<= callback2(&(Merchandise.Grid),&GridCtrl::ClearFound,true,true);
	Merchandise.btnSearchGo <<= callback(&(Merchandise.Grid),&GridCtrl::DoFind);
	
	//Order Tab-----------------------------------------------------------------------
	Order.btnCreate <<= callback(&(Order.Grid),&GridCtrl::DoAppend);
	Order.btnModify <<= callback(&(Order.Grid),&GridCtrl::DoEdit);
	Order.btnDelete <<= callback(&(Order.Grid),&GridCtrl::DoRemove);
	
	Order.Grid.AddIndex(O_ID).Default(-1);//for when create row before insert row;
	Order.Grid.AddIndex(C_ID);
	Order.Grid.AddColumn(C_TITLE,t_("Customer")).Edit(mOrderGridCustomerBtn);
		mOrderGridCustomerBtn.SetDisplay(Single<ColorNotNull>());
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
	//Order Search------------------------------------------
	Order.Add(order_search_bar.LeftPosZ(286, 82).TopPosZ(4, 19));
		Order.Grid.FindBar(order_search_bar, 140);
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
	Order.BuyItemGrid.AddColumn(M_NAME,t_("Product Name - Model")).Edit(mBuyItemGridMerchBtn);
		mBuyItemGridMerchBtn.SetDisplay(Single<ColorNotNull>());
		mBuyItemGridMerchBtn.AddButton().SetLabel("...").WhenPush=THISBACK(BuyItemGridMerchBtnClick);
	//Order.BuyItemGrid.AddColumn(M_MODEL,t_("Product Model"));
	Order.BuyItemGrid.AddColumn(M_PRICE,t_("Price"));
	
	Order.BuyItemGrid.AddColumn(B_PRICE,t_("Order Price")).Edit(bed);
	Order.BuyItemGrid.AddColumn(B_NUMBER,t_("Quantity")).Edit(beis).Default(0);
		beis.NotNull();
	Order.BuyItemGrid.Appending().Removing().AskRemove().Editing().Canceling().ColorRows();
	Order.BuyItemGrid.SetToolBar();
	Order.BuyItemGrid.WhenNewRow = THISBACK(NewBuyItem);
	Order.BuyItemGrid.WhenInsertRow = THISBACK(InsertBuyItem);
	Order.BuyItemGrid.WhenUpdateRow = THISBACK(UpdateBuyItem);
	Order.BuyItemGrid.WhenRemoveRow = THISBACK(RemoveBuyItem);
	
	//Preference Tab-----------------------------------------------------------------------
	Preference.btnDatabase <<= THISBACK(ConfigDB);
}
//database control------------------------------------------------------------
void PikaCRM::LoadSetAllField()
{
	SysLog.Info("Load and Set All Fields\n");
	bool is_sql_ok=SQL.Execute("select * from Field;");
	if(!is_sql_ok)
	{
		SysLog.Error(SQL.GetLastError()+"\n");
		return;
		///@todo Exclamation("[* " + DeQtfLf(e) + "]");
	}

	while(SQL.Fetch())
	{
		mFieldEditList.Add(new EditString());
		if(SQL[F_TABLE]=="c")
		{
			FieldId & field=mFieldMap.Get("c")[SQL[F_ROWID]];//c [0] is FieldId with C_0
			int c_index=Customer.Grid.FindCol(field.Id);
			if(-1!=c_index)
			{
				Customer.Grid.GetColumn(c_index).Edit(mFieldEditList.Top()).Name(SQL[F_NAME].ToString()).Hidden(false);
				field.IsUsed=true;
			}
		}	
		else if(SQL[F_TABLE]=="co")
		{
			//SqlId sqlid=mFieldMap.Get("co")[SQL[F_ROWID]];
			//Contact.Grid.AddColumn(sqlid, SQL[F_NAME].ToString()).Edit(mFieldEditList.Top());
		}	
		else if(SQL[F_TABLE]=="m")
		{
			//SqlId sqlid=mFieldMap.Get("m")[SQL[F_ROWID]];
			//Merchandise.Grid.AddColumn(sqlid, SQL[F_NAME].ToString()).Edit(mFieldEditList.Top());
		}
	}
	//int zz=mFieldEditList.Top().use_count();
}
void PikaCRM::CreateField()
{
	//UI--------------------------------------------
	TopWindow d;
	Button ok, cancel;

	d.Title(t_("Create a customer field")).SetRect(0, 0, 300, 150);
	d.Add(ok.SetLabel("OK").LeftPosZ(20, 45).TopPosZ(50, 16));
	d.Add(cancel.SetLabel("Cancel").LeftPosZ(100, 45).TopPosZ(50, 16));
	ok.Ok() <<= d.Acceptor(IDOK);
	cancel.Cancel() <<= d.Rejector(IDCANCEL);
	
	EditString edit;
	Label title;
	title.SetLabel(t_("Field title: "));
	d.Add(title.LeftPosZ(15, 75).TopPosZ(20, 16));
	d.Add(edit.LeftPosZ(70, 75).TopPosZ(20, 16));
	//end UI--------------------------------------------
	if(d.Run()==IDOK) {
		if(!IsNull(edit.GetData()))
		{
			//mEventDropStatus.Add(edit.GetData());
			//mEventDropStatus.SetData(edit.GetData());
		}
	}
}

void PikaCRM::LoadCustomer()
{
	SysLog.Info("Load Customers\n");
	Customer.Grid.Clear();
	bool is_sql_ok=SQL.Execute("select * from Customer;");
	if(is_sql_ok)
	{
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
			Customer.Grid(C_0) = SQL[C_0];
			Customer.Grid(C_1) = SQL[C_1];
			Customer.Grid(C_2) = SQL[C_2];
			Customer.Grid(C_3) = SQL[C_3];
			Customer.Grid(CONTACTS_MAP) = raw_map;//this is must, "=" will set the same typeid for Value of GridCtrl with RawDeepToValue
			Customer.Grid(CO_NAME) = ConvContactNames().Format(Customer.Grid(CONTACTS_MAP));
		}
	}
	else
	{
		SysLog.Error(SQL.GetLastError()+"\n");
		///@todo Exclamation("[* " + DeQtfLf(e) + "]");
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
			(C_3,  Customer.Grid(C_3));

		Customer.Grid(C_ID) = SQL.GetInsertedId();//it will return only one int primary key
		
		//database set C_ID of CONTACTS_MAP's Contact to now
		const VectorMap<int, String> & contact_map= ValueTo< VectorMap<int, String> >(Customer.Grid(CONTACTS_MAP));
		for(int i = 0; i < contact_map.GetCount(); i++)//add already select contact to customer
		{
			int contact_id=contact_map.GetKey(i);
			try
			{
				SQL & ::Update(CONTACT) (C_ID, Customer.Grid(C_ID)).Where(CO_ID == contact_id);				
				//update Contact.Grid(C_TITLE);
				int contact_row=Contact.Grid.Find(contact_id,CO_ID);
				Contact.Grid.Set(contact_row,C_TITLE,Customer.Grid(C_TITLE));
				Contact.Grid.Set(contact_row,C_ID,Customer.Grid(C_ID));
			}
			catch(SqlExc &e)
			{
				Exclamation("[* " + DeQtfLf(e) + "]");
				continue;
			}
		}			
	}
	catch(SqlExc &e)
	{
		Customer.Grid.CancelInsert();
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
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveCustomer()
{
	const VectorMap<int, String> & contact_map= ValueTo< VectorMap<int, String> >(Customer.Grid(CONTACTS_MAP));
	try
	{
		SQL & Delete(CUSTOMER).Where(C_ID == Customer.Grid(C_ID));
		///@remark just clear customer in contact, this will be a performance issue
		for(int i = 0; i < contact_map.GetCount(); i++)
		{
			int contact_id=contact_map.GetKey(i);
			
			try
			{
				SQL.Execute("UPDATE main.Contact SET c_id = NULL WHERE co_id = ?;", contact_id);
				//clear Contact.Grid(C_TITLE);
				int contact_row=Contact.Grid.Find(contact_id,CO_ID);
				Contact.Grid.Set(contact_row,C_TITLE,"");
				Contact.Grid.Set(contact_row,C_ID,NULL);
			}
			catch(SqlExc &e)
			{
				continue;
				Exclamation("[* " + DeQtfLf(e) + "]");
			}			
		}
	}
	catch(SqlExc &e)
	{
		Customer.Grid.CancelRemove();
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}

void PikaCRM::LoadContact()
{
	SysLog.Info("Load Contacts\n");
	Contact.Grid.Clear();
	bool is_sql_ok=SQL.Execute("select co_id, Contact.c_id, c_title, co_name, co_phone, co_address, co_email from Contact left outer join Customer on Contact.c_id = Customer.c_id;");
	if(is_sql_ok)
	{
		while(SQL.Fetch())
		{
			Contact.Grid.Add(SQL[CO_ID],SQL[CO_NAME],SQL[C_ID],SQL[C_TITLE],SQL[CO_PHONE],SQL[CO_ADDRESS],SQL[CO_EMAIL]);
		}
	}
	else
	{
		SysLog.Error(SQL.GetLastError()+"\n");
	}
}
void PikaCRM::InsertContact()
{	
	try
	{
		SQL & Insert(CONTACT)
			(CO_NAME,  Contact.Grid(CO_NAME))
			(CO_PHONE,  Contact.Grid(CO_PHONE))
			(CO_ADDRESS,Contact.Grid(CO_ADDRESS))
			(CO_EMAIL,  Contact.Grid(CO_EMAIL));

		Contact.Grid(CO_ID) = SQL.GetInsertedId();//it will return only one int primary key
	}
	catch(SqlExc &e)
	{
		Contact.Grid.CancelInsert();
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::UpdateContact()
{
	try
	{
		SQL & ::Update(CONTACT)
			(CO_NAME,  Contact.Grid(CO_NAME))
			(CO_PHONE,  Contact.Grid(CO_PHONE))
			(CO_ADDRESS,Contact.Grid(CO_ADDRESS))
			(CO_EMAIL,  Contact.Grid(CO_EMAIL))
			.Where(CO_ID == Contact.Grid(CO_ID));
			
		//UpdateCustomerContact2(Contact.Grid(C_ID));----------------------------
		if(Contact.Grid(C_ID).IsNull()) return;
		
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
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveContact()
{
	try
	{
		SQL & Delete(CONTACT).Where(CO_ID == Contact.Grid(CO_ID));
		
		//UpdateCustomerContact1(Contact.Grid(C_ID));----------------------------
		if(Contact.Grid(C_ID).IsNull()) return;
		
		int customer_id=Contact.Grid(C_ID);
		int customer_row=Customer.Grid.Find(customer_id,C_ID);
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
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::Update_dg_contact()
{
	/*dg_contact.Clear();
	SQL & Select(CO_ID, CO_NAME)
		.From(CONTACT)
		.Where(IsNull(C_ID));

	while(SQL.Fetch())
		dg_contact.Add(SQL[C_ID], SQL[CO_NAME]);//C_ID: "CO_NAME1\nCO_NAME2\nCO_NAME3...
	*/
	//dg_contact.SetEditable(false);
}

void PikaCRM::LoadEvent()
{
	SysLog.Info("Load Events\n");
	Event.Grid.Clear();
	String sql ="select e_id, Event.c_id, c_title, e_ask, e_status, "
				"strftime('%m/%d/%Y %H:%M',e_rtime) as e_rtime, "
				"strftime('%m/%d/%Y %H:%M',e_ctime) as e_ctime, "
				"e_note from Event left outer join Customer on Event.c_id = Customer.c_id;";
	bool is_sql_ok=SQL.Execute(sql);
	if(is_sql_ok)
	{
		while(SQL.Fetch())
		{
			Event.Grid.Add(SQL[E_ID],SQL[C_ID],SQL[C_TITLE],SQL[E_ASK],SQL[E_STATUS],SQL[E_RTIME],SQL[E_CTIME],SQL[E_NOTE]);
		}
		UpdateEventDropStatus();
	}
	else
	{
		SysLog.Error(SQL.GetLastError()+"\n");
	}
}
void PikaCRM::InsertEvent()
{	
	try
	{
		SQL & Insert(EVENT)
			(C_ID,		Event.Grid(C_ID))
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
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::UpdateEvent()
{
	try
	{
		SQL & ::Update(EVENT)
			(C_ID,		Event.Grid(C_ID))
			(E_ASK,		Event.Grid(E_ASK))
			(E_STATUS,  Event.Grid(E_STATUS))
			(E_RTIME,	Event.Grid(E_RTIME))
			(E_NOTE,	Event.Grid(E_NOTE))
			.Where(E_ID == Event.Grid(E_ID));
	}
	catch(SqlExc &e)
	{
		Event.Grid.CancelUpdate();
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveEvent()
{
	try
	{
		SQL & Delete(EVENT).Where(E_ID == Event.Grid(E_ID));
	}
	catch(SqlExc &e)
	{
		Event.Grid.CancelRemove();
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::UpdateEventDropStatus()
{
	mEventDropStatus.Clear();
	SQL & Select(E_STATUS).From(EVENT).GroupBy(E_STATUS);

	while(SQL.Fetch())
		mEventDropStatus.Add(SQL[0]);
}

void PikaCRM::LoadMerchandise()
{
	SysLog.Info("Load Merchandises\n");
	Merchandise.Grid.Clear();
	bool is_sql_ok=SQL.Execute("select * from Merchandise;");
	if(is_sql_ok)
	{
		while(SQL.Fetch())
		{
			Merchandise.Grid.Add(SQL[M_ID],SQL[M_NAME],SQL[M_MODEL],SQL[M_PRICE]);
		}
	}
	else
	{
		SysLog.Error(SQL.GetLastError()+"\n");
	}
}
void PikaCRM::InsertMerchandise()
{	
	try
	{
		SQL & Insert(MERCHANDISE)
			(M_NAME,	Merchandise.Grid(M_NAME))
			(M_MODEL,	Merchandise.Grid(M_MODEL))
			(M_PRICE,	Merchandise.Grid(M_PRICE));

		Merchandise.Grid(M_ID) = SQL.GetInsertedId();//it will return only one int primary key
	}
	catch(SqlExc &e)
	{
		Merchandise.Grid.CancelInsert();
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::UpdateMerchandise()
{
	try
	{
		SQL & ::Update(MERCHANDISE)
			(M_NAME,	Merchandise.Grid(M_NAME))
			(M_MODEL,	Merchandise.Grid(M_MODEL))
			(M_PRICE,	Merchandise.Grid(M_PRICE))
			.Where(M_ID == Merchandise.Grid(M_ID));
	}
	catch(SqlExc &e)
	{
		Merchandise.Grid.CancelUpdate();
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveMerchandise()
{
	try
	{
		SQL & Delete(MERCHANDISE).Where(M_ID == Merchandise.Grid(M_ID));
	}
	catch(SqlExc &e)
	{
		Merchandise.Grid.CancelRemove();
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}

void PikaCRM::LoadOrder()
{
	SysLog.Info("Load Orders\n");
	Order.Grid.Clear();
	String sql ="select o_id, Orders.c_id, c_title, o_ship_add, o_bill_add, "
				"strftime('%m/%d/%Y',o_order_date) as o_order_date, "
				"strftime('%m/%d/%Y',o_ship_date) as o_ship_date, "
				"o_status, o_note from Orders left outer join Customer on Orders.c_id = Customer.c_id;";
	bool is_sql_ok=SQL.Execute(sql);
	if(is_sql_ok)
	{
		while(SQL.Fetch())
		{
			Order.Grid.Add(SQL[O_ID],SQL[C_ID],SQL[C_TITLE],SQL[O_SHIP_ADD],SQL[O_BILL_ADD],SQL[O_ORDER_DATE],SQL[O_SHIP_DATE],SQL[O_STATUS],SQL[O_NOTE]);
		}
	}
	else
	{
		SysLog.Error(SQL.GetLastError()+"\n");
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
		///@todo Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::InsertOrder()
{	
	try
	{
		SQL & Insert(ORDERS)
			(C_ID,	Order.Grid(C_ID))
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
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::UpdateOrder()
{
	String now_time="CURRENT_TIMESTAMP";
	try
	{
		SQL & ::Update(ORDERS)
			(C_ID,	Order.Grid(C_ID))
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
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveOrder()
{
	try
	{
		SQL & Delete(ORDERS).Where(O_ID == Order.Grid(O_ID));
		RemoveOrderBuyItem();
		Order.BuyItemGrid.Clear();
	}
	catch(SqlExc &e)
	{
		Order.Grid.CancelRemove();
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::ChangeOrder()
{
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
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::UpdateBuyItem()
{
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
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveBuyItem()
{
	try
	{
		SQL & Delete(BUYITEM).Where(B_ID == Order.BuyItemGrid(B_ID));
	}
	catch(SqlExc &e)
	{
		Order.BuyItemGrid.CancelRemove();
		Exclamation("[* " + DeQtfLf(e) + "]");
	}
}
void PikaCRM::RemoveOrderBuyItem()
{
	try
	{
		SQL & Delete(BUYITEM).Where(O_ID == Order.Grid(O_ID));
	}
	catch(SqlExc &e)
	{
		Order.BuyItemGrid.CancelRemove();
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
	mSplash.ShowSplash();
	String config_file_path = getConfigDirPath()+FILE_CONFIG;	
	String database_file_path = getConfigDirPath()+FILE_DATABASE;
	
	mSplash.ShowSplashStatus(t_("Loading Settings..."));
	SysLog.Info(t_("Loading Settings..."))<<"\n";
	LoadConfig(config_file_path);

	if(mConfig.IsDBEncrypt)
	{
		mSplash.HideSplash();
		if(mConfig.IsRememberPW)
		{
			SysLog.Info("config: Remeber the PW\n");
			String key=CombineKey(GetSystemKey(), mConfig.Password);
			SysLog.Debug("systemPWKey:"+key+"\n");
			if(mConfig.SystemPWKey.IsEmpty() || key!=mConfig.SystemPWKey)//use different PC
			{
				SysLog.Info("config: application is running on different PC\n");
				if(!IsInputPWCheck()) return;//false
			}
			//else
			//	;//just using mConfig.Password;
		}
		else//not Remember PW 
		{
			SysLog.Info("config: Not Remeber the PW\n");
			if(!IsInputPWCheck()) return;
		}
		mSplash.ShowSplash();
	}

	
	mSplash.ShowSplashStatus(t_("Loading Database..."));
	SysLog.Info(t_("Loading Database..."))<<"\n";
	if(IsHaveDBFile(database_file_path))
	{
		SysLog.Info("open the database file\n");
		CreateOrOpenDB(database_file_path);//OpenDB
		/*if(GetDBVersion()<DATABASE_VERSION)
			UpdateDB();
		else if(GetDBVersion()>DATABASE_VERSION)
			show can not up compatibility, please use the Latest version
		
		
		
		*/
	}
	else
	{
		SysLog.Info("create the database file\n");
		mSplash.HideSplash();
		///@todo first welcome
		if(!IsSetupDB(config_file_path)) return;
		mSplash.ShowSplash();
		CreateOrOpenDB(database_file_path);//CreateDB
		InitialDB();
	}
	/*
	if(IsDBWork())
		//donothing
	else
	{
		//pw error or not the file?
		showErrorMsg();
		goto enter PW
	}
	
	
	*/	
	//test if database OK
	bool is_sql_ok=SQL.Execute("select * from System;");
	if(is_sql_ok)
		while(SQL.Fetch())//user,ap_ver,sqlite_ver,db_ver
			SysLog.Debug("") << SQL[USER]<<", "<<SQL[CTIME]<<", "<<
								SQL[AP_VER]<<", "<<SQL[SQLITE_VER]<<", "<<SQL[DB_VER]<<"\n";
	else
	{
		SysLog.Error(SQL.GetLastError()+"\n");
	}

	//Load and set customer field(UI+data)
	LoadSetAllField();
	//Load all tab data
	LoadCustomer();
	LoadContact();
	LoadEvent();
	LoadMerchandise();
	LoadOrder();	
	
	
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

bool PikaCRM::IsHaveDBFile(const String & database_file_path)
{	
	return FileExists(database_file_path);
}

void PikaCRM::CreateOrOpenDB(const String & database_file_path)
{
	mSqlite3Session.Close();
	if(!mSqlite3Session.Open(database_file_path))
	{
		SysLog.Error("can't create or open database file: "+database_file_path+"\n");
		///@todo thow 
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
void PikaCRM::InitialDB()
{
	SysLog.Info("initial the database file\n");
	FileIn initial("initial.sql");
	if(!initial)
	{
		SysLog.Error("can't open database initial file: initial.sql\n");
		///@todo thow 
	}
	bool is_sql_ok = SqlPerformScript(mSqlite3Session,initial);
	
	is_sql_ok=SQL.Execute("INSERT INTO System (user,ap_ver,sqlite_ver,db_ver) VALUES (?,?,?,?);",
							"System",SOFTWARE_VERSION,mSqlite3Session.VersionInfo(),DATABASE_VERSION);
							///@todo set user name
	//@todo handel is_sql_ok						
#ifdef _DEBUG
	//in ebug
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
	d.esPassword.Password();
	d.esCheckPassword.Password();
	d.optRequire.SetEditable(false);
	d.optPW.WhenAction=THISBACK1(OnOptPWAction, &d);
	d.optRevealPW.WhenAction=THISBACK1(OnOptRevealPWAction, &d);
	d.ok.WhenPush = THISBACK1(CheckPWSame, &d);
	
	String note,note2;
	note<<"[1 "<<t_("Encrypted database can't be read even if someone has the database file.")<<" ]";
	d.rtNoteEncrypted.SetQTF(note);
	note2<<"[1 "<<t_("Important: if you forgot the password, there is no way to access your database.")<<" ]";
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
		if(mConfig.Load(config_file_path))
		{
			SysLog.Info("loaded the config file\n");
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
		mConfig.Password=PW_EMPTY;
		mConfig.IsRememberPW=false;
		mConfig.SystemPWKey="";
		SaveConfig(config_file_path);
	}
}
void PikaCRM::SetConfig()
{
}
void PikaCRM::SaveConfig(const String & config_file_path)
{
	if(mConfig.Save(config_file_path))
	{
		SysLog.Debug("Save the config file\n");
	}
	else
	{
		SysLog.Error("Save config file fail\n");
		///@todo thow can't save
	}
}


bool PikaCRM::IsInputPWCheck()
{
	String config_file_path=getConfigDirPath()+FILE_CONFIG;
	WithInputPWLayout<TopWindow> d;
	CtrlLayoutOKCancel(d,t_("Pika Customer Relationship Management"));
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

String PikaCRM::GetSystemKey()
{
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
				//printf("printf %s\n",buf);
			}
			pclose(p_process);
		}
		else 
		{
		    //perror("fail to execute hdsn");
		    output="fail to execute hdsn";
		}

#elif defined(PLATFORM_WIN32)
	///@todo include SystemInfo

#endif

	if( -1 != (pos=output.Find("serial_no:")) )
		key=getMD5(output);
	else
		SysLog.Error(output+"\n");///@remark throw error?
	
	
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
	mFieldMap.Add("c",c);
	
	Vector<FieldId> co;
	co.Add().SetId(CO_0);
	co.Add().SetId(CO_1);
	co.Add().SetId(CO_2);
	co.Add().SetId(CO_3);
	mFieldMap.Add("co",co);
		
	Vector<FieldId> m;
	m.Add().SetId(M_0);
	m.Add().SetId(M_1);
	m.Add().SetId(M_2);
	m.Add().SetId(M_3);
	mFieldMap.Add("m",m);
}
//end application control-----------------------------------------------------------
//interactive with GUI==============================================================
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

    d.SetRect(0, 0, 400, 400);
	d.Add(ok.SetLabel("OK").LeftPosZ(40, 64).TopPosZ(175, 24));
	d.Add(cancel.SetLabel("Cancel").LeftPosZ(130, 64).TopPosZ(175, 24));
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
	
	//add no costomer contact to select
	SQL & Select(CO_ID, CO_NAME).From(CONTACT).Where(IsNull(C_ID));
	while(SQL.Fetch())
		list.Add(SQL[CO_ID], SQL[CO_NAME],true);
	

	if(d.Run()==IDOK) {
		
		///@remark just clear costomer in contact, this will be a performance issue
		for(int i = 0; i < contact_map.GetCount(); i++)
		{
			int contact_id=contact_map.GetKey(i);
			
			try
			{
				//SQL & ::Update(CONTACT) (C_ID, NULL).Where(CO_ID == contact_id);//fail, NULL will be 0
				SQL.Execute("UPDATE main.Contact SET c_id = NULL WHERE co_id = ?;", contact_id);
				
				//clear Contact.Grid(C_TITLE);
				int contact_row=Contact.Grid.Find(contact_id,CO_ID);
				Contact.Grid.Set(contact_row,C_TITLE,"");
				Contact.Grid.Set(contact_row,C_ID,NULL);
			}
			catch(SqlExc &e)
			{
				continue;
				Exclamation("[* " + DeQtfLf(e) + "]");
			}			
		}
		
		VectorMap<int, String> new_contact_map;
		for(int i = 0; i < list.GetCount(); i++)
		{
			if(list.IsSel(i))
			{
				int contact_id=list.Get(i);
				//update in the database. if costomer_id ==-1, no need to do, it will do in InsertCustomer()
				try
				{
					if(-1 != costomer_id)
					{
						SQL & ::Update(CONTACT) (C_ID,  costomer_id).Where(CO_ID == contact_id);
						//update Contact.Grid(C_TITLE);
						int contact_row=Contact.Grid.Find(contact_id,CO_ID);
						Contact.Grid.Set(contact_row,C_TITLE,Customer.Grid(C_TITLE));
						Contact.Grid.Set(contact_row,C_ID,Customer.Grid(C_ID));
					}
				}
				catch(SqlExc &e)
				{
					continue;//Contact.Grid.CancelUpdate();
					Exclamation("[* " + DeQtfLf(e) + "]");
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

void PikaCRM::EventGridCustomerBtnClick()
{
	//UI--------------------------------------------
	TopWindow d;
	Button ok, cancel;

    d.SetRect(0, 0, 400, 400);
	d.Add(ok.SetLabel("OK").LeftPosZ(40, 64).TopPosZ(175, 24));
	d.Add(cancel.SetLabel("Cancel").LeftPosZ(130, 64).TopPosZ(175, 24));
	ok.Ok() <<= d.Acceptor(IDOK);
	cancel.Cancel() <<= d.Rejector(IDCANCEL);
	
	ColumnList list;
	d.Add(list);
	list.SetRect(0, 0, 400, 325);
	list.Columns(3);
	//list.MultiSelect();
	//end UI--------------------------------------------
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
				try
				{
					if(-1 != Event.Grid(E_ID))
					{
						SQL & ::Update(EVENT) (C_ID,  costomer_id).Where(E_ID == Event.Grid(E_ID));
					}
				}
				catch(SqlExc &e)
				{
					continue;//Contact.Grid.CancelUpdate();
					Exclamation("[* " + DeQtfLf(e) + "]");
				}
			}
		}
		Event.Grid.Set(C_ID,costomer_id);		
		Event.Grid.Set(C_TITLE,title);
    }
}
void PikaCRM::EventNewStatusClick()
{
	//UI--------------------------------------------
	TopWindow d;
	Button ok, cancel;

	d.Title(t_("New status")).SetRect(0, 0, 300, 150);
	d.Add(ok.SetLabel("OK").LeftPosZ(20, 45).TopPosZ(50, 16));
	d.Add(cancel.SetLabel("Cancel").LeftPosZ(100, 45).TopPosZ(50, 16));
	ok.Ok() <<= d.Acceptor(IDOK);
	cancel.Cancel() <<= d.Rejector(IDCANCEL);
	
	EditString edit;
	Label title;
	title.SetLabel(t_("Status: "));
	d.Add(title.LeftPosZ(18, 75).TopPosZ(20, 16));
	d.Add(edit.LeftPosZ(70, 75).TopPosZ(20, 16));
	//end UI--------------------------------------------
	if(d.Run()==IDOK) {
		if(!IsNull(edit.GetData()))
		{
			mEventDropStatus.Add(edit.GetData());
			mEventDropStatus.SetData(edit.GetData());
		}
	}
}

void PikaCRM::OrderGridCustomerBtnClick()
{
	//UI--------------------------------------------
	TopWindow d;
	Button ok, cancel;

    d.SetRect(0, 0, 400, 400);
	d.Add(ok.SetLabel("OK").LeftPosZ(40, 64).TopPosZ(175, 24));
	d.Add(cancel.SetLabel("Cancel").LeftPosZ(130, 64).TopPosZ(175, 24));
	ok.Ok() <<= d.Acceptor(IDOK);
	cancel.Cancel() <<= d.Rejector(IDCANCEL);
	
	ColumnList list;
	d.Add(list);
	list.SetRect(0, 0, 400, 325);
	list.Columns(3);
	//end UI--------------------------------------------
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
				try
				{
					if(-1 != Order.Grid(O_ID))
					{
						SQL & ::Update(ORDERS) (C_ID,  costomer_id).Where(O_ID == Order.Grid(O_ID));
					}
				}
				catch(SqlExc &e)
				{
					continue;//Contact.Grid.CancelUpdate();
					Exclamation("[* " + DeQtfLf(e) + "]");
				}
			}
		}
		Order.Grid.Set(C_ID,costomer_id);		
		Order.Grid.Set(C_TITLE,title);
		LoadOrderCustomer();
    }
}

void PikaCRM::BuyItemGridMerchBtnClick()
{
	//UI--------------------------------------------
	TopWindow d;
	Button ok, cancel;

    d.SetRect(0, 0, 400, 400);
	d.Add(ok.SetLabel("OK").LeftPosZ(40, 64).TopPosZ(175, 24));
	d.Add(cancel.SetLabel("Cancel").LeftPosZ(130, 64).TopPosZ(175, 24));
	ok.Ok() <<= d.Acceptor(IDOK);
	cancel.Cancel() <<= d.Rejector(IDCANCEL);
	
	ColumnList list;
	d.Add(list);
	list.SetRect(0, 0, 400, 325);
	list.Columns(3);
	//end UI--------------------------------------------
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
				try
				{
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
				catch(SqlExc &e)
				{
					continue;
					Exclamation("[* " + DeQtfLf(e) + "]");
				}
			}
		}
		Order.BuyItemGrid.Set(M_ID,merch_id);		
		Order.BuyItemGrid.Set(M_NAME,title);	
		Order.BuyItemGrid.Set(M_PRICE,price);	
    }
}

void PikaCRM::ConfigDB()
{
	SysLog.Info("configure the database\n");
	String config_file_path = getConfigDirPath()+FILE_CONFIG;	
	String database_file_path = getConfigDirPath()+FILE_DATABASE;
	CreateOrOpenDB(database_file_path);//must resetkey before any operation after open db, so we re-open
	if(mConfig.Password.IsEqual(PW_EMPTY))
	{
		;
	}
	else //if no pw (PW_EMPTY) not need
	{
		if(!IsInputPWCheck()) return;
	}
	
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
	
};
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