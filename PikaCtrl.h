#ifndef _PikaCRM_PikaCtrl_h_
#define _PikaCRM_PikaCtrl_h_

class HidePanel : public Ctrl
{
public:
	enum Position
	{
		TOP,
		BOTTOM,
		LEFT,
		RIGHT
	};

private:	
	StaticRect 	mBack;
	bool 		mIsShow;
	Position 	mPos;
	int 		mHideSize;
	int 		mShowSize;
	
	int mLength;

	Button mSwitch;
	
	VectorMap<Ctrl*, Rect> mIndentCtrl;

public:	
	Callback WhenShow;
	Callback WhenHide;
	
	HidePanel():mHideSize(14),mShowSize(200),mIsShow(true),mLength(200)
	{
		mBack.Color(SColorFace);
		Add(mBack.SizePos());
		
		mSwitch <<= callback(this,&HidePanel::Switch);
		Add(mSwitch);
	}
	
	void SetPos(Position pos)
	{
		mPos=pos;
		int size=mIsShow?mShowSize:mHideSize;
		switch(pos){
		case TOP:
			TopPosZ(1, size);
			HCenterPosZ(mLength, 0);
			mSwitch.LeftPosZ(1, 12).BottomPosZ(1, 12);
			mSwitch.SetImage(mIsShow? CtrlImg::SortUp() : CtrlImg::SortDown());
			break;
		case BOTTOM:
			BottomPosZ(1, size);
			HCenterPosZ(mLength, 0);
			mSwitch.LeftPosZ(1, 12).TopPosZ(1, 12);
			mSwitch.SetImage(mIsShow? CtrlImg::SortDown() : CtrlImg::SortUp());
			break;
		case LEFT:
			LeftPosZ(1, size);
			break;
		case RIGHT:
			RightPosZ(1, size);
			break;
		default:
			;
		}
		mIsShow?WhenShow():WhenHide();
		IndentAllCtrl();
		Refresh();
	};
	
	void SetSize(int hide, int show){mHideSize=hide;mShowSize=show;};
	void SetLength(int l){mLength=l;SetPos(mPos);};
	
	void Enable(bool b)
	{
		Ctrl * ctrl = GetFirstChild();
		while(ctrl)
		{
			ctrl->Enable(b);
			ctrl = ctrl->GetNext();
		}
	}

	void Show(bool b)
	{
		mIsShow=b;
		SetPos(mPos);
	}
	
	void Switch(void){mIsShow?Show(false):Show(true);}

	bool IsShow(void){return mIsShow;}
	
	void Indent(Ctrl * ctrl)
	{
		mIndentCtrl.Add(ctrl, ctrl->GetRect());
	}
	
	void IndentAllCtrl(void)
	{
		int indentH = Ctrl::HorzLayoutZoom(mShowSize-mHideSize);
		int indentV = Ctrl::VertLayoutZoom(mShowSize-mHideSize);
		for(int i = 0; i < mIndentCtrl.GetCount(); i++)
		{
			Rect rc=mIndentCtrl[i];
			
			switch(mPos){
			case TOP:
				rc.Offset(0, mIsShow?indentV:0);
				rc.SetSize(rc.Width(), mIsShow?rc.Height()-indentV:rc.Height());
				break;
			case BOTTOM:
				rc.Offset(0, 0);
				rc.SetSize(rc.Width(), mIsShow?rc.Height()-indentV:rc.Height());
				break;
			case LEFT:
				rc.Offset(mIsShow?indentH:0, 0);
				rc.SetSize(mIsShow?rc.Width()-indentH:rc.Width(), rc.Height());
				break;
			case RIGHT:
				rc.Offset(0, 0);
				rc.SetSize(mIsShow?rc.Width()-indentH:rc.Width(), rc.Height());
				break;
			default:
				;
			}
			
			mIndentCtrl.GetKey(i)->SetRect(rc);
		}
	}
};



#endif
