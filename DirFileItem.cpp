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
#include <TranslationUtils.h>
#include <String.h>
#include <Directory.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Path.h>
#include <fs_attr.h>

#include <stdio.h>
#include <stdlib.h>

#include "Globals.h"
#include "DirFileItem.h"
#include "YLanguageClass.h"
#include "BugOutDef.h"

extern BugOut db;

/*******************************************************
*   Set up the item and copy the name localy so we can
*   hang on too it.
*******************************************************/
DirFileItem::DirFileItem(BPath p,const char *s,bool IsDirectory,bool ThumbView,bool isQ):BListItem(){
   BNodeInfo node_info;
   BString Str;
   BNode node;
   char *buf=NULL;
   attr_info info;
   bool DoSystemIcons = true;
   TextAtBottom = false;

   IsQuery = isQ;
   
   myPath = p;
   
   FontSize = 16;
   //printf("Adding Item %s/%s\n",p.Path(),s);

   Img = NULL;
   name = (char *) malloc(strlen(s)+1);
   strcpy(name, s);
   IsDir = IsDirectory;
   Thumb = ThumbView;

   if(!Thumb){
      Size = 16; // This should be 16 or font hight .. which is the largest
      SetHeight(Size);
   }else{
      Size = 32;
      if(TextAtBottom && Thumb){
         Size += FontSize;
      }
      SetHeight(Size);
   }
   if(IsDir){
      if(!strcmp(name,"..")){
         p.GetParent(&p);
         Str.SetTo(p.Path());
      }else{
         Str.SetTo(p.Path());
         Str.Append("/");
         Str.Append(name);
      }
      node.SetTo(Str.String());
      node_info.SetTo(&node);
      if(!Thumb){
         if(DoSystemIcons){
            Img = new BBitmap(BRect(0, 0, B_MINI_ICON - 1, B_MINI_ICON - 1), B_COLOR_8_BIT);
            node_info.GetTrackerIcon(Img,B_MINI_ICON);
         }else{
            Img = BTranslationUtils::GetBitmap("BeDir");
         }
      }else{
         if(DoSystemIcons){
            Img = new BBitmap(BRect(0, 0, B_LARGE_ICON - 1, B_LARGE_ICON - 1), B_COLOR_8_BIT);
            node_info.GetTrackerIcon(Img,B_LARGE_ICON);
         }else{
            Img = BTranslationUtils::GetBitmap("BigBeDir");
         }
      }
   }else{
      Str.SetTo(p.Path());
      Str.Append("/");
      Str.Append(name);
      node.SetTo(Str.String());
      node_info.SetTo(&node);
      if(!Thumb){
         if(DoSystemIcons){
            Img = new BBitmap(BRect(0, 0, B_MINI_ICON - 1, B_MINI_ICON - 1), B_COLOR_8_BIT);
            node_info.GetTrackerIcon(Img,B_MINI_ICON);
         }else{
            Img = BTranslationUtils::GetBitmap("BeFile");
         }
      }else{
         Str.SetTo(p.Path());
         Str.Append("/");
         Str.Append(name);
         node.SetTo(Str.String());
         if(node.GetAttrInfo(THUMBNAIL_ATTR, &info) == B_OK){
            //printf("this node has a thumbnail attribute\n");
            buf = new char[info.size];
            if(info.size == node.ReadAttr(THUMBNAIL_ATTR,B_MESSAGE_TYPE, 0,buf,info.size)){
               BMessage archive;
               archive.Unflatten(buf);	
               Img = (BBitmap*)instantiate_object(&archive);
            }else{
               //printf("FAILED READ OF ATTIB\n");
               Img = BTranslationUtils::GetBitmapFile(Str.String());
               if(Img){
                  resizeImg();
               }
            }
         }else{
            //printf("no attrib\n");
            Img = BTranslationUtils::GetBitmapFile(Str.String());
            if(Img){
               // So there is a img but no attribut info ... lets do it then
               BRect OrigSize = Img->Bounds();
               resizeImg();
               //printf("Makeing Attribute Ourselfs\n");
               // MAKE ATTIBUTE AND WRITE IT IF USER WANTS
               /*
              "GRAFX:Thumbnail";
              "GRAFX:Width";
              "GRAFX:Height";
              */
              if(true){ // THIS SHOUD BE A VOLUME CHECK TO SEE IF WE SUPPORT ATTRIBUTS
                 BMessage archive;
                 Img->Archive(&archive);
                 ssize_t size = archive.FlattenedSize();
                 char *buf = new char[size];
                 archive.Flatten(buf,size);
                 node.WriteAttr(THUMBNAIL_ATTR,B_MESSAGE_TYPE, 0, buf,size);
                 delete buf;
                 int32 h,w;
                 w = OrigSize.IntegerWidth() + 1;
                 h = OrigSize.IntegerHeight() + 1;
                 node.WriteAttr(WIDTH_ATTR, B_INT32_TYPE, 0, (void *)&w, sizeof(int32));
                 node.WriteAttr(HEIGHT_ATTR,B_INT32_TYPE, 0, (void *)&h, sizeof(int32));
              }
            }
         }
         if(!Img){
            //printf("images was null\n");
            if(DoSystemIcons){
               Img = new BBitmap(BRect(0, 0, B_LARGE_ICON - 1, B_LARGE_ICON - 1), B_COLOR_8_BIT);
               node_info.GetTrackerIcon(Img,B_LARGE_ICON);
            }else{
               Img = BTranslationUtils::GetBitmap("BigBeFile");
            }
            
         }
      }
   }
   delete buf;
}

/*******************************************************
*   Free up the name we are hanging onto.
*******************************************************/
DirFileItem::~DirFileItem() {
   free(name);
   free(Img);
}

/*******************************************************
*   A little hack we put in so that we dont have to do
*   a bunch of extra work for sym links.
*******************************************************/
void DirFileItem::SetName(const char *s) {
   name = (char *) malloc(strlen(s)+1);
   strcpy(name, s);
}

/*******************************************************
*   So we draw a cute little item here
*******************************************************/
void DirFileItem::Update(BView *owner, const BFont *finfo){
   // catch update so they dont mess with my size!!
   SetHeight(Size);
}

/*******************************************************
*   So we draw a cute little item here
*******************************************************/
void DirFileItem::DrawItem(BView *owner, BRect frame, bool complete) {
   BRect b;
   owner->SetDrawingMode(B_OP_OVER);
   
/*   if(renameing){
      rename = new BTextControl(BRect(frame.left+Size+3+3,frame.top,frame.right,frame.bottom),"name",name,name,new BMessage(RENAMED));
      rename->SetDivider(0);
      owner->AddChild(rename);
      BTextView *tv;
      tv = (BTextView*)rename->ChildAt(0);
    //  rename->SetText(name);
      tv->SelectAll();
      rename->MakeFocus();
      renameing = false;
      doingrename = true;
   
   }
   if(stoprenameing){
      owner->RemoveChild(rename);
      delete rename;
      stoprenameing = false;
   }
   if(doingrename){
      owner->SetHighColor(0,0,255);
   //   owner->StrokeRect(frame);
    // owner->FillRect(frame);
   }   
*/

   if(complete){
      owner->SetHighColor(owner->ViewColor());
      owner->FillRect(frame);
   }
   
   if(IsSelected()){
      owner->SetHighColor(190,190,190);
      if(TextAtBottom && Thumb){
         owner->FillRect(BRect(frame.left+Size+8-FontSize,frame.top,frame.right,frame.bottom));
         owner->FillRect(BRect(frame.left+5,frame.bottom-FontSize+1,frame.left+Size+8-FontSize,frame.bottom));
      }else{
         owner->FillRect(BRect(frame.left+Size+8,frame.top,frame.right,frame.bottom));
      }
   }
   
   
   if(Img){
      
      b = frame;
      b.bottom -= 1;
      b.left = frame.left + 5 + 1;
      b.right = frame.left + Size + 5;
      
      if(TextAtBottom && Thumb){
         b.bottom -= FontSize;
         b.right -= FontSize;
      }
      
      owner->DrawBitmap(Img,Img->Bounds(),b);
      
      b = frame;
      if(TextAtBottom && Thumb){
         b.left += 5;
      }else{
         b.left += Size + 10;
      }
      b.bottom -= 2;
      //owner->MovePenTo(frame.left+Size+10, frame.bottom-2);
   }else{
      b = frame;
      b.left += 5;
      b.bottom -= 2;
      //owner->MovePenTo(frame.left+5, frame.bottom-2);
   }
   
   owner->MovePenTo(b.left, b.bottom);
   
   owner->SetHighColor(0,0,0);
   if(IsDir){
      owner->DrawString(name);
   }else{
      owner->DrawString(name);
   }
}

/*******************************************************
*   Make a menu based on what ports we have
*******************************************************/
void DirFileItem::MakeMenu(BMenu* menu){
   printf("Makeing menu for item %s/%s\n",myPath.Path(),name);
   if(IsDir){
      //menu->AddItem(new BMenuItem("Directory Menu",NULL));
      //menu->AddItem(new BSeparatorItem());
      menu->AddItem(new BMenuItem(Language.get("RENAME"),new BMessage(RENAME_ENTRY)));
      menu->AddItem(new BMenuItem(Language.get("DELETE"),new BMessage(DELETE_ENTRY)));
      menu->AddItem(new BMenuItem(Language.get("MOVE"),new BMessage(MOVE_ENTRY)));
      menu->AddItem(new BMenuItem(Language.get("REMOVE_THUMBS"),new BMessage(REMOVE_DIR_THUMB)));
      menu->AddItem(new BMenuItem(Language.get("ADD_BOOKMARK"),new BMessage(ADD_BOOKMARK)));
      menu->AddItem(new BSeparatorItem());
      menu->AddItem(new BMenuItem(Language.get("LAUNCH_TRACKER"),new BMessage(TRACKER_THERE)));
      if(IsQuery){
         menu->AddItem(new BMenuItem(Language.get("EDIT_QUERY"),new BMessage(EDIT_QUERY)));
      }
      menu->AddItem(new BSeparatorItem());
      menu->AddItem(new BMenuItem(Language.get("GET_INFO"),new BMessage(GET_FILE_INFO)));
      
   }else{
     // menu->AddItem(new BMenuItem("File Menu",NULL));
     // menu->AddItem(new BSeparatorItem());
      menu->AddItem(new BMenuItem(Language.get("RENAME"),new BMessage(RENAME_ENTRY)));
      menu->AddItem(new BMenuItem(Language.get("DELETE"),new BMessage(DELETE_ENTRY)));
      menu->AddItem(new BMenuItem(Language.get("MOVE"),new BMessage(MOVE_ENTRY)));
      menu->AddItem(new BMenuItem(Language.get("REMOVE_THUMBS"),new BMessage(REMOVE_FILE_THUMB)));
           
      BMenu *submenu = new BMenu(Language.get("SET_AS_BG"));
      submenu->AddItem(new BMenuItem(Language.get("CENTERED"),new BMessage(FILE_SET_BG_CENTER)));
      submenu->AddItem(new BMenuItem(Language.get("SCALED"),new BMessage(FILE_SET_BG_SCALED)));
      submenu->AddItem(new BMenuItem(Language.get("TILED"),new BMessage(FILE_SET_BG_TILED)));
      menu->AddItem(submenu);
      menu->AddItem(new BSeparatorItem());
      menu->AddItem(new BMenuItem(Language.get("LAUNCH_TRACKER"),new BMessage(TRACKER_HERE)));
      menu->AddItem(new BSeparatorItem());
      menu->AddItem(new BMenuItem(Language.get("GET_INFO"),new BMessage(GET_FILE_INFO)));
   }
}

/*******************************************************
*   Set a custom size for the Thumb View
*******************************************************/
void DirFileItem::SetSize(int s){
   Size = s;
   if(TextAtBottom && Thumb){
      Size += FontSize;
   }
   SetHeight(Size);
}

/*******************************************************
*   Ask the Item if it is a directory or not
*******************************************************/
bool DirFileItem::IsDirectory(){
   return IsDir;
}

/*******************************************************
*   Get the text that is this item
*******************************************************/
char* DirFileItem::Text(){
   return name;
}

/*******************************************************
*   Resizes the image so its nice and small for thumbs
*******************************************************/
void DirFileItem::resizeImg(){
   int ThumbSize = 64;
   BBitmap *new_img = new BBitmap(BRect(0,0,ThumbSize,ThumbSize),B_RGB32,true);
   if(!new_img->IsValid()){
      //This could potentaly cause a problem as we are acutaly inside of a 
      // thread. If we post 2 alerts the app will most likely lock
      (new BAlert(NULL,Language.get("RESIZE_ERROR"),Language.get("OK")))->Go();
      return;
   }
   BView  *blah = new BView(BRect(0,0,ThumbSize,ThumbSize),"drawer",B_FOLLOW_NONE,B_WILL_DRAW);
   
   new_img->AddChild(blah);
   new_img->Lock();
   
   blah->DrawBitmap(Img,Img->Bounds(),blah->Bounds());
  // blah->SetHighColor(255,0,0);
  // blah->FillRect(blah->Bounds(),B_SOLID_HIGH);
   
   blah->Sync();
   new_img->RemoveChild(blah);
   new_img->Unlock();
   
   delete blah;
   delete Img;
   Img = new BBitmap(new_img);
   delete new_img;
}

/*******************************************************
*   Gets the full path to this entry
*******************************************************/
BPath DirFileItem::GetPath(){
   return myPath;
}








