#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include <interface/Bitmap.h>

#if BUILDING_ADDON
#define FILTER_SYMBOLS _EXPORT
#else
#define FILTER_SYMBOLS _IMPORT
#endif
class FILTER_SYMBOLS ImageFilter;

#define FILTER_API_VER 1

class ImageFilter {
   protected:
      uint32 id;
      char name[255];
   public:
      ImageFilter(){
         id = 0;
         strcpy(name, "");
         next = NULL;
      }
      virtual BBitmap *Run(BBitmap *input) = 0;
      const char *GetName(){  return name; }
      const uint32 GetId(){   return id; }
      const uint32 GetVer(){ return FILTER_API_VER; }
      virtual ~ImageFilter(){ }
      
      ImageFilter *next;
};
#endif
