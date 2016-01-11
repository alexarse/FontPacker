//
//  FontTexture.cpp
//  FontTexture
//
//  Created by Alexandre Arsenault on 2016-01-07.
//  Copyright © 2016 ax. All rights reserved.
//

#include "FontTexture.hpp"

#include <iomanip>

namespace ft {

Font::CharData::CharData()
	: character(0)
	, rect(-1, -1, -1, -1)
	, delta(0, 0)
	, x_next(0)
	, flipped(false)
	, buffer(nullptr)
{
}

Font::Font()
	: _ft_ready(false)
	, _face_ready(false)
	, _font_size(0)
{
	if (FT_Init_FreeType(&_freeType)) {
		ax::Error("Could not init freetype library.");
		FT_Done_FreeType(_freeType);
	}
	else {
		_ft_ready = true;
	}
}

bool Font::LoadFont(const std::string& font_path)
{
	// Zero mean succes.
	if (FT_New_Face(_freeType, font_path.c_str(), 0, &_face) == 0) {
		_face_ready = true;
		return true;
	}

	ax::Error("Can't open font :", font_path);
	return false;
}

bool Font::SetFontSize(const int& size)
{
	if (_face_ready) {
		_font_size = size;
		FT_Set_Pixel_Sizes(_face, 0, size);
		return true;
	}

	ax::Error("Can't set font size.");

	return false;
}

bool Font::IsFreetypeReady() const
{
	return _ft_ready;
}

bool Font::IsFontReady() const
{
	return _face_ready;
}

Font::CharData Font::SetChar(const char& c)
{
	CharData c_data;

	if (FT_Load_Char(_face, c, FT_LOAD_RENDER) != 0) {
		ax::Error("Could not load character", c);
		return c_data;
	}

	FT_GlyphSlot glyph = _face->glyph;
	
	c_data.character = c;
	c_data.rect.size = ax::Size(glyph->bitmap.width, glyph->bitmap.rows);
	c_data.delta = ax::Point(_face->glyph->bitmap_left, _face->glyph->bitmap_top);
	c_data.x_next = glyph->advance.x / 64.0;

	const std::size_t buffer_size(c_data.rect.size.x * c_data.rect.size.y);

	// Copy font buffer data.
	c_data.buffer = new unsigned char[buffer_size];
	memcpy(c_data.buffer, glyph->bitmap.buffer, buffer_size);

	return c_data;
}

void Font::PrintCharData(const Font::CharData& c_data)
{
	const ax::Size& size(c_data.rect.size);
	
	std::cout << "Char  : " << c_data.character << std::endl;
	std::cout << "Size  : " << c_data.rect.size.x << " " << c_data.rect.size.y << std::endl;
	std::cout << "Delta : " << c_data.delta.x << " " << c_data.delta.y << std::endl;
	std::cout << "XNext : " << c_data.x_next << std::endl;
	std::cout << "Buffer data :" << std::endl;
	
	for(int y = 0; y < size.y; y++) {
		for(int x = 0; x < size.x; x++) {
			std::cout << std::setw(3) << (int)c_data.buffer[y * size.y + x] << " ";
		}
		std::cout << std::endl;
	}
}
}
