#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Alert.h>

#include "DoubleFilter.h"

DoubleFilter *instantiate_filter() {
	return new DoubleFilter();
}

DoubleFilter::DoubleFilter() : ImageFilter() {
	id = 11;
	strcpy(name, "Double");
}

BBitmap *DoubleFilter::Run(BBitmap *img) {
   if (img == NULL) return NULL;
   BRect b = img->Bounds();
  
   BRect rect(0,0,b.IntegerWidth()*2,b.IntegerHeight()*2);
 

   BBitmap *new_img = new BBitmap(rect,img->ColorSpace(),true);
   BView *drawer = new BView(rect,"drawer",B_FOLLOW_NONE,B_WILL_DRAW);
   new_img->Lock();
   new_img->AddChild(drawer);
   
   drawer->DrawBitmap(img,b,rect);
   
   drawer->Sync();
   new_img->RemoveChild(drawer);
   new_img->Unlock();
   delete drawer;
   
   
   return new_img;
}

DoubleFilter::~DoubleFilter() {

}








