#ifndef _IMAGESHOW_WINDOW_H
#define _IMAGESHOW_WINDOW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>

#include "ImageShowView.h"


// A list of all the filters.. 
struct filter_entry{
   entry_ref eref;
   char *catagory;
   char *name;
   char *desc;
};

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
      static int32 DoBookMarkMenu(void* obj){
         return ((ImageShowWindow*)obj)->MakeBookMarkMenu();
      }
      static int32 DoLanguageMenu(void* obj){
         return ((ImageShowWindow*)obj)->MakeLanguageMenu();
      }
      void AddBookmarkItem(BMenuItem*);
      rgb_color AppColor;
      rgb_color ShowerColor;
   private:
      void MakeMainMenu();
      int32 MakeMenu();
      int32 MakeBookMarkMenu();
      int32 MakeLanguageMenu();
      #ifdef BUILD_INF_SUPPORT
      status_t AddFilter(BEntry entry);
      void BuildFilterMenu(BMenu *fmenu);
      void StartFilter(entry_ref ref);
      #endif
  private:
      TranslatorSavePanel *savepanel;
      BMenuBar *menubar;
      BMenu *menu;
      BMenu *modemenu;
      BMenu *submenu;
      BMenu *filtermenu;
      BMenu *bmmenu;
      BMenu *langmenu;
      BMenuItem *item;
      BMenuItem *shtb;
      BMenuItem *filter_item;
      BMenuItem *thumb_item;
      BMenuItem *undo_item;
      ImageShowView *View;
      uint lastDrawMode;
      bool Maximized;
      BRect lastSize;
      bool ImgFilter;
      bool ViewThumb;
      BWindow *ListOptionsWin;
      BWindow *SSOpWin;
      int32 ThumbSize;
      bool IsToolBarHidden;
      int32 ssspeed;
      BMessage *PrinterSettings;
      
      BList Filters;
};
#endif