#ifndef MaskFILTER_H
#define MaskFILTER_H

#include "ImageFilter.h"

class FILTER_SYMBOLS MaskFilter : public ImageFilter {
	public:
		MaskFilter();
		BBitmap *Run(BBitmap *input);
		~MaskFilter();
   private:
      int theMask[25];
};

extern "C" FILTER_SYMBOLS MaskFilter *instantiate_filter();

#endif
