#ifndef _SLIDESHOW_WINDOW_H
#define _SLIDESHOW_WINDOW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>


class SlideShowWin : public BWindow {
   public:
      SlideShowWin(BWindow*);
      virtual void MessageReceived(BMessage*);
      virtual bool QuitRequested();
   private:
     BWindow *theApp;
     BSlider *slide;
};
#endif