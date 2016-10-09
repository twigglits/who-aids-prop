#include "tiffdensityfile.h"
#include <stdint.h>
#include <tiffio.h>
#include <iostream>

using namespace std;

TIFFDensityFile::TIFFDensityFile()
{
	m_width = 0;
	m_height = 0;
}

TIFFDensityFile::~TIFFDensityFile()
{
}

bool TIFFDensityFile::init(const string &fileName, bool noNegativeValues)
{
	if (m_values.size() > 0)
	{
		setErrorString("Already initialized");
		return false;
	}

	TIFFErrorHandler oldHandler = TIFFSetWarningHandler(NULL);

	bool status = readTiffFile(fileName, noNegativeValues);

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

bool TIFFDensityFile::readTiffFile(const string &fileName, bool noNeg)
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

	if (!(bps == 32 && sampleFormat == SAMPLEFORMAT_IEEEFP))
		TIFFDENSITYFILE_ERRRETURN("Only 32 bit floating point samples are currently supported");

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

	if (tileWidth && tileHeight > 0)
	{
		uint32_t numXTiles = width/tileWidth + ((width%tileWidth == 0)?0:1);
		uint32_t numYTiles = height/tileHeight + ((height%tileHeight == 0)?0:1);

		uint32_t numTiles = numXTiles*numYTiles;
#if TIFFVERSION >= 4
		uint64_t *pNumTileBytes = 0;
#else
		uint32_t *pNumTileBytes = 0;
#endif
		vector<vector<Tile> > tiles;

		if (!TIFFGetField(pTiff, TIFFTAG_TILEBYTECOUNTS, &pNumTileBytes))
			TIFFDENSITYFILE_ERRRETURN("Unable to get tile byte counts");

		tiles.resize(numYTiles);
		for (size_t y = 0 ; y < numYTiles ; y++)
		{
			for (size_t x = 0 ; x < numXTiles ; x++)
			{
				size_t tileNumber = x+y*numXTiles;
				size_t s = pNumTileBytes[tileNumber];
				
				tiles[y].push_back(Tile(s));
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

				float *pData = tiles[ty][tx].getData();
				float val = pData[xp+yp*tileWidth];

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
	}
	else
	{
		TIFFDENSITYFILE_ERRRETURN("Only tiled images are currently supported");
	}

	m_width = (int)width;
	m_height = (int)height;

	return true;
}
