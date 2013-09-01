/*
 * Image.cpp
 *
 *  Created on: Sep 1, 2013
 *      Author: arndtjenssen
 */

#include "Image.h"
#include "Ov7670.h"

Image::Image(Ov7670 *c, TFT *s) {
	camera = c;
	screen = s;
}

void Image::displayRgbImage(const char * path) {
	file.open(path, O_READ);

	for (uint8_t y=0; y<SIZEY; y++) {
		file.read(camera->buffer, camera->BUFFER_SIZE);

		uint8_t x = 0;
		for (uint16_t i = 0; i<camera->BUFFER_SIZE; i+=3) {
			screen->drawPixel(x++, y, screen->newColor(camera->buffer[i], camera->buffer[i+1], camera->buffer[i+2]));
		}
	}

	file.close();
}

void Image::displayRgbImageNr(uint16_t nr) {
	displayRgbImage(nrToPictureString(0, nr));
}

bool Image::displayBitmap(const char * filename) {
	uint16_t word;
	uint32_t dword;
	uint32_t bmpImageoffset, rowSize;
	int32_t bmpWidth, bmpHeight;
	uint8_t bmpDepth;
	bool flip = true;
	int16_t x, y, w, h;
	uint8_t  r, g, b;

	uint32_t pos = 0;
  uint8_t sdbuffer[3*20]; // pixel buffer (R+G+B per pixel)
  uint8_t buffidx = sizeof(sdbuffer); // Current position in sdbuffer

	#ifdef SERIAL_INFO
	Serial << F("loading: ") << filename  << endll;
	#endif


	if (!file.open(filename, O_READ)) {
		#ifdef SERIAL_INFO
		Serial << F("couldn't open file") << endll;
		#endif
		goto error;
	}

	// check BMP signature
	file.read(&word, 2);
	if(word != 0x4D42) {
		#ifdef SERIAL_INFO
		Serial << F("wrong BMP signature") << endll;
		#endif
		goto error;
	}

	file.read(&dword, 4);
	#ifdef SERIAL_INFO
	Serial << F("file size: ") << dword << endll;
	#endif

	file.read(&dword, 4); // ignore creator bytes
	file.read(&bmpImageoffset, 4);
	file.read(&dword, 4); // dib header size
	file.read(&bmpWidth, 4);
	file.read(&bmpHeight, 4);

	file.read(&word, 2); // nr of planes
	if (word != 1) {
		#ifdef SERIAL_INFO
		Serial << F("wrong nr of planes: ") << word << endll;
		#endif
		goto error;
	}

	file.read(&bmpDepth, 2); // bits per pixel
	file.read(&dword, 4); // compression: 0 = uncompressed

	// we only accept bmp24 uncompressed format
  if ((bmpDepth != 24) || (dword != 0)) {
		#ifdef SERIAL_INFO
    Serial << F("invalid format") << endll;
		#endif
    goto error;
  }

	#ifdef SERIAL_DEBUG
  Serial <<  F("size: ") << bmpWidth << "x" << bmpHeight << endll;
	#endif

  // BMP rows are padded (if needed) to 4-byte boundary
  rowSize = (bmpWidth * 3 + 3) & ~3;

  // If bmpHeight is negative, image is in top-down order.
  if (bmpHeight < 0) {
    bmpHeight = -bmpHeight;
    flip = false;
  }

  // Crop area to be loaded
  x = 0;
  y = 0;
  w = bmpWidth;
  h = bmpHeight;

  if ((x + w - 1) >= screen->width())  w = screen->width() - x;
  if ((y + h - 1) >= screen->height()) h = screen->height() - y;

  for (int16_t row=0; row<h; row++) { // For each scanline...
    if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
      pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
    else     // Bitmap is stored top-to-bottom
      pos = bmpImageoffset + row * rowSize;

    if (file.curPosition() != pos) { // Need seek?
      file.seekSet(pos);
      buffidx = sizeof(sdbuffer); // Force buffer reload
    }

    for (int16_t col=0; col<w; col++) { // For each pixel...
      // Time to read more pixel data?
      if (buffidx >= sizeof(sdbuffer)) { // Indeed
        file.read(sdbuffer, sizeof(sdbuffer));
        buffidx = 0; // Set index to beginning
      }

      b = sdbuffer[buffidx++];
      g = sdbuffer[buffidx++];
      r = sdbuffer[buffidx++];
      screen->drawPixel(x + col, y + row, screen->newColor(r, g, b));
    }
  }


	file.close();
	return true;

	error:
	file.close();
	return false;
}

bool Image::displayBitmapNr(uint16_t nr) {
	return displayBitmap(nrToPictureString(1, nr));
}

char * Image::nrToPictureString(uint8_t dir, uint16_t nr) {
	static char buffer[20];

	if (dir == 0) {
		sprintf(buffer, "%s%05u%s", "/snaps/", nr, ".rgb");
	} else {
		sprintf(buffer, "%s%04u%s", "/photos/", nr, ".bmp");
	}

	return buffer;
}


