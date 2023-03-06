#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "hash.h"

// https://csrc.nist.gov/projects/cryptographic-standards-and-guidelines/example-values#aHashing

TEST_CASE("One block sample") {
    std::string hashed("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    std::string message{"abc"};
    CHECK(hash::sha256(message) == hashed);
}

TEST_CASE("Two block sample") {
    std::string hashed("248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    std::string message = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    CHECK(hash::sha256(message) == hashed);
}
