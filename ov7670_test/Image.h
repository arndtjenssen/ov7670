/*
 * Image.h
 *
 *	Encapsulates reading images from sd card and show them on the display
 *
 *  Created on: Sep 1, 2013
 *      Author: arndtjenssen
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include <SdFat.h>
#include "TFT.h"
#include "Ov7670.h"

class Image {
public:
	Image(Ov7670 *camera, TFT *screen);
	void displayRgbImage(const char *);
	void displayRgbImageNr(uint16_t nr);

	bool displayBitmap(const char *);
	bool displayBitmapNr(uint16_t nr);

	char * nrToPictureString(uint8_t dir, uint16_t nr);

private:
	SdFile file;
	Ov7670 *camera;
	TFT *screen;
};

#endif /* IMAGE_H_ */
