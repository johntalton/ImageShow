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
#include <FindDirectory.h>

#include <stdio.h>

#include "Globals.h"
#include "SP_Globals.h"
#include "ImageShowWindow.h"
#include "OptionsWindow.h"
#include "SlideShowWin.h"
#include "ImageShowView.h"
#include "YPreferences.h"
//#include "Addons/ImageFilter.h"
#include "YLanguageClass.h"
#include "BugOutDef.h"
#ifdef BUILD_INF_SUPPORT
#include "InfernoFilter.h"
#include "InfernoCommon.h"
#include "InfernoDocument.h"
#endif

extern BugOut db;

#define DO_FILTER RUN_FILTER
#ifdef BUILD_INF_SUPPORT
typedef InfernoFilter*    (*IFilterFunc) (BMessage*,InfernoCommon*,InfernoDocument*);
#endif

/*******************************************************
*   Our wonderful BWindow, ya its kewl like that.
*   we dont do much here but set up the menubar and 
*   let the view take over.  We also nead some message
*   redirection and handling
*******************************************************/
ImageShowWindow::ImageShowWindow(BRect frame) : BWindow(frame,"ImageShow",B_DOCUMENT_WINDOW,B_ASYNCHRONOUS_CONTROLS){//B_NOT_ANCHORED_ON_ACTIVATE|
   BRect r;
   db.SendMessage("Windows starting up");
   Maximized = false;
   lastDrawMode = ASPECT;
   ThumbSize = 32;
   ssspeed = 2;
   IsToolBarHidden = false;
   AppColor.red = ui_color(B_PANEL_BACKGROUND_COLOR).red;
   AppColor.green = ui_color(B_PANEL_BACKGROUND_COLOR).green;
   AppColor.blue = ui_color(B_PANEL_BACKGROUND_COLOR).blue;
   // AppColor.red =  ui_color(B_MENU_BACKGROUND_COLOR).red;
   //AppColor.green =  ui_color(B_MENU_BACKGROUND_COLOR).green;
   //AppColor.blue =  ui_color(B_MENU_BACKGROUND_COLOR).blue;

   ShowerColor.red = 0; ShowerColor.green = 0; ShowerColor.blue = 0;
   
   SetPulseRate(800);
   
   PrinterSettings = NULL;

   MakeMainMenu();
   
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

   ListOptionsWin = new OptionsWindow(this);
   SSOpWin = new SlideShowWin(this);
   Show();
   //Run(); // this makes more sence but toolbar has problems
}

/*******************************************************
*
*******************************************************/
void ImageShowWindow::MakeMainMenu(){
   db.SendMessage("Createing Main Menu");
   BRect r;
   r = Bounds();
   // Creat a standard menubar
   menubar = new BMenuBar(r, "MenuBar");
   // Standard File menu
   menu = new BMenu(Language.get("FILE"));
   menu->AddItem(item=new BMenuItem(Language.get("SAVE_AS"), new BMessage(SAVE_IMAGE_AS),'S'));
   menu->AddItem(item=new BMenuItem(Language.get("PRINT_SETUP"), new BMessage(PRINT_SETUP),'P',B_SHIFT_KEY));
   menu->AddItem(item=new BMenuItem(Language.get("PRINT"), new BMessage(PRINT),'P'));
   menu->AddItem(new BSeparatorItem());
   menu->AddItem(item=new BMenuItem(Language.get("ABOUT"), new BMessage(B_ABOUT_REQUESTED), 'A'));
   item->SetTarget(be_app);
   menu->AddItem(new BSeparatorItem());
   menu->AddItem(item=new BMenuItem(Language.get("QUIT"), new BMessage(B_QUIT_REQUESTED), 'Q'));
   // add File menu now
   menubar->AddItem(menu);
   
   // EDIT MENU
   menu = new BMenu(Language.get("EDIT"));
   menu->AddItem(undo_item = new BMenuItem(Language.get("UNDO"),new BMessage(UNDO),'U'));
   undo_item->SetEnabled(false);
   menu->AddItem(new BSeparatorItem());
   menu->AddItem(item = new BMenuItem(Language.get("CUT"),new BMessage(B_CUT),'X'));
   menu->AddItem(item = new BMenuItem(Language.get("COPY"),new BMessage(B_COPY),'C'));
   menu->AddItem(item = new BMenuItem(Language.get("PASTE"),new BMessage(B_PASTE),'V'));
   //menu->AddItem(item = new BMenuItem("Clear",new BMessage(CLEAR)));
   menu->AddItem(new BSeparatorItem());
   menu->AddItem(item = new BMenuItem(Language.get("SELECT_ALL"),new BMessage(B_SELECT_ALL),'A'));
   menu->AddItem(new BSeparatorItem());
   //filter menu
   filtermenu = new BMenu(Language.get("FILTERS"));
   menu->AddItem(filtermenu);
   menubar->AddItem(menu);
  
   /// do filter menu
   //MakeMenu();
   resume_thread(spawn_thread(DoFilterMenu, "Makeing Filter Menu", B_NORMAL_PRIORITY, this));
   
   //OPTIONS
   menu = new BMenu(Language.get("OPTIONS"));
   //submenu Modes
   modemenu = new BMenu(Language.get("MODE"));
   modemenu->AddItem(item = new BMenuItem(Language.get("ACTUAL_SIZE"),new BMessage(ACTUAL_SIZE),'1'));
   //item->SetTarget(View);
   modemenu->AddItem(item = new BMenuItem(Language.get("NEW_ASPECT"),new BMessage(ASPECT),'2'));
   modemenu->AddItem(item = new BMenuItem(Language.get("ASPECT"),new BMessage(OLD_ASPECT),'3'));
   //item->SetTarget(View);
   modemenu->AddItem(item = new BMenuItem(Language.get("STRETCH"),new BMessage(STRETCH),'4'));
   //item->SetTarget(View);
   modemenu->AddItem(item = new BMenuItem(Language.get("RESIZE_TO_FIT"),new BMessage(WINDOW_FIT),'5'));
   //item->SetTarget(View);
   modemenu->AddItem(item = new BMenuItem(Language.get("TILE"),new BMessage(TILE),'6'));
   //item->SetTarget(View);
   menu->AddItem(modemenu);
   //submenu Sort By
   submenu = new BMenu(Language.get("SORT_BY"));
   submenu->AddItem(item = new BMenuItem(Language.get("TYPE"),new BMessage(NO_FUNCTION_YET)));
   submenu->AddItem(item = new BMenuItem(Language.get("ALPHA"),new BMessage(NO_FUNCTION_YET)));
   submenu->AddItem(item = new BMenuItem(Language.get("SIZE"),new BMessage(NO_FUNCTION_YET)));
   menu->AddItem(submenu); 
   //language menu
   langmenu = new BMenu(Language.get("LANGUAGE"));
   resume_thread(spawn_thread(DoLanguageMenu, "Loading Languages", B_NORMAL_PRIORITY, this));
   menu->AddItem(langmenu); 
   menu->AddItem(filter_item = new BMenuItem(Language.get("ONLY_SHOW_IMAGE"),new BMessage(FILTER_IMG)));
   menu->AddItem(thumb_item  = new BMenuItem(Language.get("SHOW_THUMB"),new BMessage(THUMB_VIEW)));
   menu->AddItem(new BMenuItem(Language.get("THUMB_OPTIONS"),new BMessage(LIST_OPTIONS)));
   menu->AddItem(new BMenuItem(Language.get("SLIDE_OPTIONS"),new BMessage(SLIDE_OPTIONS)));
   //submenu Background
   submenu = new BMenu(Language.get("SET_AS_BG"));
   submenu->AddItem(item = new BMenuItem(Language.get("CENTERED"),new BMessage(SET_BG_CENTER)));
   submenu->AddItem(item = new BMenuItem(Language.get("SCALED"),new BMessage(SET_BG_SCALED)));
   submenu->AddItem(item = new BMenuItem(Language.get("TILED"),new BMessage(SET_BG_TILED)));
   menu->AddItem(submenu);
   menu->AddItem(shtb = new BMenuItem(Language.get("HIDE_TOOLBAR"),new BMessage(HIDE_TOOLBAR)));
   menu->AddItem(item = new BMenuItem(Language.get("TOGGLE_FULL_SCREEN"),new BMessage(FULLSCREEN),'F'));
   //item->SetTarget(View);
   menubar->AddItem(menu);
   // Niffty view menu
   menu = new BMenu(Language.get("VIEW"));
   menu->AddItem(item = new BMenuItem(Language.get("NEXT_IMAGE"),new BMessage(NEXT_IMAGE)));//,B_DOWN_ARROW
   menu->AddItem(item = new BMenuItem(Language.get("PREV_IMAGE"),new BMessage(PREV_IMAGE)));//,B_UP_ARROW
   menu->AddItem(item = new BMenuItem(Language.get("FIRST_IMAGE"),new BMessage(FIRST_IMAGE)));//,B_PAGE_UP
   menu->AddItem(item = new BMenuItem(Language.get("LAST_IMAGE"),new BMessage(LAST_IMAGE)));//,B_PAGE_DOWN
   menu->AddItem(new BSeparatorItem());
   menu->AddItem(item = new BMenuItem(Language.get("SHOWSLIDE"),new BMessage(DO_SHOWSLIDE)));
   //item->SetTarget(View);
   // add View menu
   menubar->AddItem(menu);
   
   bmmenu = new BMenu(Language.get("BOOKMARKS"));
   bmmenu->AddItem(item = new BMenuItem(Language.get("ADD_BOOKMARK"),new BMessage(ADD_BOOKMARK_CURRENT)));
   bmmenu->AddItem(item = new BMenuItem(Language.get("SHOW_BOOKMARKS"),new BMessage(SHOW_BOOKMARKS)));
   bmmenu->AddItem(new BSeparatorItem());
   
   menubar->AddItem(bmmenu);
   /// do bookmark menu
   //MakeMenu();
   db.SendMessage("Spawning Bookmark Thread");
   resume_thread(spawn_thread(DoBookMarkMenu, "Loading Bookmarks", B_NORMAL_PRIORITY, this));
}

/*******************************************************
*
*******************************************************/
int32 ImageShowWindow::MakeMenu(){
   db.SendMessage("Makeing Addon Menu");
   app_info ai;
   be_app->GetAppInfo(&ai);
   BEntry entry(&ai.ref);
   BPath path;
   entry.GetPath(&path);
   path.GetParent(&path);
//   path.Append("Addons");
   path.Append("Filters");
   BDirectory directory(path.Path());

   while (directory.GetNextEntry(&entry, true) == B_OK) {
      //entry.GetPath(&path);
      //image_id image = load_add_on(path.Path());
      //if (image < 0) continue;
      
      /*ImageFilter *(*instantiate_filter)();
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
      }*/
      
      #ifdef BUILD_INF_SUPPORT
      AddFilter(entry);
      #endif
      
   }
   db.SendMessage("Finished addon menu");
   
   #ifdef BUILD_INF_SUPPORT
   BuildFilterMenu(filtermenu);
   #endif
   
   return B_OK;
}

/*******************************************************
*   
*******************************************************/
#ifdef BUILD_INF_SUPPORT
status_t ImageShowWindow::AddFilter(BEntry entry){
//   if(expired){ return B_ERROR; }

   BPath path;
   char name[B_FILE_NAME_LENGTH];
   
   entry.GetPath(&path);
   entry.GetName(name);
   
   printf("%s\n",path.Path());
   
   image_id image = load_add_on(path.Path());
   if(image < 0){
      printf("oops - %i\n",(int)image);
      return B_ERROR;
   }
      
   int32 *ver = NULL;
   if(get_image_symbol(image, "FILTER_API_VERSION", B_SYMBOL_TYPE_ANY,(void **)&ver) != B_OK){
      printf("cand find ver\n");
      unload_add_on(image);
      return B_ERROR;
   }
   if(*ver != FILTER_API_VER){
      printf("bad ver\n");
      unload_add_on(image);
      return B_ERROR;
   }
  printf("here\n");
   
   char *cat;
   if(get_image_symbol(image, "FILTER_CATAGORY", B_SYMBOL_TYPE_DATA,(void **)&cat) != B_OK){
      // Damn error but we handle it :)
      cat = NULL;
   }
   char *desc;
   if(get_image_symbol(image, "FILTER_DESCRIPTION", B_SYMBOL_TYPE_DATA,(void **)&desc) != B_OK){
      // Damn error but we handle it :)
      desc = NULL;
   }
   
   filter_entry *fe = new filter_entry;
   fe->name = new char[strlen(name)+1]; strcpy(fe->name,name);
   if(cat && *cat){
      fe->catagory = new char[strlen(cat)+1]; strcpy(fe->catagory,cat);
   }else{
      fe->catagory = new char[strlen(Language.get("GENERAL_FILTER_MENU"))+1]; strcpy(fe->catagory,Language.get("GENERAL_FILTER_MENU"));
   }
   if(desc && *desc){
      fe->desc = new char[strlen(desc)+1]; strcpy(fe->desc,desc);
   }else{
      fe->desc = NULL;
   }
   entry.GetRef(&(fe->eref));
   
      
   unload_add_on(image);
   Filters.AddItem(fe);
   
   return B_OK;
}

/*******************************************************
*   
*******************************************************/
void ImageShowWindow::BuildFilterMenu(BMenu *fmenu){
   BMessage *msg = NULL;
   filter_entry *fe = NULL;
   BMenuItem *item = NULL;
   BMenu *nmenu = NULL;
   
   fmenu->AddItem(item = new BMenuItem("none",NULL));
   item->SetEnabled(false);
   fmenu->AddSeparatorItem();
   
   // Image menu and Color menu are defualt
   fmenu->AddItem(nmenu = new BMenu(COLOR_CATAGORY));
   fmenu->AddItem(nmenu = new BMenu(IMAGE_CATAGORY));
   fmenu->AddSeparatorItem();
   
   for(int32 i = 0; i < Filters.CountItems();i++){
      fe = (filter_entry*)Filters.ItemAt(i);
      if(fe){
         item = NULL;
         msg = new BMessage(DO_FILTER);
         msg->AddRef("Ifilter_ref",&(fe->eref));
         if(fe->catagory == NULL){
            // catagory was null
            // we should actaully add to general filter cat
            continue;
         }
         item = (fmenu->FindItem(fe->catagory));
         if(item){
            nmenu = item->Submenu();
            if(nmenu){
               nmenu->AddItem(item = new BMenuItem(fe->name,msg));
               item->SetTarget(be_app);
            }else{
               // we found the cat but it didn't have a sub menu.
               // what should we do
            }
         }else{
            fmenu->AddItem(nmenu = new BMenu(fe->catagory));
            nmenu->AddItem(item = new BMenuItem(fe->name,msg));
            item->SetTarget(be_app);
         }
      }
   }
}
/*******************************************************
*   
*******************************************************/
void ImageShowWindow::StartFilter(entry_ref ref){
   BPath path;
   BEntry entry(&ref,true);
   IFilterFunc instantiate_filter;
   
   entry.GetPath(&path);
   image_id image = load_add_on(path.Path());
   if(image == B_ERROR){
      return;
   }
   
   if(get_image_symbol(image, "instantiate_filter", B_SYMBOL_TYPE_TEXT,(void **)&instantiate_filter) != B_OK){
      unload_add_on(image);
      return;
   }
   
   
   InfernoDocument *IDoc = new InfernoDocument(0,0);
 //  InfernoLayer *ILay = new InfernoLayer(NULL,ICommon);
 //  IDoc->AddLayer(ILay);
   
   if(IDoc){
      BMessage msg;
      InfernoFilter *filter = (*instantiate_filter)(&msg,&ICommon,IDoc);
      BView *v = filter->GetSettingsView();
      if(v){
         //Run as a active windowed filter
         // Modal so it blocks
         /*if(fiterwin){
            fiterwin->Lock();
            fiterwin->Quit();
            fiterwin = NULL;
         }
         BEntry ent(&ref,true);
         char name[B_FILE_NAME_LENGTH];
         ent.GetName(name);
         fiterwin = new FilterConfigWindow(filter,image,v,name);*/
         (new BAlert(NULL,"ImageShow does not support Windowed filters","Ok"))->Go();
      }else{
         // Run Once filter
         filter->Go();
         //ICommon.UpdateCanvas(IDoc->Bounds());
         delete filter;
         unload_add_on(image); // Trash the memory now!! this is a must
      }
   }
   
   //unload_add_on(image); // Trash the memory now!! this is a must
}
#endif

/*******************************************************
*
*******************************************************/
int32 ImageShowWindow::MakeBookMarkMenu(){
   db.SendMessage("Making bookmark menu");
   app_info ai;
   be_app->GetAppInfo(&ai);
   BEntry entry(&ai.ref);
   BPath path;
   BMessage *m = NULL;
   entry.GetPath(&path);
   path.GetParent(&path);
   path.Append(BOOK_DIRECTORY);
   BDirectory directory(path.Path());

   char name[B_FILE_NAME_LENGTH];

   while (directory.GetNextEntry(&entry, false) == B_OK){
      if(entry.IsSymLink()){
         entry.GetPath(&path);
         entry.GetName(name);
         m = new BMessage(GO_TO_BOOKMARK);
         m->AddFlat("bmpath",&path);
         BMenuItem *menuitem = new BMenuItem(name, m, 0, 0);
         bmmenu->AddItem(menuitem);
      }else if(entry.IsDirectory()){
        //should do a recursive thingy like Net+
        // but that too much work
      }
   }
   
   // This is support for the Bookmark files that ABeSee uses
   // they are stored in "~/settings/aBeSee/Beloved Places"
   // So we should add them in I gess
   bool firsttime = true;
   find_directory(B_USER_SETTINGS_DIRECTORY, &path);// ~/settings
   path.Append(ABC_DIRECTORY);
   directory.SetTo(path.Path());
   while (directory.GetNextEntry(&entry, false) == B_OK){
      if(firsttime){
         firsttime = false;
         bmmenu->AddItem(new BSeparatorItem());
      }
      if(entry.IsSymLink()){
         entry.GetPath(&path);
         entry.GetName(name);
         m = new BMessage(GO_TO_BOOKMARK);
         m->AddFlat("bmpath",&path);
         BMenuItem *menuitem = new BMenuItem(name, m, 0, 0);
         bmmenu->AddItem(menuitem);
      }else if(entry.IsDirectory()){
        //should do a recursive thingy like Net+
        // but that too much work
      }
   }
   
   
   db.SendMessage("Finished bookmark menu");
   return B_OK;
}

/*******************************************************
*
*******************************************************/
int32 ImageShowWindow::MakeLanguageMenu(){
   db.SendMessage("Makeing Lang Menu");
   app_info ai;
   be_app->GetAppInfo(&ai);
   BEntry entry(&ai.ref);
   BPath path;
   entry.GetPath(&path);
   path.GetParent(&path);
   path.Append("Languages");
   BDirectory directory(path.Path());

   char name[B_FILE_NAME_LENGTH];

   while (directory.GetNextEntry(&entry, false) == B_OK){
      if(entry.IsFile()){
         entry.GetPath(&path);
         entry.GetName(name);
         BMessage *m = new BMessage(CHANGE_LANGUAGE);
         m->AddString("language",name);
         BMenuItem *menuitem = new BMenuItem(name, m, 0, 0);
         langmenu->AddItem(menuitem);
         if(!strcmp(name,Language.Name())){
            menuitem->SetMarked(true);
         }
      }else if(entry.IsDirectory()){
        //should do a recursive thingy like Net+
        // but that too much work
      }
   }
   db.SendMessage("Finished Lang menu");
   return B_OK;
}


/*******************************************************
*
*******************************************************/
void ImageShowWindow::AddBookmarkItem(BMenuItem *i){
   bmmenu->AddItem(i);
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
   int32 s;
   char *name;
   BMessenger tracker(TRACKER_MIME);

   switch(msg->what){
   case PRINT_SETUP:{
      status_t result;
      BPrintJob job("ImageShow PrintJob");
      if(PrinterSettings){
         job.SetSettings(new BMessage(*(PrinterSettings)));
      }
      
      // This pops up a niffty little window that 
      // lets the user config things
      result = job.ConfigPage();
      
      if(result == B_OK){
         PrinterSettings = job.Settings();
      }else{
         PrinterSettings = NULL;
      }
      }break;
   case PRINT:{
      status_t result = B_OK;
      BPrintJob job("ImageShow PrintJob");
      
      if(!PrinterSettings){
         result = job.ConfigPage();
         if(result == B_OK){
            PrinterSettings = job.Settings();
         }
      }
      
      if(result == B_OK){
         if(PrinterSettings){
            job.SetSettings(PrinterSettings);
            
            result = job.ConfigJob();
            if(result == B_OK){
               if(PrinterSettings){
                  //delete PrinterSettings;
                  PrinterSettings = NULL;
               }
               PrinterSettings = job.Settings();
               
               job.BeginJob();
               if(job.CanContinue()){
                  // PrintView *pv = printView(info);
                  // pv->SetUp();
                  //job.DrawView(pv, pv->Bounds(),BPoint(0,0));
                  //job.DrawView(View, View->Bounds(),BPoint(0,0));
                  BView *tmpV = View->GetIS();
                  if(tmpV){
                     BRect tmpR = tmpV->Bounds();
                     BRect pR = job.PrintableRect();
                     int32 x = (pR.IntegerWidth() - tmpR.IntegerWidth())/2;
                     if(x < 0){ x = 0; }
                     int32 y = (pR.IntegerHeight() - tmpR.IntegerHeight())/2;
                     if(y < 0){ y = 0; }
                     job.DrawView(tmpV, tmpR,BPoint(x,y));
                     job.SpoolPage();
                     job.CommitJob();
                  }else{
                     result = B_ERROR;
                  }
               }
            }else{
               result = B_ERROR;
            }
         }else{
            result = B_ERROR;
         }
      }
     
      if(result != B_OK){
         // Printer Error!!!!
      }
      
      }break;   
   case MAKE_LIST_LIST:
      View->MessageReceived(msg);
      break;
   case UNDO:
      View->MessageReceived(msg);
      break;
   case UNDO_TRUE:
      undo_item->SetEnabled(true);
      break;
   case UNDO_FALSE:
      undo_item->SetEnabled(false);
      break;
   case EDIT_QUERY:
      tmp_msg = new BMessage(TRACKER_FIND);
      tracker.SendMessage(tmp_msg);
      break;
   case CHANGE_LANGUAGE:
      if(msg->FindString("language",(const char**)&name) == B_OK){
         item = langmenu->FindItem(Language.Name());
         if(item){
            item->SetMarked(false);
         }else{
            //Some problems here
         }
         Language.SetName(name);
         item = langmenu->FindItem(Language.Name());
         if(item){
            item->SetMarked(true);
         }else{
            //Item is NULL not good
         }
         RemoveChild(menubar);
         delete menubar;
         //delete menu;
         //delete item;
         MakeMainMenu();
         AddChild(menubar);
         //remember to mark the selected items
         filter_item->SetMarked(ImgFilter);
         thumb_item->SetMarked(ViewThumb);
         item = modemenu->FindItem(lastDrawMode);
         if(item){
            item->SetMarked(true);
         }else{
            //
         }
         View->MessageReceived(msg);
         ListOptionsWin->PostMessage(msg);
         SSOpWin->PostMessage(msg);
      }
      break;
   case ADD_BOOKMARK:
   case ADD_BOOKMARK_CURRENT:
      View->MessageReceived(msg);
      break;
   case GO_TO_BOOKMARK:
      if(msg->FindFlat("bmpath",&path) == B_OK){
         tmp_msg = new BMessage(REFS_CHANGE_DIR);
         tmp_msg->AddFlat("imgpath",&path);
         PostMessage(tmp_msg);
      }else{
         // Cant find bookmard entry in message .. what?
         (new BAlert(NULL,Language.get("CANT_FIND_BM"),Language.get("OK")))->Go();
      }
      break;
   case SHOW_BOOKMARKS:
      View->MessageReceived(msg);
      break;
   case SHOW_TOOLBAR:
      shtb->SetLabel(Language.get("HIDE_TOOLBAR"));
      shtb->SetMessage(new BMessage(HIDE_TOOLBAR));
      View->MessageReceived(msg);
      IsToolBarHidden = false;
      break;
   case HIDE_TOOLBAR:
      shtb->SetLabel(Language.get("SHOW_TOOLBAR"));
      shtb->SetMessage(new BMessage(SHOW_TOOLBAR));
      View->MessageReceived(msg);
      IsToolBarHidden = true;
      break;
   case SLIDE_OPTIONS:
      if(SSOpWin->IsMinimized()){
         SSOpWin->Minimize(false);
      }
      SSOpWin->Lock();
      if(SSOpWin->IsHidden()){
         SSOpWin->Show();
      }
      SSOpWin->Unlock();
      SSOpWin->Activate(true);
      break;   
   case LIST_OPTIONS:
      if(ListOptionsWin->IsMinimized()){
         ListOptionsWin->Minimize(false);
      }
      ListOptionsWin->Lock();
      if(ListOptionsWin->IsHidden()){
         ListOptionsWin->Show();
      }
      ListOptionsWin->Unlock();
      ListOptionsWin->Activate(true);
      break;
   case THUMB_SIZE:
      if(msg->FindInt32("IconSize",&s) == B_OK){
         ThumbSize = s;
         View->MessageReceived(msg);
      }
      break;
   case B_CUT:
   case B_COPY:
   case B_SELECT_ALL:   
   case B_PASTE:
      View->MessageReceived(msg);
      break;
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
   case FILE_SET_BG_CENTER:
   case FILE_SET_BG_SCALED:
   case FILE_SET_BG_TILED:
   case NEXT_IMAGE:
   case PREV_IMAGE:
   case FIRST_IMAGE:
   case LAST_IMAGE:
   case LIST_SELECT:
   case IMAG_SELECT:
   case DELETE_ENTRY:
   case RENAME_ENTRY:
   case MOVE_ENTRY://
   case RENAMED:
   case REMOVE_DIR_THUMB:
   case REMOVE_FILE_THUMB:
   case WORKING:
   case FINISHED://
   case TRACKER_HERE:
   case TRACKER_THERE:
   case B_MOUSE_WHEEL_CHANGED:
      //printf("passing to view\n");
      View->MessageReceived(msg);
      break;
   case RUN_FILTER:
      if(msg->FindRef("Ifilter_ref",&ref) == B_OK){
         #ifdef BUILD_INF_SUPPORT
         StartFilter(ref);
         #endif
      }
      break;
   case SLIDE_SPEED:
      View->MessageReceived(msg);
      msg->FindInt32("ShowSlideSpeed",&ssspeed);
      break;
   case NULL_IMAGE:
      db.SendMessage("NULL Image ....");
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
         // item is NULL
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
         if(entry.GetPath(&path) != B_OK){
            break; // Something is wrong
         }
         if(entry.IsDirectory()){//
            tmp_msg = new BMessage(REFS_CHANGE_DIR);
            tmp_msg->AddFlat("imgpath",&path);
            View->MessageReceived(tmp_msg);
         }else{
           // its a sym link of file
           tmp_msg = new BMessage(CHANGE_IMAGE);
           tmp_msg->AddFlat("imgpath",&path);
           View->MessageReceived(tmp_msg);
           tmp_msg = new BMessage(REFS_CHANGE_DIR);
           path.GetParent(&path);
           tmp_msg->AddFlat("imgpath",&path);
           View->MessageReceived(tmp_msg);
         }
      }     
      break;
   case NO_FUNCTION_YET:
      // We just havent finished whatever it is you selected
      (new BAlert(NULL,Language.get("NOT_IMPLEMENTED"),Language.get("BUMMER")))->Go();
      break;
   case B_NODE_MONITOR:
      View->MessageReceived(msg);
      break;
   case SHOW_LOGO:
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
   
   YPreferences prefs("ImageShow_prefs");
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
   
   prefs.SetInt8("appcolorR",(int8)AppColor.red);
   prefs.SetInt8("appcolorG",(int8)AppColor.green);
   prefs.SetInt8("appcolorB",(int8)AppColor.blue);

   prefs.SetInt8("showercolorR",(int8)ShowerColor.red);
   prefs.SetInt8("showercolorG",(int8)ShowerColor.green);
   prefs.SetInt8("showercolorB",(int8)ShowerColor.blue);

   prefs.SetInt32("ThumbSize",ThumbSize);

   prefs.SetBool("toolbarhidden",IsToolBarHidden);
   
   prefs.SetString("Language_Name",Language.Name());

   prefs.SetInt32("ShowSlidSpeed",ssspeed);
   
   be_app->PostMessage(I_AM_DIEING_AHH);
   // delete SPMessage;
   return true;
}
