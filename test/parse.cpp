/**
 * @file write.cpp
 * @author Halkaze
 * @date 2024-10-12
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

#include <bit>
#include <fstream>
#include <ios>
#include <string_view>

using namespace std::string_view_literals;

static constexpr std::string_view NOT_EXISTING_FILE = "assets/not-existing-file.def"sv;

TEST(test_parse, common_parse_error) {
  auto parser = mugen::pcx::PcxParserWin{};
  EXPECT_ANY_THROW(parser.parse(NOT_EXISTING_FILE));
  EXPECT_THROW(parser.parse(NOT_EXISTING_FILE), mugen::pcx::FileIOError);
}

TEST(test_parse, parse_win_kfm) {
  static constexpr std::string_view kfmpcx = "assets/good/kfm.pcx"sv;

  auto parser = mugen::pcx::PcxParserWin{};
  ASSERT_NO_THROW(parser.parse(kfmpcx));
  try {
    auto pcx = parser.parse(kfmpcx);

    EXPECT_EQ(pcx.width(), 25);

    EXPECT_EQ(pcx.height(), 25);

    EXPECT_EQ(pcx.bytes_per_line(), 25);

    EXPECT_TRUE(pcx.pallete());
    if (pcx.pallete()) {
      auto pallete = *(pcx.pallete());
      EXPECT_EQ(pallete[0].alpha, 0);
    }

    EXPECT_TRUE(pcx.indexes());
    if (pcx.indexes()) {
      EXPECT_EQ((pcx.indexes())->size(), pcx.data().size());
    }

    EXPECT_EQ(pcx.data().size(), pcx.width() * pcx.height());

    if (pcx.pallete() && pcx.indexes()) {
      for (std::size_t y = 0; y < pcx.height(); ++y) {
        for (std::size_t x = 0; x < pcx.width(); ++x) {
          std::size_t index = y * pcx.width() + x;
          auto&& pixel = pcx.data()[index];

          std::size_t palIndex = (*(pcx.indexes()))[index];
          auto&& pal = (*(pcx.pallete()))[palIndex];

          EXPECT_EQ(pixel.red, pal.red);
          EXPECT_EQ(pixel.green, pal.green);
          EXPECT_EQ(pixel.blue, pal.blue);
          if (palIndex == 0) {
            EXPECT_EQ(pixel.alpha, 0);
          } else {
            EXPECT_EQ(pixel.alpha, 255);
          }
        }
      }
    }
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

TEST(test_parse, parse_win_testEGA16) {
  static constexpr std::string_view testpcx = "assets/good/testEGA16.pcx"sv;

  auto parser = mugen::pcx::PcxParserWin{};
  ASSERT_NO_THROW(parser.parse(testpcx));
  try {
    auto pcx = parser.parse(testpcx);

    EXPECT_EQ(pcx.width(), 2);

    EXPECT_EQ(pcx.height(), 2);

    EXPECT_EQ(pcx.bytes_per_line(), 3);

    EXPECT_TRUE(pcx.pallete());
    if (pcx.pallete()) {
      auto pallete = *(pcx.pallete());
      EXPECT_EQ(pallete[0].alpha, 0);
      for (std::size_t i = 0; i < 16; ++i) {
        EXPECT_EQ(pallete[i].red, i * 0x10 + i);
        EXPECT_EQ(pallete[i].green, i * 0x10 + i);
        EXPECT_EQ(pallete[i].blue, i * 0x10 + i);
      }
      for (std::size_t i = 16; i < pallete.size(); ++i) {
        EXPECT_EQ(pallete[i].red, 0);
        EXPECT_EQ(pallete[i].green, 0);
        EXPECT_EQ(pallete[i].blue, 0);
        EXPECT_EQ(pallete[i].alpha, 255);
      }
    }

    EXPECT_TRUE(pcx.indexes());
    if (pcx.indexes()) {
      auto indexes = *(pcx.indexes());
      EXPECT_EQ(indexes.size(), 4);
      EXPECT_EQ(indexes.size(), pcx.data().size());
      EXPECT_EQ(indexes, std::vector<std::uint8_t>({0x01, 0x02, 0x04, 0x05}));
    }

    std::vector<mugen::pcx::Pcx::Pixel> data(pcx.data().size());
    data[0].red = 0x11;
    data[0].green = 0x11;
    data[0].blue = 0x11;
    data[0].alpha = 255;
    data[1].red = 0x22;
    data[1].green = 0x22;
    data[1].blue = 0x22;
    data[1].alpha = 255;
    data[2].red = 0x44;
    data[2].green = 0x44;
    data[2].blue = 0x44;
    data[2].alpha = 255;
    data[3].red = 0x55;
    data[3].green = 0x55;
    data[3].blue = 0x55;
    data[3].alpha = 255;
    for (std::size_t i = 0; i < data.size(); ++i) {
      EXPECT_EQ(pcx.data()[i].red, data[i].red);
      EXPECT_EQ(pcx.data()[i].green, data[i].green);
      EXPECT_EQ(pcx.data()[i].blue, data[i].blue);
      EXPECT_EQ(pcx.data()[i].alpha, data[i].alpha);
    }
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

TEST(test_parse, parse_win_test256) {
  static constexpr std::string_view testpcx = "assets/good/test256.pcx"sv;

  auto parser = mugen::pcx::PcxParserWin{};
  ASSERT_NO_THROW(parser.parse(testpcx));
  try {
    auto pcx = parser.parse(testpcx);

    EXPECT_EQ(pcx.width(), 2);

    EXPECT_EQ(pcx.height(), 2);

    EXPECT_EQ(pcx.bytes_per_line(), 1);

    EXPECT_TRUE(pcx.pallete());
    if (pcx.pallete()) {
      auto pallete = *(pcx.pallete());
      EXPECT_EQ(pallete[0].alpha, 0);
      for (std::size_t i = 0; i < pallete.size(); ++i) {
        EXPECT_EQ(pallete[i].red, i);
        EXPECT_EQ(pallete[i].green, i);
        EXPECT_EQ(pallete[i].blue, i);
      }
    }

    EXPECT_TRUE(pcx.indexes());
    if (pcx.indexes()) {
      auto indexes = *(pcx.indexes());
      EXPECT_EQ(indexes.size(), 4);
      EXPECT_EQ(indexes.size(), pcx.data().size());
      EXPECT_EQ(indexes, std::vector<std::uint8_t>({0x01, 0xFF, 0x02, 0xFF}));
    }

    std::vector<mugen::pcx::Pcx::Pixel> data(pcx.data().size());
    data[0].red = 1;
    data[0].green = 1;
    data[0].blue = 1;
    data[0].alpha = 255;
    data[1].red = 255;
    data[1].green = 255;
    data[1].blue = 255;
    data[1].alpha = 255;
    data[2].red = 2;
    data[2].green = 2;
    data[2].blue = 2;
    data[2].alpha = 255;
    data[3].red = 255;
    data[3].green = 255;
    data[3].blue = 255;
    data[3].alpha = 255;
    for (std::size_t i = 0; i < data.size(); ++i) {
      EXPECT_EQ(pcx.data()[i].red, data[i].red);
      EXPECT_EQ(pcx.data()[i].green, data[i].green);
      EXPECT_EQ(pcx.data()[i].blue, data[i].blue);
      EXPECT_EQ(pcx.data()[i].alpha, data[i].alpha);
    }
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

TEST(test_parse, parse_win_test24bits) {
  static constexpr std::string_view testpcx = "assets/good/test24bits.pcx"sv;

  auto parser = mugen::pcx::PcxParserWin{};
  ASSERT_NO_THROW(parser.parse(testpcx));
  try {
    auto pcx = parser.parse(testpcx);

    EXPECT_EQ(pcx.width(), 2);

    EXPECT_EQ(pcx.height(), 2);

    EXPECT_EQ(pcx.bytes_per_line(), 2);

    EXPECT_FALSE(pcx.pallete());

    EXPECT_FALSE(pcx.indexes());

    std::vector<mugen::pcx::Pcx::Pixel> data(pcx.data().size());
    data[0].red = 0x01;
    data[0].green = 0x03;
    data[0].blue = 0x05;
    data[0].alpha = 255;
    data[1].red = 0x02;
    data[1].green = 0x04;
    data[1].blue = 0x06;
    data[1].alpha = 255;
    data[2].red = 0x11;
    data[2].green = 0x13;
    data[2].blue = 0x15;
    data[2].alpha = 255;
    data[3].red = 0x12;
    data[3].green = 0x14;
    data[3].blue = 0x16;
    data[3].alpha = 255;
    for (std::size_t i = 0; i < data.size(); ++i) {
      EXPECT_EQ(pcx.data()[i].red, data[i].red);
      EXPECT_EQ(pcx.data()[i].green, data[i].green);
      EXPECT_EQ(pcx.data()[i].blue, data[i].blue);
      EXPECT_EQ(pcx.data()[i].alpha, data[i].alpha);
    }
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

TEST(test_parse, insufficient_win) {
  static constexpr std::string_view missingAfterPallete = "assets/bad/missing_pallete.pcx"sv;
  static constexpr std::string_view missingAfterData = "assets/bad/missing_data.pcx"sv;
  static constexpr std::string_view missingAfterScreenSize = "assets/bad/missing_screensize.pcx"sv;
  static constexpr std::string_view missingAfterPalleteMode = "assets/bad/missing_palletemode.pcx"sv;
  static constexpr std::string_view missingAfterBytesPerLine = "assets/bad/missing_bytesperline.pcx"sv;

  auto parser = mugen::pcx::PcxParserWin{};

  EXPECT_NO_THROW(parser.parse(missingAfterPallete));
  try {
    auto pcx = parser.parse(missingAfterPallete);
    EXPECT_TRUE(pcx.pallete());
    if (pcx.pallete()) {
      auto pallete = *(pcx.pallete());
      for (std::size_t i = 0; i < pallete.size(); ++i) {
        EXPECT_EQ(pallete[i].red, 0xFF);
        EXPECT_EQ(pallete[i].green, 0xFF);
        EXPECT_EQ(pallete[i].blue, 0xFF);
        if (i == 0) {
          EXPECT_EQ(pallete[i].alpha, 0x00);
        } else {
          EXPECT_EQ(pallete[i].alpha, 0xFF);
        }
      }
    }
  } catch (...) {
    EXPECT_TRUE(false);
  }

  EXPECT_NO_THROW(parser.parse(missingAfterData));
  try {
    auto pcx = parser.parse(missingAfterData);
    EXPECT_TRUE(pcx.indexes());
    EXPECT_EQ(pcx.width(), 25);
    EXPECT_EQ(pcx.height(), 25);
    EXPECT_EQ(pcx.data().size(), pcx.width() * pcx.height());
    if (pcx.indexes()) {
      auto indexes = *(pcx.indexes());
      for (std::size_t y = 0; y < pcx.height(); ++y) {
        for (std::size_t x = 0; x < pcx.width(); ++x) {
          EXPECT_EQ(indexes[y * pcx.width() + x], 0xFF);
        }
      }
    }
  } catch (...) {
    EXPECT_TRUE(false);
  }

  EXPECT_NO_THROW(parser.parse(missingAfterScreenSize));
  EXPECT_NO_THROW(parser.parse(missingAfterPalleteMode));
  EXPECT_THROW(parser.parse(missingAfterBytesPerLine), mugen::pcx::IllegalFormatError);
}

TEST(test_parse, incompatible_format_win) {
  static constexpr std::string_view colorDepthIsNot8 = "assets/bad/kfm16.pcx"sv;
  static constexpr std::string_view colorPlanesIs4 = "assets/bad/test32bits.pcx"sv;
  static constexpr std::string_view sizeIs0 = "assets/bad/zero.pcx"sv;

  auto parser = mugen::pcx::PcxParserWin{};

  // BitsPerPixel != 8
  EXPECT_THROW(parser.parse(colorDepthIsNot8), mugen::pcx::IncompatibleFormatError);

  // ColorPlanes != 1 && ColorPlanes != 3
  EXPECT_THROW(parser.parse(colorPlanesIs4), mugen::pcx::IncompatibleFormatError);

  // width * height == 0
  EXPECT_THROW(parser.parse(sizeIs0), mugen::pcx::IncompatibleFormatError);
}

TEST(test_parse, parse_from_stream_win) {
  static constexpr std::string_view testpcx = "assets/good/testEGA16.pcx"sv;

  std::ifstream ifs{testpcx.data(), std::ios_base::binary};

  auto parser = mugen::pcx::PcxParserWin{};
  ASSERT_NO_THROW(parser.parse(ifs));

  ifs.clear();
  ifs.seekg(0, std::ios_base::beg);
  try {
    auto pcx = parser.parse(ifs);
    EXPECT_EQ(pcx.width(), 2);
    EXPECT_EQ(pcx.height(), 2);
    EXPECT_EQ(pcx.bytes_per_line(), 3);
    EXPECT_TRUE(pcx.pallete());
    if (pcx.pallete()) {
      auto pallete = *(pcx.pallete());
      EXPECT_EQ(pallete[0].alpha, 0);
      for (std::size_t i = 0; i < 16; ++i) {
        EXPECT_EQ(pallete[i].red, i * 0x10 + i);
        EXPECT_EQ(pallete[i].green, i * 0x10 + i);
        EXPECT_EQ(pallete[i].blue, i * 0x10 + i);
      }
      for (std::size_t i = 16; i < pallete.size(); ++i) {
        EXPECT_EQ(pallete[i].red, 0);
        EXPECT_EQ(pallete[i].green, 0);
        EXPECT_EQ(pallete[i].blue, 0);
        EXPECT_EQ(pallete[i].alpha, 255);
      }
    }
    EXPECT_TRUE(pcx.indexes());
    if (pcx.indexes()) {
      auto indexes = *(pcx.indexes());
      EXPECT_EQ(indexes.size(), 4);
      EXPECT_EQ(indexes.size(), pcx.data().size());
      EXPECT_EQ(indexes, std::vector<std::uint8_t>({0x01, 0x02, 0x04, 0x05}));
    }
    std::vector<mugen::pcx::Pcx::Pixel> data(pcx.data().size());
    data[0].red = 0x11;
    data[0].green = 0x11;
    data[0].blue = 0x11;
    data[0].alpha = 255;
    data[1].red = 0x22;
    data[1].green = 0x22;
    data[1].blue = 0x22;
    data[1].alpha = 255;
    data[2].red = 0x44;
    data[2].green = 0x44;
    data[2].blue = 0x44;
    data[2].alpha = 255;
    data[3].red = 0x55;
    data[3].green = 0x55;
    data[3].blue = 0x55;
    data[3].alpha = 255;
    for (std::size_t i = 0; i < data.size(); ++i) {
      EXPECT_EQ(pcx.data()[i].red, data[i].red);
      EXPECT_EQ(pcx.data()[i].green, data[i].green);
      EXPECT_EQ(pcx.data()[i].blue, data[i].blue);
      EXPECT_EQ(pcx.data()[i].alpha, data[i].alpha);
    }
  } catch (...) {
    ASSERT_TRUE(false);
  }
}

TEST(test_parse, parse_from_mem_win) {
  static constexpr std::uint8_t buf[] = {
      0x00, 0x00, 0x00, 0x08, 0x05, 0x00, 0x05, 0x00, 0x06, 0x00, 0x06, 0x00, 0xFF, 0x7F, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x22,
      0x22, 0x22, 0x33, 0x33, 0x33, 0x44, 0x44, 0x44, 0x55, 0x55, 0x55, 0x66, 0x66, 0x66, 0x77, 0x77, 0x77, 0x88, 0x88, 0x88, 0x99, 0x99, 0x99,
      0xAA, 0xAA, 0xAA, 0xBB, 0xBB, 0xBB, 0xCC, 0xCC, 0xCC, 0xDD, 0xDD, 0xDD, 0xEE, 0xEE, 0xEE, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x03, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
      0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
      0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

  auto parser = mugen::pcx::PcxParserWin{};
  ASSERT_NO_THROW(parser.parse(buf, sizeof(buf)));

  try {
    auto pcx = parser.parse(buf, sizeof(buf));
    EXPECT_EQ(pcx.width(), 2);
    EXPECT_EQ(pcx.height(), 2);
    EXPECT_EQ(pcx.bytes_per_line(), 3);
    EXPECT_TRUE(pcx.pallete());
    if (pcx.pallete()) {
      auto pallete = *(pcx.pallete());
      EXPECT_EQ(pallete[0].alpha, 0);
      for (std::size_t i = 0; i < 16; ++i) {
        EXPECT_EQ(pallete[i].red, i * 0x10 + i);
        EXPECT_EQ(pallete[i].green, i * 0x10 + i);
        EXPECT_EQ(pallete[i].blue, i * 0x10 + i);
      }
      for (std::size_t i = 16; i < pallete.size(); ++i) {
        EXPECT_EQ(pallete[i].red, 0);
        EXPECT_EQ(pallete[i].green, 0);
        EXPECT_EQ(pallete[i].blue, 0);
        EXPECT_EQ(pallete[i].alpha, 255);
      }
    }
    EXPECT_TRUE(pcx.indexes());
    if (pcx.indexes()) {
      auto indexes = *(pcx.indexes());
      EXPECT_EQ(indexes.size(), 4);
      EXPECT_EQ(indexes.size(), pcx.data().size());
      EXPECT_EQ(indexes, std::vector<std::uint8_t>({0x01, 0x02, 0x04, 0x05}));
    }
    std::vector<mugen::pcx::Pcx::Pixel> data(pcx.data().size());
    data[0].red = 0x11;
    data[0].green = 0x11;
    data[0].blue = 0x11;
    data[0].alpha = 255;
    data[1].red = 0x22;
    data[1].green = 0x22;
    data[1].blue = 0x22;
    data[1].alpha = 255;
    data[2].red = 0x44;
    data[2].green = 0x44;
    data[2].blue = 0x44;
    data[2].alpha = 255;
    data[3].red = 0x55;
    data[3].green = 0x55;
    data[3].blue = 0x55;
    data[3].alpha = 255;
    for (std::size_t i = 0; i < data.size(); ++i) {
      EXPECT_EQ(pcx.data()[i].red, data[i].red);
      EXPECT_EQ(pcx.data()[i].green, data[i].green);
      EXPECT_EQ(pcx.data()[i].blue, data[i].blue);
      EXPECT_EQ(pcx.data()[i].alpha, data[i].alpha);
    }
  } catch (...) {
    ASSERT_TRUE(false);
  }

  std::span<std::uint8_t> s{std::bit_cast<std::uint8_t*>(&buf), sizeof(buf)};

  ASSERT_NO_THROW(parser.parse(s));

  try {
    auto pcx = parser.parse(s);
    EXPECT_EQ(pcx.width(), 2);
    EXPECT_EQ(pcx.height(), 2);
    EXPECT_EQ(pcx.bytes_per_line(), 3);
    EXPECT_TRUE(pcx.pallete());
    if (pcx.pallete()) {
      auto pallete = *(pcx.pallete());
      EXPECT_EQ(pallete[0].alpha, 0);
      for (std::size_t i = 0; i < 16; ++i) {
        EXPECT_EQ(pallete[i].red, i * 0x10 + i);
        EXPECT_EQ(pallete[i].green, i * 0x10 + i);
        EXPECT_EQ(pallete[i].blue, i * 0x10 + i);
      }
      for (std::size_t i = 16; i < pallete.size(); ++i) {
        EXPECT_EQ(pallete[i].red, 0);
        EXPECT_EQ(pallete[i].green, 0);
        EXPECT_EQ(pallete[i].blue, 0);
        EXPECT_EQ(pallete[i].alpha, 255);
      }
    }
    EXPECT_TRUE(pcx.indexes());
    if (pcx.indexes()) {
      auto indexes = *(pcx.indexes());
      EXPECT_EQ(indexes.size(), 4);
      EXPECT_EQ(indexes.size(), pcx.data().size());
      EXPECT_EQ(indexes, std::vector<std::uint8_t>({0x01, 0x02, 0x04, 0x05}));
    }
    std::vector<mugen::pcx::Pcx::Pixel> data(pcx.data().size());
    data[0].red = 0x11;
    data[0].green = 0x11;
    data[0].blue = 0x11;
    data[0].alpha = 255;
    data[1].red = 0x22;
    data[1].green = 0x22;
    data[1].blue = 0x22;
    data[1].alpha = 255;
    data[2].red = 0x44;
    data[2].green = 0x44;
    data[2].blue = 0x44;
    data[2].alpha = 255;
    data[3].red = 0x55;
    data[3].green = 0x55;
    data[3].blue = 0x55;
    data[3].alpha = 255;
    for (std::size_t i = 0; i < data.size(); ++i) {
      EXPECT_EQ(pcx.data()[i].red, data[i].red);
      EXPECT_EQ(pcx.data()[i].green, data[i].green);
      EXPECT_EQ(pcx.data()[i].blue, data[i].blue);
      EXPECT_EQ(pcx.data()[i].alpha, data[i].alpha);
    }
  } catch (...) {
    ASSERT_TRUE(false);
  }
}
