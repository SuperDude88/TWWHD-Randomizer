#include "common.hpp"



std::string readNullTerminatedStr(std::istream& in, const unsigned int& offset) {
	in.seekg(offset, std::ios::beg);

	std::string ret;
	char character = '\0';
	do {
		if (!in.read(&character, sizeof(char))) {
			ret.clear();
			return ret;
		}
		ret += character;
	} while (character != '\0');

	return ret;
}

std::u16string readNullTerminatedWStr(std::istream& in, const unsigned int offset) {
	in.seekg(offset, std::ios::beg);

	std::u16string ret;
	char16_t character = u'\0';
	do {
		if (!in.read(reinterpret_cast<char*>(&character), sizeof(char16_t))) {
			ret.clear();
			return ret;
		}
		ret += character;
	} while (character != u'\0');

	return ret;
}


size_t padToLen(std::ostream& out, const unsigned int& len, const char pad) {
	if (len == 0) return 0; //don't pad to no alignment (also cant % by 0)

	size_t padLen = len - (static_cast<size_t>(out.tellp()) % len);
	if (padLen == len) return 0; //doesnt write any padding, return length 0

	for (size_t i = 0; i < padLen; i++) {
		out.write(&pad, 1);
	}

	return padLen; //return number of bytes written
}

HSV RGBToHSV(const double& r, const double& g, const double& b) {
	double min, max, delta;

	HSV out;

	min = r < g ? r : g;
    min = min < b ? min : b;

    max = r > g ? r : g;
    max = max > b ? max : b;

    out.V = max;
    delta = max - min;

	if(max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
        out.S = (delta / max);          // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.S = 0.0;
    }

	if (delta < 0.001) {
		out.H = 0.0;
	} else {
		if (r >= max)  {
            out.H = (g - b) / delta;        // between yellow & magenta
		} else if (g >= max) {
			out.H = 2.0 + (b - r) / delta;  // between cyan & yellow
		} else {
			out.H = 4.0 + (r - g) / delta;  // between magenta & cyan
		}
			
		out.H *= 60.0;                      // degrees

		if(out.H < 0.0) {
			out.H += 360.0;
		}
	}    
    return out;
}

RGBA<double> HSVToRGB(const HSV& hsv) {
	double      hh, p, q, t, ff;
    long        i;
    RGBA<double> out;

    if(hsv.S <= 0.0) { 
        out.R = hsv.V;
        out.G = hsv.V;
        out.B = hsv.V;
        return out;
    }
    hh = hsv.H;
    if (hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = hsv.V * (1.0 - hsv.S);
    q = hsv.V * (1.0 - (hsv.S * ff));
    t = hsv.V * (1.0 - (hsv.S * (1.0 - ff)));

    switch(i) {
    case 0:
        out.R = hsv.V;
        out.G = t;
        out.B = p;
        break;
    case 1:
        out.R = q;
        out.G = hsv.V;
        out.B = p;
        break;
    case 2:
        out.R = p;
        out.G = hsv.V;
        out.B = t;
        break;
    case 3:
        out.R = p;
        out.G = q;
        out.B = hsv.V;
        break;
    case 4:
        out.R = t;
        out.G = p;
        out.B = hsv.V;
        break;
    case 5:
    default:
        out.R = hsv.V;
        out.G = p;
        out.B = q;
        break;
    }
    return out;   
}