#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "RotateCCWFilter.h"
#include "BitmapDrawer.h"

RotateCCWFilter *instantiate_filter() {
	return new RotateCCWFilter();
}

RotateCCWFilter::RotateCCWFilter() : ImageFilter() {
	id = 9;
	strcpy(name, "Rotate CW 90");
}

BBitmap *RotateCCWFilter::Run(BBitmap *img) {
	if (img == NULL) return NULL;
	BRect b = img->Bounds();
    BRect rect(0,0,b.bottom,b.right);

   BBitmap *new_img = new BBitmap(rect,img->ColorSpace());

   int32 x,y;
   BitmapDrawer I(img);
   BitmapDrawer NI(new_img);
   for(y=0;y <= b.IntegerHeight(); y++){
      for(x=0;x <= b.IntegerWidth(); x++){     
         NI.SetPixel(b.IntegerHeight()-y,x,I.GetPixel(x,y));
      }
   }
   return new_img;
}

RotateCCWFilter::~RotateCCWFilter() {

}