#include "oink_judge/utils/crypto.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <string>

using namespace oink_judge::utils::crypto;

// ---------------------------------------------------------------------------
// sha256
// ---------------------------------------------------------------------------

TEST(CryptoSha256Test, MatchesKnownDigest) {
    EXPECT_EQ(sha256("hello"), "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

TEST(CryptoSha256Test, IsDeterministicForSameInput) { EXPECT_EQ(sha256("hello"), sha256("hello")); }

TEST(CryptoSha256Test, DiffersForDifferentInput) { EXPECT_NE(sha256("hello"), sha256("world")); }

TEST(CryptoSha256Test, Produces64LowercaseHexCharacters) {
    const auto hash = sha256("test input");

    ASSERT_EQ(hash.size(), 64U);
    EXPECT_TRUE(std::ranges::all_of(
        hash, [](char character) -> bool { return std::isxdigit(static_cast<unsigned char>(character)) != 0; }));
}

TEST(CryptoSha256Test, EmptyInputHasStableDigest) {
    const auto empty_hash = sha256("");
    EXPECT_EQ(empty_hash.size(), 64U);
    EXPECT_EQ(empty_hash, sha256(""));
}

// ---------------------------------------------------------------------------
// Base64
// ---------------------------------------------------------------------------

TEST(CryptoBase64Test, RoundTripPreservesBinaryData) {
    const std::string input = "hello\x00world\xff"; // NOLINT

    EXPECT_EQ(fromBase64(toBase64(input)), input);
}

TEST(CryptoBase64Test, EncodesKnownAsciiValue) { EXPECT_EQ(toBase64("hello"), "aGVsbG8="); }

TEST(CryptoBase64Test, EmptyInputRoundTrip) {
    const std::string input;

    EXPECT_EQ(toBase64(input), "");
    EXPECT_EQ(fromBase64(toBase64(input)), input);
}

TEST(CryptoBase64Test, InvalidInputThrows) { EXPECT_THROW(fromBase64("!!!"), std::runtime_error); }
