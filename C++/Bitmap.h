#pragma once

#include <cstdint>
#include <fstream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <limits>
#include <functional>
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>



//This class provides easy manipulation of colors in an image.
struct Color
{
public:
	Color();
	Color(float r, float g, float b);
	Color(uint32_t col);
	Color(const Color& other);

	//Add a Color channelwise
	Color& operator+=(const Color& other);
	//Subtract a Color channelwise
	Color& operator-=(const Color& other);
	//Multiply a Color channelwise
	Color& operator*=(const Color& other);
	//Multiply each color channel by fac
	Color& operator*=(float fac);
	friend Color operator+(Color c1, const Color& c2);
	friend Color operator-(Color c1, const Color& c2);
	friend Color operator*(Color c1, const Color& c2);
	friend Color operator*(Color col, float fac);
	friend Color operator*(float fac, Color col);
	bool operator==(const Color& other) const;
	bool operator!=(const Color& other) const;

	//Mix this color with another color
	//fac == 0.0 : 100% this  color
	//fac == 1.0 : 100% other color
	Color& mix(const Color& other, float fac);
	friend Color mix(Color c1, const Color& c2, float fac);

	//Return the color channels as integers (optionally provide the number of bits used)
	uint8_t intR() const;
	uint32_t intR(int bitcount) const;
	uint8_t intG() const;
	uint32_t intG(int bitcount) const;
	uint8_t intB() const;
	uint32_t intB(int bitcount) const;

private:
	float r, g, b;
};


//Overload the hasher for Color instances
namespace std
{
	template<>
	struct hash<Color>
	{
		size_t operator()(const Color& col) const
		{
			const int bpc = std::numeric_limits<size_t>::digits / 4;	//Bits per Color Channel
			//Use integer representation of the color as a hash
			return (col.intR(bpc) << (2 * bpc)) | (col.intG(bpc) << bpc) || col.intB(bpc);
		}
	};
}



//Class for saving Windows Bitmap (BMP) Files
//No support for reading bitmaps from file yet
//Very basic line drawing support
class Bitmap
{
public:
	//Indicate the quality of the saved image
	//Determines the colordepth
	enum struct Quality { LOW, MEDIUM, HIGH };

	Bitmap(uint32_t width, uint32_t height);
	Bitmap(uint32_t width, uint32_t height, Color fillcolor);
	Bitmap(const Bitmap& other);
	Bitmap(Bitmap&& other);

	Bitmap& operator=(const Bitmap& other);
	Bitmap& operator=(Bitmap&& other);
	//Access pixel color values
	Color& operator()(uint32_t x, uint32_t y);
	Color operator()(uint32_t x, uint32_t y) const;
	//Linera interpolation between the pixels
	Color operator()(float x, float y) const;
	Bitmap& operator+=(const Bitmap& other);
	Bitmap& operator-=(const Bitmap& other);
	Bitmap& operator*=(const Bitmap& other);
	Bitmap& operator*=(float fac);
	friend Bitmap operator+(Bitmap b1, const Bitmap& b2);
	friend Bitmap operator-(Bitmap b1, const Bitmap& b2);
	friend Bitmap operator*(Bitmap b1, const Bitmap& b2);
	friend Bitmap operator*(Bitmap b, float fac);
	friend Bitmap operator*(float fac, Bitmap b);

	//Draw a line with thickness 1 and no AA
	void drawLine(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, const Color& color);

	//Save the Image to a BMP File
	//quality == LOW :    16bpp ( 5bits per channel)
	//quality == MEDIUM : 24bpp ( 8bits per channel)
	//quality == HIGH   : 32bpp (10bits per channel)
	//The quality tag only gets used if the file contains more than 256 colors and
	//thus cannot use a colortable
	void save(const std::string& path, Quality quality = Quality::MEDIUM) const;

	//Get the dimensions of the image
	uint32_t Width() const;
	uint32_t Height() const;


private:
	const int MAX_COLORTABLE_SIZE = 256;
	uint32_t width;
	uint32_t height;
	std::vector<Color> image;

	std::unordered_map<Color, uint8_t> generateColorTable() const;
	void saveColorTable(std::ostream& os, const std::unordered_map<Color, uint8_t>& colortable) const;
	void saveImageData1bpp(std::ostream& os, const std::unordered_map<Color, uint8_t>& colortable) const;
	void saveImageData4bpp(std::ostream& os, const std::unordered_map<Color, uint8_t>& colortable) const;
	void saveImageData8bpp(std::ostream& os, const std::unordered_map<Color, uint8_t>& colortable) const;
	void saveImageData16bpp(std::ostream& os) const;
	void saveImageData24bpp(std::ostream& os) const;
	void saveImageData32bpp(std::ostream& os) const;
	template<typename T> std::ostream& write(std::ostream& os, T value, int length) const;

	uint32_t ceil4(uint32_t n) const;

	void swap(Bitmap& other);
};



//---------- Color Implementation ----------//

Color::Color() : r(0), g(0), b(0)
{
}


Color::Color(float r, float g, float b) : r(r), g(g), b(b)
{
	assert(r >= 0 && r <= 1);
	assert(g >= 0 && g <= 1);
	assert(b >= 0 && b <= 1);
}


Color::Color(uint32_t col) : r((float)((col & 0x00FF0000) >> 16) / 255),
							 g((float)((col & 0x0000FF00) >>  8) / 255),
							 b((float)((col & 0x000000FF) >>  0) / 255)
{
}


Color::Color(const Color& other) : r(other.r), g(other.g), b(other.b)
{
}


inline Color& Color::operator+=(const Color& other)
{
	r = std::max(0.0f, std::min(1.0f, r + other.r));
	g = std::max(0.0f, std::min(1.0f, g + other.g));
	b = std::max(0.0f, std::min(1.0f, b + other.b));
	return *this;
}


inline Color& Color::operator-=(const Color& other)
{
	r = std::max(0.0f, std::min(1.0f, r - other.r));
	g = std::max(0.0f, std::min(1.0f, g - other.g));
	b = std::max(0.0f, std::min(1.0f, b - other.b));
	return *this;
}


inline Color& Color::operator*=(const Color& other)
{
	r = std::max(0.0f, std::min(1.0f, r * other.r));
	g = std::max(0.0f, std::min(1.0f, g * other.g));
	b = std::max(0.0f, std::min(1.0f, b * other.b));
	return *this;
}


inline Color& Color::operator*=(float fac)
{
	r = std::max(0.0f, std::min(1.0f, r * fac));
	g = std::max(0.0f, std::min(1.0f, g * fac));
	b = std::max(0.0f, std::min(1.0f, b * fac));
	return *this;
}


inline Color operator+(Color c1, const Color& c2)
{
	c1 += c2;
	return c1;
}


inline Color operator-(Color c1, const Color& c2)
{
	c1 -= c2;
	return c1;
}


inline Color operator*(Color c1, const Color& c2)
{
	c1 *= c2;
	return c1;
}


inline Color operator*(Color col, float fac)
{
	col *= Color(fac, fac, fac);
	return col;
}


inline Color operator*(float fac, Color col)
{
	col *= Color(fac, fac, fac);
	return col;
}


inline uint8_t Color::intR() const
{
	return (uint8_t)(255 * r);
}


inline uint32_t Color::intR(int bitcount) const
{
	return (int32_t)(((1 << bitcount) - 1) * r);
}


inline uint8_t Color::intG() const
{
	return (uint8_t)(255 * g);
}


inline uint32_t Color::intG(int bitcount) const
{
	return (int32_t)(((1 << bitcount) - 1) * g);
}


inline uint8_t Color::intB() const
{
	return (uint8_t)(255 * b);
}


inline uint32_t Color::intB(int bitcount) const
{
	return (int32_t)(((1 << bitcount) - 1) * b);
}


inline bool Color::operator==(const Color& other) const
{
	return r == other.r && g == other.g && b == other.b;
}


inline bool Color::operator!=(const Color& other) const
{
	return !(*this == other);
}


inline Color& Color::mix(const Color& other, float fac)
{
	*this = fac * other + (1.0 - fac) * *this;
	return *this;
}


inline Color mix(Color c1, const Color& c2, float fac)
{
	return c1.mix(c2, fac);
}



//---------- Bitmap Implementation ----------//

Bitmap::Bitmap(uint32_t width, uint32_t height) : width(width), height(height), image(width*height, Color())
{
	assert(width  < 0x80000000);
	assert(height < 0x80000000);
}


Bitmap::Bitmap(uint32_t width, uint32_t height, Color fillcolor) : width(width), height(height), image(width*height, fillcolor)
{
	assert(width  < 0x80000000);
	assert(height < 0x80000000);
}


Bitmap::Bitmap(const Bitmap& other)
{
	this->operator=(other);
}


Bitmap::Bitmap(Bitmap&& other) : width(0), height(0), image(0, Color())
{
	swap(other);
}


Bitmap& Bitmap::operator=(const Bitmap& other)
{
	width = other.width;
	height = other.height;
	image = other.image;
	return *this;
}


Bitmap& Bitmap::operator=(Bitmap&& other)
{
	swap(other);
	return *this;
}


inline Color& Bitmap::operator()(uint32_t x, uint32_t y)
{
	assert(x < width && y < height);
	return image[y*width + x];
}


inline Color Bitmap::operator()(uint32_t x, uint32_t y) const
{
	assert(x < width && y < height);
	return image[y*width + x];
}


inline Color Bitmap::operator()(float x, float y) const
{
	Color c00 = this->operator()((uint32_t)std::floor(x), (uint32_t)std::floor(y));
	Color c01 = this->operator()((uint32_t)std::floor(x), (uint32_t)std::ceil(y));
	Color c10 = this->operator()((uint32_t)std::ceil(x), (uint32_t)std::floor(y));
	Color c11 = this->operator()((uint32_t)std::ceil(x), (uint32_t)std::ceil(y));
	Color c0 = (y - std::floor(y)) * c00 + (1.0 - y + std::floor(y)) * c01;
	Color c1 = (y - std::floor(y)) * c10 + (1.0 - y + std::floor(y)) * c11;
	return (x - std::floor(x)) * c0 + (1.0 - x + std::floor(x)) * c1;
}


inline Bitmap& Bitmap::operator+=(const Bitmap& other)
{
	assert(width == other.width && height == other.height);
	for (size_t i = 0; i < image.size(); ++i)
		image[i] += other.image[i];
	return *this;
}


inline Bitmap& Bitmap::operator-=(const Bitmap& other)
{
	assert(width == other.width && height == other.height);
	for (size_t i = 0; i < image.size(); ++i)
		image[i] -= other.image[i];
	return *this;
}


inline Bitmap& Bitmap::operator*=(const Bitmap& other)
{
	assert(width == other.width && height == other.height);
	for (size_t i = 0; i < image.size(); ++i)
		image[i] *= other.image[i];
	return *this;
}


inline Bitmap& Bitmap::operator*=(float fac)
{
	for (size_t i = 0; i < image.size(); ++i)
		image[i] *= fac;
	return *this;
}


inline Bitmap operator+(Bitmap b1, const Bitmap& b2)
{
	b1 += b2;
	return b1;
}


inline Bitmap operator-(Bitmap b1, const Bitmap& b2)
{
	b1 -= b2;
	return b1;
}


inline Bitmap operator*(Bitmap b1, const Bitmap& b2)
{
	b1 *= b2;
	return b1;
}


inline Bitmap operator*(Bitmap b, float fac)
{
	b *= fac;
	return b;
}


inline Bitmap operator*(float fac, Bitmap b)
{
	b *= fac;
	return b;
}


void Bitmap::drawLine(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, const Color& color)
{
	int32_t dx = x2 - x1;
	int32_t dy = y2 - y1;
	if (dx >= 0 && ((dy >= 0 && dy <= dx) || (dy <= 0 && -dy <= dx))) {		//First and eighth octant
		float m = ((float)dy) / dx;
		float err = 0.5;
		uint32_t y = y1;
		for (uint32_t x = x1; x <= x2; ++x) {
			err += m;
			if (err >= 1) {
				y += 1;
				err -= 1;
			}
			if (err <= -1) {
				y -= 1;
				err += 1;
			}
			this->operator()(x, y) = color;
		}
	}
	else if (dx <= 0 && ((dy >= 0 && dy <= -dx) || (dy <= 0 && -dy <= -dx))) {		//Fourth and fifth octant
		float m = ((float)dy) / dx;
		float err = 0.5;
		uint32_t y = y2;
		for (uint32_t x = x2; x <= x1; ++x) {
			err += m;
			if (err >= 1) {
				y += 1;
				err -= 1;
			}
			if (err <= -1) {
				y -= 1;
				err += 1;
			}
			this->operator()(x, y) = color;
		}
	}
	else if (dy <= 0 && ((dx >= 0 && dx <= -dy) || (dx <= 0 && -dx <= -dy))) {		//Sixth and seventh octant
		float m = ((float)dx) / dy;
		float err = 0.5;
		uint32_t x = x2;
		for (uint32_t y = y2; y <= y1; ++y) {
			err += m;
			if (err >= 1) {
				x += 1;
				err -= 1;
			}
			if (err <= -1) {
				x -= 1;
				err += 1;
			}
			this->operator()(x, y) = color;
		}
	}
	else if (dy >= 0 && ((dx >= 0 && dx <= dy) || (dx <= 0 && -dx <= dy))) {		//Second and third octant
		float m = ((float)dx) / dy;
		float err = 0.5;
		uint32_t x = x1;
		for (uint32_t y = y1; y <= y2; ++y) {
			err += m;
			if (err >= 1) {
				x += 1;
				err -= 1;
			}
			if (err <= -1) {
				x -= 1;
				err += 1;
			}
			this->operator()(x, y) = color;
		}
	}
}


void Bitmap::save(const std::string& path, Bitmap::Quality quality) const
{
	std::ofstream file(path, std::ofstream::out | std::ofstream::binary);
	uint32_t bfSize;
	uint16_t biBitCount;
	uint32_t biCompression = 0;
	uint32_t biClrUsed;

	//Make Place for the file and info header
	for (int i = 0; i < 54; ++i)
		write(file, 0, 1);

	//Write the color table
	auto colortable = generateColorTable();
	biClrUsed = colortable.size();
	//Only use colortable if less than 256 colors are used and the colordepth is less than 256
	biClrUsed = biClrUsed <= MAX_COLORTABLE_SIZE ? biClrUsed : 0;
	//Determine the colordepth
	if (biClrUsed) {
		if (biClrUsed <= 2)
			biBitCount = 1;
		else if (biClrUsed <= 16)
			biBitCount = 4;
		else
			biBitCount = 8;
	}
	else {	//Bit Count determined by quality tag
			if (quality == Quality::LOW)
				biBitCount = 16;
			else if (quality == Quality::MEDIUM)
				biBitCount = 24;
			else {
				biBitCount = 32;
				biCompression = 3;	//User defined 10bit colors
			}
		}

	//Save the bitmask if 32bits are used to achieve 10bit colors
	if (biCompression == 3) {
		write(file, 0x3FF00000, 4);	//Red
		write(file, 0x000FFC00, 4);	//Green
		write(file, 0x000003FF, 4);	//Blue
	}
	//Save the colortable if one should be used
	if (biClrUsed)
		saveColorTable(file, colortable);
	//Save the Image Data
	switch (biBitCount) {
	case 1:
		saveImageData1bpp(file, colortable);
		break;
	case 4:
		saveImageData4bpp(file, colortable);
		break;
	case 8:
		saveImageData8bpp(file, colortable);
		break;
	case 16:
		saveImageData16bpp(file);
		break;
	case 24:
		saveImageData24bpp(file);
		break;
	case 32:
		saveImageData32bpp(file);
		break;
	}
	
	//Write the file and info headers
	bfSize = file.tellp();	//Get the file size
	file.seekp(0);

	//Fileheader
	//bfType
	write(file, 'B', 1);
	write(file, 'M', 1);
	//bfSize
	write(file, bfSize, 4);
	//bfReserved
	write(file, 0, 4);
	//bfOffBits
	write(file, 54 + biClrUsed * 4, 4);
	
	//Infoheader
	//biSize
	write(file, 40, 4);
	//biWidth
	write(file, width, 4);
	//biHeight
	write(file, height, 4);
	//biPlanes
	write(file, 1, 2);
	//biBitCount
	write(file, biBitCount, 2);
	//biCompression
	write(file, biCompression, 4);
	//bfSizeImage
	write(file, bfSize - 54 - biClrUsed * 4 , 4);
	//biXPelsPerMeter
	write(file, 0, 4);
	//biYPelsPerMeter
	write(file, 0, 4);
	//biClrUsed
	write(file, biClrUsed, 4);
	//biClrImportant
	write(file, 0, 4);

	file.close();
}


inline uint32_t Bitmap::Width() const
{
	return width;
}


inline uint32_t Bitmap::Height() const
{
	return height;
}


inline std::unordered_map<Color, uint8_t> Bitmap::generateColorTable() const
{
	std::unordered_map<Color, uint8_t> colortable;
	for (const Color& col : image) {
		if (!colortable.count(col))
			colortable.insert({ col, colortable.size() });
		if (colortable.size() > MAX_COLORTABLE_SIZE)	//Break, as there are too much colors for a colortable anyway
			break;
	}
	return colortable;
}


inline void Bitmap::saveColorTable(std::ostream& os, const std::unordered_map<Color, uint8_t>& colortable) const
{
	//Write the colortable into a vector
	std::vector<Color> table(colortable.size());
	for (const auto& col : colortable)
		table[col.second] = col.first;
	//Write the table inside the vector to os
	for (const Color& col : table) {
		write(os, col.intB(), 1);
		write(os, col.intG(), 1);
		write(os, col.intR(), 1);
		write(os, 0x00, 1);
	}
}


inline void Bitmap::saveImageData1bpp(std::ostream& os, const std::unordered_map<Color, uint8_t>& colortable) const
{
	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; x += 8) {
			uint8_t byte = 0x00;
			byte |= x + 0 < width ? (colortable.at(this->operator()(x + 0, y)) << 7) : 0;
			byte |= x + 1 < width ? (colortable.at(this->operator()(x + 1, y)) << 6) : 0;
			byte |= x + 2 < width ? (colortable.at(this->operator()(x + 2, y)) << 5) : 0;
			byte |= x + 3 < width ? (colortable.at(this->operator()(x + 3, y)) << 4) : 0;
			byte |= x + 4 < width ? (colortable.at(this->operator()(x + 4, y)) << 3) : 0;
			byte |= x + 5 < width ? (colortable.at(this->operator()(x + 5, y)) << 2) : 0;
			byte |= x + 6 < width ? (colortable.at(this->operator()(x + 6, y)) << 1) : 0;
			byte |= x + 7 < width ? (colortable.at(this->operator()(x + 7, y)) << 0) : 0;
			write(os, byte, 1);
		}
		//Pad to a multiple of 4 bytes
		write(os, 0, ceil4(std::ceil(((double)width) / 8)) - std::ceil(((double)width) / 8));
	}
}


inline void Bitmap::saveImageData4bpp(std::ostream& os, const std::unordered_map<Color, uint8_t>& colortable) const
{
	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; x += 2) {
			uint8_t byte = 0x00;
			byte |= x + 0 < width ? (0xF0 & (colortable.at(this->operator()(x + 0, y)) << 4)) : 0;
			byte |= x + 1 < width ? (0x0F & (colortable.at(this->operator()(x + 1, y)) << 0)) : 0;
			write(os, byte, 1);
		}
		//Pad to a multiple of 4 bytes
		write(os, 0, ceil4(std::ceil(((double)width) / 2)) - std::ceil(((double)width) / 2));
	}
}


inline void Bitmap::saveImageData8bpp(std::ostream& os, const std::unordered_map<Color, uint8_t>& colortable) const
{
	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			write(os, colortable.at(this->operator()(x, y)), 1);
		}
		//Pad to a multiple of 4 bytes
		write(os, 0, ceil4(width) - width);
	}
}


inline void Bitmap::saveImageData16bpp(std::ostream& os) const
{
	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			uint16_t color = 0x0000;
			color |= 0x7C00 & (this->operator()(x, y).intR(5) << 10);	//Red
			color |= 0x03E0 & (this->operator()(x, y).intG(5) << 5);	//Green
			color |= 0x001F & (this->operator()(x, y).intB(5) << 0);	//Blue
			write(os, color, 2);
		}
		//Pad to a multiple of 4 bytes
		uint32_t bpr = 2 * width;	//Bytes per Row
		write(os, 0, ceil4(bpr) - bpr);
	}
}


inline void Bitmap::saveImageData24bpp(std::ostream& os) const
{
	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			write(os, this->operator()(x, y).intB(), 1);
			write(os, this->operator()(x, y).intG(), 1);
			write(os, this->operator()(x, y).intR(), 1);
		}
		//Pad to a multiple of 4 bytes
		uint32_t bpr = 3 * width;	//Bytes per Row
		write(os, 0, ceil4(bpr) - bpr);
	}
}


inline void Bitmap::saveImageData32bpp(std::ostream& os) const
{
	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			uint32_t color = 0x00000000;
			color |= 0x000003FF & (this->operator()(x, y).intB(10) <<  0);
			color |= 0x000FFC00 & (this->operator()(x, y).intG(10) << 10);
			color |= 0x3FF00000 & (this->operator()(x, y).intR(10) << 20);
			write(os, color, 4);
		}
	}
}


template<typename T>
inline std::ostream& Bitmap::write(std::ostream& os, T value, int length) const
{
	while (length--) {
		os.put((uint8_t)value);
		value >>= 8;
	}
	return os;
}


uint32_t Bitmap::ceil4(uint32_t n) const
{
	return (uint32_t)(4*std::ceil(((double)n)/4));
}


inline void Bitmap::swap(Bitmap& other)
{
	std::swap(width, other.width);
	std::swap(height, other.height);
	std::swap(image, other.image);
}