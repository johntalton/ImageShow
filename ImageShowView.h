#ifndef _IMAGESHOW_VIEW_H
#define _IMAGESHOW_VIEW_H

#include <Application.h>
#include <AppKit.h>
#include <InterfaceKit.h>

#include "ImageShower.h"
#include "TranslatorSavePanel.h"
#include "FileListView.h"
#include "SplitPane.h"

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
   private:
      int32 ShowSlidRunner();
      ImageShower *imgPanel;
      FileListView *selectPanel;
      FileListView *selectPanel2;
    
      BPath path;

      BWindow *parentWindow;
      BBox *Bb;
      BBox *StatusBar;
      BPath WorkingPath;
      BStringView *Directory;
      BStringView *ImgSize;

      BScrollView *ListSelector;
      BScrollView *ImgSelector;
      
      SplitPane *SP;
      thread_id ShowSlideThread;
      
};
#endif