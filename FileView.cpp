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

//#include "DirFileItem.h"
#include "FileView.h"
#include "Globals.h"
#include "YLanguageClass.h"
#include "ListViewer.h"
#include "ListViewerTypes.h"

/*******************************************************
*   
*******************************************************/
FileView::FileView(BRect frame,uint32 mode):CColumnListView(frame,"",mode,B_WILL_DRAW){
   SetSorting(true);
	
   int32 ID = 0;
   // Name shoud be a CNameColum so we can hook for the Directory first case
   AddColumn(new CStringColumn("Name",100,5,1000),ID,"Name",ID);
   ID++;
   AddColumn(new CStringColumn("Size",100,5,1000),ID,"Size",ID);
   ID++;
   AddColumn(new CStringColumn("Modified",100,5,1000),ID,"Modified",ID);
   ID++;
   
   /*AddColumn(new CStringColumn("Created",100,5,1000),ID++,"Created");
   AddColumn(new CStringColumn("Owner",100,5,1000),ID++,"Owner");
   AddColumn(new CStringColumn("Group",100,5,1000),ID++,"Group");
   AddColumn(new CStringColumn("Path",100,5,1000),ID++,"Path");
   AddColumn(new CStringColumn("Permissions",100,5,1000),ID++,"Permission");*/
   
   BMimeType mt("image");
   BMessage msg;
   mt.GetAttrInfo(&msg);
   
   int32 i = 0;
   int32 type;
   BString str;
   while(msg.FindString("attr:public_name", i, &str) == B_OK){
      if(msg.FindInt32("attr:type", i, &type) == B_OK){
         switch(type){
         case B_STRING_TYPE:
            AddColumn(new CStringColumn(str.String(),100,5,1000),ID+i,(char*)str.String(),ID+i);
            break;
         case B_TIME_TYPE:
            AddColumn(new CDateColumn(str.String(),100,5,1000),ID+i,(char*)str.String(),ID+i);
            break;
         case B_INT64_TYPE:   
         case B_INT32_TYPE:
         case B_INT16_TYPE:
         case B_INT8_TYPE:
            AddColumn(new CIntColumn(str.String(),100,5,1000),ID+i,(char*)str.String(),ID+i);
            break;
         case B_DOUBLE_TYPE:
            break;
         case B_FLOAT_TYPE:
            break;
         case B_BOOL_TYPE:
            break;
         case B_CHAR_TYPE:
            break;
         
         }
         
      }
      i++;
   }
   
   CRow *row = new CRow;
	row->AddField(new CStringField(".."),0);
	row->AddField(new CStringField("10k"),1);
	//row->AddField(new CStringField("/boot/home"),6);
	AddRow(row);
   
   row = new CRow;
   row->AddField(new CStringField("test.jpg"),0);
	row->AddField(new CStringField("100k"),1);
	//row->AddField(new CStringField("/boot/home"),6);
	AddRow(row);
   
}

/*******************************************************
*   
*******************************************************/
FileView::~FileView(){

}

/*******************************************************
*   
*******************************************************/
status_t FileView::AddEntry(BEntry entry){
   
   
}

/*******************************************************
*   
*******************************************************/
BPath FileView::GetPath(){
   BPath p;
   return p;
}

/*******************************************************
*   
*******************************************************/
void FileView::SetPath(const char *path){

}

/*******************************************************
*   
*******************************************************/
BString FileView::GetSelected(){
   BString str;
   return str;
}

/*******************************************************
*   
*******************************************************/
void FileView::MessageReceived(BMessage *msg){
  switch(msg->what){
  case MAKE_LIST_LIST:
      break;
   case NEXT_IMAGE:
      break;
   case PREV_IMAGE:
      break;
   case FIRST_IMAGE:
      break;
   case LAST_IMAGE:
      break;
   case FILTER_IMG:
      break;
   case THUMB_VIEW:
      break;
   case THUMB_SIZE:
      break;
   case REMOVE_DIR_THUMB:
   case REMOVE_FILE_THUMB:
      break;
   case RENAME_ENTRY:
      break;
   case RENAMED:
      break;
   case MOVE_ENTRY:
      break;
  default:
      CColumnListView::MessageReceived(msg);
   }
}











