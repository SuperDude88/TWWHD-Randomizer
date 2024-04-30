#pragma once

#include <utility/common.hpp>

struct HSV {
	double H = 0;
	double S = 0;
	double V = 0;

	HSV() = default;

	HSV(const double& h_, const double& s_, const double& v_) :
		H(h_),
		S(s_),
		V(v_)
	{}
};

HSV RGBToHSV(const double& r, const double& g, const double& b);

HSV RGBToHSV(RGBA<double> color);

RGBA<double> HSVToRGB(const HSV& hsv);

HSV color16BitToHSV(const uint16_t& color);

uint16_t colorHSVTo16Bit(const HSV& hsv);

RGBA<double> hexColorStrToRGB(const std::string& hexColor);

std::string RGBToHexColorStr(const RGBA<double>& color);

uint16_t hexColorStrTo16Bit(const std::string& hexColor);

bool isValidHexColor(const std::string& hexColor);

uint16_t colorExchange(const uint16_t& baseColor, const uint16_t& replacementColor, const uint16_t& curColor);

std::string HSVShiftColor(const std::string& hexColor, const int& hShift, const int& vShift);

std::pair<int, int> get_random_h_and_v_shifts_for_custom_color(const std::string& hexColor);
