#ifndef _WARNING_VIEW_H
#define _WARNING_VIEW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>

class WarningView : public BView{
   public:
      WarningView(BRect,uint32);
      ~WarningView();
      virtual void AllAttached();
      virtual void Pulse();
      virtual void Draw(BRect);
      virtual void MessageReceived(BMessage*);
   private:
      int32 count;
      bool warn;
};
#endif