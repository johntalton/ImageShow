#ifndef _PROGRESS_VIEW_H
#define _PROGRESS_VIEW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>

class ProgressView : public BView{
   public:
      ProgressView(BRect,uint32);
      ~ProgressView();
      virtual void AllAttached();
      virtual void Pulse();
      virtual void Draw(BRect);
      void SetActive(bool);
      virtual void MessageReceived(BMessage*);
   private:
      bool active;
      int32 count;
      int step;
};
#endif