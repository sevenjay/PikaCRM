#ifndef _PikaCRM_PikaCtrl_h_
#define _PikaCRM_PikaCtrl_h_

class PanelCtrl : public Ctrl
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
	StaticRect mBack;
	bool mIsShow;
	Position mPos;
	int mHideSize;
	int mShowSize;
	
	int mLength;

public:	
	Button mSwitch;
	PanelCtrl():mHideSize(14),mShowSize(200),mIsShow(true),mLength(200)
	{
		mBack.Color(SColorFace).SetFrame(BlackFrame());
		Add(mBack.SizePos());
		
		mSwitch <<= callback(this,&PanelCtrl::Switch);
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
			mSwitch.RightPosZ(1, 12).BottomPosZ(1, 12);
			mSwitch.SetImage(mIsShow? CtrlImg::SortUp() : CtrlImg::SortDown());
			break;
		case BOTTOM:
			BottomPosZ(1, size);
			HCenterPosZ(mLength, 0);
			mSwitch.RightPosZ(1, 12).TopPosZ(1, 12);
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
};



#endif
