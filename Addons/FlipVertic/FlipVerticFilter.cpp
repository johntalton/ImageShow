#include <stdlib.h>
#include <stdio.h>

#include "FlipVerticFilter.h"
#include "BitmapDrawer.h"

FlipVerticFilter *instantiate_filter() {
	return new FlipVerticFilter();
}

FlipVerticFilter::FlipVerticFilter() : ImageFilter() {
	id = 4;
	strcpy(name, "Flip Vertical");
}

BBitmap *FlipVerticFilter::Run(BBitmap *img) {
   if (img == NULL) return NULL;
   BRect b = img->Bounds();
   BRect rect(b);
  
   BBitmap *new_img = new BBitmap(rect,img->ColorSpace());

   int32 x,y;
   BitmapDrawer I(img);
   BitmapDrawer NI(new_img);
   for(y=0;y <= b.IntegerHeight(); y++){
      for(x=0;x <= b.IntegerWidth(); x++){     
         NI.SetPixel(b.IntegerWidth()-x,y,I.GetPixel(x,y));
      }
   }

   return new_img;
}

FlipVerticFilter::~FlipVerticFilter() {

}