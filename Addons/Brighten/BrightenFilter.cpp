#include <Alert.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "BrightenFilter.h"
#include "BitmapDrawer.h"

BrightenFilter *instantiate_filter() {
	return new BrightenFilter();
}

BrightenFilter::BrightenFilter() : ImageFilter() {
	id = 1;
	strcpy(name, "Brighten");
}

BBitmap *BrightenFilter::Run(BBitmap *img) {
   if (img == NULL) return NULL;
   BRect b = img->Bounds();
   BRect rect(0,0,b.right,b.bottom);
   rgb_color c;

   int32 step = 10; // this should be somehow configuable

   uint32 small = 256;
   uint32 cred,cblue,cgreen;
   double RedR,BlueR,GreenR;
   RedR = BlueR = GreenR = 1;
   char type = 'X';
   
   BBitmap *new_img = new BBitmap(rect,img->ColorSpace());

   int32 x,y;
   BitmapDrawer I(img);
   BitmapDrawer NI(new_img);
   for(y=0;y <= b.IntegerHeight(); y++){
      for(x=0;x <= b.IntegerWidth(); x++){     
         I.GetPixel(x,y,&c);
         // Find the smallest value
         if((c.red < c.green) && (c.red > 0)){
            small = c.red;
            type = 'R';
         }else if(c.green > 0){
            small = c.green;
            type = 'G';
         }
         if((small > c.blue) && (c.blue > 0)){
            small = c.blue;
            type = 'B';
         }
         if(small > 255){ // all three where zero
            printf("Hmm\n");
            break;
         }
         //  printf("(%i,%i,%i) -> %i (%c)\n",c.red,c.green,c.blue,(int)small,type);
         if(small <= 0){ printf("ouch\n"); return NULL; }//this is a error
         // Found the smallest -> small
         // Now divide all colors by small to get ratio
         /*RedR =   (double)c.red   / small;
         GreenR = (double)c.green / small;
         BlueR =  (double)c.blue  / small;*/
         // Ok now lets add our stepp
         switch(type){
         case 'R':
            RedR = 1;
            GreenR = c.green / (double)small;
            BlueR =  c.blue  / (double)small;
            c.red += step;
            small = c.red;
            break;
         case 'G':
            RedR =   c.red   / (double)small;
            GreenR = 1;
            BlueR =  c.blue  / (double)small;
            c.green += step;
            small = c.green;
            break;
         case 'B':
            RedR =   c.red   / (double)small;
            GreenR = c.green / (double)small;
            BlueR =  1;
            c.blue += step;
            small = c.blue;
             break;
         }
        // printf("(%f,%f,%f)\n",RedR,GreenR,BlueR);
         // Ok we have added our step to the smallest
         // now lets multiply them all by there Ratio
         cred =   (int)(small * RedR);
         cgreen = (int)(small * GreenR);
         cblue =  (int)(small * BlueR);
        // printf("(%i,%i,%i)\n",c.red,c.green,c.blue);
         // so now it is all good. 
         // But some of them could be > 255 so lets check
         if(cred > 255){ cred = 255; }
         if(cgreen > 255){ cgreen = 255; }
         if(cblue > 255){ cblue = 255; }
         
         c.red = cred; c.green = cgreen; c.blue = cblue;
         // Well lets write that pixel back .. 
         NI.SetPixel(x,y,c);
      }
   }
   return new_img;
}

BrightenFilter::~BrightenFilter() {

}