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
#include <Mime.h>

#include <stdio.h>

#include "Globals.h"
#include "SP_Globals.h"
#include "ImageShowView.h"
#include "ImageShower.h"
#include "TranslatorSavePanel.h"

#include "SplitPane.h"
#include "ImageShowWindow.h"
#include "ToolBarView.h"
#include "ProgressView.h"
#include "WarningView.h"
#include "YLanguageClass.h"
#include "BugOutDef.h"
#ifdef USE_CCOLUMN 
 #include "FileView.h"
#else
 #include "FileListView.h"
#endif

extern BugOut db;

/*******************************************************
*   Setup the main view. Add in all the niffty components
*   we have made and get things rolling
*******************************************************/
ImageShowView::ImageShowView(BWindow *parentWin, BRect frame):BView(frame, "", B_FOLLOW_ALL_SIDES, B_WILL_DRAW){//B_PULSE_NEEDED|B_FRAME_EVENTS
   SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
   //SetViewColor(233,0,0);
   BRect a,b;
   b = Bounds();
   db.SendMessage("ImageShowView starting up");
   ToolbarHeight = 30;
   sec = 2; // slide show time
   parentWindow = parentWin;
   
   StatusBar = new BBox(BRect(b.left-3,b.bottom-14,b.right,b.bottom+3),"StatusBar",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM,B_WILL_DRAW); 
   AddChild(StatusBar);

   find_directory(B_USER_DIRECTORY, &path);
   
   Directory = new BStringView(BRect(10,2,250,14),"",path.Path());
   StatusBar->AddChild(Directory);
 
   ImgSize = new BStringView(BRect(275,2,360,14),"","0x0 pixels");
   StatusBar->AddChild(ImgSize);
   
   Progress = new ProgressView(BRect(b.right-100,3,b.right-15,12),B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
   StatusBar->AddChild(Progress);
   
   a = Progress->Frame();
   a.right = a.left - 10;
   a.left = a.right - 100;
   ValidImage = new WarningView(a,B_FOLLOW_RIGHT|B_FOLLOW_BOTTOM);
   StatusBar->AddChild(ValidImage);
 
 
   Bb = new BBox(BRect(b.left,b.top,200,b.bottom - 15),"Box",B_FOLLOW_TOP_BOTTOM);//
   Bb->SetLabel(Language.get("SELECT_FILE_FOLDER"));
      
   BRect r = Bb->Bounds();
   r.InsetBy(8,8);
   r.top += 10;     
   

   #ifdef USE_CCOLUMN 
    selectPanel = new FileView(r,B_FOLLOW_ALL_SIDES);
    Bb->AddChild(selectPanel);
    a = selectPanel->Bounds();
   #else
    r.right -= B_V_SCROLL_BAR_WIDTH;
    r.bottom -= B_H_SCROLL_BAR_HEIGHT + 0;
    selectPanel = new FileListView(r);  //B_FRAME_EVENTS
    ListSelector = new BScrollView("scroll", selectPanel, B_FOLLOW_ALL_SIDES, B_WILL_DRAW, true, true);
    ListSelector->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
    ListSelector->MakeFocus(false);
    Bb->AddChild(ListSelector);  
    a = ListSelector->Bounds();
    a.top = a.bottom - B_H_SCROLL_BAR_HEIGHT - 1;
    a.left = a.right - B_V_SCROLL_BAR_WIDTH - 1;
    BBox *box1 = new BBox(a,"leftover",B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT); 
    ListSelector->AddChild(box1);
   #endif
   
	
   
   
 
   r = BRect(Bb->Bounds().right+10,b.top,b.right-B_V_SCROLL_BAR_WIDTH,b.bottom-15-B_H_SCROLL_BAR_HEIGHT);

   imgPanel = new ImageShower(r,B_FOLLOW_ALL_SIDES);

   BScrollView *sv = new BScrollView("Scroller", imgPanel, B_FOLLOW_ALL_SIDES,B_WILL_DRAW, true, true);
   imgPanel->TargetedByScrollView(sv); // This get called a second time. Pritty strange that we have to do this
   sv->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
   
   a = sv->Bounds();
   a.top = a.bottom - B_H_SCROLL_BAR_HEIGHT - 1;
   a.left = a.right - B_V_SCROLL_BAR_WIDTH - 1;
   BBox *box2 = new BBox(a,"leftover",B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT); 
   sv->AddChild(box2);

   b = Bounds();
   toolbar = new ToolBarView(BRect(b.left,b.top,b.right,ToolbarHeight));
   AddChild(toolbar);
   
   b.top = ToolbarHeight;
   SP = new SplitPane(BRect(b.left,b.top,b.right,b.bottom-15),Bb,sv,B_FOLLOW_ALL_SIDES);

   AddChild(SP);

   //SP->SetViewOneDetachable(true); 
   SP->SetBarPosition(150); // our default
   db.SendMessage("View inited just fine");
}

/*******************************************************
*   
*******************************************************/
void ImageShowView::AllAttached(){
   db.SendMessage("Attaching all");
   toolbar->AddItem("LUP","LDN",new BMessage(PREV_IMAGE));
   toolbar->AddItem("RUP","RDN",new BMessage(NEXT_IMAGE));
   toolbar->AddItem("FS","FSDN",new BMessage(FULLSCREEN));
   //toolbar->AddItem("","",new BMessage());

   app_info ai;
   be_app->GetAppInfo(&ai);
   BEntry entry(&ai.ref);
   entry.GetPath(&AppPath);
   AppPath.GetParent(&AppPath);
   
   //selectPanel->SetPath(path.Path());
     
   db.SendMessage("Spawing slide show");
   ShowSlideThread = spawn_thread(ShowSlide, "Slide Show Runner", B_NORMAL_PRIORITY, this);
   db.SendMessage("slide show spawned Ok");

}

/*******************************************************
*   
*******************************************************/
BView* ImageShowView::GetIS(){
   return (BView*)imgPanel;
}

/*******************************************************
*   Saves the image off in the given formate. The 
*   BMessage containds tranlator id and file name/path
*   this is prity much standard, go most of it from
*   on of the examples in the news letter
*******************************************************/
void ImageShowView::SaveImage(BMessage *message){
   // Recover the necessary data from the message
   translator_id *id;
   uint32 format;
   ssize_t length = sizeof(translator_id);
   if (message->FindData("translator_id", B_RAW_TYPE, (const void **)&id, &length) != B_OK){ 
      (new BAlert(NULL,Language.get("NO_TRANS_ID"),Language.get("OK")))->Go();
     
      return;
   }
   if (message->FindInt32("translator_format", (int32 *)&format) != B_OK){
      (new BAlert(NULL,Language.get("INVALIDE_FORMAT"),Language.get("OK")))->Go();
      return;
   }
   entry_ref dir;
   if (message->FindRef("directory", &dir) != B_OK) return;
   BDirectory bdir(&dir);
   const char *name = message->FindString("name");
   if (message->FindString("name", &name) != B_OK) return;
   if (name == NULL) return;

   // Clobber any existing file or create a new one if it didn't exist
   BFile file(&bdir, name, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
   if (file.InitCheck() != B_OK) {
      (new BAlert(NULL, Language.get("CANT_OVERWRITE_FILE"), Language.get("OK")))->Go();
      return;
   }
    
   BTranslatorRoster *roster = BTranslatorRoster::Default();
   BBitmap *bitmap = imgPanel->GetBitmap();
   if(bitmap==NULL){
      (new BAlert(NULL,Language.get("NO_IMAGE_TO_SAVE"),Language.get("OK")))->Go();
   }
   BBitmapStream stream(bitmap);
   
   // If the id is no longer valid or the translator fails for any other
   // reason, catch it here
   status_t err = roster->Translate(*id, &stream, NULL, &file, format);
   if(err == B_NO_TRANSLATOR){ (new BAlert("",Language.get("NO_TRANS_FOUND"),Language.get("BUMMBER")))->Go();}
   if(err == B_NOT_INITIALIZED){ (new BAlert("",Language.get("NOT_INITALIZED"),Language.get("")))->Go();}
   if(err == B_BAD_VALUE){ (new BAlert("",Language.get("IO_ERROR"),Language.get("")))->Go();}

   // Reclaim the ownership of the bitmap, otherwise it would be deleted
   // when stream passes out of scope
   stream.DetachBitmap(&bitmap);

   BNode fileNode(&bdir,name);
   BNodeInfo fNodeInfo(&fileNode);
      
   #if (B_BEOS_VERSION <= B_BEOS_VERSION_4_5) || (__POWERPC__  && (B_BEOS_VERSION <= B_BEOS_VERSION_4_5))
    SetFileType(&file, *id, format); 
    printf("Useing Old MimeType Gess...\n");
   #else
    BMimeType result;
    BEntry ent(&bdir,name);
    entry_ref fref;
    ent.GetRef(&fref);
    BMimeType::GuessMimeType(&fref,&result);
    BNodeInfo ninfo(&file); 
    ninfo.SetType(result.Type()); 
   #endif
}

/*******************************************************
*   
*******************************************************/
#if (B_BEOS_VERSION <= B_BEOS_VERSION_4_5) || (__POWERPC__  && (B_BEOS_VERSION <= B_BEOS_VERSION_4_5))
status_t ImageShowView::SetFileType(BFile * file, translator_id translator, uint32 type){ 
   const translation_format *formats; 
   int32 count;
   BString mime("image"); //defalut
   BTranslatorRoster *rost = BTranslatorRoster::Default();
   status_t err = rost->GetOutputFormats(translator, &formats, &count); 
   switch(err){
   case B_NO_TRANSLATOR:
      //printf("Translator (%i) is a bad ID\n",(int)translator);
      break;
   case B_NOT_INITIALIZED:
      //printf("Internal Translator Kit error\n");
      break;      
   case B_BAD_VALUE:
      //printf("NUll Peramiters error\n");
      break;
   case B_OK:
      for (int ix=0; ix<count; ix++) { 
         if (formats[ix].type == type) { 
            mime.SetTo(formats[ix].MIME);
            break; 
         } 
      } 
      break;
   }
   printf("Seting mime type to %s\n",mime.String());
   /* use BNodeInfo to set the file type */ 
   BNodeInfo ninfo(file); 
   return ninfo.SetType(mime.String()); 
} 
#endif

/*******************************************************
*   A pass through methoud so the window can grab
*   the preffs of the SplitPane
*******************************************************/
BMessage* ImageShowView::GetSplitPaneState(){
   return SP->GetState();
}


/*******************************************************
*   
*******************************************************/
int32 ImageShowView::ShowSlidRunner(){
  // printf("Entering Runner\n");
  db.SendMessage("Entering SSRunner");
   while(true){
      //printf("Snoozeing\n");
      //NEXT_IMAGE:
      //PREV_IMAGE:
      Window()->Lock();
      selectPanel->MessageReceived(new BMessage(NEXT_IMAGE));
      Window()->Unlock();
      bigtime_t time = 1000000;
      time *= sec;
      snooze(time);
   }
   db.SendMessage("Exiting SSRunner");
   return B_OK;
}

/*******************************************************
*   Central messageing area for the whole app. All msgs
*   get sent here if they are one of ours, then they
*   get sent back to the apropriate place. This is the
*   command center of the app!
*******************************************************/
void ImageShowView::MessageReceived(BMessage *msg){
   BPath path;
   char type[B_MIME_TYPE_LENGTH];
   Looper()->Lock();
   path = imgPanel->GetPath();
   Looper()->Unlock();
   BNode nod;
   BNodeInfo NInfo;
   BRect rec;
   BString tmpS;
   int32 answer;
   BEntry ent;
   BString name;
   BDirectory TrashDir;
   BPath TrashPath;
   rgb_color *c = NULL;
   ssize_t s;
   BDirectory dir;
   BMessage *m;
   entry_ref eref;
   const char *pathname[2]={0,0};
   BPath bookpath;
     
   switch(msg->what){
   case MAKE_LIST_LIST:
      //Window()->Lock();
      selectPanel->MessageReceived(msg);
      Directory->SetText("User Defined List");
      //Window()->Unlock();
      break;
   case UNDO:
      imgPanel->MessageReceived(msg);
      break;
   case SLIDE_SPEED:
      if(msg->FindInt32("ShowSlideSpeed",&answer) == B_OK){
         sec = answer;
      }
      break;
   case NULL_IMAGE:
      ValidImage->MessageReceived(msg);
      break;
   case CHANGE_LANGUAGE:
      Bb->SetLabel(Language.get("SELECT_FILE_FOLDER"));  
      break;
   case TRACKER_HERE:
      tmpS.SetTo(selectPanel->GetPath().Path());
      pathname[0]= tmpS.String();      
      be_roster->Launch(MIME_TYPE_DIR, 1, (char**)&pathname);
      break;
   case TRACKER_THERE:
      tmpS.SetTo(selectPanel->GetPath().Path());
      tmpS.Append("/");
      tmpS.Append(selectPanel->GetSelected());
      pathname[0]= tmpS.String();      
      be_roster->Launch(MIME_TYPE_DIR, 1, (char**)&pathname);
      break;
   case WORKING:
   case FINISHED:
      Progress->MessageReceived(msg);
      break;
   case ADD_BOOKMARK:
      name.SetTo(this->path.Path());
      this->path.Append(selectPanel->GetSelected().String(),true);
   case ADD_BOOKMARK_CURRENT:
      tmpS.SetTo(AppPath.Path());
      tmpS.Append("/Bookmarks/");
      if(bookpath.SetTo(tmpS.String()) != B_OK){
         (new BAlert(NULL,Language.get("MAKE_BM_DIR"),Language.get("OK")))->Go();
         dir.SetTo(AppPath.Path());
         dir.CreateDirectory("Bookmarks",NULL);
      }
      tmpS.Append(this->path.Leaf());
      if(dir.CreateSymLink(tmpS.String(),this->path.Path(),NULL) == B_OK){
         m = new BMessage(GO_TO_BOOKMARK);
         m->AddFlat("bmpath",&(this->path));
         ((ImageShowWindow*)Window())->AddBookmarkItem(new BMenuItem(this->path.Leaf(), m, 0, 0));
      }else{
         (new BAlert(NULL,Language.get("COULD_NOT_ADD_BM"),Language.get("OK")))->Go();
      }
      if(msg->what == ADD_BOOKMARK){ this->path.SetTo(name.String()); }
      break;
   case SHOW_BOOKMARKS:
      //Open a tracker at AppPath/Bookmakrs/
      tmpS.SetTo(AppPath.Path());
      tmpS.Append("/Bookmarks/");
      pathname[0]= tmpS.String();      
      be_roster->Launch(MIME_TYPE_DIR, 1, (char**)&pathname);
      break;
   case SHOW_TOOLBAR:
      if(toolbar->IsHidden()){
         toolbar->Show();
      }
      SP->MoveTo(0,ToolbarHeight);
      SP->ResizeTo(Bounds().Width(),Bounds().Height()-15-ToolbarHeight);
      break;
   case HIDE_TOOLBAR:
      if(!toolbar->IsHidden()){
         toolbar->Hide();
      }
      SP->MoveTo(0,0);
      SP->ResizeTo(Bounds().Width(),Bounds().Height()-15);
      break;
   case DO_SHOWSLIDE:
      //Set up view
      // We should go full screen
      resume_thread(ShowSlideThread);
      break;
   case KILL_SHOWSLIDE:
      suspend_thread(ShowSlideThread);
      break;
   case B_CUT:
   case B_COPY:
   case B_SELECT_ALL:
      imgPanel->MessageReceived(msg);
      break;
   case B_PASTE:
      if(msg->FindData("RGBColor",B_RGB_COLOR_TYPE,(const void**)&c,&s) == B_OK){
	     //printf("view got %i, %i, %i\n",c.red,c.green,c.blue);
         ((ImageShowWindow*)Window())->AppColor = *c;

         SetViewColor(*c);
         toolbar->SetViewColor(*c);
         toolbar->Invalidate();
         SP->SetViewColor(*c);
         Bb->SetViewColor(*c);
         StatusBar->SetViewColor(*c);
         ImgSize->SetViewColor(*c);
         ImgSize->Invalidate();
         Directory->SetViewColor(*c);
         Directory->Invalidate();
         SP->Invalidate();
         Bb->Invalidate();
         StatusBar->Invalidate();
         Invalidate();
         Window()->UpdateIfNeeded();
      }else{
         imgPanel->MessageReceived(msg);
      }
      break;
   case SHOWER_COLOR:
      Looper()->Lock();
      imgPanel->MessageReceived(msg);
      Looper()->Unlock();
      break;
   case SPLITPANE_STATE:
      SP->SetState(msg);
      break;
   case FILTER_IMG:
      Window()->Lock();
      selectPanel->MessageReceived(msg);
      Window()->Unlock();
      break;
   case THUMB_VIEW:
      Window()->Lock();
      selectPanel->MessageReceived(msg);
      Window()->Unlock();
      break;
   case THUMB_SIZE:
      Window()->Lock();
      selectPanel->MessageReceived(msg);
      Window()->Unlock();
      break;
   case REMOVE_DIR_THUMB:
   case REMOVE_FILE_THUMB:
      selectPanel->MessageReceived(msg);
      break;
   /*case IMAG_SELECT:
      if(!ListSelector->IsHidden()){ ListSelector->Hide(); }
      if(ImgSelector->IsHidden()){ ImgSelector->Show(); }
      break;
   case LIST_SELECT:
      if(ListSelector->IsHidden()){ ListSelector->Show(); }
      if(!ImgSelector->IsHidden()){ ImgSelector->Hide(); }
      break;*/
   case B_KEY_DOWN: // Let the selectPanel handle ALL key downs
      suspend_thread(ShowSlideThread);
      Window()->Lock();
      selectPanel->MessageReceived(msg);
      Window()->Unlock();
      break;
   case SAVE_IMAGE_SPESIFIC:
      SaveImage(msg);
	  break;
   case WINDOW_FIT:  // pass all of these to imgPanel
   case ASPECT:
   case OLD_ASPECT:
   case ACTUAL_SIZE:
   case STRETCH:
   case FULLSCREEN:
   case TILE:
   case SET_BG_CENTER:
   case SET_BG_SCALED:
   case SET_BG_TILED:
      Window()->Lock();
      imgPanel->MessageReceived(msg);
      Window()->Unlock();
      break;
   case FILE_SET_BG_CENTER:
   case FILE_SET_BG_SCALED:
   case FILE_SET_BG_TILED:
      //selectPanel->MessageReceived(msg);
      break;
   case CHANGE_IMAGE:
      Window()->Lock();
      imgPanel->MessageReceived(msg);
      Window()->Unlock();
      break;
   case CHANGE_DIR:  // pass all of thest to the selectionPanel
      // and update the status bar.
      if(msg->FindFlat("imgpath",&path) == B_OK){
         Window()->Lock();
         Directory->SetText(path.Path());
         Window()->Unlock();
         this->path = path;
      }
      break;
   case REFS_CHANGE_DIR:
     // (new BAlert(NULL,"This is not Good\nThis is a old messge and should not be here\n Please email me and tell me about it","ok"))->Go();
      if(msg->FindFlat("imgpath",&path) == B_OK){
         Window()->Lock();
         //selectPanel->MakeList(path);
         selectPanel->SetPath(path.Path());
         Directory->SetText(path.Path());
         Window()->Unlock();
      }
      break;
   case IMAGE_SIZE:
      tmpS.SetTo("");
      if(msg->FindRect("imgsize",&rec) == B_OK){
         tmpS << (int32)rec.right+1;
         tmpS << "x";
         tmpS << (int32)rec.bottom+1;
         tmpS << " pixels";
         ImgSize->SetText(tmpS.String());
      }
      break;
   case MAX_SCREEN:
   case RESIZE_TO_IMAGE:
      //Parent()->MessageReceived(msg);
      //Parent() did not work here .. WHY?
      // is Window not the parent of View?
      parentWindow->MessageReceived(msg);
      break;
   case NEXT_IMAGE:
   case PREV_IMAGE:
   case FIRST_IMAGE:
   case LAST_IMAGE:
   case LIST_OPTIONS:
   case B_MOUSE_WHEEL_CHANGED:
      //printf("passing to selection panel\n");
      Window()->Lock();
      selectPanel->MessageReceived(msg);
      Window()->Unlock();
      break;
   case GET_FILE_INFO:
   case GET_IMG_INFO:
      tmpS.SetTo(selectPanel->GetPath().Path());
      tmpS.Append("/");
      tmpS.Append(selectPanel->GetSelected());
      nod.SetTo(tmpS.String());
      NInfo.SetTo(&nod);
      NInfo.GetType(type);
       
      tmpS.SetTo("Name:\t");
      tmpS.Append(selectPanel->GetSelected());
      tmpS.Append("\n");
      tmpS.Append("Path:\t\t");
      tmpS.Append(selectPanel->GetPath().Path());
      tmpS.Append("\n");
      tmpS.Append("Type:\t\t");
      tmpS.Append(type);
      tmpS.Append("\n");
      (new BAlert(NULL,tmpS.String(),Language.get("OK")))->Go();
      break;
   case DELETE_ENTRY:
      printf("DELETING ENTRY - this is the good part\n");
      tmpS.SetTo(selectPanel->GetPath().Path());
      tmpS.Append("/");
      name = selectPanel->GetSelected();
      tmpS.Append(name.String());
      path.SetTo(tmpS.String());
      tmpS.Prepend(Language.get("ARE_YOU_SURE"));
      answer = (new BAlert(NULL,tmpS.String(),Language.get("CANCEL"),Language.get("DELETE"),NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go();
      if(answer == 1){
         printf("You have choosend to slauter this file\n");
         ent.SetTo(path.Path());
         if(ent.InitCheck() != B_OK){
            printf("Invalid entry\n");
         }
         find_directory(B_TRASH_DIRECTORY,&TrashPath);
         TrashDir.SetTo(TrashPath.Path());
         printf("File: %s\nTrash Dir: %s\nString %s\n",path.Path(),TrashPath.Path(),name.String());

         status_t s = ent.MoveTo(&TrashDir,NULL);//name.String()
         switch(s){
         case B_OK:
            printf("OK\n");
            break;
         case B_NO_INIT:
            printf("init\n");
            break;
         case B_ENTRY_NOT_FOUND:
            printf("not found\n");
            break;
         case B_FILE_EXISTS:
            printf("exists\n");
            break;
         case B_BUSY:
            printf("busy\n");
            break;
         default:
           printf("Error \n");
         }

         //WE WOULD LEAVE THIS IN BUT WE DO LIVE DIR WATCHING AND 
         // IT HANDLES WHEN TO REMOVE THE ITEM, SO DONT EXPLICITYLY
         // REMOVE IT HERE. LET THE LIST DO THAT
         //selectPanel->RemoveItem(selectPanel->CurrentSelection());
         
         //THIS WOULD HAVE PERMANATLY KILLED THE FILE. NOT MOVE 
         // TO TRASH BUT KILL.
         //ent.Remove(); 
      }
      break;
   case B_NODE_MONITOR:
      selectPanel->MessageReceived(msg);
      break;
   case RENAME_ENTRY:
      selectPanel->MessageReceived(msg);
      (new BAlert(NULL,"This is a Tracker like Rename, but on occation crashed the app. So I took it out.","Ok"))->Go();
      break;
   case RENAMED:
      selectPanel->MessageReceived(msg);
      break;
   case MOVE_ENTRY:
      (new BAlert(NULL,"I am currently working on a Tracker like MoveTo popup. This is going to take a while.","Ok"))->Go();
      break;
   case RUN_FILTER:
      imgPanel->MessageReceived(msg);      
      break;
   case SHOW_LOGO:
      imgPanel->MessageReceived(msg);
      break;
   default:
      BView::MessageReceived(msg);
   }
}






