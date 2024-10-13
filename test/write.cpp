/**
 * @file write.cpp
 * @author Halkaze
 * @date 2024-10-14
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

#include <gtest/gtest.h>

#include <mpcxparser/mpcxparser.h>

#include <ios>
#include <sstream>
#include <string_view>
#include <utility>

using namespace std::string_view_literals;

TEST(test_write, write_kfm_win) {
  static constexpr std::string_view kfmpcx = "assets/good/kfm.pcx"sv;
  static constexpr std::string_view pathpcx = "assets/kfm.pcx"sv;
  static constexpr std::string_view pathico = "assets/kfm.ico"sv;
  static constexpr std::string_view pathbmp = "assets/kfm.bmp"sv;
  static constexpr std::string_view pathabmp = "assets/kfm_a.bmp"sv;

  auto parser = mugen::pcx::PcxParserWin{};
  ASSERT_NO_THROW(parser.parse(kfmpcx));

  auto pcx = parser.parse(kfmpcx);
  EXPECT_NO_THROW(pcx.write_as_pcx(pathpcx));
  EXPECT_NO_THROW(pcx.write_as_ico(pathico));
  EXPECT_NO_THROW(pcx.write_as_bmp(pathbmp));
  EXPECT_NO_THROW(pcx.write_as_abmp(pathabmp));

  auto saved = parser.parse(kfmpcx);

  EXPECT_EQ(saved.width(), pcx.width());
  EXPECT_EQ(saved.height(), pcx.height());
  EXPECT_EQ(saved.bytes_per_line(), pcx.bytes_per_line());

  EXPECT_TRUE(pcx.pallete());
  EXPECT_TRUE(saved.pallete());
  if (saved.pallete() && pcx.pallete()) {
    auto&& src = *(pcx.pallete());
    auto&& dest = *(saved.pallete());
    for (std::size_t i = 0; i < dest.size(); ++i) {
      EXPECT_EQ(dest[i].red, src[i].red);
      EXPECT_EQ(dest[i].green, src[i].green);
      EXPECT_EQ(dest[i].blue, src[i].blue);
      EXPECT_EQ(dest[i].alpha, src[i].alpha);
    }
  }

  EXPECT_TRUE(pcx.indexes());
  EXPECT_TRUE(saved.indexes());
  if (saved.indexes() && pcx.indexes()) {
    auto&& src = *(pcx.indexes());
    auto&& dest = *(saved.indexes());
    EXPECT_EQ(dest.size(), src.size());
    EXPECT_EQ(dest, src);
  }

  EXPECT_EQ(pcx.data().size(), saved.data().size());
  for (std::size_t i = 0; i < pcx.data().size(); ++i) {
    auto&& src = pcx.data()[i];
    auto&& dest = saved.data()[i];
    EXPECT_EQ(dest.red, src.red);
    EXPECT_EQ(dest.green, src.green);
    EXPECT_EQ(dest.blue, src.blue);
    EXPECT_EQ(dest.alpha, src.alpha);
  }
}

TEST(test_write, write_as_pcx256) {
  static constexpr std::size_t width = 2;
  static constexpr std::size_t height = 2;

  static constexpr std::string_view path = "assets/pcx256.pcx"sv;

  std::array<mugen::pcx::Pcx::Pixel, 256> pallete{};
  std::vector<std::uint8_t> indexes(width * height);

  for (std::size_t i = 0; i < pallete.size(); ++i) {
    pallete[i].red = static_cast<std::uint8_t>(i);
    pallete[i].green = static_cast<std::uint8_t>(i);
    pallete[i].blue = static_cast<std::uint8_t>(i);
    pallete[i].alpha = static_cast<std::uint8_t>(i);
  }

  for (std::size_t y = 0; y < height; ++y) {
    for (std::size_t x = 0; x < width; ++x) {
      indexes[y * width + x] = static_cast<std::uint8_t>(y * 0x10 + x);
    }
  }

  mugen::pcx::Pcx pcx{width, height, width, std::move(pallete), std::move(indexes)};

  EXPECT_NO_THROW(pcx.write_as_pcx(path));

  auto parser = mugen::pcx::PcxParserWin{};
  ASSERT_NO_THROW(parser.parse(path));

  auto saved = parser.parse(path);

  EXPECT_EQ(saved.width(), width);
  EXPECT_EQ(saved.height(), height);
  EXPECT_EQ(saved.bytes_per_line(), width);

  EXPECT_TRUE(saved.pallete());
  if (pcx.pallete() && saved.pallete()) {
    auto srcPallete = *(pcx.pallete());
    auto destPallete = *(saved.pallete());

    for (std::size_t i = 0; i < srcPallete.size(); ++i) {
      EXPECT_EQ(destPallete[i].red, srcPallete[i].red);
      EXPECT_EQ(destPallete[i].green, srcPallete[i].green);
      EXPECT_EQ(destPallete[i].blue, srcPallete[i].blue);
      if (i == 0) {
        EXPECT_EQ(destPallete[i].alpha, 0);
      } else {
        EXPECT_EQ(destPallete[i].alpha, 255);
      }
    }
  }

  EXPECT_TRUE(saved.indexes());
  if (pcx.indexes() && saved.indexes()) {
    EXPECT_EQ(*(pcx.indexes()), *(saved.indexes()));
  }

  EXPECT_EQ(saved.data().size(), saved.width() * saved.height());
  for (std::size_t y = 0; y < saved.height(); ++y) {
    for (std::size_t x = 0; x < saved.width(); ++x) {
      auto&& pixel = saved.data()[y * saved.width() + x];
      EXPECT_EQ(pixel.red, static_cast<std::uint8_t>(y * 0x10 + x));
      EXPECT_EQ(pixel.green, static_cast<std::uint8_t>(y * 0x10 + x));
      EXPECT_EQ(pixel.blue, static_cast<std::uint8_t>(y * 0x10 + x));
    }
  }
}

TEST(test_write, write_as_pcx24bits) {
  static constexpr std::size_t width = 2;
  static constexpr std::size_t height = 2;

  static constexpr std::string_view path = "assets/pcx24bits.pcx"sv;

  std::vector<mugen::pcx::Pcx::Pixel> data(width * height);

  for (std::size_t i = 0; i < data.size(); ++i) {
    data[i].red = static_cast<std::uint8_t>(i + 0x00);
    data[i].green = static_cast<std::uint8_t>(i + 0x10);
    data[i].blue = static_cast<std::uint8_t>(i + 0x20);
    data[i].alpha = static_cast<std::uint8_t>(i + 0x30);
  }

  mugen::pcx::Pcx pcx{width, height, width, std::move(data)};

  EXPECT_NO_THROW(pcx.write_as_pcx(path));

  auto parser = mugen::pcx::PcxParserWin{};
  ASSERT_NO_THROW(parser.parse(path));

  auto saved = parser.parse(path);

  EXPECT_EQ(saved.width(), width);
  EXPECT_EQ(saved.height(), height);
  EXPECT_EQ(saved.bytes_per_line(), width);

  EXPECT_FALSE(saved.pallete());
  EXPECT_FALSE(saved.indexes());

  EXPECT_EQ(saved.data().size(), saved.width() * saved.height());
  for (std::size_t y = 0; y < saved.height(); ++y) {
    for (std::size_t x = 0; x < saved.width(); ++x) {
      std::size_t index = y * saved.width() + x;
      auto&& pixel = saved.data()[index];
      EXPECT_EQ(pixel.red, static_cast<std::uint8_t>(index + 0x00));
      EXPECT_EQ(pixel.green, static_cast<std::uint8_t>(index + 0x10));
      EXPECT_EQ(pixel.blue, static_cast<std::uint8_t>(index + 0x20));
      EXPECT_EQ(pixel.alpha, 255);
    }
  }
}

TEST(test_write, write_as_ico_small) {
  static constexpr std::size_t width = 4;
  static constexpr std::size_t height = 10;

  static constexpr std::string_view path = "assets/small.ico"sv;

  std::array<mugen::pcx::Pcx::Pixel, 256> pallete{};
  std::vector<std::uint8_t> indexes(width * height);

  for (std::size_t i = 0; i < pallete.size(); ++i) {
    pallete[i].red = 255;
    pallete[i].green = 0;
    pallete[i].blue = 0;
    pallete[i].alpha = static_cast<std::uint8_t>(i);
  }

  for (std::size_t y = 0; y < height; ++y) {
    for (std::size_t x = 0; x < width; ++x) {
      indexes[y * width + x] = static_cast<std::uint8_t>(((x + y) % 2 == 0) * 255);
    }
  }

  mugen::pcx::Pcx pcx{width, height, width, std::move(pallete), std::move(indexes)};

  EXPECT_NO_THROW(pcx.write_as_ico(path));
}

TEST(test_write, write_as_ico_medium) {
  static constexpr std::size_t width = 256;
  static constexpr std::size_t height = 10;

  static constexpr std::string_view path = "assets/medium.ico"sv;

  std::array<mugen::pcx::Pcx::Pixel, 256> pallete{};
  std::vector<std::uint8_t> indexes(width * height);

  for (std::size_t i = 0; i < pallete.size(); ++i) {
    pallete[i].red = 255;
    pallete[i].green = 0;
    pallete[i].blue = 0;
    pallete[i].alpha = static_cast<std::uint8_t>(i);
  }

  for (std::size_t y = 0; y < height; ++y) {
    for (std::size_t x = 0; x < width; ++x) {
      indexes[y * width + x] = static_cast<std::uint8_t>(((x + y) % 2 == 0) * 255);
    }
  }

  mugen::pcx::Pcx pcx{width, height, width, std::move(pallete), std::move(indexes)};

  EXPECT_NO_THROW(pcx.write_as_ico(path));
}

TEST(test_write, write_as_ico_large) {
  static constexpr std::size_t width = 257;
  static constexpr std::size_t height = 10;

  static constexpr std::string_view path = "assets/large.ico"sv;

  std::array<mugen::pcx::Pcx::Pixel, 256> pallete{};
  std::vector<std::uint8_t> indexes(width * height);

  for (std::size_t i = 0; i < pallete.size(); ++i) {
    pallete[i].red = 255;
    pallete[i].green = 0;
    pallete[i].blue = 0;
    pallete[i].alpha = static_cast<std::uint8_t>(i);
  }

  for (std::size_t y = 0; y < height; ++y) {
    for (std::size_t x = 0; x < width; ++x) {
      indexes[y * width + x] = static_cast<std::uint8_t>(((x + y) % 2 == 0) * 255);
    }
  }

  mugen::pcx::Pcx pcx{width, height, width, std::move(pallete), std::move(indexes)};

  EXPECT_THROW(pcx.write_as_ico(path), mugen::pcx::IllegalFormatError);
}

TEST(test_write, write_as_bmp) {
  static constexpr std::size_t width = 4;
  static constexpr std::size_t height = 4;

  static constexpr std::string_view path = "assets/BITMAPFILE.bmp"sv;

  std::array<mugen::pcx::Pcx::Pixel, 256> pallete{};
  std::vector<std::uint8_t> indexes(width * height);

  for (std::size_t i = 0; i < pallete.size(); ++i) {
    pallete[i].red = 255;
    pallete[i].green = 0;
    pallete[i].blue = 0;
    pallete[i].alpha = static_cast<std::uint8_t>(i);
  }

  for (std::size_t y = 0; y < height; ++y) {
    for (std::size_t x = 0; x < width; ++x) {
      indexes[y * width + x] = static_cast<std::uint8_t>(((x + y) % 2 == 0) * 255);
    }
  }

  mugen::pcx::Pcx pcx{width, height, width, std::move(pallete), std::move(indexes)};

  EXPECT_NO_THROW(pcx.write_as_bmp(path));
}

TEST(test_write, write_as_abmp) {
  static constexpr std::size_t width = 4;
  static constexpr std::size_t height = 4;

  static constexpr std::string_view path = "assets/BITMAPV4.bmp"sv;

  std::array<mugen::pcx::Pcx::Pixel, 256> pallete{};
  std::vector<std::uint8_t> indexes(width * height);

  for (std::size_t i = 0; i < pallete.size(); ++i) {
    pallete[i].red = 255;
    pallete[i].green = 0;
    pallete[i].blue = 0;
    pallete[i].alpha = static_cast<std::uint8_t>(i);
  }

  for (std::size_t y = 0; y < height; ++y) {
    for (std::size_t x = 0; x < width; ++x) {
      indexes[y * width + x] = static_cast<std::uint8_t>(((x + y) % 2 == 0) * 255);
    }
  }

  mugen::pcx::Pcx pcx{width, height, width, std::move(pallete), std::move(indexes)};

  EXPECT_NO_THROW(pcx.write_as_abmp(path));
}

TEST(test_write, write_to_stream_win) {
  static constexpr std::size_t width = 2;
  static constexpr std::size_t height = 2;

  std::vector<mugen::pcx::Pcx::Pixel> data(width * height);

  for (std::size_t i = 0; i < data.size(); ++i) {
    data[i].red = static_cast<std::uint8_t>(i + 0x00);
    data[i].green = static_cast<std::uint8_t>(i + 0x10);
    data[i].blue = static_cast<std::uint8_t>(i + 0x20);
    data[i].alpha = static_cast<std::uint8_t>(i + 0x30);
  }

  mugen::pcx::Pcx pcx{width, height, width, std::move(data)};

  std::stringstream ss{};

  EXPECT_NO_THROW(pcx.write_as_pcx(ss));

  ss.seekg(0, std::ios_base::beg);

  auto parser = mugen::pcx::PcxParserWin{};
  ASSERT_NO_THROW(parser.parse(ss));

  ss.seekg(0, std::ios_base::beg);

  auto saved = parser.parse(ss);

  EXPECT_EQ(saved.width(), width);
  EXPECT_EQ(saved.height(), height);
  EXPECT_EQ(saved.bytes_per_line(), width);

  EXPECT_FALSE(saved.pallete());
  EXPECT_FALSE(saved.indexes());

  EXPECT_EQ(saved.data().size(), saved.width() * saved.height());
  for (std::size_t y = 0; y < saved.height(); ++y) {
    for (std::size_t x = 0; x < saved.width(); ++x) {
      std::size_t index = y * saved.width() + x;
      auto&& pixel = saved.data()[index];
      EXPECT_EQ(pixel.red, static_cast<std::uint8_t>(index + 0x00));
      EXPECT_EQ(pixel.green, static_cast<std::uint8_t>(index + 0x10));
      EXPECT_EQ(pixel.blue, static_cast<std::uint8_t>(index + 0x20));
      EXPECT_EQ(pixel.alpha, 255);
    }
  }
}
