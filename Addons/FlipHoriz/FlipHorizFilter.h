#ifndef FlipHorizFILTER_H
#define FlipHorizFILTER_H

#include "ImageFilter.h"

class FILTER_SYMBOLS FlipHorizFilter : public ImageFilter {
	public:
		FlipHorizFilter();
		BBitmap *Run(BBitmap *input);
		~FlipHorizFilter();
};

extern "C" FILTER_SYMBOLS FlipHorizFilter *instantiate_filter();

#endif
