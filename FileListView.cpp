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
#include <Directory.h>
#include <Path.h>
#include <StopWatch.h>

//#include <stdio.h>

#include "Globals.h"
#include "FileListView.h"
#include "DirFileItem.h"

/*******************************************************
*   Set up our lovely List View
*******************************************************/
FileListView::FileListView(BRect frame):BListView(frame, "", B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL_SIDES,B_FRAME_EVENTS|B_WILL_DRAW){//B_NAVIGABLE|
   //SetViewColor(216,216,216,0);
   //Set up a entry and start watching it. This will give us 
   // upto the date info ..  :)
   BPath WatchPath;
   find_directory(B_USER_DIRECTORY, &WatchPath);
   BEntry entry(WatchPath.Path());
   node_ref nref;
   entry.GetNodeRef(&nref);
   watch_node(&nref, B_WATCH_DIRECTORY|B_WATCH_ATTR,be_app_messenger);
   
   filterImg = false;
   DoThumb = false;
   
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
   menu->AddItem(new BMenuItem("List Menu",NULL));
   menu->AddItem(new BSeparatorItem());
   //menu->AddItem(new BMenuItem("Options",new BMessage(LIST_OPTIONS)));
   //menu->AddItem(new BSeparatorItem());
   //menu->AddItem(new BMenuItem("Get Info",new BMessage(GET_IMG_INFO)));
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
   
   Window()->PostMessage(new BMessage(KILL_SHOWSLIDE));
   
   if (currentMsg->what == B_MOUSE_DOWN) {
      uint32 buttons = 0;
      currentMsg->FindInt32("buttons", (int32 *)&buttons);
      uint32 modifiers = 0;
      currentMsg->FindInt32("modifiers", (int32 *)&modifiers);
      uint32 clicks = 0;
      currentMsg->FindInt32("clicks",(int32*)&clicks);
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
        
      if(buttons & B_PRIMARY_MOUSE_BUTTON){
         if((clicks >= 2) && (i == OldSelection) && (tmpItem->IsDirectory())){ // double click on a dir
            // send out dir changed message
            //OurPath.Append("..");
            OurPath.Append(tmpItem->Text());
            dir.SetTo(OurPath.Path());
            dir.FindEntry(OurPath.Path(),&entry);
            if(entry.IsDirectory()){
               msg = new BMessage(CHANGE_DIR);
               if(msg->AddFlat("imgpath",&OurPath) != B_OK){ 
                  //some error
                  (new BAlert(NULL,"Error packing message","Oh!"))->Go();
               }else{
                  //Window()->MessageReceived(msg);
                  Parent()->Looper()->PostMessage(msg);
               }
            }
            MakeList(OurPath);
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
                  (new BAlert(NULL,"Error packing message","Oh!"))->Go();
               }
            }
         }
      }else if(buttons & B_SECONDARY_MOUSE_BUTTON){
         Select(i);
         //Popup a menu with stuff about the IP
         BPopUpMenu *menu = new BPopUpMenu("DirFileMenu");
         tmpItem = (DirFileItem*)ItemAt(i);//
         tmpItem->MakeMenu(menu);
         BMenuItem *selected = menu->Go(ConvertToScreen(p));
         if(selected){
            if(selected->Message()){
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
*   Define all the key board stuff that uses want
*******************************************************/
void FileListView::KeyDown(const char *bytes, int32 numBytes){
   int i = CurrentSelection();
   BPath path;
   
   DirFileItem *tmpItem = (DirFileItem*)ItemAt(i);
   BMessage *msg = NULL;
   BPath Path;

   if(numBytes==1){
      switch(bytes[0]){
      
         
      case B_UP_ARROW: // go to previous item
         Select(--i);
         i = CurrentSelection();
         if(i<0){return;}
         tmpItem = (DirFileItem*)ItemAt(i);
         ScrollToSelection();
         if(!tmpItem->IsDirectory()){
            msg = new BMessage(CHANGE_IMAGE);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) != B_OK){ 
               (new BAlert(NULL,"Error packing message","Oh!"))->Go();
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
         ScrollToSelection();
         if(!tmpItem->IsDirectory()){
            msg = new BMessage(CHANGE_IMAGE);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) == B_OK){ 
                 Parent()->Looper()->PostMessage(msg);
            }else{
               (new BAlert(NULL,"Error packing message","Oh!"))->Go();
            }
         } 
         break;
      case B_HOME: // Go to first Image
         if(CountItems() <= 1){return;}
         i = 1;
         Select(i);
         if(i<0){return;}
         tmpItem = (DirFileItem*)ItemAt(i);
         ScrollToSelection();
         if(!tmpItem->IsDirectory()){
            msg = new BMessage(CHANGE_IMAGE);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) != B_OK){ 
               (new BAlert(NULL,"Error packing message","Oh!"))->Go();
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
         if(!tmpItem->IsDirectory()){
            msg = new BMessage(CHANGE_IMAGE);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) != B_OK){ 
               (new BAlert(NULL,"Error packing message","Oh!"))->Go();
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
         if(!tmpItem->IsDirectory()){
            msg = new BMessage(CHANGE_IMAGE);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) != B_OK){ 
               (new BAlert(NULL,"Error packing message","Oh!"))->Go();
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
         ScrollToSelection();
         if(!tmpItem->IsDirectory()){
            msg = new BMessage(CHANGE_IMAGE);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) != B_OK){ 
               (new BAlert(NULL,"Error packing message","Oh!"))->Go();
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
            (new BAlert(NULL,"Error packing message","Oh!"))->Go();
         }else{
            Parent()->Looper()->PostMessage(msg);
         }
         break;   
      case B_ENTER: // if we are a dir go into it
         tmpItem = (DirFileItem*)ItemAt(i);
         ScrollToSelection();
         if(i<0){return;}
         if(tmpItem->IsDirectory()){
            msg = new BMessage(REFS_CHANGE_DIR);
            Path = OurPath;
            Path.Append(tmpItem->Text());
            if(msg->AddFlat("imgpath",&Path) != B_OK){ 
               (new BAlert(NULL,"Error packing message","Oh!"))->Go();
            }else{
               Parent()->Looper()->PostMessage(msg);
            }
         } 
         break;
      case B_DELETE:
         Parent()->Looper()->PostMessage(new BMessage(DELETE_ENTRY));
         break;
      default:
         BListView::KeyDown(bytes,numBytes);
      }
   }
}

/*******************************************************
*   Start MakeThatList thread with curent path
*******************************************************/
void FileListView::MakeList(){
   kill_thread(MakeListThread);
   MakeListThread = spawn_thread(MakeList_Hook, "We Just Maken da list", B_NORMAL_PRIORITY, this);
   resume_thread(MakeListThread);
}

/*******************************************************
*   Start MakeThatList thread with new path 
*******************************************************/
void FileListView::MakeList(BPath p){
   OurPath.SetTo(p.Path());
   kill_thread(MakeListThread);
   MakeListThread = spawn_thread(MakeList_Hook, "Maken da List via path", B_NORMAL_PRIORITY, this);
   resume_thread(MakeListThread);
}

/*******************************************************
*   This is the make list thread. It does the actuall
*   work.
*******************************************************/
int32 FileListView::MakeThatList(){
   BStopWatch Swatch("List Timer");
   BDirectory dir;
   BEntry entry;
   char name[B_FILE_NAME_LENGTH];
   BString p;
   BNode n;
   BNodeInfo ni;
   char type[B_MIME_TYPE_LENGTH];
   BString s;
   int i;
   int count = 1;
   DirFileItem *item = NULL;
   
   // Stop watching whatever we were watching.
   stop_watching(be_app_messenger);
   entry.SetTo(OurPath.Path());
   node_ref nref;
   entry.GetNodeRef(&nref);
   watch_node(&nref, B_WATCH_DIRECTORY|B_WATCH_ATTR,be_app_messenger);

   
   Parent()->Looper()->Lock();
   MakeEmpty();
   AddItem(item = new DirFileItem(OurPath,"..",true,DoThumb)); // add parent dir :)
   Parent()->Looper()->Unlock();

   
   dir.SetTo(OurPath.Path());
   // The we loop and get all the files
   // We should put some sort of sort in here!!!
   // mabey add them all to a BList and then do something ?
   while (dir.GetNextEntry(&entry, true) == B_NO_ERROR) {
      if (entry.IsFile()){ // THIS IS A FILE
         entry.GetName(name);
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
               AddItem(new DirFileItem(OurPath,name,false,DoThumb));
               Parent()->Looper()->Unlock();
            }else{
               //(new BAlert(NULL,s.String(),"ok"))->Go();
            }
         }else{
            Parent()->Looper()->Lock();
            AddItem(new DirFileItem(OurPath,name,false,DoThumb));
            Parent()->Looper()->Unlock();
         }
      }else{ // THIS IS A DIRECTORY OR LINK
         entry.GetName(name);
         Parent()->Looper()->Lock();
         AddItem(new DirFileItem(OurPath,name,true,DoThumb),count++);
         Parent()->Looper()->Unlock();
      }
   }
   Parent()->Looper()->Lock();
   DeselectAll();   
   Parent()->Looper()->Unlock();
 
   return B_OK;
}
/*******************************************************
*   We got a message from someone, "You've got Mail"
*******************************************************/
void FileListView::MessageReceived(BMessage *msg){
   const char *bytes;
   char b;
   bool FI;
   bool TV;
   DirFileItem *tmpItem = NULL;
   int32 opcode;
 //  dev_t device;
  // ino_t directory;
//   ino_t node_i;
 //  node_ref nref;
 
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
   DirFileItem *item;
   
   
   switch(msg->what){
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
         KeyDown(bytes,1);
      }
      break;   
   case FILTER_IMG:
      if(msg->FindBool("filter",&FI) == B_OK){
         filterImg = FI;
      }else{
         filterImg = !filterImg;
      }
      MakeList();
      break;
   case THUMB_VIEW:
      if(msg->FindBool("thumbview",&TV) == B_OK){
         DoThumb = TV;
      }else{
         DoThumb = !DoThumb;
      }
      MakeList();
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
            // (new BAlert(NULL,tmp.String(),""))->Go();
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
                        RemoveItem(i);
                        Window()->Unlock();
                     }
                  }
               }
            }else{
               if(fromPath == toPath){
                 //(new BAlert(NULL,"Rename","ok"))->Go();
                 //!!!!!!printf("Rename File: we add the new file but we should del the old\n");
               }
               msg->FindString("name",&name);
               tmp.SetTo(OurPath.Path());
               tmp.Append("/");
               tmp.Append(name);
               dir.SetTo(tmp.String());
               // (new BAlert(NULL,tmp.String(),""))->Go();
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
            //(new BAlert(NULL,"Entry Removed\nIt didn't go to trash we dont handle this yet","ok"))->Go(); 
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



