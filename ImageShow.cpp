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
#include <InterfaceDefs.h>

#include <stdio.h>

#include "Globals.h"
#include "ImageShow.h"
#include "ImageShowWindow.h"
#include "YPreferences.h"
#include "YLanguageClass.h"
//#include "AboutBox.h"
#include "BugOutDef.h"

extern BugOut db;

/*******************************************************
*
*******************************************************/
ImageShow::ImageShow() : BApplication(APP_SIGNATURE){
   BRect wind_pos;
   BPath path;
   BRect defaultSize(50,50,650,350);
   bool Imgfilter = false;
   bool ThumbView = false;
   bool IsToolbarHidden = false;
   BMessage SPMessage;
   BMessage Color(B_PASTE);
   rgb_color c,sc;
   int32 thumbsize;
   char* name;
   
   db.SendMessage("Starting up ImageShow Application");
   
   YPreferences prefs("ImageShow_prefs");
   if (prefs.InitCheck() != B_OK) {
      // New User!
      // Should we nag him?
      //(new BAlert(NULL,"ImageShow©\nLots code rewite here. Check out the new SplitPane slider bar.","Kewl"))->Go();
   }

   if(prefs.FindString("Language_Name",(const char**)&name) == B_OK){
      Language.SetName(name);
   }else{
      Language.SetName("English");
   }
   
   if(prefs.FindRect("window_pos", &wind_pos) != B_OK){
      wind_pos = defaultSize;
   }
   
   if(!wind_pos.Intersects(BScreen().Frame())){
      (new BAlert(NULL,Language.get("WIN_OUT_OF_BOUNDS"),Language.get("OK")))->Go();
      theWin = new ImageShowWindow(defaultSize);
   }else{
      // this is the normal start up.
      theWin = new ImageShowWindow(wind_pos);
   }
   
   db.SendMessage("GUI Created");
   
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
      c.red = ui_color(B_PANEL_BACKGROUND_COLOR).red;
   }
   if(prefs.FindInt8("appcolorG", (int8*)&c.green) != B_OK){
      c.green = ui_color(B_PANEL_BACKGROUND_COLOR).green;
   }
   if(prefs.FindInt8("appcolorB", (int8*)&c.blue) != B_OK){
      c.blue = ui_color(B_PANEL_BACKGROUND_COLOR).blue;
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
   if(prefs.FindInt32("ThumbSize", &thumbsize) != B_OK){
      thumbsize = 32;
   }
   if(prefs.FindBool("toolbarhidden",&IsToolbarHidden) == B_OK){
      if(IsToolbarHidden){
         theWin->PostMessage(new BMessage(HIDE_TOOLBAR));
      }else{
         theWin->PostMessage(new BMessage(SHOW_TOOLBAR));
      }
   }
   int32 speed;
   if(prefs.FindInt32("ShowSlidSpeed",&speed) == B_OK){
      BMessage sss(SLIDE_SPEED);
      sss.AddInt32("ShowSlideSpeed",speed);
      theWin->PostMessage(&sss);
   }
   
   db.SendMessage("prefs read in");
   
   if(!SPMessage.IsEmpty()){  //&& (SPMessage->what == SPLITPANE_STATE)
      theWin->PostMessage(&SPMessage);
   }
   
   BMessage tsize(THUMB_SIZE);
   tsize.AddInt32("IconSize",thumbsize);
   theWin->PostMessage(&tsize);

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
    
   db.SendMessage("Finished with app Consruction");

}

/*******************************************************
*   Do this once everone is ready to rock and roll
*******************************************************/
void ImageShow::ReadyToRun(){
   db.SendMessage("Ready to run called");
   //theWin->Show();
   // this will work after I find bug with toolbar
}

/*******************************************************
*   This is our Hook for when the Programe is started
*   by double clicking on a file we support (ie jpg, png)
*******************************************************/
void ImageShow::RefsReceived(BMessage *message) {
   db.SendMessage("RefsReceived called");
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
void ImageShow::ArgvReceived(int32 argc, char** argv){
   char *buff = NULL;
   int32 count = 1;
   BString *str = NULL;
   
   while(count < argc){
      buff = argv[count];
      if(!strcmp("--help",buff) || !strcmp("-h",buff)){
         printf("usage: %s [OPTION] imagePath\n",argv[0]);
         printf("  imagePath\t\tPath to a spesific image/dir\n");
         printf("  -l,-list\t\tA list of file to populate filelist\n");
         printf("  -s,-show\t\tDo a slide show once loaded\n");
         printf("  --help\n");
      }else if(!strcmp("-l",buff) || !strcmp("-list",buff)){
         // everthing after the -l is a list item
         count++;
         BList *list = new BList();
         while(count < argc){
            str = new BString(argv[count]);
            list->AddItem(str);
            count++;
         }
         // now make a message out of it :)
         BMessage msg(MAKE_LIST_LIST);
         msg.AddPointer("path_list_pointer",list);
         theWin->PostMessage(&msg);
         break;
      }else{  // I gess it is a path
         BPath path;
         BEntry entry;
         entry.SetTo(argv[count],true);
         if(entry.GetPath(&path)==B_OK){
            if(entry.IsFile()){
               BMessage *msg = new BMessage(CHANGE_IMAGE);
               if(msg->AddFlat("imgpath",&path) != B_OK){ }else{
                  theWin->Lock();
                  theWin->MessageReceived(msg);
                  theWin->Unlock();
               }
               BMessage *msg2 = new BMessage(REFS_CHANGE_DIR);
               path.GetParent(&path);
            
               if(msg2->AddFlat("imgpath",&path) != B_OK){ }else{
                  theWin->MessageReceived(msg2);
               }
            }
         }
      }
      count++;
   }

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
      theWin->PostMessage(new BMessage(SHOW_LOGO));
   }else if(B_CONTROL_KEY & mask){
      (new BAlert(NULL,
      "ImageShow "B_UTF8_COPYRIGHT"1999,2000\n\nIf you spent the time to find this:\n\tA. You need a hobbie\n\tB. Your finger slipped\n\tC. You really want to register this app!",
      " A "," B ","  C  ",B_WIDTH_AS_USUAL,B_IDEA_ALERT))->Go();
   }else if(B_COMMAND_KEY & mask){
    //  new AboutBox("Inferno ©1999,2000\tTheAbstractCompany\nVersion: 1.0\t\t\t\tynop@acm.org","Splash");
   }else{
      (new BAlert("About ImageShow","ImageShow "B_UTF8_COPYRIGHT"1999,2000\n\nTheAbstractCompany\nynop@acm.org\n\nVersion: 2.x","Thats Nice"))->Go();
   }
}

/*******************************************************
*
*******************************************************/
void ImageShow::MessageReceived(BMessage *msg){
   switch(msg->what){
   case RUN_FILTER:
      theWin->PostMessage(msg);
      break;
   case B_KEY_DOWN:
      theWin->PostMessage(msg);
      break;
   case B_NODE_MONITOR:
      db.SendMessage("Node monitoring");
      theWin->PostMessage(msg);
      break;
   case I_AM_DIEING_AHH:
      db.SendMessage("I am dieing ...");
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




