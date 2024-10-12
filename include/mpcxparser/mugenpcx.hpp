/**
 * @file mugenpcx.hpp
 * @author Halkaze
 * @date 2024-10-13
 *
 * @copyright Copyright (c) 2024
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MPCXPARSER_MUGENPCX_HPP__
#define MPCXPARSER_MUGENPCX_HPP__

#include "mpcxparser/mpcxparser.h"

#include <algorithm>
#include <array>
#include <optional>

namespace mugen {
namespace pcx {

class Pcx {
 public:
  struct Pixel {
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
    std::uint8_t alpha;

    inline Pixel() noexcept : red{0}, green{0}, blue{0}, alpha{255} {}
  };

 private:
  const std::size_t width_;
  const std::size_t height_;

  const std::size_t bytesPerLine_;

  const std::optional<std::array<Pixel, 256>> pallete_;
  const std::optional<std::vector<std::uint8_t>> indexes_;

  const std::vector<Pixel> data_;

 public:
  inline explicit Pcx(std::size_t width, std::size_t height, std::size_t bytesPerLine, std::vector<Pixel>&& data) noexcept
      : width_{width}, height_{height}, bytesPerLine_{bytesPerLine}, pallete_{}, indexes_{}, data_{data} {}

  inline explicit Pcx(std::size_t width,
                      std::size_t height,
                      std::size_t bytesPerLine,
                      std::array<Pixel, 256>&& pallete,
                      std::vector<std::uint8_t>&& indexes) noexcept
      : width_{width}, height_{height}, bytesPerLine_{bytesPerLine}, pallete_{pallete}, indexes_{indexes}, data_{[this]() {
          std::vector<Pixel> data(indexes_->size());
          std::transform(indexes_->cbegin(), indexes_->cend(), data.begin(), [this](std::uint8_t index) { return (*pallete_)[index]; });
          return data;
        }()} {}

  inline std::size_t width() const noexcept { return width_; }
  inline std::size_t height() const noexcept { return height_; }

  inline std::size_t bytes_per_line() const noexcept { return bytesPerLine_; }

  inline const std::optional<std::array<Pixel, 256>>& pallete() const noexcept { return pallete_; }
  inline const std::optional<std::vector<std::uint8_t>>& indexes() const noexcept { return indexes_; }

  inline const std::vector<Pixel>& data() const noexcept { return data_; }

  // pcx形式として出力する
  void write_as_pcx(const std::filesystem::path& path) const;

  // 透明度付きico形式として出力する
  void write_as_ico(const std::filesystem::path& path) const;

  // bmp形式（Windows3.0形式 = BITMAPFILE）として出力する
  void write_as_bmp(const std::filesystem::path& path) const;

  // 透明度付きbmp形式（Windows95形式 = BITMAPV4）として出力する
  void write_as_abmp(const std::filesystem::path& path) const;
};

namespace internal {

MPCXPARSER_PACK(struct PcxHeader {
  std::uint8_t signature;     // MUGEN is always ignored
  std::uint8_t version;       // MUGEN is always ignored
  std::uint8_t encoding;      // MUGEN is always ignored
  std::uint8_t bitsPerPixel;  // MUGEN is required it is 8
  std::uint16_t startX;
  std::uint16_t startY;
  std::uint16_t endX;
  std::uint16_t endY;
  std::uint16_t hRes;  // MUGEN is always ignored
  std::uint16_t vRes;  // MUGEN is always ignored
  std::uint8_t pallete[16][3];
  std::uint8_t reserved;
  std::uint8_t colorPlanes;  // MUGEN is required it is 1 or 3
  std::uint16_t bytesPerLine;
  std::uint16_t palleteMode;   // MUGEN is always ignored
  std::uint16_t hScreenSize;   // MUGEN is always ignored
  std::uint16_t vScreenSize;   // MUGEN is always ignored
  std::uint8_t reserved2[54];  // MUGEN is always ignored
});

MPCXPARSER_PACK(struct PcxHeaderMinimum {
  std::uint8_t signature;     // MUGEN is always ignored
  std::uint8_t version;       // MUGEN is always ignored
  std::uint8_t encoding;      // MUGEN is always ignored
  std::uint8_t bitsPerPixel;  // MUGEN is required it is 8
  std::uint16_t startX;
  std::uint16_t startY;
  std::uint16_t endX;
  std::uint16_t endY;
  std::uint16_t hRes;  // MUGEN is always ignored
  std::uint16_t vRes;  // MUGEN is always ignored
  std::uint8_t pallete[16][3];
  std::uint8_t reserved;
  std::uint8_t colorPlanes;  // MUGEN is required it is 1 or 3
  std::uint16_t bytesPerLine;
});

};  // namespace internal

};  // namespace pcx
};  // namespace mugen

#endif  // MPCXPARSER_MUGENPCX_HPP__
