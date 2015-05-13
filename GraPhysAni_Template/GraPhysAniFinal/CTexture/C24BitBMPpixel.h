#ifndef _C24BitBMPpixel_HEADER_GUARD_
#define _C24BitBMPpixel_HEADER_GUARD_

// Written by Michael Feeney, Fanshawe College, 2010
// mfeeney@fanshawec.on.ca
// It may be distributed under the terms of the General Public License:
// http://www.fsf.org/licenses/gpl.html
// Use this code at your own risk. It is indented only as a learning aid.
//

typedef unsigned char uchar;

struct C24BitBMPpixel
{
public:
	C24BitBMPpixel();	// Default constructor - all zeros
	C24BitBMPpixel(uchar red, uchar green, uchar blue);
	~C24BitBMPpixel();
	uchar redPixel;
	uchar greenPixel;
	uchar bluePixel;
};

struct C32BitBMPpixel
{
public:
	C32BitBMPpixel();	// Default constructor - all zeros
	C32BitBMPpixel(uchar red, uchar green, uchar blue);
	C32BitBMPpixel(uchar red, uchar green, uchar blue, uchar aplha);
	~C32BitBMPpixel();
	uchar redPixel;
	uchar greenPixel;
	uchar bluePixel;
	uchar alphaPixel;
};

#endif
