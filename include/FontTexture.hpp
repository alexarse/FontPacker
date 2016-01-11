//
//  FontTexture.hpp
//  FontTexture
//
//  Created by Alexandre Arsenault on 2016-01-07.
//  Copyright © 2016 ax. All rights reserved.
//

#ifndef FontTexture_hpp
#define FontTexture_hpp

#include <axLib/axUtils.h>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace ft {
class Font {
public:
	struct CharData {
		CharData();
	
		char character;
		ax::Rect rect;
		ax::Point delta;
		int x_next;
		bool flipped;
		unsigned char* buffer;
	};

	Font();

	bool LoadFont(const std::string& font_path);

	bool SetFontSize(const int& size);

	CharData SetChar(const char& c);
	
	bool IsFreetypeReady() const;
	bool IsFontReady() const;
	
	static void PrintCharData(const Font::CharData& c_data);

private:
	FT_Library _freeType;
	FT_Face _face;
	bool _ft_ready;
	bool _face_ready;
	int _font_size;
};
}

#endif /* FontTexture_hpp */
