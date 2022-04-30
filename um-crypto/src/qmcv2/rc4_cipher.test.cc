#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "um-crypto/qmcv2.h"

using ::testing::ElementsAreArray;

using umc::qmcv2::RC4Cipher;
using namespace umc;

inline Vec<u8> InitKey(usize len) {
  Vec<u8> key(len);

  for (usize i = 0; i < len; i++) {
    key[i] = i & 0xff;
  }

  return key;
}

TEST(CryptoQMCv2RC4, TestDecryption) {
  auto key = InitKey(400);

  Vec<u8> expected[5] = {
      Vec<u8>({0x32, 0x10}), Vec<u8>({0x00, 0x00}), Vec<u8>({0xC2, 0x33}),
      Vec<u8>({0xF9, 0xFE}), Vec<u8>({0xF9, 0xFE}),
  };

  RC4Cipher cipher(key);
  for (int i = 0, offset = 1; i < 5; i++, offset *= 16) {
    cipher.Seek(offset);

    Vec<u8> result;
    ASSERT_EQ(cipher.Decrypt(result, Vec<u8>({0, 0})), true);
    ASSERT_THAT(result, ElementsAreArray(expected[i]));
  }
}

TEST(CryptoQMCv2RC4, TestDecryptionWithFF) {
  auto key = InitKey(400);

  Vec<u8> expected[5] = {Vec<u8>({0xCD, 0xEF}), Vec<u8>({0xFF, 0xFF}),
                         Vec<u8>({0x3D, 0xCC}), Vec<u8>({0x06, 0x01}),
                         Vec<u8>({0x06, 0x01})};

  RC4Cipher cipher(key);
  for (int i = 0, offset = 1; i < 5; i++, offset *= 16) {
    cipher.Seek(offset);

    Vec<u8> result;
    ASSERT_EQ(cipher.Decrypt(result, Vec<u8>({0xff, 0xff})), true);
    ASSERT_THAT(result, ElementsAreArray(expected[i]));
  }
}

TEST(CryptoQMCv2RC4, TestDecryptionWithFFAtFirstSegmentEndBoundary) {
  auto key = InitKey(400);

  Vec<u8> input(10);
  std::fill(input.begin(), input.end(), 0xff);

  Vec<u8> expected =
      Vec<u8>({{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x8C, 0x77, 0xE0, 0x7F, 0xB9}});

  RC4Cipher cipher(key);
  cipher.Seek(0x80 - 5);

  Vec<u8> result;
  ASSERT_EQ(cipher.Decrypt(result, input), true);
  ASSERT_THAT(result, ElementsAreArray(expected));
}

TEST(CryptoQMCv2RC4, TestDecryptionWithFFAtSecondSegmentEndBoundary) {
  auto key = InitKey(400);

  Vec<u8> input(10);
  std::fill(input.begin(), input.end(), 0xff);

  Vec<u8> expected =
      Vec<u8>({{0x98, 0x70, 0xE6, 0xF5, 0xB4, 0xE5, 0x7B, 0xCC, 0xEA, 0x15}});

  RC4Cipher cipher(key);
  cipher.Seek(0x1400 - 5);

  Vec<u8> result;
  ASSERT_EQ(cipher.Decrypt(result, input), true);
  ASSERT_THAT(result, ElementsAreArray(expected));
}