#ifndef _IMAGESHOWER_H
#define _IMAGESHOWER_H

#include <AppKit.h>
#include <InterfaceKit.h>
#include <TranslationUtils.h>
#include <Path.h>
#include "Addons/ImageFilter.h"


class ImageShower : public BView {
   public:
      ImageShower(BRect, uint32);
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
      void RotateImgCW90();
      void RotateImgCCW90();
      void FlipH();
      void FlipV();
   private:
      void FixUPScrollBars();
      void SetImgAsBG(uint32);
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
      ImageFilter *filters;
      rgb_color BGColor;
};
#endif