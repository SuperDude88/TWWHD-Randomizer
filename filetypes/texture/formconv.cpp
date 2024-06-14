#include "formconv.hpp"

std::string rgb8torgbx8(const std::string& data) {
    const size_t numPixels = data.size() / 3;
    std::string newData(numPixels * 4, '\0');

    for(size_t i = 0; i < numPixels; i++) {
        newData[4 * i + 0] = data[3 * i + 0];
        newData[4 * i + 1] = data[3 * i + 1];
        newData[4 * i + 2] = data[3 * i + 2];
        newData[4 * i + 3] = '\xFF';
    }

    return newData;
}

uint16_t _swapRB_rgb565(uint16_t pixel) {
    uint8_t red = pixel & 0x1F;
    uint8_t green = (pixel & 0x7E0) >> 5;
    uint8_t blue = (pixel & 0xF800) >> 11;

    return (red << 11) | (green << 5) | blue;
}

uint16_t _swapRB_rgb5a1(uint16_t pixel) {
    uint8_t red = pixel & 0x1F;
    uint8_t green = (pixel & 0x3E0) >> 5;
    uint8_t blue = (pixel & 0x7c00) >> 10;
    uint8_t alpha = (pixel & 0x8000) >> 15;

    return (alpha << 15) | (red << 10) | (green << 5) | blue;
}

uint16_t _swapRB_rgba4(uint16_t pixel) {
    uint8_t red = pixel & 0xF;
    uint8_t green = (pixel & 0xF0) >> 4;
    uint8_t blue = (pixel & 0xF00) >> 8;
    uint8_t alpha = (pixel & 0xF000) >> 12;

    return (alpha << 12) | (red << 8) | (green << 4) | blue;
}

uint16_t _swapRB_argb4(uint16_t pixel) {
    uint8_t alpha = pixel & 0xF;
    uint8_t red = (pixel & 0xF0) >> 4;
    uint8_t green = (pixel & 0xF00) >> 8;
    uint8_t blue = (pixel & 0xF000) >> 12;

    return (red << 12) | (green << 8) | (blue << 4) | alpha;
}

std::string swapRB_16bpp(const std::string& data, const std::string& format_) {
    const size_t numPixels = data.size() / 2;

    std::string new_data(numPixels * 2, '\0');

    for (size_t i = 0; i < numPixels; i++) {
        uint16_t pixel = (data[2 * i + 1] << 8) | data[2 * i + 0];

        uint16_t new_pixel;
        if (format_ == "rgb565") {
            new_pixel = _swapRB_rgb565(pixel);
        }
        else if (format_ == "rgb5a1") {
            new_pixel = _swapRB_rgb5a1(pixel);
        }
        else if (format_ == "rgba4") {
            new_pixel = _swapRB_rgba4(pixel);
        }
        else {
            new_pixel = _swapRB_argb4(pixel);
        }

        new_data[2 * i + 1] = (new_pixel & 0xFF00) >> 8;
        new_data[2 * i + 0] = new_pixel & 0xFF;
    }

    return new_data;
}

std::string rgba4_to_argb4(const std::string& data) {
    const size_t numPixels = data.size() / 2;

    std::string new_data;
    new_data.resize(numPixels * 2);

    for (size_t i = 0; i < numPixels; i++) {
        uint16_t pixel = ((data[2 * i + 1] << 8) | data[2 * i + 0]);

        uint16_t rgb = (pixel & 0xFFF);
        uint16_t alpha = (pixel & 0xF000) >> 12;

        uint16_t new_pixel = (rgb << 4) | alpha;

        new_data[2 * i + 1] = (new_pixel & 0xFF00) >> 8;
        new_data[2 * i + 0] = new_pixel & 0xFF;
    }

    return new_data;
}

uint32_t _swapRB_bgr10a2(uint32_t pixel) {
    uint8_t red = (pixel & 0x3FF00000) >> 20;
    uint8_t green = (pixel & 0xFFC00) >> 10;
    uint8_t blue = pixel & 0x3FF;
    uint8_t alpha = (pixel & 0xC0000000) >> 30;

    return (alpha << 30) | (blue << 20) | (green << 10) | red;
}

uint32_t _swapRB_rgba8(uint32_t pixel) {
    uint8_t red = pixel & 0xFF;
    uint8_t green = (pixel & 0xFF00) >> 8;
    uint8_t blue = (pixel & 0xFF0000) >> 16;
    uint8_t alpha = (pixel & 0xFF000000) >> 24;

    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

std::string swapRB_32bpp(const std::string& data, const std::string& format_) {
    const size_t numPixels = data.size() / 4;

    std::string new_data(numPixels * 4, '\0');

    for (size_t i = 0; i < numPixels; i++) {
        uint32_t pixel = (data[4 * i + 3] << 24) | (data[4 * i + 2] << 16) | (data[4 * i + 1] << 8) | data[4 * i + 0];
        uint32_t new_pixel;

        if (format_ == "bgr10a2") {
            new_pixel = _swapRB_bgr10a2(pixel);
        }
        else {
            new_pixel = _swapRB_rgba8(pixel);
        }

        new_data[4 * i + 3] = (new_pixel & 0xFF000000) >> 24;
        new_data[4 * i + 2] = (new_pixel & 0xFF0000) >> 16;
        new_data[4 * i + 1] = (new_pixel & 0xFF00) >> 8;
        new_data[4 * i + 0] = new_pixel & 0xFF;
    }

    return new_data;
}
