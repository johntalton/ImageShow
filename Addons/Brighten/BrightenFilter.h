#ifndef BrightenFILTER_H
#define BrightenFILTER_H

#include "ImageFilter.h"

class FILTER_SYMBOLS BrightenFilter : public ImageFilter {
	public:
		BrightenFilter();
		BBitmap *Run(BBitmap *input);
		~BrightenFilter();
};

extern "C" FILTER_SYMBOLS BrightenFilter *instantiate_filter();

#endif
