#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "MaskFilter.h"
#include "BitmapDrawer.h"

MaskFilter *instantiate_filter() {
	return new MaskFilter();
}

MaskFilter::MaskFilter() : ImageFilter() {
	id = 7;
	strcpy(name, "Mask");
}

BBitmap *MaskFilter::Run(BBitmap *img) {
   if (img == NULL) return NULL;
   BRect b = img->Bounds();
   BRect rect(0,0,b.right,b.bottom);
   BBitmap *new_img = new BBitmap(rect,img->ColorSpace());

   int gridCounter = 0;
   uint32 finalR,finalG,finalB;   
   rgb_color final,c;
   int32 x,y,x2,y2;
   BitmapDrawer I(img);
   BitmapDrawer NI(new_img);

   /*theMask = { 0,0,0,0,
               0,0,0,0,
               0,0,0,0,
               0,0,0,0
              };
*/
   theMask[0] =  1; theMask[1] =  0; theMask[2] =  0; theMask[3] =  0; theMask[4] =  1;
   theMask[5] =  0; theMask[6] =  0; theMask[7] =  0; theMask[8] =  0; theMask[9] =  0;
   theMask[10] = 0; theMask[11] = 0; theMask[12] = 0; theMask[13] = 0; theMask[14] = 0;
   theMask[15] = 0; theMask[16] = 0; theMask[17] = 0; theMask[18] = 0; theMask[19] = 0;
   theMask[20] = 1; theMask[21] = 0; theMask[22] = 0; theMask[23] = 0; theMask[24] = 1;

   int divisionFactor = 4;

   //This is where we could pop up a window that lets the used define theMask;


   for(y=0;y <= b.IntegerHeight(); y++){
      for(x=0;x <= b.IntegerWidth(); x++){     
         gridCounter = 0;  // reset values
         final.red = 0;final.green = 0;final.blue= 0;
         
         for(y2=-2; y2 <= 2; y2++){
            for(x2=-2; x2<=2; x2++){
               if(!(x+x2 < 0) && !(y+y2 < 0) && !(y+y2 > b.IntegerHeight()) && !(x+x2 > b.IntegerWidth())){
                  I.GetPixel(x+x2,y+y2,&c);
                  finalR   += c.red   * theMask[gridCounter];
                  finalG += c.green * theMask[gridCounter];
                  finalB  += c.blue  * theMask[gridCounter];
                  gridCounter++;
               }
            }
         }
         finalR   /= divisionFactor;
         finalG /= divisionFactor;
         finalB  /= divisionFactor;
         if(finalR > 255)  { finalR = 255; }
         if(finalG > 255){ finalG = 255; }
         if(finalB > 255) { finalB = 255; }
         final.red = finalR; final.green = finalG; final.blue = finalB;
         NI.SetPixel(x,y, final);
      }
   }
   
   return new_img;
}

MaskFilter::~MaskFilter() {

}



/* for(y=top; y<=bottom; y++)      // for each pixel in the image 
          for(x=left; x<=right; x++) 
          { 
            gridCounter=0;      // reset some values 
            final = 0; 

            for(y2=-2; y2<=2; y2++)     // and for each pixel around our 
              for(x2=-2; x2<=2; x2++)   //  "hot pixel"... 
              { 
                // Add to our running total 
                final += image[x+x2][y+y2] * filter[gridCounter]; 
                // Go to the next value on the filter grid 
                gridCounter++; 
              } 
            // and put it back into the right range 
            final /= divisionFactor; 

            destination[x][y] = final; 

          } 
          
*/          
