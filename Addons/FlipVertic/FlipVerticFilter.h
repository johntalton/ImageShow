#ifndef FlipVerticFILTER_H
#define FlipVerticFILTER_H

#include "ImageFilter.h"

class FILTER_SYMBOLS FlipVerticFilter : public ImageFilter {
	public:
		FlipVerticFilter();
		BBitmap *Run(BBitmap*);
		~FlipVerticFilter();
};

extern "C" FILTER_SYMBOLS FlipVerticFilter *instantiate_filter();

#endif
