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
//#include <Autolock.h>

//#include <stdio.h>
#include <stdlib.h>

#include "Globals.h"
#include "DirFileItem.h"

/*******************************************************
*   Set up the item and copy the name localy so we can
*   hang on too it.
*******************************************************/
DirFileItem::DirFileItem(BPath p,const char *s,bool IsDirectory,bool ThumbView):BListItem(){
   BNodeInfo node_info;
   BString Str;
   BNode node;
   char *buf=NULL;
   attr_info info;

   bool DoSystemIcons = true;

   Img = NULL;

   name = (char *) malloc(strlen(s)+1);
   strcpy(name, s);
  
   IsDir = IsDirectory;

   Thumb = ThumbView;

//   renameing = false;
//   stoprenameing = false;
//   doingrename = false;

   /*
   "GRAFX:Thumbnail";
   "BEOS:L:STD_ICON";
   "BEOS:M:STD_ICON";
   "GRAFX:Width";
   "GRAFX:Height";
   */
   
   
    if(!Thumb){
      Size = 15;
      SetHeight(Size);
   }else{
      Size = 32;
      SetHeight(Size);
   }
   
   if(IsDir){
      Str.SetTo(p.Path());
      Str.Append("/");
      Str.Append(name);
      node.SetTo(Str.String());
      node_info.SetTo(&node);
      //BNodeInfo::GetTrackerIcon(&ref,Img,B_MINI_ICON);
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
      //BNodeInfo::GetTrackerIcon(&ref,Img,B_MINI_ICON);
      //node_info.GetTrackerIcon(Img,B_MINI_ICON);
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
         if(node.GetAttrInfo("GRAFX:Thumbnail", &info) == B_OK){
            //printf("this node has a thumbnail attribute\n");
            buf = new char[info.size];
            if(info.size == node.ReadAttr("GRAFX:Thumbnail",B_MESSAGE_TYPE, 0,buf,info.size)){
               //printf("Ah.. our atribe is here now..\n");
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
               resizeImg();
               
               // MAKE ATTIBUTE AND WRITE IT IF USER WANTS
               
               
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
   if(Img){
      b = frame;
      b.bottom -= 1;
      b.right = frame.left + Size + 5;
      b.left = frame.left + 5 +1;
      owner->DrawBitmap(Img,Img->Bounds(),b);
      owner->MovePenTo(frame.left+Size+10, frame.bottom-2);
   }else{
      owner->MovePenTo(frame.left+5, frame.bottom-2);
   }
   
   if(IsSelected()){
      owner->SetHighColor(190,190,190);
      //owner->SetDrawingMode(B_OP_BLEND);//SELECT
      //owner->FillRect(BRect(frame.left,frame.top,frame.right,frame.bottom));
      owner->FillRect(BRect(frame.left+Size+8,frame.top,frame.right,frame.bottom));
      //owner->SetDrawingMode(B_OP_OVER);
      owner->SetHighColor(0,0,0);
   }else{
      owner->SetHighColor(0,0,0);
   }
   
   if(IsDir){
      owner->DrawString(name);
   }else{
      owner->DrawString(name);
   }



/*
   if (IsSelected() || complete) {
//      rgb_color color;
      if (IsSelected()) {
         //owner->SetHighColor(190,190,190);
         owner->SetHighColor(255,0,0);
         owner->SetLowColor(0,255,0);
      }else{
         owner->SetHighColor(owner->ViewColor());
         owner->SetLowColor(0,0,255);
      }
      
      owner->FillRect(BRect(frame.left+Size+8,frame.top,frame.right,frame.bottom));
   }
   
   if(Img){
      b = frame;
      b.bottom -= 1;
      b.right = frame.left + Size + 5;
      b.left = frame.left + 5 +1;
      owner->DrawBitmap(Img,Img->Bounds(),b);
      owner->MovePenTo(frame.left+Size+10, frame.bottom-2);
   }else{
      owner->MovePenTo(frame.left+5, frame.bottom-2);
   }
   
   if (IsEnabled()) {
      owner->SetLowColor(255,0,0);
      owner->SetHighColor(0,0,0);
   }else{
     owner->SetHighColor(255,0,0);
     owner->SetLowColor(255,0,0);
   }
   
   
   
   if(IsDir){
      owner->DrawString(name);
   }else{
      owner->DrawString(name);
   }
   
   //self = owner;
*/      
}

/*******************************************************
*   Make a menu based on what ports we have
*******************************************************/
void DirFileItem::MakeMenu(BMenu* menu){
  // BMenuItem* item;
   if(IsDir){
      menu->AddItem(new BMenuItem("Directory Menu",NULL));
      menu->AddItem(new BSeparatorItem());
      menu->AddItem(new BMenuItem("Rename",new BMessage(RENAME_ENTRY)));
      menu->AddItem(new BMenuItem("Delete",new BMessage(DELETE_ENTRY)));
      menu->AddItem(new BMenuItem("Move",new BMessage(MOVE_ENTRY)));
      menu->AddItem(new BSeparatorItem());
      menu->AddItem(new BMenuItem("Get Info",new BMessage(GET_IMG_INFO)));
   }else{
      menu->AddItem(new BMenuItem("File Menu",NULL));
      menu->AddItem(new BSeparatorItem());
      menu->AddItem(new BMenuItem("Rename",new BMessage(RENAME_ENTRY)));
      menu->AddItem(new BMenuItem("Delete",new BMessage(DELETE_ENTRY)));
      menu->AddItem(new BMenuItem("Move",new BMessage(MOVE_ENTRY)));
      menu->AddItem(new BSeparatorItem());
      menu->AddItem(new BMenuItem("Get Info",new BMessage(GET_FILE_INFO)));
   }
}

/*******************************************************
*   Ask the Item if it is a directory or not
*******************************************************/
bool DirFileItem::IsDirectory(){
   return IsDir;
}


/*******************************************************
*   Ask the Item if it is a directory or not
*******************************************************/
char* DirFileItem::Text(){
   return name;
}


/*******************************************************
*   Ask the Item if it is a directory or not
*******************************************************/
void DirFileItem::Rename(){
//   renameing = true;
}

/*******************************************************
*   Ask the Item if it is a directory or not
*******************************************************/
void DirFileItem::Renamed(){
   BString new_name;
//   new_name.SetTo(rename->Text());
   
   //Get the entry for this  path/name and make it path/new_name
   name = (char *) malloc(strlen(new_name.String())+1);
   strcpy(name, new_name.String());
   
//   stoprenameing = true;
//   self->Invalidate();
}


float min(float a,float b){
   if(a < b) return a;
   return b;
}
float max(float a,float b){
   if(a > b) return a;
   return b;
}

/*******************************************************
*   Resizes the image so its nice and small for thumbs
*******************************************************/
void DirFileItem::resizeImg(){
   BBitmap *new_img = new BBitmap(BRect(0,0,Size,Size),B_RGB32,true);
   if(!new_img->IsValid()){
      (new BAlert(NULL,"Image is not Valid - thats not good!","Oh"))->Go();
   }
   BView  *blah = new BView(BRect(0,0,Size,Size),"drawer",B_FOLLOW_NONE,B_WILL_DRAW);
   
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






