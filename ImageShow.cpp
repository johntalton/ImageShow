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

#include "Globals.h"
#include "ImageShow.h"
#include "ImageShowWindow.h"
#include "TPreferences.h"

/*******************************************************
*
*******************************************************/
ImageShow::ImageShow() : BApplication(APP_SIGNATURE){
   BRect wind_pos;
   BPath path;
 //  int ref = 0;
   BRect defaultSize(50,50,650,350);
   bool Imgfilter = false;
   bool ThumbView = false;
   BMessage SPMessage;
   BMessage Color(B_PASTE);
   rgb_color c,sc;
   
   TPreferences prefs("ImageShow_prefs");
   if (prefs.InitCheck() != B_OK) {
      // New User!
      // Should we nag him?
      //(new BAlert(NULL,"ImageShow©\nLots code rewite here. Check out the new SplitPane slider bar.","Kewl"))->Go();
   }

   if(prefs.FindRect("window_pos", &wind_pos) != B_OK){
      wind_pos = defaultSize;
   }
   
   if(!wind_pos.Intersects(BScreen().Frame())){
      (new BAlert(NULL,"The window was somehow off the screen. We reset it position for you","Thanks"))->Go();
      theWin = new ImageShowWindow(defaultSize);
   }else{
      // this is the normal start up.
      theWin = new ImageShowWindow(wind_pos);
   }
   
   int32 ldm;
   if(prefs.FindInt32("draw_mode", &ldm) != B_OK){
      lastDrawMode = ASPECT; 
   }else{
      lastDrawMode = ldm;
   }
   if(prefs.FindBool("filter", &Imgfilter) != B_OK){
      Imgfilter = false;
   }
   if(prefs.FindBool("thumb", &ThumbView) != B_OK){
      ThumbView = false;
   }
   if(prefs.FindMessage("splitpane_state",&SPMessage) != B_OK){
   }	
   if(prefs.FindInt8("appcolorR", (int8*)&c.red) != B_OK){
      c.red = 216;
   }
   if(prefs.FindInt8("appcolorG", (int8*)&c.green) != B_OK){
      c.green = 216;
   }
   if(prefs.FindInt8("appcolorB", (int8*)&c.blue) != B_OK){
      c.blue = 216;
   }

   if(prefs.FindInt8("showercolorR", (int8*)&sc.red) != B_OK){
      sc.red = 0;
   }
   if(prefs.FindInt8("showercolorG", (int8*)&sc.green) != B_OK){
      sc.green = 0;
   }
   if(prefs.FindInt8("showercolorB", (int8*)&sc.blue) != B_OK){
      sc.blue = 0;
   }
   
   if(!SPMessage.IsEmpty()){  //&& (SPMessage->what == SPLITPANE_STATE)
      theWin->PostMessage(&SPMessage);
   }

   Color.AddData("RGBColor",B_RGB_COLOR_TYPE,&c,sizeof(rgb_color));
   theWin->PostMessage(&Color);      

   Color.MakeEmpty();
   Color.what = SHOWER_COLOR;
   Color.AddData("RGBColor",B_RGB_COLOR_TYPE,&sc,sizeof(rgb_color));
   theWin->PostMessage(&Color);      

   
   if((lastDrawMode == WINDOW_FIT) ||
      (lastDrawMode == ASPECT)     ||
      (lastDrawMode == OLD_ASPECT) ||
      (lastDrawMode == ACTUAL_SIZE)||
      (lastDrawMode == STRETCH)    ||
      (lastDrawMode == TILE)       ||
      (lastDrawMode == FULLSCREEN)){ 
   }else{
      lastDrawMode = ASPECT;
   }
   theWin->PostMessage(new BMessage(lastDrawMode));
   theWin->FilterImages(Imgfilter);
   theWin->ViewThumbs(ThumbView);
  // close(ref);

}

/*******************************************************
*   This is our Hook for when the Programe is started
*   by double clicking on a file we support (ie jpg, png)
*******************************************************/
void ImageShow::RefsReceived(BMessage *message) {
   entry_ref ref; // The entry_ref to open
   status_t err; // The error code
   int32 ref_num; // The index into the ref list
   BPath path;
   // Loop through the ref list and open each one
   ref_num = 0;
   //do {
      if ((err = message->FindRef("refs", ref_num, &ref)) != B_OK) {
         return;
      }
      BEntry entry(&ref, true);
      if(entry.GetPath(&path)==B_OK){
         if(entry.IsFile()){      
            BMessage *msg = new BMessage(CHANGE_IMAGE);
            if(msg->AddFlat("imgpath",&path) != B_OK){ /*some error*/}else{
               theWin->Lock();
               theWin->MessageReceived(msg);
               theWin->Unlock();
            }
            BMessage *msg2 = new BMessage(REFS_CHANGE_DIR);
            path.GetParent(&path);
            
            if(msg2->AddFlat("imgpath",&path) != B_OK){ /*some error*/}else{
               theWin->MessageReceived(msg2);
            }
         }   
      }
   //ref_num++;
   //} while (1);
   
   //Bring to forground!
   if(theWin->IsMinimized()){ theWin->Minimize(false); }
   theWin->Activate(true);
}

/*******************************************************
*
*******************************************************/
void ImageShow::AboutRequested(){
   uint32 mask;
   mask = modifiers();
   if(B_SHIFT_KEY & mask){
      (new BAlert("About ImageShow",
      "ImageShow ©\n\nIf you spent the time to find this:\n\tA. You need a hobbie\n\tB. Your finger slipped\n\tC. You really want to register this app!",
      " A "," B ","  C  ",B_WIDTH_AS_USUAL,B_IDEA_ALERT))->Go();
   }else{
      (new BAlert("About ImageShow","ImageShow ©1999\n\nTheAbstractCompany\nynop@acm.org\n\nVersion: 1.0 Beta 5.x","Thats Nice"))->Go();
   }
}

/*******************************************************
*
*******************************************************/
void ImageShow::MessageReceived(BMessage *msg){
   switch(msg->what){
   case B_NODE_MONITOR:
      theWin->MessageReceived(msg);
      break;
   case I_AM_DIEING_AHH:
      be_app->Lock();
      be_app->Quit();
      be_app->Unlock();
      break;
   default:
      BApplication::MessageReceived(msg);
      break; 
   }  
}

/*******************************************************
*
*******************************************************/
bool ImageShow::QuitRequested(){
   theWin->Lock();
   if(theWin->QuitRequested()){
      theWin->Unlock(); 
      return true; // Ok .. fine .. leave then
   }else{
      theWin->Unlock();
      return false;
   }
}




