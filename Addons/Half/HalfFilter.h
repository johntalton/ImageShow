#ifndef HalfFILTER_H
#define HalfFILTER_H

#include "ImageFilter.h"

class FILTER_SYMBOLS HalfFilter : public ImageFilter {
	public:
		HalfFilter();
		BBitmap *Run(BBitmap *input);
		~HalfFilter();
};

extern "C" FILTER_SYMBOLS HalfFilter *instantiate_filter();

#endif
