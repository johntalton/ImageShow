/*******************************************************
*   This is our def of the class. Niffty little class
*   that simplifys the reading and writeing of pixels
*   to and from a BBitmap.  This class if verry easy to
*   use, but it slows your filter down a little bit.
*   also you cant do quick pointer fills and other 
*   kewl things with this. So use when you are lazy.
*******************************************************/
class BitmapDrawer{ 
   public: 
      BitmapDrawer(BBitmap *target); 
      inline void SetPixel(int32 x, int32 y, uint32 c);
      inline uint32 GetPixel(int32 x, int32 y); 
      inline void SetPixel(int32 x, int32 y, const rgb_color &c); 
      inline void GetPixel(int32 x, int32 y, rgb_color *c); 
   private: 
      uint32 *bits;
      int32 bpr; 
};
 
/*******************************************************
*   Constructor, Just pass it the image ..
*******************************************************/
BitmapDrawer::BitmapDrawer(BBitmap *target) 
{ 
	bits = (uint32*)target->Bits(); 
	bpr = target->BytesPerRow()/4; 
} 
 
/*******************************************************
*   Set a pixel to a uint32 value (not realy usefull 
*   exept if you are useing this and bits in your app
*******************************************************/
void BitmapDrawer::SetPixel(int32 x, int32 y, uint32 c) 
{ 
	*(bits + x + y*bpr) = c; 
} 
 
/*******************************************************
*   The jucy part. Set a Pixel to a RGBA color
*******************************************************/ 
void BitmapDrawer::SetPixel(int32 x, int32 y, const rgb_color &c) 
{ 
	union { 
		uint8 bytes[4]; 
		uint32 word;	 
	} c1; 
 
	c1.bytes[0] = c.blue; 
	c1.bytes[1] = c.green; 
	c1.bytes[2] = c.red; 
	c1.bytes[3] = c.alpha; 
 
	SetPixel(x,y,c1.word); 
} 
 
/*******************************************************
*   Again this gets a pixel as a uint32
*******************************************************/ 
uint32 BitmapDrawer::GetPixel(int32 x, int32 y) 
{ 
	return *(bits + x + y*bpr); 
} 
 
/*******************************************************
*   This is the wonderfull get pixel as a RGBA color
*******************************************************/
void BitmapDrawer::GetPixel(int32 x, int32 y, rgb_color *c) 
{ 
	union { 
		uint8 bytes[4]; 
		uint32 word;	 
	} c1; 
 
	c1.word = GetPixel(x,y); 
 
	c->blue = c1.bytes[0]; 
	c->green = c1.bytes[1]; 
	c->red = c1.bytes[2]; 
	c->alpha = c1.bytes[3]; 
} 

