#ifndef _FILE_VIEW_H
#define _FILE_VIEW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>
#include <Path.h>

#include "ListViewer.h"

//bool SetItemSize(BListItem*, void*);

class FileView : public CColumnListView{
   public:
      FileView(BRect,uint32 mode);
      ~FileView();
      //virtual void MouseDown(BPoint);
      //virtual void KeyDown(const char *, int32);
      BPath GetPath();
      BString GetSelected();
      void SetPath(const char*);
      virtual void MessageReceived(BMessage*);
private:
   status_t AddEntry(BEntry);
/*   private:
      static int32 MakeList_Hook(void* obj){
         return ((FileListView*)obj)->MakeThatList();
      }
      static int32 MakeQueryList_Hook(void* obj){
         return ((FileListView*)obj)->MakeThatListQuery();
      }
      static int32 MakeListList_Hook(void* obj){
         return ((FileListView*)obj)->MakeThatListList();
      }
      void MakeList(bool IsQuery = false);
      void MakeList(BPath, bool IsQuery = false);
      void MakeList(BList *plist);
      int32 MakeThatList();
      int32 MakeThatListQuery();
      int32 MakeThatListList();
   

      void MakeMenu(BMenu*);
      void RemoveDirAttr(BString,BPath);
      bool IsEntryQueryFile(BEntry);
      BString GetQryStr(BEntry);
      status_t GetQryVol(BVolume*,BEntry,int32);
      status_t MatchArchivedVolume(BVolume*, const BMessage *,int32);*/
private:    

};
#endif