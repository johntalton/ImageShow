#ifndef _IMAGESHOW_H
#define _IMAGESHOW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>

#include "ImageShowWindow.h"

extern const char *APP_SIGNATURE;

class ImageShow : public BApplication {
   public:
      ImageShow();
      virtual void ReadyToRun();
      virtual void RefsReceived(BMessage*);
      virtual void ArgvReceived(int32, char**);
      virtual void AboutRequested();
      virtual void MessageReceived(BMessage*);
      virtual bool QuitRequested();
   private:   
      ImageShowWindow *theWin;
      uint lastDrawMode;
};
#endif
