#ifndef RotateCCWFILTER_H
#define RotateCCWFILTER_H

#include "ImageFilter.h"

class FILTER_SYMBOLS RotateCCWFilter : public ImageFilter {
	public:
		RotateCCWFilter();
		BBitmap *Run(BBitmap *input);
		~RotateCCWFilter();
};

extern "C" FILTER_SYMBOLS RotateCCWFilter *instantiate_filter();

#endif
