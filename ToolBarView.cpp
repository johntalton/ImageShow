/*******************************************************
*   ImageShow©
*
*   This is hopefully going to be a realy nice image
*   viewer. Not like there are a 100 of them out there
*   or anything.
*
*   @author  YNOP (ynop@acm.org)
*   @version beta
*   @date    Sept 18 1999
*******************************************************/
#include <InterfaceKit.h>
#include <StorageKit.h>
#include <String.h>
#include <Path.h>
#include <TranslationKit.h>
#include <TranslationUtils.h>

//#include <stdio.h>

#include "Globals.h"
#include "ToolBarView.h"
#include "Dead.h"
#include "BugOutDef.h"

extern BugOut db;

/*******************************************************
*   Setup the main view. Add in all the niffty components
*   we have made and get things rolling
*******************************************************/
ToolBarView::ToolBarView(BRect frame):BView(frame, "", B_FOLLOW_TOP|B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW){
   SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
   space = 10; 
   left = space;
   buttonsize.Set(0,0,25,25);
   db.SendMessage("Toolbar init OK");
}


/*******************************************************
*   
*******************************************************/
void ToolBarView::AllAttached(){
   SetViewColor(Parent()->ViewColor());
}

/*******************************************************
*   
*******************************************************/
void ToolBarView::Draw(BRect){
   BRect b = Bounds();
   SetHighColor(255,255,255);
   StrokeLine(BPoint(b.left,b.top),BPoint(b.right,b.top));
}

/*******************************************************
*   
*******************************************************/
void ToolBarView::AddItem(const char *up,const char *down, BMessage *msg){
   BBitmap *img = BTranslationUtils::GetBitmap(up);
   if(!img){
      img = new BBitmap(buttonsize,B_RGB32,false);
      img->SetBits(DeadImg,DeadW*DeadH*3,0,B_RGB32);
   }
   
   BPicture *Up,*Down;

   //SetDrawingMode(B_OP_ALPHA);
   BeginPicture(new BPicture);
   DrawBitmap(img,img->Bounds(),buttonsize);
   Up = EndPicture();
   
   delete img;
   img = BTranslationUtils::GetBitmap(down);
   if(!img){
      img = new BBitmap(buttonsize,B_RGB32,false);
      img->SetBits(DeadImg,DeadW*DeadH*3,0,B_RGB32);
   }
   
   BeginPicture(new BPicture);
   DrawBitmap(img,img->Bounds(),buttonsize);
   Down = EndPicture();
  

   SetDrawingMode(B_OP_OVER);
      
   BRect f = buttonsize;
   f.left += left;
   f.right += left;
   f.top += 1;
   f.bottom += 1;
   
   BPictureButton *PBut = new BPictureButton(f,up,Up,Down,msg,B_ONE_STATE_BUTTON,B_FOLLOW_LEFT,B_WILL_DRAW);
   AddChild(PBut);
   
   //if(IsHidden()){
   //   PBut->Hide();
   //}
   
   left += buttonsize.IntegerWidth() + space;
   delete img;
   delete Up;
   delete Down;
   db.SendMessage("Added Item to toolbar");
}

/*******************************************************
*   
*******************************************************/
void ToolBarView::MessageReceived(BMessage *msg){
   switch(msg->what){
   default:
      BView::MessageReceived(msg);
   }
}





