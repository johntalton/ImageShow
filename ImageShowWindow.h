#ifndef _IMAGESHOW_WINDOW_H
#define _IMAGESHOW_WINDOW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>

#include "ImageShowView.h"

class ImageShowWindow : public BWindow {
   public:
      ImageShowWindow(BRect);
      virtual void MessageReceived(BMessage*);
      virtual bool QuitRequested();
      virtual void FrameResized(float,float);
      void FilterImages(bool);
      void ViewThumbs(bool);
      static int32 DoFilterMenu(void* obj){
         return ((ImageShowWindow*)obj)->MakeMenu();
      }
      
      rgb_color AppColor;
      rgb_color ShowerColor;
   private:
      int32 MakeMenu();
      TranslatorSavePanel *savepanel;
      BMenuBar *menubar;
      BMenu *menu;
      BMenu *modemenu;
      BMenu *submenu;
      BMenu *filtermenu;
      BMenuItem *item;
      ImageShowView *View;
      uint lastDrawMode;
      bool Maximized;
      BRect lastSize;
      bool ImgFilter;
      BMenuItem *filter_item;
      bool ViewThumb;
      BMenuItem *thumb_item;
      BWindow *ListOptionsWin;
      
      
};
#endif