#ifndef _TOOLBAR_VIEW_H
#define _TOOLBAR_VIEW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>

class ToolBarView : public BView {
   public:
      ToolBarView(BRect);
      virtual void AllAttached();
      virtual void Draw(BRect);
      virtual void MessageReceived(BMessage*);
      void AddItem(const char*, const char*, BMessage*);
   private:
      int32 left;
      int32 space;
      BRect buttonsize;
      
};
#endif