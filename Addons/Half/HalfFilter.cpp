#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Alert.h>

#include "HalfFilter.h"

HalfFilter *instantiate_filter() {
	return new HalfFilter();
}

HalfFilter::HalfFilter() : ImageFilter() {
	id = 10;
	strcpy(name, "Half");
}

BBitmap *HalfFilter::Run(BBitmap *img) {
   if (img == NULL) return NULL;
   BRect b = img->Bounds();
  
   BRect rect(0,0,b.IntegerWidth()/2,b.IntegerHeight()/2);
 

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

HalfFilter::~HalfFilter() {

}








