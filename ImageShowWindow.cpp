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
#include <TranslationKit.h>
#include <TranslationUtils.h>

//#include <stdio.h>

#include "Globals.h"
#include "ImageShowWindow.h"
#include "ImageShowView.h"
#include "TPreferences.h"
#include "Addons/ImageFilter.h"

/*******************************************************
*   Our wonderful BWindow, ya its kewl like that.
*   we dont do much here but set up the menubar and 
*   let the view take over.  We also nead some message
*   redirection and handling
*******************************************************/
ImageShowWindow::ImageShowWindow(BRect frame) : BWindow(frame,"ImageShow",B_DOCUMENT_WINDOW,B_ASYNCHRONOUS_CONTROLS){//B_NOT_ANCHORED_ON_ACTIVATE|
   BRect r;
      
   Maximized = false;
   lastDrawMode = ASPECT;
   AppColor.red = 216; AppColor.green = 216; AppColor.blue = 216;
   ShowerColor.red = 0; ShowerColor.green = 0; ShowerColor.blue = 0;
   
   SetPulseRate(800);

   r = Bounds();
   // Creat a standard menubar
   menubar = new BMenuBar(r, "MenuBar");
   // Standard File menu
   menu = new BMenu("File");
   menu->AddItem(item=new BMenuItem("Save As", new BMessage(SAVE_IMAGE_AS),'S'));
   menu->AddItem(new BSeparatorItem());
   menu->AddItem(item=new BMenuItem("About ImageShow", new BMessage(B_ABOUT_REQUESTED), 'A'));
   item->SetTarget(be_app);
   menu->AddItem(new BSeparatorItem());
   menu->AddItem(item=new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));
   // add File menu now
   menubar->AddItem(menu);
   
   // EDIT MENU
   menu = new BMenu("Edit");
   menu->AddItem(item = new BMenuItem("Cut",new BMessage(NO_FUNCTION_YET)));
   menu->AddItem(item = new BMenuItem("Copy",new BMessage(NO_FUNCTION_YET)));
   menu->AddItem(item = new BMenuItem("Past",new BMessage(NO_FUNCTION_YET)));
   menu->AddItem(new BSeparatorItem());
   menu->AddItem(item = new BMenuItem("Select All",new BMessage(NO_FUNCTION_YET)));
   menu->AddItem(new BSeparatorItem());
   //filter menu
   filtermenu = new BMenu("Filters");
   
  
   /// do filter menu
   //MakeMenu();
   resume_thread(spawn_thread(DoFilterMenu, "Makeing Filter Menu", B_NORMAL_PRIORITY, this));
  
   menu->AddItem(filtermenu);
   menubar->AddItem(menu);
   
   //OPTIONS
   menu = new BMenu("Options");
   //submenu Modes
   modemenu = new BMenu("Mode");
   modemenu->AddItem(item = new BMenuItem("Actual Size",new BMessage(ACTUAL_SIZE),'C'));
   //item->SetTarget(View);
   modemenu->AddItem(item = new BMenuItem("New Aspect Ratio",new BMessage(ASPECT),'R'));
   modemenu->AddItem(item = new BMenuItem("Aspect Ratio",new BMessage(OLD_ASPECT),'P'));
   //item->SetTarget(View);
   modemenu->AddItem(item = new BMenuItem("Stretch",new BMessage(STRETCH),'E'));
   //item->SetTarget(View);
   modemenu->AddItem(item = new BMenuItem("Resize Win to fit",new BMessage(WINDOW_FIT),'Z'));
   //item->SetTarget(View);
   modemenu->AddItem(item = new BMenuItem("Tile image",new BMessage(TILE),'T'));
   //item->SetTarget(View);
   menu->AddItem(modemenu);
   //submenu Sort By
   submenu = new BMenu("Sort By");
   submenu->AddItem(item = new BMenuItem("Type",new BMessage(NO_FUNCTION_YET)));
   submenu->AddItem(item = new BMenuItem("Alpha",new BMessage(NO_FUNCTION_YET)));
   submenu->AddItem(item = new BMenuItem("Size",new BMessage(NO_FUNCTION_YET)));
   menu->AddItem(submenu); 
   menu->AddItem(filter_item = new BMenuItem("Only show image files",new BMessage(FILTER_IMG)));
   menu->AddItem(thumb_item  = new BMenuItem("Show Thumbnails",new BMessage(THUMB_VIEW)));
   //submenu Background
   submenu = new BMenu("Set as Background");
   submenu->AddItem(item = new BMenuItem("Centered",new BMessage(SET_BG_CENTER)));
   submenu->AddItem(item = new BMenuItem("Scaled",new BMessage(SET_BG_SCALED)));
   submenu->AddItem(item = new BMenuItem("Tiled",new BMessage(SET_BG_TILED)));
   menu->AddItem(submenu);
   menu->AddItem(item = new BMenuItem("Toggle Full Screen",new BMessage(FULLSCREEN),'F'));
   //item->SetTarget(View);
   
   menubar->AddItem(menu);

   // Niffty view menu
   menu = new BMenu("View");
   menu->AddItem(item = new BMenuItem("Next Image",new BMessage(NEXT_IMAGE)));//,B_DOWN_ARROW
   menu->AddItem(item = new BMenuItem("Prev Image",new BMessage(PREV_IMAGE)));//,B_UP_ARROW
   menu->AddItem(item = new BMenuItem("First Image",new BMessage(FIRST_IMAGE)));//,B_PAGE_UP
   menu->AddItem(item = new BMenuItem("Last Image",new BMessage(LAST_IMAGE)));//,B_PAGE_DOWN
   menu->AddItem(new BSeparatorItem());
   menu->AddItem(item = new BMenuItem("ShowSlide of this directory",new BMessage(DO_SHOWSLIDE)));
   //item->SetTarget(View);
   // add View menu
   menubar->AddItem(menu);
   
   // Attach the menu bar to he window
   AddChild(menubar);
   
   // Do a little claculating to take menubar into account
   r = Bounds();
   r.bottom = r.bottom - menubar->Bounds().bottom;

   View = new ImageShowView(this,r);
   
   View->MoveBy(0, menubar->Bounds().Height() + 1);
   AddChild(View);

  // BRect showslideframe(20,20,200,300);
   //showslideframe.left = ConvertToScreen(Bounds().left);
   //((Bounds()).left+20,(Bounds()).top+20,230,300);
 /*  BWindow *blah = new BWindow(showslideframe,"ShowSlide",
         B_FLOATING_WINDOW,
	     B_NOT_ZOOMABLE);//B_NOT_CLOSABLE|B_NOT_RESIZABLE|B_WILL_ACCEPT_FIRST_CLICK
   blah->Show(); 
 */
   lastSize = Frame();
   savepanel = new TranslatorSavePanel("TranslatorSavePanel", new BMessenger(this), NULL, 0, false, new BMessage(SAVE_IMAGE_SPESIFIC));

  // ListOptionsWin = new BWindow(BRect(10,10,100,100),"Options",B_TITLED_WINDOW,B_ASYNCHRONOUS_CONTROLS|B_NOT_RESIZABLE);

   Show();
}


/*******************************************************
*
*******************************************************/
int32 ImageShowWindow::MakeMenu(){
   app_info ai;
   be_app->GetAppInfo(&ai);
   BEntry entry(&ai.ref);
   BPath path;
   entry.GetPath(&path);
   path.GetParent(&path);
   path.Append("Addons");
   BDirectory directory(path.Path());

   while (directory.GetNextEntry(&entry, true) == B_OK) {
      entry.GetPath(&path);
      image_id image = load_add_on(path.Path());
      //if (image < 0) continue;
      ImageFilter *(*instantiate_filter)();
      if (get_image_symbol(image, "instantiate_filter", B_SYMBOL_TYPE_TEXT,
         (void **)&instantiate_filter) != B_OK) {
        // printf("get_image_symbol failed for %s\n", path.Path());
         continue;
      }
      ImageFilter *filter = (*instantiate_filter)();
      if (filter != NULL) {
         BMessage *m = new BMessage(RUN_FILTER);
         m->AddInt32("filter_id", (int32)filter->GetId());
         BMenuItem *menuitem = new BMenuItem(filter->GetName(), m, 0, 0);
         filtermenu->AddItem(menuitem);
         //testview->AddFilter(filter);
      }
   }
   return B_OK;
}


/*******************************************************
*
*******************************************************/
void ImageShowWindow::FrameResized(float,float){
}

/*******************************************************
*   Set weather we are filtering just images in the 
*   Selection box or we are displaying all files.
*   We also update the menu Item to reflect current state
*******************************************************/
void ImageShowWindow::FilterImages(bool b){
   ImgFilter = b;
   BMessage *msg = new BMessage(FILTER_IMG);
   msg->AddBool("filter",ImgFilter);
   Lock();
   View->MessageReceived(msg);
   Unlock();
   filter_item->SetMarked(ImgFilter);
}

/*******************************************************
*   
*******************************************************/
void ImageShowWindow::ViewThumbs(bool b){
   ViewThumb = b;
   BMessage *msg = new BMessage(THUMB_VIEW);
   msg->AddBool("thumbview",ViewThumb);
   Lock();
   View->MessageReceived(msg);
   Unlock();
   thumb_item->SetMarked(ViewThumb);
}

/*******************************************************
*   To make things a little nicer and more organized
*   we are gona let View be the message handler for 
*   the whole app. All messages that are ours send to
*   View for redistribution.  But we must handle our
*   own BWindow messages or else (crash)
*******************************************************/
void ImageShowWindow::MessageReceived(BMessage* msg){
   float x,y;
   BRect screenSize;
   BScreen screen(B_MAIN_SCREEN_ID);
   uint32 mask;
   BPath path;
   BMessage *tmp_msg;
   BEntry entry;
   entry_ref ref;

   switch(msg->what){
   case B_PASTE:
   case SHOWER_COLOR:
      View->MessageReceived(msg);
      break;
   case SPLITPANE_STATE:
      View->MessageReceived(msg);  
      break;
   case B_KEY_DOWN: // Pass ALL key messages down to the view
      View->MessageReceived(msg);
      break;
   case DO_SHOWSLIDE:
   case KILL_SHOWSLIDE:
      View->MessageReceived(msg);
      break;
   case SAVE_IMAGE_AS:
      Lock();
      savepanel->Show();
      Unlock();
      break;
   // pass almost all onto the View
   case CHANGE_DIR:
   case REFS_CHANGE_DIR:
   case SAVE_IMAGE_SPESIFIC:
   case GET_IMG_INFO:
   case GET_FILE_INFO:
   case SET_BG_CENTER:
   case SET_BG_SCALED:
   case SET_BG_TILED:
   case NEXT_IMAGE:
   case PREV_IMAGE:
   case FIRST_IMAGE:
   case LAST_IMAGE:
   case LIST_SELECT:
   case IMAG_SELECT:
   case DELETE_ENTRY:
   case RENAME_ENTRY:
   case MOVE_ENTRY:
   case RENAMED:
   case RUN_FILTER:
      View->MessageReceived(msg);
      break;
   case FILTER_IMG:
      ImgFilter = !ImgFilter;
      FilterImages(ImgFilter);
      break;
   case THUMB_VIEW:
      ViewThumb = !ViewThumb;
      ViewThumbs(ViewThumb);
      break;
   case IMAGE_SIZE:
      //View->Looper()->PostMessage(msg);
      View->MessageReceived(msg); // IF you comment this line it will go to 116
      break;
   case CHANGE_IMAGE:  
      //if(msg->FindFlat("imgpath",&path) != B_OK){ 
      View->MessageReceived(msg);
      break; 
   case WINDOW_FIT:
   case ASPECT:
   case OLD_ASPECT:
   case ACTUAL_SIZE:
   case STRETCH:
   case FULLSCREEN:
   case TILE:
      //modemenu->FindMarked()
      item = modemenu->FindItem(lastDrawMode);
      if(item){
         item->SetMarked(false);
      }else{
         //(new BAlert(NULL,"",""))->Go();
      }
      if(msg->what == FULLSCREEN){
         // Done not need to save as the 
         // draw mode has not changed.
      }else{
         lastDrawMode = msg->what;
      }
      item = modemenu->FindItem(lastDrawMode);
      if(item){
         item->SetMarked(true);
     }
      View->MessageReceived(msg);
      break;
   case RESIZE_TO_IMAGE:  // someone image wants us to change sizes
      msg->FindFloat("offx",&x);
      msg->FindFloat("offy",&y);
      this->ResizeBy(x,y);
      break;
   case MAX_SCREEN:
      if(Maximized){
         BRect tempRect = ConvertFromScreen(lastSize);
         this->MoveTo(lastSize.left,lastSize.top);
         this->ResizeTo(tempRect.right - lastSize.left,tempRect.bottom - lastSize.top);
      }else{
         mask = modifiers();
         if(B_SHIFT_KEY & mask){
            //Hide();
         }else{
            lastSize = ConvertToScreen(Bounds());//ConvertToScreen(Bounds());
            screenSize = screen.Frame();
            this->MoveTo(0,0);
            this->ResizeTo(screenSize.right,screenSize.bottom);
         }
      }
 	  Maximized = !Maximized;
 	  break;
   case B_SIMPLE_DATA: // DRAG-N-DROP
      // Look for a ref in the message
      if(msg->FindRef("refs", &ref) == B_OK){
         entry.SetTo(&ref, true);
         if(entry.IsDirectory() && entry.GetPath(&path)==B_OK){
            tmp_msg = new BMessage(REFS_CHANGE_DIR);
            tmp_msg->AddFlat("imgpath",&path);
            View->MessageReceived(tmp_msg);
         }else{
         
         }
      }     
      break;
   case NO_FUNCTION_YET:
      (new BAlert(NULL,"This is not implemented yet.\n\nIf it is something you realy want to see added in, mail me at ynop@acm.org and I will put more time into it :)","Bummer!"))->Go();
      break;
   case B_NODE_MONITOR:
      View->MessageReceived(msg);
      break;
   default:
      BWindow::MessageReceived(msg);
   }
}

/*******************************************************
*   Someone asked us nicely to quit. I gess we should
*   so clean up. save our setings (position size of win)
*   and tell the main be_app to shut it all down .. bye
*******************************************************/
bool ImageShowWindow::QuitRequested(){
   BMessage SPMessage;
   BPath path;
   
   TPreferences prefs("ImageShow_prefs");
   if (prefs.InitCheck() != B_OK) {
   }
   BRect frm = ConvertToScreen(Bounds());
   if(!Maximized){
      prefs.SetRect("window_pos", frm);
   }else{
       prefs.SetRect("window_pos", lastSize);
   }
   prefs.SetInt32("draw_mode", lastDrawMode);
   prefs.SetBool("filter", ImgFilter);
   prefs.SetBool("thumb", ViewThumb);
 
   SPMessage = *View->GetSplitPaneState();
   prefs.SetMessage("splitpane_state",&SPMessage);

//   printf("Write color %i, %i, %i\n",AppColor.red,AppColor.green,AppColor.blue);
   
   prefs.SetInt8("appcolorR",(int8)AppColor.red);
   prefs.SetInt8("appcolorG",(int8)AppColor.green);
   prefs.SetInt8("appcolorB",(int8)AppColor.blue);

   prefs.SetInt8("showercolorR",(int8)ShowerColor.red);
   prefs.SetInt8("showercolorG",(int8)ShowerColor.green);
   prefs.SetInt8("showercolorB",(int8)ShowerColor.blue);

   be_app->PostMessage(I_AM_DIEING_AHH);
   // delete SPMessage;
   return true;
}
