#pragma once

#include <string>
#include <cmath>

std::string rgb8torgbx8(const std::string& data);

uint16_t _swapRB_rgb565(uint16_t pixel);

uint16_t _swapRB_rgb5a1(uint16_t pixel);

uint16_t _swapRB_rgba4(uint16_t pixel);

uint16_t _swapRB_argb4(uint16_t pixel);

std::string swapRB_16bpp(const std::string& data, const std::string& format_);

std::string rgba4_to_argb4(const std::string& data);

uint32_t _swapRB_bgr10a2(uint32_t pixel);

uint32_t _swapRB_rgba8(uint32_t pixel);

std::string swapRB_32bpp(const std::string& data, const std::string& format_);
