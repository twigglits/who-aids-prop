#include "tiffdensityfile.h"
#include <stdint.h>
#include <tiffio.h>
#include <string.h>
#include <iostream>

using namespace std;

TIFFDensityFile::TIFFDensityFile()
{
	m_width = 0;
	m_height = 0;
	m_yFlipped = false;
}

TIFFDensityFile::~TIFFDensityFile()
{
}

bool TIFFDensityFile::init(const string &fileName, bool noNegativeValues, bool flipY)
{
	if (m_values.size() > 0)
	{
		setErrorString("Already initialized");
		return false;
	}

	TIFFErrorHandler oldHandler = TIFFSetWarningHandler(NULL);

	bool status = readTiffFile(fileName, noNegativeValues, flipY);

	TIFFSetWarningHandler(oldHandler);

	return status;
}

class TIFFAutoClose
{
public:
	TIFFAutoClose(TIFF *pFile) { m_pFile = pFile; }
	~TIFFAutoClose() { TIFFClose(m_pFile); }
private:
	TIFF *m_pFile;
};

#define TIFFDENSITYFILE_ERRRETURN(x) { setErrorString(x); return false; }

bool TIFFDensityFile::readTiffFile(const string &fileName, bool noNeg, bool flipY)
{
	TIFF *pTiff = TIFFOpen(fileName.c_str(), "r");
	if (pTiff == 0)
		TIFFDENSITYFILE_ERRRETURN("Unable to open file " + fileName);

	TIFFAutoClose autoClose(pTiff); // Automatically close the file when exiting this scope

	uint16_t bps, compression, sampleFormat, sampPerPixel;

	if (!TIFFGetField(pTiff, TIFFTAG_BITSPERSAMPLE, &bps))
		TIFFDENSITYFILE_ERRRETURN("Can't read bits per sample");

	if (!TIFFGetField(pTiff, TIFFTAG_SAMPLEFORMAT, &sampleFormat))
		TIFFDENSITYFILE_ERRRETURN("Can't read sample format");

	if (!TIFFGetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, &sampPerPixel))
		TIFFDENSITYFILE_ERRRETURN("Can't get number of samples per pixel");

	if (!TIFFGetField(pTiff, TIFFTAG_COMPRESSION, &compression))
		TIFFDENSITYFILE_ERRRETURN("Can't get the compression setting");

	if (!((bps == 32 || bps == 64) && sampleFormat == SAMPLEFORMAT_IEEEFP))
		TIFFDENSITYFILE_ERRRETURN("Only 32 bit or 64 bit floating point samples are currently supported");

	if (sampPerPixel != 1)
		TIFFDENSITYFILE_ERRRETURN("Only one value per pixel is supported");

	if (compression != COMPRESSION_NONE)
		TIFFDENSITYFILE_ERRRETURN("Only uncompressed data is currently supported");

	uint32_t width, height, depth;

	if (!TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &width))
		TIFFDENSITYFILE_ERRRETURN("Can't read the image width");

	if (!TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &height))
		TIFFDENSITYFILE_ERRRETURN("Can't read the image height");

	if (width > 16384 || height > 16384)
		TIFFDENSITYFILE_ERRRETURN("Image width or height exceeds 16384");

	// Image depth should be absent or 1
	if (TIFFGetField(pTiff, TIFFTAG_IMAGEDEPTH, &depth))
	{
		if (depth != 1)
			TIFFDENSITYFILE_ERRRETURN("3D Images are not supported");
	}

	uint32_t tileWidth = 0;
	uint32_t tileHeight = 0;

	TIFFGetField(pTiff, TIFFTAG_TILEWIDTH, &tileWidth);
	TIFFGetField(pTiff, TIFFTAG_TILELENGTH, &tileHeight);

	if (tileWidth > 0 && tileHeight > 0)
	{
		if (bps == 32)
		{
			if (!readTilesFromTIFF<float>(pTiff, tileWidth, tileHeight, width, height, noNeg, fileName))
				return false;
		}
		else if (bps == 64)
		{
			if (!readTilesFromTIFF<double>(pTiff, tileWidth, tileHeight, width, height, noNeg, fileName))
				return false;
		}
		else
			TIFFDENSITYFILE_ERRRETURN("Internal error: unexpected bits per sample");

	}
	else
	{
		m_values.resize(width*height);

		if (bps == 64)
		{
			assert(TIFFScanlineSize(pTiff) == width*sizeof(double));

			for (int y = 0 ; y < height ; y++)
			{
				if(TIFFReadScanline(pTiff, &(m_values[width*y]), y, 0) < 0)
					TIFFDENSITYFILE_ERRRETURN("Error reading scan line from file " + fileName);
			}
		}
		else if (bps == 32)
		{
			assert(TIFFScanlineSize(pTiff) == width*sizeof(float));
			vector<float> tmpBuf(width);

			for (int y = 0 ; y < height ; y++)
			{
				if(TIFFReadScanline(pTiff, &(tmpBuf[0]), y, 0) < 0)
					TIFFDENSITYFILE_ERRRETURN("Error reading scan line from file " + fileName);

				int offset = y*width;
				for (int x = 0 ; x < width ; x++, offset++)
					m_values[offset] = tmpBuf[x];
			}
		}
		else
			TIFFDENSITYFILE_ERRRETURN("Internal error: unexpected bits per sample");
	}

	m_width = (int)width;
	m_height = (int)height;

	if (flipY)
	{
		// Note: since we're always swapping two lines, we can't iterate over more
		//       than half the lines, or the flip would be undone again
		int yMax = m_height/2;
		vector<double> tmp(m_width);

		for (int y = 0 ; y < yMax ; y++)
		{
			int y0 = y;
			int y1 = m_height-1-y;

			memcpy(&(tmp[0]),&(m_values[y0*m_width]), sizeof(double)*m_width);
			memcpy(&(m_values[y0*m_width]), &(m_values[y1*m_width]), sizeof(double)*m_width);
			memcpy(&(m_values[y1*m_width]), &(tmp[0]), sizeof(double)*m_width);
		}
	}
	m_yFlipped = flipY;

	return true;
}

template<class T>
bool TIFFDensityFile::readTilesFromTIFF(void *pTiffVoid, int tileWidth, int tileHeight, int width, int height, bool noNeg,
		                        const string &fileName)
{
	TIFF *pTiff = (TIFF *)pTiffVoid;
	uint32_t numXTiles = width/tileWidth + ((width%tileWidth == 0)?0:1);
	uint32_t numYTiles = height/tileHeight + ((height%tileHeight == 0)?0:1);

	uint32_t numTiles = numXTiles*numYTiles;
#if TIFFVERSION >= 4
	uint64_t *pNumTileBytes = 0;
#else
	uint32_t *pNumTileBytes = 0;
#endif
	if (!TIFFGetField(pTiff, TIFFTAG_TILEBYTECOUNTS, &pNumTileBytes))
		TIFFDENSITYFILE_ERRRETURN("Unable to get tile byte counts");
	vector<vector<Tile<T> > > tiles;

	tiles.resize(numYTiles);
	for (size_t y = 0 ; y < numYTiles ; y++)
	{
		for (size_t x = 0 ; x < numXTiles ; x++)
		{
			size_t tileNumber = x+y*numXTiles;
			size_t s = pNumTileBytes[tileNumber];

			assert(s == tileWidth*tileHeight*sizeof(T));
			
			tiles[y].push_back(Tile<T>(s/sizeof(T)));
			TIFFReadRawTile(pTiff, tileNumber, tiles[y][x].getData(), s);
		}
	}

	m_values.resize(width*height);

	bool gotNeg = false;
	for (size_t y = 0 ; y < height ; y++)
	{
		for (size_t x = 0 ; x < width ; x++)
		{
			size_t tx = x/tileWidth;
			size_t ty = y/tileHeight;
			size_t xp = x%tileWidth;
			size_t yp = y%tileHeight;

			T *pData = tiles[ty][tx].getData();
			T val = pData[xp+yp*tileWidth];

			if (val < 0 && noNeg)
			{
				gotNeg = true;
				val = 0;
			}

			m_values[x+y*width] = val;
		}
	}


	if (gotNeg)
		cerr << "# WARNING! Ignoring negative values when reading " << fileName << endl;

	return true;
}
