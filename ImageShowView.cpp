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
#include "ImageShowView.h"
#include "ImageShower.h"
#include "TranslatorSavePanel.h"
#include "FileListView.h"
#include "SplitPane.h"
#include "ImageShowWindow.h"

/*******************************************************
*   Setup the main view. Add in all the niffty components
*   we have made and get things rolling
*******************************************************/
ImageShowView::ImageShowView(BWindow *parentWin, BRect frame):BView(frame, "", B_FOLLOW_ALL_SIDES, B_WILL_DRAW){//B_PULSE_NEEDED|B_FRAME_EVENTS
   SetViewColor(216,216,216,0);
   //SetViewColor(233,0,0);
   BRect a,b;
   b = Bounds();

   parentWindow = parentWin;

   StatusBar = new BBox(BRect(b.left-3,b.bottom-14,b.right,b.bottom+3),"StatusBar",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM,B_WILL_DRAW); 
   AddChild(StatusBar);

   find_directory(B_USER_DIRECTORY, &path);
   
   Directory = new BStringView(BRect(10,2,250,14),"",path.Path());
   StatusBar->AddChild(Directory);
 
   ImgSize = new BStringView(BRect(275,2,360,14),"","0x0 pixels");
   StatusBar->AddChild(ImgSize);

   Bb = new BBox(BRect(b.left,b.top,200,b.bottom - 15),"Box",B_FOLLOW_TOP_BOTTOM);//
   Bb->SetLabel("Select File/Folder");
      
   BRect r = Bb->Bounds();
   r.InsetBy(8,8);
   r.top += 10;     
   r.right -= B_V_SCROLL_BAR_WIDTH;
   r.bottom -= B_H_SCROLL_BAR_HEIGHT + 0;

   selectPanel = new FileListView(r);  //B_FRAME_EVENTS
   ListSelector = new BScrollView("scroll", selectPanel, B_FOLLOW_ALL_SIDES, B_WILL_DRAW, true, true);
   ListSelector->SetViewColor(216,216,216);
   ListSelector->MakeFocus(false);
   Bb->AddChild(ListSelector);  

   a = ListSelector->Bounds();
   a.top = a.bottom - B_H_SCROLL_BAR_HEIGHT - 1;
   a.left = a.right - B_V_SCROLL_BAR_WIDTH - 1;
   BBox *box1 = new BBox(a,"leftover",B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT); 
   ListSelector->AddChild(box1);
 
   r = BRect(Bb->Bounds().right+10,b.top,b.right-B_V_SCROLL_BAR_WIDTH,b.bottom-15-B_H_SCROLL_BAR_HEIGHT);

   imgPanel = new ImageShower(r,B_FOLLOW_ALL_SIDES);

   BScrollView *sv = new BScrollView("Scroller", imgPanel, B_FOLLOW_ALL_SIDES,B_WILL_DRAW, true, true);
   imgPanel->TargetedByScrollView(sv);
   sv->SetViewColor(216,216,216);
   
   a = sv->Bounds();
   a.top = a.bottom - B_H_SCROLL_BAR_HEIGHT - 1;
   a.left = a.right - B_V_SCROLL_BAR_WIDTH - 1;
   BBox *box2 = new BBox(a,"leftover",B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT); 
   sv->AddChild(box2);

   b = Bounds();
   SP = new SplitPane(BRect(b.left,b.top,b.right,b.bottom-15),Bb,sv,B_FOLLOW_ALL_SIDES);
   
   AddChild(SP);
   SP->SetBarPosition(200); // our default
}


/*******************************************************
*   
*******************************************************/
void ImageShowView::AllAttached(){
   selectPanel->MakeList(path);  
   
   ShowSlideThread = spawn_thread(ShowSlide, "Slide Show Runner", B_NORMAL_PRIORITY, this);
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
      (new BAlert("Oh","No translator ID","Hmm"))->Go();
      return;
   }
   if (message->FindInt32("translator_format", (int32 *)&format) != B_OK){
      (new BAlert("Oh","Format not valid","Hmm"))->Go();
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
      (new BAlert(NULL, "Couldn't create of overwrite that file", "Bummer"))->Go();
      return;
   }
    
   BTranslatorRoster *roster = BTranslatorRoster::Default();
   BBitmap *bitmap = imgPanel->GetBitmap();
   if(bitmap==NULL){
      (new BAlert("Oh","No Image to Save","Hmm"))->Go();
   }
   BBitmapStream stream(bitmap);
   
   // If the id is no longer valid or the translator fails for any other
   // reason, catch it here
   status_t err = roster->Translate(*id, &stream, NULL, &file, format);
   if(err == B_NO_TRANSLATOR){ (new BAlert("","No Translator found","What"))->Go();}
   if(err == B_NOT_INITIALIZED){ (new BAlert("","Not initialiszed!!","Ouch"))->Go();}
   if(err == B_BAD_VALUE){ (new BAlert("","I/O error","Not good"))->Go();}

   // Reclaim the ownership of the bitmap, otherwise it would be deleted
   // when stream passes out of scope
   stream.DetachBitmap(&bitmap);

   BNode fileNode(&bdir,name);
   BNodeInfo fNodeInfo(&fileNode);
   //WE REALY NEED TO FIX THIS SO IT USES THE CORRECT MIME TYPE
   // "image" is good enough but we can do better than that
   fNodeInfo.SetType("image");
   // Nead to do something so Tracker knows what this is
   //file.WriteAttr("image/png");
   
}

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
   while(true){
      //printf("Snoozeing\n");
      snooze(1000000*2);
      //NEXT_IMAGE:
      //PREV_IMAGE:
      Window()->Lock();
      selectPanel->MessageReceived(new BMessage(NEXT_IMAGE));
      Window()->Unlock();
   }
   return 0;
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
   path = imgPanel->GetPath();
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
   
   switch(msg->what){
   case DO_SHOWSLIDE:
      //Set up view
      // We should go full screen
   
      resume_thread(ShowSlideThread);
      break;
   case KILL_SHOWSLIDE:
      suspend_thread(ShowSlideThread);
      break;
   case SHOWER_COLOR:
      imgPanel->MessageReceived(msg);
      break;
   case B_PASTE:
      if(msg->FindData("RGBColor",B_RGB_COLOR_TYPE,(const void**)&c,&s) == B_OK){
	     //printf("view got %i, %i, %i\n",c.red,c.green,c.blue);
         ((ImageShowWindow*)Window())->AppColor = *c;

         SetViewColor(*c);
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
      }
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
 /*  case IMAG_SELECT:
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
   case REFS_CHANGE_DIR:
      if(msg->FindFlat("imgpath",&path) == B_OK){};
      Window()->Lock();
      selectPanel->MakeList(path);
      Directory->SetText(path.Path());
      Window()->Unlock();
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
      (new BAlert(NULL,tmpS.String(),"Ok"))->Go();
      break;
   case DELETE_ENTRY:
      tmpS.SetTo(selectPanel->GetPath().Path());
      tmpS.Append("/");
      name = selectPanel->GetSelected();
      tmpS.Append(name.String());
      path.SetTo(tmpS.String());
      tmpS.Prepend("Are you shure you want to delete the file:\n\t");
      answer = (new BAlert("Conferm Del",
      tmpS.String(),
      "Cancel","Delete",NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT))->Go();
      //if(answer == 0){ }
      if(answer == 1){
         ent.SetTo(path.Path());
         find_directory(B_TRASH_DIRECTORY,&TrashPath);
         TrashDir.SetTo(TrashPath.Path());
         ent.MoveTo(&TrashDir,name.String());
         
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
   default:
      BView::MessageReceived(msg);
   }
}






