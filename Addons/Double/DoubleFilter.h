#ifndef DoubleFILTER_H
#define DoubleFILTER_H

#include "ImageFilter.h"

class FILTER_SYMBOLS DoubleFilter : public ImageFilter {
	public:
		DoubleFilter();
		BBitmap *Run(BBitmap *input);
		~DoubleFilter();
};

extern "C" FILTER_SYMBOLS DoubleFilter *instantiate_filter();

#endif
