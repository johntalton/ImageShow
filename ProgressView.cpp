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

//#include <stdio.h>

#include "Globals.h"
#include "ProgressView.h"
#include "BugOutDef.h"

extern BugOut db;

/*******************************************************
*   Setup the main view. Add in all the niffty components
*   we have made and get things rolling
*******************************************************/
ProgressView::ProgressView(BRect frame,uint32 mode):BView(frame, "", mode, B_WILL_DRAW|B_PULSE_NEEDED){
   SetViewColor(B_TRANSPARENT_32_BIT); // go tran so we have control over drawing
   active = false;
   count = 0;
   step = 0;
   db.SendMessage("Progress View init just perfect");
}

/*******************************************************
*   
*******************************************************/
ProgressView::~ProgressView(){

}

/*******************************************************
*   
*******************************************************/
void ProgressView::AllAttached(){
   //SetViewColor(Parent()->ViewColor());
}

/*******************************************************
*   
*******************************************************/
void ProgressView::Pulse(){
   if(active && (count == 0)){
      Invalidate();
   }
   count++;
   if(count < 10){ count = 0; }
}

/*******************************************************
*   
*******************************************************/
void ProgressView::Draw(BRect){
   BRect b = Bounds();
   SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR)); // fille the main box
   FillRect(BRect(b.left-2,b.top-2,b.right-1,b.bottom-1));
   SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),B_DARKEN_3_TINT)); // fill outer top/left 
   StrokeLine(BPoint(b.left,b.top),BPoint(b.right,b.top));
   StrokeLine(BPoint(b.left,b.top),BPoint(b.left,b.bottom));
   SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),B_DARKEN_2_TINT)); // fill inner top left
//   StrokeLine(BPoint(b.left+1,b.top+1),BPoint(b.right-1,b.top+1));
 //  StrokeLine(BPoint(b.left+1,b.top+1),BPoint(b.left+1,b.bottom-1));
   SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),B_DARKEN_2_TINT)); // fill outer bottom/right
   StrokeLine(BPoint(b.right,b.top),BPoint(b.right,b.bottom));
   StrokeLine(BPoint(b.left,b.bottom),BPoint(b.right,b.bottom));
   SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),B_DARKEN_1_TINT));  // fill inner bottom/right
  // StrokeLine(BPoint(b.right-1,b.top+1),BPoint(b.right-1,b.bottom-1));
  // StrokeLine(BPoint(b.left+1,b.bottom-1),BPoint(b.right-1,b.bottom-1));
      
   if(active){
      b.top += 2;
      b.bottom -= 2;
      b.left += 2;
      b.right -= 2;
      SetHighColor(90,170,255); // file main
      FillRect(BRect(b.left+1,b.top+1,b.right-1,b.bottom-1));
      SetHighColor(160,200,255); // top edge
      StrokeLine(BPoint(b.left,b.top),BPoint(b.right,b.top));
      StrokeLine(BPoint(b.left,b.top),BPoint(b.left,b.bottom));
      SetHighColor(0,50,190); // bottom edge
      StrokeLine(BPoint(b.right,b.top),BPoint(b.right,b.bottom));
      StrokeLine(BPoint(b.left,b.bottom),BPoint(b.right,b.bottom));
      
      b.top += 1;
      b.bottom -= 1;
      b.left += 1;
      b.right -= 1;

      SetDrawingMode(B_OP_BLEND);
//      SetHighColor(0,50,190);
      SetHighColor(0,0,0);
      SetHighColor(60,60,127);

      BRegion reg;
      reg.Set(b);
      ConstrainClippingRegion(&reg);

      int slant = 3;
      for(int x =-step; x <= b.right-(slant);x+=(slant*2)){
         for(int s=0;s < slant;s++){
            StrokeLine(BPoint(b.left+x+s,b.top),BPoint(b.left+x+slant+s,b.bottom));
         }
      }
      step++;
      if(step > slant+1){ step = 0; }
      SetDrawingMode(B_OP_OVER);
   }
}

/*******************************************************
*   
*******************************************************/
void ProgressView::SetActive(bool a){
   active = a;
   Invalidate();
}

/*******************************************************
*   
*******************************************************/
void ProgressView::MessageReceived(BMessage *msg){
   switch(msg->what){
   case WORKING:
      SetActive(true);
      break;
   case FINISHED:
      SetActive(false);
      break;
   default:
      BView::MessageReceived(msg);
   }
}






