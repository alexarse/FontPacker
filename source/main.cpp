#include "FontTexture.hpp"

#include "MaxRectsBinPack.h"
#include "ShelfBinPack.h"
#include <cstdio>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

std::vector<ft::Font::CharData> LoadFontData(const std::string& font_path, const int& font_size)
{
	std::vector<ft::Font::CharData> char_data;

	ft::Font font;

	if (!font.IsFreetypeReady()) {
		return char_data;
	}

	if (!font.LoadFont(font_path)) {
		return char_data;
	}

	if (!font.SetFontSize(font_size)) {
		return char_data;
	}

	// 95 characters.
	std::string all_chars(" !\"#$%&'()*+,-./0123456789:;<=>?"
						  "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
						  "`abcdefghijklmnopqrstuvwxyz{|}~");

	//	ax::Print("NUMBER OF CHARS :", all_chars.size());

	char_data.reserve(all_chars.size());

	for (auto& n : all_chars) {
		ft::Font::CharData c_data = font.SetChar(n);

		if (c_data.rect.size.x == -1 || c_data.rect.size.y == -1) {
			return std::vector<ft::Font::CharData>();
		}

		char_data.push_back(c_data);
	}

	return char_data;
}

bool PackFont(const ax::Size& texture_size, std::vector<ft::Font::CharData>& char_data)
{
	using namespace rbp;

	// Create a bin to pack to, use the bin size from command line.
	MaxRectsBinPack bin(texture_size.x, texture_size.y);
	//	ShelfBinPack bin(texture_size.x, texture_size.y, false);

	//	printf("Initializing bin to size %dx%d.\n", texture_size.x, texture_size.y);
	//	bin.Init(texture_size.x, texture_size.y);

	// Pack each rectangle (w_i, h_i) the user inputted on the command line.
	for (int i = 0; i < char_data.size(); i++) {

		if (char_data[i].rect.size.x > 0 && char_data[i].rect.size.y > 0) {
			// Read next rectangle to pack.
			ax::Size size(char_data[i].rect.size);

			// Perform the packing.
			// This can be changed individually even for each rectangle packed.
			MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBestShortSideFit;
			//			ShelfBinPack::ShelfChoiceHeuristic heuristic = ShelfBinPack::ShelfNextFit;
			Rect packedRect = bin.Insert(size.x, size.y, heuristic);

			// Test success or failure.
			if (packedRect.height <= 0) {
				ax::Error("Failed! Could not find a proper position to pack this rectangle into.");
				return false;
			}

			if (packedRect.width != char_data[i].rect.size.x) {
				char_data[i].flipped = true;
			}

			char_data[i].rect.position = ax::Point(packedRect.x, packedRect.y);
		}
	}

	ax::Print(ax::Console::Color::Green, "All characters where packed in bin.");
	ax::Print("Bin packed at", bin.Occupancy(), "%");

	return true;
}

struct ExportCharData {
	ax::Rect rect;
	ax::Point delta;
	int x_next;
	int flipped;
};

void ExportFontData(const std::string& file_path, const ax::Size& texture_size,
	std::vector<ft::Font::CharData>& char_data, std::vector<unsigned char>& buffer)
{
	std::ofstream outfile;
	outfile.open(file_path, std::ios::binary | std::ios::out);
	outfile.write((const char*)&texture_size, sizeof(ax::Size));

	//	ax::Print("CHAR DATA SIZE :", char_data.size());
	ax::Print("Number of characters in font pack :", char_data.size());

	for (auto& n : char_data) {
		ExportCharData c_data;
		c_data.rect = n.rect;
		c_data.delta = n.delta;
		c_data.x_next = n.x_next;
		c_data.flipped = (int)n.flipped;
		outfile.write((const char*)&c_data, sizeof(ExportCharData));
	}

	outfile.write((const char*)buffer.data(), texture_size.x * texture_size.y * sizeof(unsigned char));
	outfile.close();
}

void PrintProgramHelp()
{
	ax::Print(ax::Console::Color::BoldGreen, "Usage :");
	ax::Print(ax::Console::Color::Blue, "[1]", "font_file_path");
	ax::Print(ax::Console::Color::Blue, "[2]", "font_size");
	ax::Print(ax::Console::Color::Blue, "[3]", "out_width");
	ax::Print(ax::Console::Color::Blue, "[4]", "out_height");
	ax::Print(ax::Console::Color::Blue, "[5]", "output_file_path");
	ax::Print(ax::Console::Color::Blue, "[6]", "show_font_bin [default = FALSE]");
	ax::Print(ax::Console::Color::Blue, "[7]",  "show_font_data [default = FALSE]");
}

struct ProgramArgs {
	int font_size = 12;
	int pack_bin_width = 100;
	int pack_bin_height = 100;
	bool show_img = false;
	bool show_font_data = false;
	std::string input_file;
	std::string output_file;
};

bool str_is_number(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

int ParseArguments(int argc, const char* argv[], ProgramArgs& args)
{
	if (argc < 6) {
		ax::Print(ax::Console::Color::BoldRed, "Error : Not enough arguments.");
		return 1;
	}

	if(!str_is_number(std::string(argv[2]))) {
		ax::Print(ax::Console::Color::BoldRed, "Error : font_size is not a valid number.");
		return 1;
	}
	
	if(!str_is_number(std::string(argv[3]))) {
		ax::Print(ax::Console::Color::BoldRed, "Error : pack_bin_width is not a valid number.");
		return 1;
	}
	
	if(!str_is_number(std::string(argv[4]))) {
		  ax::Print(ax::Console::Color::BoldRed, "Error : pack_bin_height is not a valid number.");
		  return 1;
	}

	try {
		args.font_size = std::stoi(argv[2]);
		args.pack_bin_width = std::stoi(argv[3]);
		args.pack_bin_height = std::stoi(argv[4]);
	}
	catch (std::exception& e) {
		ax::Print(ax::Console::Color::BoldRed, "Error : Incorrect output size.");
		return 1;
	}

	args.input_file = std::string(argv[1]);
	args.output_file = std::string(argv[5]);

	if (argc > 6 && (std::string(argv[6]) == "TRUE")) {
		args.show_img = true;
	}

	if (argc > 7 && (std::string(argv[7]) == "TRUE")) {
		args.show_font_data = true;
	}

	return 0;
}

int main(int argc, const char* argv[])
{
	ProgramArgs args;

	if (ParseArguments(argc, argv, args)) {
		PrintProgramHelp();
		return -1;
	}
	
	ax::Print("Arguments :");
	ax::Print("font_size :", args.font_size);
	ax::Print("pack_bin_width :", args.pack_bin_width);
	ax::Print("pack_bin_height :", args.pack_bin_height);
	ax::Print("show_img :", args.show_img);
	ax::Print("show_font_data :", args.show_font_data);
	ax::Print("input_file :", args.input_file);
	ax::Print("output_file :", args.output_file);

	//	const std::string font_path("FreeSansBold.ttf");
	const std::string font_path(args.input_file);
	const int font_size = args.font_size;

	std::vector<ft::Font::CharData> char_data = LoadFontData(font_path, font_size);

	if (char_data.empty()) {
		ax::Print(ax::Console::Color::BoldRed, "Error : Could not load all character in font.");
		return -1;
	}

	//	ax::Print("Total character number :", char_data.size());
	//	ax::Print("All char loaded.");
	//
	//	int total_width = 0;
	//	int total_height = 0;
	//
	//	for (auto& n : char_data) {
	//		total_width += n.rect.size.x;
	//		total_height += n.rect.size.y;
	//	}
	//
	//	ax::Print("W total :", total_width);
	//	ax::Print("H total :", total_height);

	ax::Size texture_size(args.pack_bin_width, args.pack_bin_height);

	if (!PackFont(texture_size, char_data)) {
		ax::Print(ax::Console::Color::BoldRed, "Characters don't fit in texture size.");
		return -1;
	}

	//	// Print characters content.
	//	for (auto& n : char_data) {
	//		ax::Print(n.character, n.rect.position.x, n.rect.position.y, n.rect.size.x, n.rect.size.y);
	//	}


//	char character;
//	ax::Rect rect;
//	ax::Point delta;
//	int x_next;
//	bool flipped;
//	unsigned char* buffer;
	if (args.show_font_data) {
		for (auto& n : char_data) {
			ax::Print("character =", n.character, ": rect = (", n.rect.position.x, n.rect.position.y, n.rect.size.x,
				n.rect.size.y, "), delta = (", n.delta.x, n.delta.y, "), x_next =", n.x_next, ", flipped =", n.flipped ? "TRUE" : "FALSE");
		}
	}

	const std::size_t buffer_size = texture_size.x * texture_size.y;
	std::vector<unsigned char> buffer(buffer_size, (char)0);

	// Copy font buffer data to rectangle font bin.
	for (auto& n : char_data) {
		const ax::Point& char_pos(n.rect.position);
		const ax::Size& char_size(n.rect.size);

		if (n.flipped) {
			for (int y = 0; y < char_size.y; y++) {
				for (int x = 0; x < char_size.x; x++) {
					const std::size_t b_index = (char_pos.y + x) * texture_size.x + char_pos.x + y;
					const std::size_t c_index = y * char_size.x + x;
					buffer[b_index] = n.buffer[c_index];
				}
			}
		}
		else {
			for (int y = 0; y < char_size.y; y++) {
				for (int x = 0; x < char_size.x; x++) {
					const std::size_t b_index = (char_pos.y + y) * texture_size.x + char_pos.x + x;
					const std::size_t c_index = y * char_size.x + x;
					buffer[b_index] = n.buffer[c_index];
				}
			}
		}
	}

	// Export binary font data to output file.
	ExportFontData(args.output_file, texture_size, char_data, buffer);

	if (args.show_img) {
		cv::Mat imageWithData(texture_size.y, texture_size.x, CV_8U, (void*)buffer.data());

		if (!imageWithData.data) {
			std::cout << "Could not open or find the image" << std::endl;
			return -1;
		}

		namedWindow("Display window", cv::WINDOW_AUTOSIZE);
		imshow("Display window", imageWithData);

		// Wait for a keystroke in the window.
		cv::waitKey(0);
	}
	return 0;
}