#include <stdlib.h>
#include <stdio.h>

#include "FlipHorizFilter.h"
#include "BitmapDrawer.h"

FlipHorizFilter *instantiate_filter() {
	return new FlipHorizFilter();
}

FlipHorizFilter::FlipHorizFilter() : ImageFilter() {
	id = 5;
	strcpy(name, "Flip Horizontal");
}

BBitmap *FlipHorizFilter::Run(BBitmap *img) {
   if (img == NULL) return NULL;
   BRect b = img->Bounds();
   BRect rect(b);
  
   BBitmap *new_img = new BBitmap(rect,img->ColorSpace());

   int32 x,y;
   BitmapDrawer I(img);
   BitmapDrawer NI(new_img);
   for(y=0;y <= b.IntegerHeight(); y++){
      for(x=0;x <= b.IntegerWidth(); x++){     
         NI.SetPixel(x,b.IntegerHeight()-y,I.GetPixel(x,y));
      }
   }

   return new_img;
}

FlipHorizFilter::~FlipHorizFilter() {

}