#ifndef _FILE_LIST_VIEW_H
#define _FILE_LIST_VIEW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>
#include <Path.h>

bool SetItemSize(BListItem*, void*);

class FileListView : public BListView {
   public:
      FileListView(BRect);
      ~FileListView();
      virtual void MouseDown(BPoint);
      virtual void KeyDown(const char *, int32);
      BPath GetPath();
      BString GetSelected();
      void SetPath(const char*);
      virtual void MessageReceived(BMessage*);
      
   private:
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
      status_t AddEntry(BEntry);

      void MakeMenu(BMenu*);
      void RemoveDirAttr(BString,BPath);
      bool IsEntryQueryFile(BEntry);
      BString GetQryStr(BEntry);
      status_t GetQryVol(BVolume*,BEntry,int32);
      status_t MatchArchivedVolume(BVolume*, const BMessage *,int32);
private:    
      BPath OurPath;
      BPath Path;
      int OldSelection;
      bool filterImg;
      bool DoThumb;
      thread_id MakeListThread;
      bool Tree;
      int32 ThumbSize;
      BList *pathlist;
};
#endif