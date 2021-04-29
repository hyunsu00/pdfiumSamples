// image_png.h
#pragma once

#include <vector> // std::vector
// #include <span> // std::span (C++20)
#include "span.h"
#include <stdint.h> // uint8_t
namespace std {
    template<typename T>
    using span = typename tcb::span<T>;
}

namespace image { namespace png {

    // Decode a PNG into an RGBA pixel array, or BGRA pixel array if
    // |reverse_byte_order| is set to true.
    std::vector<uint8_t> DecodePNG(
        const std::span<const uint8_t>& input,
        bool reverse_byte_order,
        int* width,
        int* height
    );

    // Encode a BGR pixel array into a PNG.
    std::vector<uint8_t> EncodeBGRPNG(
        const std::span<const uint8_t>& input,
        int width,
        int height,
        int row_byte_width
    );

    // Encode an RGBA pixel array into a PNG.
    std::vector<uint8_t> EncodeRGBAPNG(
        const std::span<const uint8_t>& input,
        int width,
        int height,
        int row_byte_width
    );

    // Encode an BGRA pixel array into a PNG.
    std::vector<uint8_t> EncodeBGRAPNG(
        const std::span<const uint8_t>& input,
        int width,
        int height,
        int row_byte_width,
        bool discard_transparency
    );

    // Encode a grayscale pixel array into a PNG.
    std::vector<uint8_t> EncodeGrayPNG(
        const std::span<const uint8_t>& input,
        int width,
        int height,
        int row_byte_width
    );

}} // image::png
