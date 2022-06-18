#pragma once

#include <cassert>
#include <cstdint>
#include <exception>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace tga {

namespace {
    template <typename T>
    auto read_little_endian(std::vector<uint8_t>::const_iterator& data) -> T
    {
        T write_to {};

        auto size = sizeof(write_to);
        for (uint32_t i {}; i < size; ++i) {
            write_to |= (uint64_t { *data }) << i * 8;
            ++data;
        }

        return write_to;
    }

    template <typename T>
    void write_little_endian(std::vector<uint8_t>& data, T write_to)
    {
        auto size = sizeof(write_to);
        for (uint32_t i {}; i < size; ++i) {
            data.push_back((write_to >> (i * 8)) & 0xff);
        }
    }

}

struct header {
    // https://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf

    /**
     * @brief This field identifies the number of bytes contained in Field 6, the Image ID Field.
     The maximum number of characters is 255.
     A value of zero indicates that no Image ID field is included with the image.
     *
     */
    uint8_t _id_length;
    /**
     * @brief This field indicates the type of color map (if any) included with the image. 1 or 0
     *
     */
    uint8_t _color_map_type;
    /**
     * @brief The TGA File Format can be used to store Pseudo-Color, True-Color and Direct-Color images of various pixel depths.
     *
     */
    uint8_t _image_type;

    // Color Map Specification

    /**
     * @brief Index of the first color map entry. Index refers to the starting entry in loading the color map.
     *
     */
    uint16_t _color_map_origin; // first entry index
    /**
     * @brief Total number of color map entries included.
     *
     */
    uint16_t _color_map_length;
    /**
     * @brief Establishes the number of bits per entry. Typically 15, 16, 24 or 32-bit values are used.
     *
     */
    uint8_t _color_map_entry_size;

    // Image Specification
    /**
     * @brief These bytes specify the absolute horizontal coordinate for the lower left
     corner of the image as it is positioned on a display device having an
     origin at the lower left of the screen
     *
     */
    uint16_t _x_origin;
    /**
     * @brief These bytes specify the absolute vertical coordinate for the lower left
     corner of the image as it is positioned on a display device having an
     origin at the lower left of the screen
     *
     */
    uint16_t _y_origin;
    /**
     * @brief This field specifies the width of the image in pixels
     *
     */
    uint16_t _width;
    /**
     * @brief This field specifies the height of the image in pixels
     *
     */
    uint16_t _height;
    /**
     * @brief This field indicates the number of bits per pixel. This number includes
     the Attribute or Alpha channel bits. Common values are 8, 16, 24 and
     32 but other pixel depths could be used.
     *
     */
    uint8_t _bits;
    /**
     * @brief Bits 3-0: These bits specify the number of attribute bits per pixel. In the case of the TrueVista, these bits
     Bits 5 & 4: These bits are used to indicate the order in which pixel data is transferred from the file to the screen.
     Bits 7 & 6: Must be zero to insure future compatibility.
     *
     */
    uint8_t _image_descriptor;

    static constexpr size_t size = sizeof(_id_length) + sizeof(_color_map_type) + sizeof(_image_type)
        + sizeof(_color_map_origin) + sizeof(_color_map_length) + sizeof(_color_map_entry_size)
        + sizeof(_x_origin) + sizeof(_y_origin) + sizeof(_width) + sizeof(_height) + sizeof(_bits) + sizeof(_image_descriptor);

    template <typename Iter>
    static auto parse(Iter& iter, Iter const& end) -> header
    {
        header h {};

        auto header_size = header::size;

        if (iter + header_size > end) {
            throw std::runtime_error { "tga::heade::parse za malo bitow do wczytania naglowka" };
        }

        h._id_length = read_little_endian<decltype(_id_length)>(iter);
        h._color_map_type = read_little_endian<decltype(_color_map_type)>(iter);
        h._image_type = read_little_endian<decltype(_image_type)>(iter);
        h._color_map_origin = read_little_endian<decltype(_color_map_origin)>(iter);
        h._color_map_length = read_little_endian<decltype(_color_map_length)>(iter);
        h._color_map_entry_size = read_little_endian<decltype(_color_map_entry_size)>(iter);
        h._x_origin = read_little_endian<decltype(_x_origin)>(iter);
        h._y_origin = read_little_endian<decltype(_y_origin)>(iter);
        h._width = read_little_endian<decltype(_width)>(iter);
        h._height = read_little_endian<decltype(_height)>(iter);
        h._bits = read_little_endian<decltype(_bits)>(iter);
        h._image_descriptor = read_little_endian<decltype(_image_descriptor)>(iter);

        return h;
    }

    inline static auto repr(header const& h) -> std::vector<uint8_t>
    {
        std::vector<uint8_t> binary {};

        write_little_endian<decltype(h._id_length)>(binary, h._id_length);
        write_little_endian<decltype(h._color_map_type)>(binary, h._color_map_type);
        write_little_endian<decltype(h._image_type)>(binary, h._image_type);
        write_little_endian<decltype(h._color_map_origin)>(binary, h._color_map_origin);
        write_little_endian<decltype(h._color_map_length)>(binary, h._color_map_length);
        write_little_endian<decltype(h._color_map_entry_size)>(binary, h._color_map_entry_size);
        write_little_endian<decltype(h._x_origin)>(binary, h._x_origin);
        write_little_endian<decltype(h._y_origin)>(binary, h._y_origin);
        write_little_endian<decltype(h._width)>(binary, h._width);
        write_little_endian<decltype(h._height)>(binary, h._height);
        write_little_endian<decltype(h._bits)>(binary, h._bits);
        write_little_endian<decltype(h._image_descriptor)>(binary, h._image_descriptor);

        return binary;
    }

    bool operator==(const header& o) const = default;
};

inline auto operator<<(std::ostream& ostream, header const& header) -> std::ostream&
{
    // clang-format off
    ostream <<
        "{" <<
        "id_length=" << std::to_string(header._id_length) << "," <<
        "color_map_type=" << std::to_string(header._color_map_type) << "," <<
        "image_type=" << std::to_string(header._image_type) << "," <<
        "color_map_origin=" << std::to_string(header._color_map_origin) << "," <<
        "color_map_length=" << std::to_string(header._color_map_length) << "," <<
        "color_map_entry_size=" << std::to_string(header._color_map_entry_size) << "," <<
        "x_origin=" << std::to_string(header._x_origin) << "," <<
        "y_origin=" << std::to_string(header._y_origin) << "," <<
        "width=" << std::to_string(header._width) << "," <<
        "height=" << std::to_string(header._height) << "," <<
        "bits=" << std::to_string(header._bits) << "," <<
        "image_descriptor=" << std::to_string(header._image_descriptor) <<
        "}";
    // clang-format on

    return ostream;
}

enum class image_format {
    MONO,
    RGB,
    RGBA,
    ERROR
};

struct image {
    header _header {};
    std::vector<uint8_t> _data {};
    uint32_t _width {};
    uint32_t _height {};
    image_format _image_format { image_format::ERROR };

    auto from_binary(std::vector<uint8_t> const& data)
    {
        auto iter = data.begin();
        _header = header::parse(iter, data.end());
        _width = _header._width;
        _height = _header._height;

        if (iter + _header._image_descriptor >= data.end()) {
            throw std::runtime_error { "_header._image_descriptor err" };
        }
        std::vector<uint8_t> image_descriptor(iter, iter + _header._image_descriptor);
        iter += _header._image_descriptor;

        size_t colmap_elem_size = _header._color_map_entry_size / 8;
        size_t colmap_size = _header._color_map_length * colmap_elem_size;

        std::vector<uint8_t> colmap {};
        if (_header._color_map_type == 1) {
            if (iter + colmap_size > data.end()) {
                throw std::runtime_error { "colmap_size err" };
            }
            colmap = std::vector<uint8_t>(iter, iter + colmap_size);
            iter += colmap_size;
        }

        size_t bytes_per_pixel = _header._color_map_length == 0 ? (_header._bits / 8) : colmap_elem_size;
        size_t data_size = data.size() - header::size - (_header._color_map_type == 1 ? colmap_size : 0);
        size_t image_size = _header._width * _header._height * bytes_per_pixel;

        if (iter + data_size > data.end()) {
            throw std::runtime_error { "data_size err" };
        }
        std::vector<uint8_t> buffer(iter, iter + data_size);
        iter += data_size;

        _data.resize(image_size);

        bool right_first = _header._image_descriptor & (1 << 4);
        bool top_first = _header._image_descriptor & (1 << 5);

        if (right_first || top_first) {
            throw std::runtime_error { "right_first=" + std::to_string(right_first) + " top_first=" + std::to_string(top_first) };
        }

        switch (_header._image_type) {
        case 0: {
            // no image

        } break;
        case 1: {
            // uncompressed palleted
            throw std::runtime_error { "not implemented" };

            // switch (_header._bits) {
            // case 8: {
            //     switch (pixel_size) {
            //     case 3: {
            //         _image_format = image_format::RGB;
            //     } break;
            //     case 4: {
            //         throw std::runtime_error { "not implemented" };
            //     } break;
            //     default: {
            //         throw std::runtime_error { "unknown pixel_size=" + std::to_string(pixel_size) };
            //     }
            //     }
            // } break;
            // case 16: {
            //     throw std::runtime_error { "not implemented" };
            // }
            // default: {
            //     throw std::runtime_error { "unknown _header._bits=" + std::to_string(_header._bits) };
            // }
            // }
        } break;
        case 2: {
            // uncompressed truecolor
            if (_header._bits == 24) {
                for (size_t i {}; i < image_size / 3; ++i) {
                    _data[3 * i + 0] = buffer[3 * i + 2];
                    _data[3 * i + 1] = buffer[3 * i + 1];
                    _data[3 * i + 2] = buffer[3 * i + 0];
                }
                _image_format = image_format::RGB;
            } else if (_header._bits == 32) {
                for (size_t i {}; i < image_size / 4; ++i) {
                    _data[4 * i + 0] = buffer[4 * i + 2];
                    _data[4 * i + 1] = buffer[4 * i + 1];
                    _data[4 * i + 2] = buffer[4 * i + 0];
                    _data[4 * i + 3] = buffer[4 * i + 3];
                }
                _image_format = image_format::RGBA;
            } else {
                throw std::runtime_error { "unknown _header._bits=" + std::to_string(_header._bits) };
            }

        } break;
        case 3: {
            // uncompressed monochrome
            throw std::runtime_error { "not implemented" };
        } break;
        case 9: {
            // compressed paletted
            throw std::runtime_error { "not implemented" };
        } break;
        case 10: {
            // compressed truecolor
            throw std::runtime_error { "not implemented" };
        } break;
        case 11: {
            // compressed monochrome
            throw std::runtime_error { "not implemented" };
        } break;
        default: {
            throw std::runtime_error { "unknown _header._image_type=" + std::to_string(_header._image_type) };
        }
        }
    }

    auto to_binary() -> std::vector<uint8_t>
    {
        std::vector<uint8_t> binary = header::repr(_header);

        // TODO tu byl wczyt image descriptor moze jest niepotrzebny

        if (_header._color_map_type == 1) {
            // size_t colmap_elem_size = _header._color_map_entry_size / 8;
            // std::vector<uint8_t> colmap {};
            // size_t colmap_size = _header._color_map_length * colmap_elem_size;
            // if (iter + colmap_size > data.end()) {
            //     throw std::runtime_error { "mniej danych niz wskazuje colmap_size" };
            // }
            // colmap = std::vector<uint8_t>(iter, iter + colmap_size);
            // iter += colmap_size;
            throw std::runtime_error { "TODO: tga::image.to_binary() nie obsluguje colormap" };
        }

        std::vector<uint8_t> transformed_data {};
        auto image_size = _data.size();
        transformed_data.resize(image_size);

        bool right_first = _header._image_descriptor & (1 << 4);
        bool top_first = _header._image_descriptor & (1 << 5);

        if (right_first || top_first) {
            throw std::runtime_error { "right_first=" + std::to_string(right_first) + " top_first=" + std::to_string(top_first) };
        }

        switch (_header._image_type) {
        case 0: {
            // no image

        } break;
        case 1: {
            // uncompressed palleted
            throw std::runtime_error { "not implemented" };

            // switch (_header._bits) {
            // case 8: {
            //     switch (pixel_size) {
            //     case 3: {
            //         _image_format = image_format::RGB;
            //     } break;
            //     case 4: {
            //         throw std::runtime_error { "not implemented" };
            //     } break;
            //     default: {
            //         throw std::runtime_error { "unknown pixel_size=" + std::to_string(pixel_size) };
            //     }
            //     }
            // } break;
            // case 16: {
            //     throw std::runtime_error { "not implemented" };
            // }
            // default: {
            //     throw std::runtime_error { "unknown _header._bits=" + std::to_string(_header._bits) };
            // }
            // }
        } break;
        case 2: {
            // uncompressed truecolor
            if (_image_format == image_format::RGB) {
                for (size_t i {}; i < image_size / 3; ++i) {
                    transformed_data[3 * i + 2] = _data[3 * i + 0];
                    transformed_data[3 * i + 1] = _data[3 * i + 1];
                    transformed_data[3 * i + 0] = _data[3 * i + 2];
                }
            } else if (_image_format == image_format::RGBA) {
                for (size_t i {}; i < image_size / 4; ++i) {
                    transformed_data[4 * i + 2] = _data[4 * i + 0];
                    transformed_data[4 * i + 1] = _data[4 * i + 1];
                    transformed_data[4 * i + 0] = _data[4 * i + 2];
                    transformed_data[4 * i + 3] = _data[4 * i + 3];
                }
            } else {
                throw std::runtime_error { "unknown _header._bits=" + std::to_string(_header._bits) };
            }

        } break;
        case 3: {
            // uncompressed monochrome
            throw std::runtime_error { "not implemented" };
        } break;
        case 9: {
            // compressed paletted
            throw std::runtime_error { "not implemented" };
        } break;
        case 10: {
            // compressed truecolor
            throw std::runtime_error { "not implemented" };
        } break;
        case 11: {
            // compressed monochrome
            throw std::runtime_error { "not implemented" };
        } break;
        default: {
            throw std::runtime_error { "unknown _header._image_type=" + std::to_string(_header._image_type) };
        }
        }

        binary.insert(binary.end(), transformed_data.begin(), transformed_data.end());

        return binary;
    }
};

struct ref_RGB {
    uint8_t& r;
    uint8_t& g;
    uint8_t& b;
};

inline auto operator<<(std::ostream& o, const ref_RGB& rgb) -> std::ostream&
{
    // clang-format off
    o << "{" <<
        "r=" << std::to_string(rgb.r) << ","
        "g=" << std::to_string(rgb.g) << ","
        "b=" << std::to_string(rgb.b) << "}";
    // clang-format on

    return o;
}

struct accessor_RGB {
    std::vector<uint8_t>& _image;
    uint32_t _width;
    uint32_t _height;

    accessor_RGB(std::vector<uint8_t>& image, uint32_t width, uint32_t height)
        : _image { image }
        , _width { width }
        , _height { height }
    {
    }

    auto check_range(uint32_t x, uint32_t y) const -> bool
    {
        return x < _width && y < _height;
    }

    auto nth(uint32_t x, uint32_t y) const -> size_t
    {
        return x + y * _width;
    }

    auto operator[](size_t nth) const -> ref_RGB
    {
        return ref_RGB {
            .r = _image[3 * nth + 0],
            .g = _image[3 * nth + 1],
            .b = _image[3 * nth + 2]
        };
    }

    auto get(uint32_t x, uint32_t y) -> ref_RGB
    {
        if (check_range(x, y)) {
            size_t nth = this->nth(x, y);

            return ref_RGB {
                .r = _image[3 * nth + 0],
                .g = _image[3 * nth + 1],
                .b = _image[3 * nth + 2]
            };
        }

        throw std::range_error { "(" + std::to_string(x) + "," + std::to_string(y) + ") -> ("
            + std::to_string(_width) + "," + std::to_string(_height) + ")" };
    }

    size_t size() const
    {
        return _image.size() / 3;
    }
};

struct accessor_MONO {
    std::vector<uint8_t>& _image;
    uint32_t _width;
    uint32_t _height;

    accessor_MONO(std::vector<uint8_t>& image, uint32_t width, uint32_t height)
        : _image { image }
        , _width { width }
        , _height { height }
    {
    }

    auto check_range(uint32_t x, uint32_t y) const -> bool
    {
        bool chk = x < _width && y < _height;
        return chk;
    }

    auto nth(uint32_t x, uint32_t y) const -> size_t
    {
        size_t nth = x + y * _width;
        return nth;
    }

    auto operator[](size_t nth) const -> uint8_t&
    {
        return _image[nth];
    }

    auto get(uint32_t x, uint32_t y) const -> uint8_t&
    {
        if (check_range(x, y)) {
            return _image[nth(x, y)];
        }

        throw std::range_error { "(" + std::to_string(x) + "," + std::to_string(y) + ") -> ("
            + std::to_string(_width) + "," + std::to_string(_height) + ")" };
    }

    size_t size() const
    {
        return _image.size();
    }
};

inline auto split_channels(std::vector<uint8_t> const& image) -> std::tuple<std::vector<uint8_t>, std::vector<uint8_t>, std::vector<uint8_t>>
{
    std::vector<uint8_t> red {}, green {}, blue {};

    size_t im_size = image.size() / 3;
    red.resize(im_size);
    green.resize(im_size);
    blue.resize(im_size);

    for (size_t i {}; i < im_size; ++i) {
        red[i] = image[3 * i + 0];
        green[i] = image[3 * i + 1];
        blue[i] = image[3 * i + 2];
    }

    return { red, green, blue };
}

inline auto join_channels(
    std::vector<uint8_t> const& r,
    std::vector<uint8_t> const& g,
    std::vector<uint8_t> const& b) -> std::vector<uint8_t>
{
    std::vector<uint8_t> data {};

    auto size = r.size();
    data.resize(3 * size);
    for (size_t i {}; i < size; ++i) {
        data[3 * i + 0] = r[i];
        data[3 * i + 1] = g[i];
        data[3 * i + 2] = b[i];
    }

    return data;
}

}
