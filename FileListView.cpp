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
#include <Directory.h>
#include <fs_attr.h>
#include <InterfaceKit.h>
#include <Path.h>
#include <StopWatch.h>
#include <StorageKit.h>
#include <String.h>

#include <stdio.h>

#include "DirFileItem.h"
#include "FileListView.h"
#include "Globals.h"
#include "YLanguageClass.h"
#include "BugOutDef.h"

extern BugOut db;

/*******************************************************
*   Set up our lovely List View
*******************************************************/
FileListView::FileListView(BRect frame):BListView(frame, "", B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL_SIDES,B_FRAME_EVENTS|B_WILL_DRAW){//B_NAVIGABLE|
   db.SendMessage("Created FileList View");
   SetViewColor(246,246,246,0);
   //Set up a entry and start watching it. This will give us 
   // upto the date info ..  :)
   BPath WatchPath;
   find_directory(B_USER_DIRECTORY, &WatchPath);
   BEntry entry(WatchPath.Path());
   node_ref nref;
   entry.GetNodeRef(&nref);
   watch_node(&nref, B_WATCH_DIRECTORY|B_WATCH_ATTR,be_app_messenger);
   OurPath.SetTo(WatchPath.Path());
   filterImg = false;
   DoThumb = false;
   ThumbSize = 32;
   db.SendMessage("File List View constructed");
}

/*******************************************************
*   Kill our list
*******************************************************/
FileListView::~FileListView(){
   kill_thread(MakeListThread);
}

/*******************************************************
*   Let others know where we point
*******************************************************/
BPath FileListView::GetPath(){
   return OurPath;
}

/*******************************************************
*   Set the list to a spesifice path
*******************************************************/
void FileListView::SetPath(const char *path){
   BDirectory dir;
   BEntry entry;
   BMessage *msg = NULL;
   bool query = false;
   BString s(OurPath.Path());
   
   if(!s.Compare(path)){
      //(new BAlert(NULL,"Path is the same",""))->Go();
      db.SendMessage("Path is the Same as befor. duh");
      return;
   }
   
   if(!strcmp(path,"..")){
      OurPath.GetParent(&OurPath);
   }else{            
      OurPath.SetTo(path);
   }
   dir.SetTo(OurPath.Path());
   dir.FindEntry(OurPath.Path(),&entry);
   
  
   
   msg = new BMessage(CHANGE_DIR);
   if(msg->AddFlat("imgpath",&OurPath) != B_OK){ 
      //some error
      (new BAlert(NULL,Language.get("PACKING_MSG_ERROR"),Language.get("OK")))->Go();
   }else{
      //Window()->MessageReceived(msg);
      Parent()->Looper()->PostMessage(msg);
   }
   
   
   
   query = IsEntryQueryFile(entry);
   MakeList(OurPath,query);
}

/*******************************************************
*   Let others know whats selected
*******************************************************/
BString FileListView::GetSelected(){
   BString s;
   int i = CurrentSelection();
   if(i < 0){
      //Opps not good
      s.SetTo("");
      return s;
   }
   DirFileItem *tmpItem = (DirFileItem*)ItemAt(i);
   s.SetTo(tmpItem->Text());
   return s;
}

/*******************************************************
*   Make our menu
*******************************************************/
void FileListView::MakeMenu(BMenu *menu){
   menu->AddItem(new BMenuItem(Language.get("OPTIONS"),new BMessage(LIST_OPTIONS)));
   menu->AddItem(new BSeparatorItem());
   menu->AddItem(new BMenuItem(Language.get("LAUNCH_TRACKER"),new BMessage(TRACKER_HERE)));
   menu->AddItem(new BSeparatorItem());
   menu->AddItem(new BMenuItem(Language.get("GET_INFO"),new BMessage(GET_IMG_INFO)));
}

/*******************************************************
*   Mickey Mouse stuff
*******************************************************/
void FileListView::MouseDown(BPoint p){
   int i;
   DirFileItem *tmpItem;
   BString tmpS;
   BMessage *currentMsg = Window()->CurrentMessage();
   BDirectory dir;
   BEntry entry;
   BMessage *msg = NULL;
   bool query = false;
   
   Window()->PostMessage(new BMessage(KILL_SHOWSLIDE));
   
   if (currentMsg->what == B_MOUSE_DOWN) {
      uint32 buttons = 0;
      currentMsg->FindInt32("buttons", (int32 *)&buttons);
      uint32 modifiers = 0;
      currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
      uint32 clicks = 0;
      currentMsg->FindInt32("clicks",(int32*)&clicks);
      printf("MouseDown\n");
/*
   char  *name;
   uint32  type;
   int32   count;
   for (int32 a = 0;a < 100;a++){
      if(currentMsg->GetInfo(B_ANY_TYPE,a,&name,&type,&count) != B_OK) break;
      cout << name << "\n";
   }
*/
      OldSelection = CurrentSelection();
      i = IndexOf(p);
      if(i < 0){
         //We are off some place we are not suposed to be :)
         // .. but lets go ahead and make ourselfs a menu
         if(buttons & B_SECONDARY_MOUSE_BUTTON){
            BPopUpMenu *menu = new BPopUpMenu("FileListViewMenu");
            MakeMenu(menu);
            BMenuItem *selected = menu->Go(ConvertToScreen(p));
            if(selected){
               if(selected->Message()){
                  Window()->PostMessage(selected->Message()->what);
                  switch(selected->Message()->what){
                     //Respond to selected Item.
                  }
               }
            }
         }
         return;
      }
      
      tmpItem = (DirFileItem*)ItemAt(i);
      OurPath = tmpItem->GetPath();
      printf("Path to item is %s\n",OurPath.Path());
      if(buttons & B_PRIMARY_MOUSE_BUTTON){
         if((clicks >= 2) && (i == OldSelection) && (tmpItem->IsDirectory())){ // double click on a dir
            // send out dir changed message
            // We must handle .. like this, it would be simple ust to do
            //    a Append and get it over with. but for things like Querys this fails
            //    horifilcly as Qeurys are not directrys and queryfile/.. does not exice
            //    and we where getting null as the path.
            if(!strcmp(tmpItem->Text(),"..")){
               OurPath.GetParent(&OurPath);
            }else{            
               OurPath.Append(tmpItem->Text());
            }
            
            //printf("New path is %s\n",OurPath.Path());
            
            dir.SetTo(OurPath.Path());
            dir.FindEntry(OurPath.Path(),&entry);
            //if(entry.IsDirectory() || entry.IsSymLink()){
               msg = new BMessage(CHANGE_DIR);
               if(msg->AddFlat("imgpath",&OurPath) != B_OK){ 
                  //some error
                  (new BAlert(NULL,Language.get("PACKING_MSG_ERROR"),Language.get("OK")))->Go();
               }else{
                  //Window()->MessageReceived(msg);
                  Parent()->Looper()->PostMessage(msg);
               }
            //}
            query = IsEntryQueryFile(entry);
            MakeList(OurPath,query);
         }else{
            // Single click with primary 
            Select(i);
            //OurPath.Append(tmpItem->Text());
            if(!(tmpItem->IsDirectory())){
               
               msg = new BMessage(CHANGE_IMAGE);
               Path.Unset();
               Path = OurPath;
               Path.Append(tmpItem->Text());
               if(msg->AddFlat("imgpath",&Path) == B_OK){
                  Parent()->Looper()->PostMessage(msg);
               }else{
                  (new BAlert(NULL,Language.get("PACKING_MSG_ERROR"),Language.get("OK")))->Go();
               }
            }
         }
      }else if(buttons & B_SECONDARY_MOUSE_BUTTON){
         Select(i);
         //Popup a menu with stuff
         BPopUpMenu *menu = new BPopUpMenu("DirFileMenu");
         tmpItem = (DirFileItem*)ItemAt(i);//
         tmpItem->MakeMenu(menu);
         BMenuItem *selected = menu->Go(ConvertToScreen(p));
         if(selected){
            //printf("item selected\n");
            if(selected->Message()){
               //printf("found a message for that item and we are posting to the window\n");
               Window()->PostMessage(selected->Message()->what);
               //switch(selected->Message()->what){
                  //Respond to selected Item.
               //}
            }
         }
      }
   }
}

/*******************************************************
*   Test to figure out if a entry (file) is of type
*   BEOS:TYPE =="application/x-vnd.Be-query"
*******************************************************/
bool FileListView::IsEntryQueryFile(BEntry entry){
   char *buf = NULL;
   bool isQuery = false;
   BNode node(&entry);
   attr_info info;
   BString type;

   if(node.GetAttrInfo(BE_FILE_TYPE, &info) == B_OK){
      // Found Attribute by that name
      buf = new char[info.size];
      if(info.size == node.ReadAttr(BE_FILE_TYPE,B_MESSAGE_TYPE, 0,buf,info.size)){
         // Life is good - all went Ok
         type.SetTo(buf);
         if(!type.Compare(MIME_TYPE_QUERY)){
            isQuery = true;
         }else{
            isQuery = false;
         }
      }else{
         // Faild to actually Read the attribute
         isQuery = false;
      }
      delete []buf;
   }else{
      // It has not attribut by that name
      isQuery = false;
   }
  
   return isQuery;
}

/*******************************************************
*   Define all the key board stuff that uses want
*******************************************************/
void FileListView::KeyDown(const char *bytes, int32 numBytes){
   Window()->Lock();
   int i = CurrentSelection();
   BPath path;

  BEntry entry;
  BDirectory dir;

   DirFileItem *tmpItem = (DirFileItem*)ItemAt(i);
   if(tmpItem != NULL){
      OurPath = tmpItem->GetPath();
   }
   BMessage *msg = NULL;
   BPath Path;

   if(numBytes==1){
      switch(bytes[0]){
      case B_BACKSPACE:
      case B_UP_ARROW: // go to previous item
         Select(--i);
         i = CurrentSelection();
         if(i<0){return;}
         tmpItem = (DirFileItem*)ItemAt(i);
         OurPath = tmpItem->GetPath();
         ScrollToSelection();
         if(!tmpItem->IsDirectory()){
            msg = new BMessage(CHANGE_IMAGE);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) != B_OK){ 
               (new BAlert(NULL,Language.get("PACKING_MSG_ERROR"),Language.get("OK")))->Go();
            }else{
               Parent()->Looper()->PostMessage(msg);
            }
         } 
         break;
      case B_SPACE:
      // Space should be a smart next image. like if we are ramomizing or 
      // if we are going backwareds .. or whatever .. but for now just 
      // do a down arrow ..
      case B_DOWN_ARROW: // go to next item
         Select(++i);
         i = CurrentSelection();
         if(i<0){return;}
         tmpItem = (DirFileItem*)ItemAt(i);
         
         OurPath = tmpItem->GetPath();
         
         ScrollToSelection();
         if(!tmpItem->IsDirectory()){
            msg = new BMessage(CHANGE_IMAGE);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) == B_OK){ 
                 Parent()->Looper()->PostMessage(msg);
            }else{
               (new BAlert(NULL,Language.get("PACKING_MSG_ERROR"),Language.get("OK")))->Go();
            }
         } 
         break;
      case B_HOME: // Go to first Image
         if(CountItems() <= 1){return;}
         i = 1;
         Select(i);
         if(i<0){return;}
         tmpItem = (DirFileItem*)ItemAt(i);
         OurPath = tmpItem->GetPath();
         ScrollToSelection();
         if(!tmpItem->IsDirectory()){
            msg = new BMessage(CHANGE_IMAGE);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) != B_OK){ 
               (new BAlert(NULL,Language.get("PACKING_MSG_ERROR"),Language.get("OK")))->Go();
            }else{
               Parent()->Looper()->PostMessage(msg);
            }
         } 
         break;
      case B_PAGE_UP: // Jump UP 10 spaces
         i -= 10;
         if(i < 1){
            i = 1;
         }     
         Select(i);
         ScrollToSelection();
         tmpItem = (DirFileItem*)ItemAt(i);
         OurPath = tmpItem->GetPath();
         if(!tmpItem->IsDirectory()){
            msg = new BMessage(CHANGE_IMAGE);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) != B_OK){ 
               (new BAlert(NULL,Language.get("PACKING_MSG_ERROR"),Language.get("OK")))->Go();
            }else{
               Parent()->Looper()->PostMessage(msg);
            }
         } 
         break;
      case B_PAGE_DOWN: // Jump DOWN 10 spaces
         i += 10;
         if(i > (CountItems()-1)){
            i = CountItems()-1;
         }     
         Select(i);
         ScrollToSelection();
         tmpItem = (DirFileItem*)ItemAt(i);
         OurPath = tmpItem->GetPath();
         if(!tmpItem->IsDirectory()){
            msg = new BMessage(CHANGE_IMAGE);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) != B_OK){ 
               (new BAlert(NULL,Language.get("PACKING_MSG_ERROR"),Language.get("OK")))->Go();
            }else{
               Parent()->Looper()->PostMessage(msg);
            }
         } 
         break;
      case B_END: // Go to last Image
         i = CountItems() -1; // dont forget to -1 .. 0 based list
         Select(i);
         if(i<0){return;}
         tmpItem = (DirFileItem*)ItemAt(i);
         OurPath = tmpItem->GetPath();
         ScrollToSelection();
         if(!tmpItem->IsDirectory()){
            msg = new BMessage(CHANGE_IMAGE);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) != B_OK){ 
               (new BAlert(NULL,Language.get("PACKING_MSG_ERROR"),Language.get("OK")))->Go();
            }else{
               //Window()->MessageReceived(msg);
               Parent()->Looper()->PostMessage(msg);
            }
         } 
         break;
      case 'H':
      case 'h':  // Go to home directory.
         find_directory(B_USER_DIRECTORY, &Path);
         msg = new BMessage(REFS_CHANGE_DIR);
         if(msg->AddFlat("imgpath",&Path) != B_OK){ 
            (new BAlert(NULL,Language.get("PACKING_MSG_ERROR"),Language.get("OK")))->Go();
         }else{
            Parent()->Looper()->PostMessage(msg);
         }
         break;   
      case B_ENTER: // if we are a dir go into it
         if(i<0){return;}
         tmpItem = (DirFileItem*)ItemAt(i);
         OurPath = tmpItem->GetPath();
         ScrollToSelection();
         if(tmpItem->IsDirectory()){
            //msg = new BMessage(REFS_CHANGE_DIR);
            msg = new BMessage(CHANGE_DIR);
            //Path.Append(tmpItem->Text());
            if(!strcmp(tmpItem->Text(),"..")){
               OurPath.GetParent(&OurPath);
            }else{            
               OurPath.Append(tmpItem->Text());
            }
            Path = OurPath;
            //printf("Path is %s\n",Path.Path());
            if(msg->AddFlat("imgpath",&Path) != B_OK){ 
               (new BAlert(NULL,Language.get("PACKING_MSG_ERROR"),Language.get("OK")))->Go();
            }else{
               Parent()->Looper()->PostMessage(msg);
            }
            dir.SetTo(OurPath.Path());
            dir.FindEntry(OurPath.Path(),&entry);
            MakeList(OurPath,IsEntryQueryFile(entry));
         } 
         break;
      case B_DELETE:
         Parent()->Looper()->PostMessage(new BMessage(DELETE_ENTRY));
         break;
      default:
         BListView::KeyDown(bytes,numBytes);
      }
   }else{
      // this is where we impliment things that have 
      // been called useing the shit key mod
      BEntry ent;
      BDirectory P_there;
      BPath P_here;
      status_t s;
      
      switch(bytes[0]){
      case B_UP_ARROW: // Send to Tmp dir
         break;
      case B_DOWN_ARROW: // move to tmp dir
         if(i<0){break;}
         P_here.SetTo(OurPath.Path());
         P_here.Append(tmpItem->Text());
         ent.SetTo(P_here.Path());
         if(ent.IsDirectory()){break;}// cand move directorys!!!
         P_there.SetTo("/tmp");
         s = ent.MoveTo(&P_there);
         switch(s){
         case B_OK:
            // all good in the hood
            
            break;
         case B_ENTRY_NOT_FOUND:
            // cand move to P_there .. it does not exist
            (new BAlert(NULL,Language.get("MOVE_FAILED"),Language.get("OK")))->Go();
            break;
         case B_FILE_EXISTS:
            // Ahh that file exists .. clober it?
            (new BAlert(NULL,Language.get("MOVE_FAILED"),Language.get("OK")))->Go();
            break;
         }
         break;      
      default:
         //error
         break;
      }
      
   }
   Window()->Unlock();
}

/*******************************************************
*   Grab the Query string out of a entry that is of
*   Query type. If its not a Query file ... we return 
*   a blank querys string.
*******************************************************/
BString FileListView::GetQryStr(BEntry entry){
   char *buf = NULL;
   BString qrystr("");
   BNode node(&entry);
   attr_info info;
   BString type;


   if(node.GetAttrInfo(QUERY_STR, &info) == B_OK){
      // Found Attribute by that name
      buf = new char[info.size];
      if(info.size == node.ReadAttr(QUERY_STR,B_MESSAGE_TYPE, 0,buf,info.size)){
         // Life is good - all went Ok
         qrystr.SetTo(buf);
      }else{
         // Faild to actually Read the attribute
         qrystr.SetTo("");
      }
      delete []buf;
   }else{
      // It has not attribut by that name
      qrystr.SetTo("");
   }
   
   return qrystr;
}

/*******************************************************
*   This is part of the GetQruVol .. this matches
*   the actuall message in our volume list with the 
*   disk. This is done useing the drive creation date
*   and the capacity of the drive. hopefully these will
*   be unique for each drive. But I dont think so.
*
*   This method is actually taken from OpenTracker :P
*******************************************************/
status_t FileListView::MatchArchivedVolume(BVolume *result, const BMessage *message, int32 index)
{
	time_t created;
	off_t capacity;
	
	if (message->FindInt32("creationDate", index, &created) != B_OK
		|| message->FindInt64("capacity", index, &capacity) != B_OK)
		return B_ERROR;

	BVolumeRoster roster;
	BVolume volume;

	roster.Rewind();
	while (roster.GetNextVolume(&volume) == B_OK){
		if (volume.IsPersistent() && volume.KnowsQuery()) {
			BDirectory root;
			volume.GetRootDirectory(&root);
			time_t cmpCreated;
			root.GetCreationTime(&cmpCreated);
			if (created == cmpCreated && capacity == volume.Capacity()) {
				*result = volume;
				return B_OK;
			}
		}
    }
	return B_DEV_BAD_DRIVE_NUM;
}

/*******************************************************
*   Get the volum we want to query out of a valid query
*   file. we return -1 it is we are to query all vols
*   if no vol is found that denotes that it is all vol
*   and thus -1  
*   They are stored as a flattened BMessage containing
*   int32 "device" dev ids. 
*******************************************************/
status_t FileListView::GetQryVol(BVolume *vol,BEntry entry,int32 index){
   char *buf = NULL;
   BNode node(&entry);
   attr_info info;
   BMessage msg;
   status_t stat = B_ERROR;
   
   // This gets the vol id for the boot drive
   BVolumeRoster roster;
   //BVolume volume;
   //Vroster.GetBootVolume(&vol);
  
   if(node.GetAttrInfo(QUERY_VOL, &info) == B_OK){
      // Found Attribute by that name
      buf = new char[info.size];
      if(info.size == node.ReadAttr(QUERY_VOL,B_MESSAGE_TYPE, 0,buf,info.size)){
         // Life is good - all went Ok
         if(msg.Unflatten(buf) == B_OK){
            // We now have a vaild message
            if((stat = MatchArchivedVolume(vol,&msg,index)) == B_OK){
               // Vol should be set to the voluem we want
               // Return stat which should be B_OK
            }else{
               // Somethign when wrong while matching the 
               // drive up. Either bad ID or invalid msg
               // set vol to null
               vol = NULL;
               // the error shoud be in stat ...
            }
         }else{
            // Had some trouble unflattening the message
            //  fall back to boot vol
            // What should we do .. 
            // set vol to null
            vol = NULL;
            stat = B_ERROR;
         }
      }else{
         // Faild to actually Read the attribute
         // Error out
         vol = NULL;
         stat = B_ERROR;
      }
   }else{
      // We do not have a vol attribute
      // This denotes we are doing all volumes
      // This is gona get messy
      printf("No attr\n");
      //(new BAlert(NULL,"This Query is a \"All Disks\" query, currently we do not support that, However is you open find and manualy select each disk (thus the same effect as all disks) and save that query - this will work fine","Ok - will do that"))->Go();
      stat = B_ERROR; 
      roster.Rewind();
      int c = 0;
	  while(roster.GetNextVolume(vol) == B_OK){
	     if(vol->IsPersistent() && vol->KnowsQuery()){
	        if(c >= index){
	           //this is the n'th volue that is
	           stat = B_OK;
	           break;
	        }//if index
	        c++;
	     }//if volume
	     
	  }// if next
	  
	  
   }
   delete []buf;
   return stat;
}


/*******************************************************
*   Start MakeThatList thread with curent path
*******************************************************/
void FileListView::MakeList(bool IsQuery){
   //OurPath.SetTo(p.Path());
   db.SendMessage(7,"MakeList(bool) about to kill thread");
   kill_thread(MakeListThread);
   if(IsQuery){
      MakeListThread = spawn_thread(MakeQueryList_Hook, "Flexen my mad query skillz", B_NORMAL_PRIORITY, this);
   }else{
      MakeListThread = spawn_thread(MakeList_Hook, "We Just Maken da list", B_NORMAL_PRIORITY, this);
   }
   resume_thread(MakeListThread);
   db.SendMessage(7,"Make list finished OK");
}

/*******************************************************
*   Start MakeThatList thread with new path 
*******************************************************/
void FileListView::MakeList(BPath p, bool IsQuery){
   OurPath.SetTo(p.Path());
   db.SendMessage(7,"MakeList(BPath,bool) about to kill thread");   
   kill_thread(MakeListThread);
   if(IsQuery){
      MakeListThread = spawn_thread(MakeQueryList_Hook, "Maken da Query list via path", B_NORMAL_PRIORITY, this);
   }else{
      MakeListThread = spawn_thread(MakeList_Hook, "Maken da List via path", B_NORMAL_PRIORITY, this);
   }
   resume_thread(MakeListThread);
   db.SendMessage(7,"Make list via path finished OK");
}

/*******************************************************
*   Start MakeThatList thread with new path 
*******************************************************/
void FileListView::MakeList(BList *plist){
   pathlist = plist;
   kill_thread(MakeListThread);
   MakeListThread = spawn_thread(MakeListList_Hook, "List via List via List", B_NORMAL_PRIORITY, this);   
   resume_thread(MakeListThread);
}

/*******************************************************
*   This is the make list thread. It does the actuall
*   work.
*******************************************************/
int32 FileListView::MakeThatListQuery(){
   Window()->PostMessage(new BMessage(WORKING));
   BStopWatch Swatch("List Timer");
   BDirectory dir;
   BEntry entry;
   BEntry Qentry;
//   char name[B_FILE_NAME_LENGTH];
   BString p;
   BNode n;
   BNodeInfo ni;
   BString s;
   BString QStr;
   DirFileItem *item = NULL;
   BEntry Tmpentry;
   BPath tmpPath;
   BQuery *q = NULL;
   BVolume vol;
   int32 i = 0;

   // Add in our .. to go back directory
   Parent()->Looper()->Lock();
   MakeEmpty();
   AddItem(item = new DirFileItem(OurPath,"..",true,DoThumb)); // add parent dir :)
   if(DoThumb){
      item->SetSize(ThumbSize);
   }
   Parent()->Looper()->Unlock();

   Qentry.SetTo(OurPath.Path());
   QStr = GetQryStr(Qentry);

   char name[255];

   while(GetQryVol(&vol,Qentry,i++) == B_OK){
      vol.GetName(name);
      printf("Doing query on volume named: %s\n",name);

      q = new BQuery();
      //q.Clear();
      q->SetVolume(&vol);
      q->SetPredicate(QStr.String());
      // Make the query Live - or send updates to this 
      q->SetTarget(this);

      // Actually Do the Query...  
      if(q->Fetch() != B_OK){
         Window()->PostMessage(new BMessage(FINISHED));
         return B_ERROR;
      }

      // Run through and add the items to the list
      while (q->GetNextEntry(&entry, false) == B_NO_ERROR) {
         AddEntry(entry);
      }
      
      delete q; // This stops all live Querys
   }
   
   // Post a finished message ... Wow thats a lot of work
   Window()->PostMessage(new BMessage(FINISHED));
      
   return B_OK;
}

/*******************************************************
*   This is the make list thread. It does the actuall
*   work.
*******************************************************/
int32 FileListView::MakeThatList(){
   Window()->PostMessage(new BMessage(WORKING));
   BStopWatch Swatch("List Timer");
   BDirectory dir;
   BEntry entry;
   BString p;
   BNode n;
   BNodeInfo ni;
   BString s;
   DirFileItem *item = NULL;
   
   if(OurPath.InitCheck() != B_OK){
      db.SendMessage(7,"NULL Path ..rrrr");
      OurPath.SetTo("/boot/home");
   }
   
   // Stop watching whatever we were watching.
   stop_watching(be_app_messenger);
   
   entry.SetTo(OurPath.Path());
   node_ref nref;
   entry.GetNodeRef(&nref);
   watch_node(&nref, B_WATCH_DIRECTORY|B_WATCH_ATTR,be_app_messenger);
   
   Parent()->Looper()->Lock();
   MakeEmpty();
   
   if(strcmp(OurPath.Path(),"/")){
      AddItem(item = new DirFileItem(OurPath,"..",true,DoThumb)); // add parent dir :)
      if(DoThumb){
         item->SetSize(ThumbSize);
      }
   }
   Parent()->Looper()->Unlock();
 
   db.SendMessage("About to populate list")  ;
   dir.SetTo(OurPath.Path());
   // The we loop and get all the files
   // We should put some sort of sort in here!!!
   // mabey add them all to a BList and then do something ?
   while (dir.GetNextEntry(&entry, false) == B_NO_ERROR) {
      AddEntry(entry);
      //db.SendMessage("Boom");
   }
   Window()->PostMessage(new BMessage(FINISHED));
   
   return B_OK;
}

/*******************************************************
*   This is a list of paths that the user passed use.
*   lets build a list out of them
*******************************************************/
int32 FileListView::MakeThatListList(){
   // Stop watching whatever we were watching.
   stop_watching(be_app_messenger);
   // Dont start watching .. nothign to watch
   
   if(LockLooper()){
      MakeEmpty();
   
      // Dont add .. as you cant go there :P but maby add home?
      DirFileItem *item = NULL;
      AddItem(item = new DirFileItem("/boot/","home",true,DoThumb)); // add home 
      if(DoThumb){
         item->SetSize(ThumbSize);
      }
     UnlockLooper();
   }
  
   if(pathlist == NULL){
      // that was a wast
      return B_ERROR;
   }
   
   BEntry entry;
   BString *str = NULL;
   for(int32 i = pathlist->CountItems()-1;i >= 0;i--){
      str = (BString*)pathlist->RemoveItem(i);
      if(str  == NULL){ continue; }
      entry.SetTo(str->String(),true);
      delete str;
      if(entry.InitCheck() == B_OK){
         AddEntry(entry);
      }
   }
   delete pathlist;
   pathlist = NULL;
   
   return B_OK;
}


/*******************************************************
*   This is the make list thread. It does the actuall
*   work.
*******************************************************/
status_t FileListView::AddEntry(BEntry entry){
   int i;
   int count = 1;
   if(CountItems() == 0){
      count = 0;
   }
   char SymName[B_FILE_NAME_LENGTH];
   char type[B_MIME_TYPE_LENGTH];
   BEntry Tmpentry;
   char name[B_FILE_NAME_LENGTH];
   DirFileItem *item = NULL;
   BString p;
   BNode n;
   BNodeInfo ni;
   BString s;
   
   /* This give us variable paths to the
   * actuall image we are looking at right now
   */
   Tmpentry.Unset();
   entry.GetParent(&Tmpentry);
   Tmpentry.GetPath(&OurPath);
   //printf("\t%s\n",OurPath.Path());

   if(entry.IsSymLink()){  // THIS IS A LINK
      entry.GetName(SymName); // link name
      entry_ref ref;
      entry.GetRef(&ref);
      BEntry tmpEntry(&ref,true);
      tmpEntry.GetName(name); // real name
      bool tmpIsDir;
      if(tmpEntry.IsDirectory()){
         tmpIsDir = true;
      }else{
         tmpIsDir = false;
      }
      BPath tmpP;
      tmpEntry.GetParent(&tmpEntry);
      tmpEntry.GetPath(&tmpP);
      
      Parent()->Looper()->Lock();
      if(tmpIsDir){
         AddItem(item = new DirFileItem(tmpP,name,tmpIsDir,DoThumb),count++);
      }else{
         AddItem(item = new DirFileItem(tmpP,name,tmpIsDir,DoThumb));
      }
      item->SetName(SymName);
      if(DoThumb){
         item->SetSize(ThumbSize);
      }
      Parent()->Looper()->Unlock();
   }else if (entry.IsFile()){ // THIS IS A FILE
      entry.GetName(name);
      
      if(IsEntryQueryFile(entry)){ // This file is a Qeury file (treat like dir
         Parent()->Looper()->Lock();
         AddItem(item = new DirFileItem(OurPath,name,true,DoThumb,true),count++);
         if(DoThumb){
            item->SetSize(ThumbSize);
         }
         Parent()->Looper()->Unlock();
      }else{
         if(filterImg){ // are we only showing image files
            p.SetTo(OurPath.Path());
            p.Append("/");
            p.Append(name);
            n.SetTo(p.String());
            ni.SetTo(&n);
            ni.GetType(type);
            s.SetTo(type);
            i = s.FindFirst("/");
            if(i > 0){ // some times the mime type is "image" and not "image/type"
               s.Truncate(i,false);
            }
            if(s == "image"){  // mime type is "image/png" or "image/x-bmp" or any other img type
               Parent()->Looper()->Lock();
               AddItem(item = new DirFileItem(OurPath,name,false,DoThumb));
               if(DoThumb){
                  item->SetSize(ThumbSize);
               }
               Parent()->Looper()->Unlock();
            }else{
               // this is not a image so dont add it :)
            }
         }else{
            Parent()->Looper()->Lock();
            AddItem(item = new DirFileItem(OurPath,name,false,DoThumb));
            if(DoThumb){
               item->SetSize(ThumbSize);
            }
            Parent()->Looper()->Unlock();
         }
      }
   }else if(entry.IsDirectory()){ // THIS IS A DIRECTORY
      entry.GetName(name);
      Parent()->Looper()->Lock();
      AddItem(item = new DirFileItem(OurPath,name,true,DoThumb),count++);
      if(DoThumb){
         item->SetSize(ThumbSize);
      }
      Parent()->Looper()->Unlock();
   }
   return B_OK;
}

/*******************************************************
*   
*******************************************************/
void FileListView::RemoveDirAttr(BString attr, BPath path){
   BDirectory dir;
   dir.SetTo(path.Path());
   BEntry entry;
   BNode node;
   status_t status;
   //char name[B_FILE_NAME_LENGTH];
   
   entry.SetTo(path.Path());
   if(entry.IsFile()){
      node.SetTo(&entry);
      if((status = node.RemoveAttr(attr.String())) != B_OK){
         switch(status){
         case B_ENTRY_NOT_FOUND:
            //printf("This Attribute does not exist.\n");
            break;
         case B_FILE_ERROR:
            //printf("File is Read-Only.\n");
            break;
         case B_NOT_ALLOWED:
            //printf("Volume is Read-Only.\n");
            break;
         default:
            //printf("Some other error happend - don't ask me.\n");
            break;
         }
      }
   }else if(entry.IsDirectory()){
      while(dir.GetNextEntry(&entry, true) == B_NO_ERROR){
         node.SetTo(&entry);
         //entry.GetName(name); 
         if((status = node.RemoveAttr(attr.String())) != B_OK){
            switch(status){
            case B_ENTRY_NOT_FOUND:
               //printf("This Attribute does not exist.\n");
               break;
            case B_FILE_ERROR:
               //printf("File is Read-Only.\n");
               break;
            case B_NOT_ALLOWED:
               //printf("Volume is Read-Only.\n");
               break;
            default:
               //printf("Some other error happend - don't ask me.\n");
               break;
            }
         }
      }
   }
}

/*******************************************************
*   
*******************************************************/
bool SetItemSize(BListItem* item, void *s){
   int size = *((int*)s);

   ((DirFileItem*)item)->SetSize(size);
   return false;
}

/*******************************************************
*   We got a message from someone, "You've got Mail"
*******************************************************/
void FileListView::MessageReceived(BMessage *msg){
   const char *bytes;
   int32 mod;
   char b;
   bool FI;
   bool TV;
   DirFileItem *tmpItem = NULL;
   db.SendMessage("FileListView Got a message");
   int32 opcode;

   entry_ref ref;
   BNode node;
   BNode to_node;
   node_ref nref;
   node_ref nref2;
   BDirectory dir;
 
   BDirectory to_dir;
   BDirectory from_dir;
   const char *name;
   BString tmp;
   BEntry entry;
   BPath toPath;
   BPath fromPath;
   
   BString Name;
   BString ItemName;
   DirFileItem *item = NULL;
   
   BString AttrName(THUMBNAIL_ATTR);
   BPath path;
   
   int32 s = 0;
   
   BList *list = NULL;
   
   switch(msg->what){
   case MAKE_LIST_LIST:
      if(msg->FindPointer("path_list_pointer",(void**)&list) == B_OK){
         MakeList(list);
      }
      break;
   case NEXT_IMAGE:
      b = B_DOWN_ARROW;
      KeyDown(&b,1);
      break;
   case PREV_IMAGE:
      b = B_UP_ARROW;
      KeyDown(&b,1);
      break;
   case FIRST_IMAGE:
      b = B_PAGE_UP;
      KeyDown(&b,1);
      break;
   case LAST_IMAGE:
      b = B_PAGE_DOWN;
      KeyDown(&b,1);
      break;
   case B_KEY_DOWN:  // we had to hack this. overriding KeyDown didn't work
      if(msg->FindString("bytes",&bytes) == B_OK){
         if(msg->FindInt32("modifiers",&mod) == B_OK){
            if(mod & B_SHIFT_KEY){
               KeyDown(bytes,2); // Ok .. this is sorta a cheep hack
            }else{
               KeyDown(bytes,1);
            }
         }
      }
      break;  
   case FILTER_IMG:
      if(msg->FindBool("filter",&FI) == B_OK){
         filterImg = FI;
      }else{
         filterImg = !filterImg;
      }
      MakeList();// Does not work with Querys .. this is bad
      break;
   case THUMB_VIEW:
      if(msg->FindBool("thumbview",&TV) == B_OK){
         DoThumb = TV;
      }else{
         DoThumb = !DoThumb;
      }
      MakeList(); // Does not work with Querys .. this is bad
      break;
   case THUMB_SIZE:
      if(DoThumb){
         if(msg->FindInt32("IconSize",&s) == B_OK){
            DoForEach(&SetItemSize,(void*)&s);
            ThumbSize = s;
            Looper()->Lock();
            Invalidate();
            Looper()->Unlock();
         }
      }   
      break;
   case REMOVE_DIR_THUMB:
   case REMOVE_FILE_THUMB:
      tmpItem = (DirFileItem*)ItemAt(CurrentSelection());
      path.SetTo(OurPath.Path());
      path.Append(tmpItem->Text(),true);
      tmpItem = NULL;
      RemoveDirAttr(AttrName,path);
      break;
   case RENAME_ENTRY:
      // select the item and throght it into rename mode
   //   tmpItem = (DirFileItem*)ItemAt(CurrentSelection());
   //   tmpItem->Rename();
      break;
   case RENAMED:
  //    tmpItem = (DirFileItem*)ItemAt(CurrentSelection());
  //    tmpItem->Renamed();
      break;
   case MOVE_ENTRY:
      break;
   case B_QUERY_UPDATE:
   case B_NODE_MONITOR:
      if(msg->FindInt32("opcode",&opcode) == B_OK){
         switch(opcode){
         case B_ENTRY_CREATED: //A file or dir was created here
            //msg->FindInt32("device", &device);
            //msg->FindInt64("directory",&directory);
            //msg->FindInt64("node",&node);
            //msg->FindInt32("device", &nref.device); 
            //msg->FindInt64("directory", &nref.node); 
            msg->FindString("name",&name);
            tmp.SetTo(OurPath.Path());
            tmp.Append("/");
            tmp.Append(name);
            dir.SetTo(tmp.String());
            entry.SetTo(tmp.String());
            if(entry.IsFile()){
               Window()->Lock();
               AddItem(new DirFileItem(OurPath,name,false,DoThumb));
               Window()->Unlock();
            }else{
               Window()->Lock();
               AddItem(new DirFileItem(OurPath,name,true,DoThumb),1);             
               Window()->Unlock();
            }
            break;   
         case B_ENTRY_MOVED: // someone droped a file or dir here.
          //  msg->FindInt64("to directory",&node_i);
          //  msg->FindInt32("device", &ref.device);
          //  msg->FindInt64("directory", &ref.directory);
            msg->FindInt32("device", &nref.device);
            msg->FindInt32("device", &nref2.device);
            msg->FindString("name", &name);
            
            msg->FindInt64("from directory", &nref2.node);
            from_dir.SetTo(&nref2);
            fromPath.SetTo(&from_dir,NULL);

            msg->FindInt64("to directory", &nref.node);
            to_dir.SetTo(&nref);
            toPath.SetTo(&to_dir,NULL);

            to_node.SetTo(toPath.Path());
          //  ref.set_name(name);
          //  to_node.SetTo(&ref);
            node.SetTo(OurPath.Path());
            if (to_node != node){ //we moved OUT of this dir Del the file
               // Remove entry with the name - name
               Name.SetTo(name);
               for(long i = CountItems()-1;i > 0; i--){
                  item = (DirFileItem*)ItemAt(i);
                  if(item){
                     ItemName.SetTo(item->Text());
                     if(Name == ItemName){
                        Window()->Lock();
                        if(i == CurrentSelection()){
                           //we are removing the item we are currently
                           // on .. thus we need to select the next item
                           RemoveItem(i);
                           if(i >= CountItems()){i = CountItems()-1;}
                           Select(i);
                           ScrollToSelection();
                           item = (DirFileItem*)ItemAt(i);
                           path = item->GetPath();
                           if(!item->IsDirectory()){
                              msg = new BMessage(CHANGE_IMAGE);
                              path.Append(item->Text());
                              if(msg->AddFlat("imgpath",&path) != B_OK){ 
                                 (new BAlert(NULL,Language.get("PACKING_MSG_ERROR"),Language.get("OK")))->Go();
                              }else{
                                 Parent()->Looper()->PostMessage(msg);
                              }
                           }
                        }else{
                           RemoveItem(i);
                        }
                        Window()->Unlock();
                        break;
                     }
                  }
               }
            }else{
               if(fromPath == toPath){
                 //!!!!!!printf("Rename File: we add the new file but we should del the old\n");
               }
               msg->FindString("name",&name);
               tmp.SetTo(OurPath.Path());
               tmp.Append("/");
               tmp.Append(name);
               dir.SetTo(tmp.String());
               entry.SetTo(tmp.String());
               if(entry.IsFile()){
                  Window()->Lock();
                  AddItem(new DirFileItem(OurPath,name,false,DoThumb));
                  Window()->Unlock();
               }else{
                  Window()->Lock();
                  AddItem(new DirFileItem(OurPath,name,true,DoThumb),1);             
                  Window()->Unlock();
               }
            }      
            break;
         case B_ENTRY_REMOVED:
            //!!!!printf("The entry was Removed from the disk. Permanatly, not to trash.\n");
            break;
         //case B_STAT_CHANGED:
         //case B_ATTR_CHANGED:
         //case B_DEVICE_MOUNTED:
         //case B_DEVICE_UNMOUNTED:
         }
      }
      break;
   default:
      BListView::MessageReceived(msg);
   }
   delete tmpItem;
}


