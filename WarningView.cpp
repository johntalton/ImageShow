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

#include <stdio.h>

#include "Globals.h"
#include "WarningView.h"
#include "BugOutDef.h"

extern BugOut db;

/*******************************************************
*   Setup the main view. Add in all the niffty components
*   we have made and get things rolling
*******************************************************/
WarningView::WarningView(BRect frame,uint32 mode):BView(frame, "", mode, B_WILL_DRAW|B_PULSE_NEEDED){
   SetViewColor(B_TRANSPARENT_32_BIT); // go tran so we have control over drawing
   count = -1;
   db.SendMessage("Warning View Init OK");
   warn = false;
}

/*******************************************************
*   
*******************************************************/
WarningView::~WarningView(){

}

/*******************************************************
*   
*******************************************************/
void WarningView::AllAttached(){
   //SetViewColor(Parent()->ViewColor());
}

/*******************************************************
*   
*******************************************************/
void WarningView::Pulse(){
   if(count==0){
      warn = false;
      count = -1;
      Invalidate();
   }else if(count > 0){
      count--;
   }
}

/*******************************************************
*   
*******************************************************/
void WarningView::Draw(BRect){
   SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
   FillRect(Bounds());
   if(warn){
      SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));//set so string alias right
      SetHighColor (150,0,0);
      MovePenTo(0,9);
      DrawString("No Translator Found");
           
   }else{
      // Dont do anything ... just chill out   
   }

}


/*******************************************************
*   
*******************************************************/
void WarningView::MessageReceived(BMessage *msg){
   switch(msg->what){
   case NULL_IMAGE:
      count = 12; // this is how long we display the warning
      warn = true;
      Invalidate();
      break;
   default:
      BView::MessageReceived(msg);
   }
}






