#ifndef _IMAGESHOW_VIEW_H
#define _IMAGESHOW_VIEW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>

#include "ImageShower.h"
#include "TranslatorSavePanel.h"

#include "SplitPane.h"
#include "ToolBarView.h"
#include "ProgressView.h"
#include "WarningView.h"
#ifdef USE_CCOLUMN 
 #include "FileView.h"
#else
 #include "FileListView.h"
#endif


class ImageShowView : public BView {
   public:
      ImageShowView(BWindow*,BRect);
      BMessage* GetSplitPaneState();
      virtual void AllAttached();
      virtual void MessageReceived(BMessage*);
      void SaveImage(BMessage *message);
      static int32 ShowSlide(void* obj){
         return ((ImageShowView*)obj)->ShowSlidRunner();
      }
      BView* GetIS();
   private:
      int32 ShowSlidRunner();
      #if (B_BEOS_VERSION <= B_BEOS_VERSION_4_5) || (__POWERPC__  && (B_BEOS_VERSION <= B_BEOS_VERSION_4_5))
      status_t SetFileType(BFile*, translator_id, uint32);
      #endif
      ImageShower *imgPanel;
      
      #ifdef USE_CCOLUMN
       FileView *selectPanel;
      #else
       BScrollView *ListSelector;
       FileListView *selectPanel;
       FileListView *selectPanel2;
      #endif
      
      BPath path;
      BPath AppPath;

      BWindow *parentWindow;
      BBox *Bb;
      BBox *StatusBar;
      WarningView *ValidImage;
      BPath WorkingPath;
      BStringView *Directory;
      BStringView *ImgSize;
      ProgressView *Progress;
         
      BScrollView *ImgSelector;
      
      SplitPane *SP;
      thread_id ShowSlideThread;
      
      ToolBarView *toolbar;
      int32 ToolbarHeight;
      
      int32 sec;
};
#endif