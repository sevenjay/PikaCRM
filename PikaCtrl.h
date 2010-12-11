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


class PreviewImage : public ImageCtrl {
	void SetData(const Value& val)
	{
		String path = val;
		if(IsNull(path.IsEmpty()))
			SetImage(Null);
 		else
			SetImage(StreamRaster::LoadFileAny(~path));
	}
};

class CutImage : public ImageCtrl {
	double mSmall;
	double mLarge;
	double mScaleTo;
	
	int mCutWidth;
	int mCutHigh;
	
	Point mMove;
	Point mStart;
	Point mLastMove;
	Point mStop;
	bool  mIsDrag;
	
	virtual void   MouseMove(Point p, dword keyflags)
	{
		if(mIsDrag)
		{
			mMove = p;
			Refresh();
		}
	}
	
	virtual void LeftDrag(Point p, dword keyflags)
	{
		mStart = p;
		mIsDrag = true;
	}
	
	virtual void   LeftUp(Point p, dword keyflags)
	{
		if(mIsDrag)
		{
			mStop=mStop+mLastMove;
			mLastMove=Point(0, 0);
			mIsDrag = false;
		}
	}

	virtual void   MouseWheel(Point p, int zdelta, dword keyflags)
	{
		if(zdelta>0) ScaleLarge();
		else ScaleSmall();
	}
	
	virtual void Paint(Draw& w)
	{
		w.DrawRect(GetSize(), White);
		if(mIsDrag) mLastMove=mMove-mStart;
		int cutx=mStop.x+mLastMove.x;
		int cuty=mStop.y+mLastMove.y;
				
		if(img)
		{
			Size rsz = img.GetSize();
			w.DrawImage(0, 0, rsz.cx*mScaleTo, rsz.cy*mScaleTo, img);
		}
		else
		{
			w.DrawText(0, 0, "No image loaded!", Arial(30).Italic());
		}
		
		Vector<Point> p;
		p << Point(cutx, cuty) << Point(cutx+mCutWidth, cuty) 
		  << Point(cutx+mCutWidth, cuty+mCutHigh) << Point(cutx, cuty+mCutHigh) << Point(cutx, cuty);
		w.DrawPolyline(p, 4, Black);		
	}
	
	
public: 
	
	CutImage():mLastMove(0,0), mMove(0,0), mStart(0,0), mStop(0,0), mIsDrag(false)
	{
		mSmall=0.8;
		mLarge=1.25;
		mScaleTo=1;
		
		mCutWidth=200;
		mCutHigh=200;
	}
	
	Image GetCutImage(void)
	{
		Size rsz = img.GetSize();
		return Crop(Rescale(img, rsz.cx*mScaleTo, rsz.cy*mScaleTo), mStop.x, mStop.y, mCutWidth, mCutHigh);
	}
	
	void ScaleLarge(){if(mScaleTo<10) mScaleTo=mScaleTo*mLarge;Refresh();}
	void ScaleSmall(){if(mScaleTo>0.1) mScaleTo=mScaleTo*mSmall;Refresh();}
};


class GrabImage : public ImageCtrl {
	PreviewImage mPreImg;
	CutImage mCutImg;
	
public:
	Callback WhenClick;
	virtual void LeftDown(Point, dword)
	{
		WhenClick(); 
		LoadCutImage();
		EditCutImage();
	}
	
	void LoadCutImage(void)
	{	
		static FileSel gFileSelImage; //static for remember user selection in one session
		gFileSelImage.Type("jpg file", "*.jpg");
		gFileSelImage.Type("png file (*.png)", "*.png");
		mPreImg.SetImage(Null);
		gFileSelImage.Preview(mPreImg);
		if(gFileSelImage.ExecuteOpen()){
			mCutImg.SetImage( StreamRaster::LoadFileAny(~gFileSelImage));
		}
	}
	
	void EditCutImage(void)
	{
		//UI--------------------------------------------
		TopWindow d;
		Button ok, cancel;
		Button large, small;
		
		d.Title(t_("Move and Crop the Image")).SetRect(0, 0, Ctrl::HorzLayoutZoom(700), Ctrl::VertLayoutZoom(460));
		d.Add(ok.SetLabel(t_("OK")).RightPosZ(170, 56).BottomPosZ(6, 20));
		d.Add(cancel.SetLabel(t_("Cancel")).RightPosZ(70, 56).BottomPosZ(6, 20));
		d.Add(small.SetImage(CtrlImg::Minus()).RightPosZ(340, 20).BottomPosZ(6, 20));
		d.Add(large.SetImage(CtrlImg::Plus()).RightPosZ(300, 20).BottomPosZ(6, 20));
		ok.Ok() <<= d.Acceptor(IDOK);
		cancel.Cancel() <<= d.Rejector(IDCANCEL);
		small<<=callback(&mCutImg,&CutImage::ScaleSmall);
		large<<=callback(&mCutImg,&CutImage::ScaleLarge);
		
		d.Add(mCutImg.LeftPosZ(8, 684).TopPosZ(8, 420));
		//end UI--------------------------------------------
		
		if(d.Run()==IDOK)
		{
			SetImage(mCutImg.GetCutImage());
			Refresh();
		}
	}
};


#endif
