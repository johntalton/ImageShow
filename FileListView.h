#ifndef _FILE_LIST_VIEW_H
#define _FILE_LIST_VIEW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>
#include <Path.h>

class FileListView : public BListView {
   public:
      FileListView(BRect);
      ~FileListView();
      virtual void MouseDown(BPoint);
      virtual void KeyDown(const char *, int32);
      void MakeList();
      void MakeList(BPath);
      static int32 MakeList_Hook(void* obj){
			return ((FileListView*)obj)->MakeThatList();
      }
      int32 MakeThatList();
      BPath GetPath();
      BString GetSelected();
      virtual void MessageReceived(BMessage*);
   private:
      void MakeMenu(BMenu*);
      BPath OurPath;
      BPath Path;
      int OldSelection;
      bool filterImg;
      bool DoThumb;
      thread_id MakeListThread;
      bool Tree;
};
#endif