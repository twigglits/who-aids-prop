#ifndef TIFFDENSITYFILE_H

#define TIFFDENSITYFILE_H

#include "errut/errorbase.h"
#include <assert.h>
#include <vector>

class TIFFDensityFile : public errut::ErrorBase
{
public:
	TIFFDensityFile();
	~TIFFDensityFile();

	// Note: everything will be converted to doubles!
	bool init(const std::string &fileName, bool noNegativeValues = true);
	int getWidth() const									{ return m_width; }
	int getHeight() const									{ return m_height; }
	double getValue(int x, int y) const;
	void setValue(int x, int y, double v);
private:
	bool readTiffFile(const std::string &fileName, bool noNeg);

	int m_width, m_height;
	std::vector<double> m_values;

	class Tile
	{
	public:
		Tile() 										{ }
		Tile(size_t s) 									{ m_buffer.resize(s); }
		Tile(const Tile &src)								{ m_buffer = src.m_buffer; }
		float *getData() 								{ assert(m_buffer.size() > 0); return &(m_buffer[0]); }
	private:
		std::vector<float> m_buffer;
	};
};

inline double TIFFDensityFile::getValue(int x, int y) const
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);
	
	int idx = x + y*m_width;
	assert(idx >= 0 && idx < m_values.size());
	
	return m_values[idx];
}

inline void TIFFDensityFile::setValue(int x, int y, double v)
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);
	
	int idx = x + y*m_width;
	assert(idx >= 0 && idx < m_values.size());

	m_values[idx] = v;
}

#endif // TIFFDENSITYFILE_H
