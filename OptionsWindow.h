#ifndef _OPTIONS_WINDOW_H
#define _OPTIONS_WINDOW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>

#define SET_TO_16 'st16'
#define SET_TO_32 'st32'
#define SET_TO_64 'st64'
#define SET_TO_96 'st96'

class OptionsWindow : public BWindow {
   public:
      OptionsWindow(BWindow*);
      virtual void MessageReceived(BMessage*);
      virtual bool QuitRequested();
   private:
     BWindow *theApp;
     BSlider *slide;
};
#endif