#ifndef _IMAGESHOWER_H
#define _IMAGESHOWER_H

#include <AppKit.h>
#include <InterfaceKit.h>
#include <TranslationUtils.h>
#include <Path.h>
//#include "Addons/ImageFilter.h"


class ImageShower : public BView {
   public:
      ImageShower(BRect, uint32);
      ~ImageShower();
      virtual void AllAttached();
      virtual void Draw(BRect);
      virtual void FrameResized(float,float);
      virtual void Pulse();
      virtual void TargetedByScrollView(BScrollView*);
      virtual void MouseDown(BPoint);
      virtual void MouseUp(BPoint);
      virtual void MouseMoved(BPoint,uint32,const BMessage*);
      virtual void MessageReceived(BMessage*);
      BPath GetPath();
      BBitmap* GetBitmap();
      void MakeMenu(BMenu*);
      static int32 TransIt(void* obj){
			return ((ImageShower*)obj)->Translator();
      }
      int32 Translator();
      static int32 DoFilterMenu(void* obj){
         return ((ImageShower*)obj)->MakeMenu();
      }
   private:
      int32 MakeMenu();
      void FixUPScrollBars();
      void SetImgAsBG(uint32);
      
      BBitmap* resizeImg(BBitmap* Img,BRect r);
      
      uint drawMode;
      uint OlddrawMode;
      BBitmap *img;
      bool redraw;
      BPath CurrentPath;
      BPath TmpImgPath;
      thread_id TranserThread;
      bool Draggin;
      BScrollBar *HSB;
      BScrollBar *VSB;
      BPoint here;
      BPoint Offset;
//      ImageFilter *filters;
      rgb_color BGColor;
      bool TIDOK;
      bool DrawingBox;
      BPoint BoxStart;
      BPoint BoxStop;
      BLocker *TransLock;
      
      BBitmap *undoimg;
      bool undoable;
      
      
      int32 flipID,mirrorID,rot_90ID,rot90ID;
      
};
#endif