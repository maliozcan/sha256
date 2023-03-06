#include "hash.h"

#include <limits>
#include <climits>
#include <cassert>
#include <string_view>
#include <iostream>
#include <iomanip>
#include <fstream>


static constexpr uint16_t num_of_bits_in_message_block_ = 512;
static_assert(CHAR_BIT == 8);
static constexpr uint8_t one_bit_size_ = CHAR_BIT; // Use CHAR_BIT for 1 bit adding since messages are always multiple of CHAR_BIT.
static constexpr uint8_t used_for_necessary_ = CHAR_BIT - 1;
static constexpr uint8_t num_of_bits_for_length_in_last_block_ = 64 - used_for_necessary_;
static constexpr uint8_t word_size_ = 32;

// Initialize hash values: first 32 bits of the fractional parts of the square roots of the first 8 primes 2..19
static constexpr std::array<uint32_t, 8> h_constants_ = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

// Initialize array of round constants: first 32 bits of the fractional parts of the cube roots of the first 64 primes 2..311
static constexpr std::array<uint32_t, 64> k_constants_ = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static inline uint32_t get_number(const char* data)
{
    uint32_t number = 0;
    for (int8_t i = 0; i < 4; ++i) {
        number |= static_cast<uint8_t>(*(data + i)) << ((3 - i) * 8);
    }
    return number;
}

static std::string len(uint64_t message_length_in_bit)
{
    std::string str(8, '\0');
    for (int8_t i = 0; i < 8; ++i) {
        str[i] = static_cast<char>((message_length_in_bit >> (8 * (7 - i))) & 0xFF);
    }
    return str;
}

static inline uint32_t rotate_right(const uint32_t x, const uint8_t n)
{
    return (x >> n) | (x << (32 - n));
}

// lower case sigma_0
static inline uint32_t lc_sigma_0(const uint32_t x)
{
    return rotate_right(x, 7) ^ rotate_right(x, 18) ^ (x >> 3);
}

// lower case sigma_1
static inline uint32_t lc_sigma_1(const uint32_t x)
{
    return rotate_right(x, 17) ^ rotate_right(x, 19) ^ (x >> 10);
}

// upper case sigma_0
static inline uint32_t uc_sigma_0(const uint32_t x)
{
    return rotate_right(x, 2) ^ rotate_right(x, 13) ^ rotate_right(x, 22);
}

// upper case sigma_1
static inline uint32_t uc_sigma_1(const uint32_t x)
{
    return rotate_right(x, 6) ^ rotate_right(x, 11) ^ rotate_right(x, 25);
}

static inline uint32_t ch(const uint32_t x, const uint32_t y, const uint32_t z)
{
    return (x & y) ^ (~x & z);
}

static inline uint32_t maj(const uint32_t x, const uint32_t y, const uint32_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

static void hash_computation_for_one_block(const std::string_view& block, std::array<uint32_t, 8>& hv)
{
    constexpr uint8_t word_number = num_of_bits_in_message_block_ / word_size_;
    static_assert(word_number == 16);
    
    // 1. initalize message schedule
    thread_local std::array<uint32_t, 64> w{}; // thread_local was used for performance since this function is called mant times for large streams.
    for (uint8_t t = 0; t < word_number; ++t) {
        w[t] = get_number(block.data() + t * (word_size_ / CHAR_BIT));
#ifdef DEBUG_LOGS
        std::printf("w[%2u]: ", t);
        std::cout << std::hex << w[t] << "\n";
#endif
    }
    for (uint8_t t = word_number; t < 64; ++t) {
        w[t] = lc_sigma_1(w[t - 2]) + w[t - 7] + lc_sigma_0(w[t - 15]) + w[t - 16];
    }

    // 2. initialize the working variables a, ,,,, h
    uint32_t a = hv[0];
    uint32_t b = hv[1];
    uint32_t c = hv[2];
    uint32_t d = hv[3];
    uint32_t e = hv[4];
    uint32_t f = hv[5];
    uint32_t g = hv[6];
    uint32_t h = hv[7];

    // 3
#ifdef DEBUG_LOGS
    std::printf("\n         a        b        c         d        e        f       g        h        \n");
    const auto prev_base = std::cout.setf(std::ios_base::hex, std::ios_base::basefield);
    const auto prev_char = std::cout.fill('0');
#endif
    for (uint8_t t = 0; t < 64; ++t) {
        const uint32_t t1 = h + uc_sigma_1(e) + ch(e, f, g) + k_constants_[t] + w[t];
        const uint32_t t2 = uc_sigma_0(a) + maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
#ifdef DEBUG_LOGS
        std::printf("t=%2u: ", t);
        std::cout
            << std::setw(sizeof(uint32_t) * 2) << a << " " 
            << std::setw(sizeof(uint32_t) * 2) << b << " " 
            << std::setw(sizeof(uint32_t) * 2) << c << " " 
            << std::setw(sizeof(uint32_t) * 2) << d << " " 
            << std::setw(sizeof(uint32_t) * 2) << e << " " 
            << std::setw(sizeof(uint32_t) * 2) << f << " " 
            << std::setw(sizeof(uint32_t) * 2) << g << " " 
            << std::setw(sizeof(uint32_t) * 2) << h << "\n";
#endif
    }
#ifdef DEBUG_LOGS
    std::cout.setf(prev_base);
    std::cout.fill(prev_char);
#endif

    // 4. compute the intermediate hash value
    hv[0] += a;
    hv[1] += b;
    hv[2] += c;
    hv[3] += d;
    hv[4] += e;
    hv[5] += f;
    hv[6] += g;
    hv[7] += h;
}

constexpr size_t block_size = num_of_bits_in_message_block_ / CHAR_BIT;
static_assert((num_of_bits_for_length_in_last_block_ + used_for_necessary_) % CHAR_BIT == 0);
constexpr auto offset_for_padding = block_size + (num_of_bits_for_length_in_last_block_ + used_for_necessary_) / CHAR_BIT;
constexpr size_t buffer_size = 8192;
static_assert(buffer_size % block_size == 0);

// There may be one or two blocks
static inline void hash_computation_for_last_blocks(std::array<char, buffer_size + offset_for_padding>& buffer, std::array<uint32_t, 8>& hv,
                                                    const std::streamsize size, const std::size_t message_length_in_byte, const std::size_t num_of_blocks)
{
    const std::size_t beginning_of_remaining_msg = num_of_blocks * block_size;
    const std::size_t remaining_size = size - beginning_of_remaining_msg;
    const uint16_t num_of_bits_in_last_block = (remaining_size * CHAR_BIT + one_bit_size_ + num_of_bits_for_length_in_last_block_) % num_of_bits_in_message_block_;
    const uint16_t num_of_padding_bits = (num_of_bits_in_last_block == 0) ? 0 : num_of_bits_in_message_block_ - num_of_bits_in_last_block;
    assert((num_of_padding_bits - used_for_necessary_) % CHAR_BIT == 0);

    // Padding
    std::string padding{};
    padding.reserve(offset_for_padding);
    padding.push_back(static_cast<char>(0x80));
    padding.append((num_of_padding_bits - used_for_necessary_) / CHAR_BIT, 0);
    padding.append(len(message_length_in_byte * CHAR_BIT));
    for (uint32_t i = 0; i < padding.size(); ++i) {
        buffer[size + i] = padding[i];
    }
    
    const std::size_t size_of_last_blocks = remaining_size * CHAR_BIT + one_bit_size_ + num_of_bits_for_length_in_last_block_ + num_of_padding_bits;
    assert(size_of_last_blocks % num_of_bits_in_message_block_ == 0);
    const std::size_t num_of_last_blocks = size_of_last_blocks / num_of_bits_in_message_block_;
    for (std::size_t block_index = 0; block_index < num_of_last_blocks; ++block_index) {
        const std::size_t begin_pos = beginning_of_remaining_msg + block_index * block_size;
        const std::string_view block(buffer.data() + begin_pos, block_size);
        hash_computation_for_one_block(block, hv);
    }
}

namespace hash {

std::string sha256(std::string& message)
{
    std::stringstream ss{message};
    return sha256(ss);
}

std::string sha256(std::istream& stream)
{
    std::array<char, buffer_size + offset_for_padding> buffer{};

    std::size_t message_length_in_byte = 0;
    std::streamsize size = 0;
    std::size_t num_of_blocks = 0;
    std::array<uint32_t, 8> hv{h_constants_}; // hash values
    while(stream.good()) {
        stream.read(buffer.data(), buffer_size);
        size = stream.gcount();
        message_length_in_byte += size;
        num_of_blocks = size / block_size;
        for (std::size_t block_index = 0; block_index < num_of_blocks; ++block_index) {
            const std::size_t begin_pos = block_index * block_size;
            const std::string_view block(buffer.data() + begin_pos, block_size);
            hash_computation_for_one_block(block, hv);
        }
    }

    hash_computation_for_last_blocks(buffer, hv, size, message_length_in_byte, num_of_blocks);

    // concatenate the hash parts
    std::stringstream ss;
    ss.fill('0');
    ss.setf(std::ios_base::hex, std::ios_base::basefield);
    constexpr uint8_t word_width = 32 / 4;
    for (auto hash_value : hv) {
        ss << std::setw(word_width) << hash_value;
    }
#ifdef DEBUG_LOGS
    std::cout << "\nhash: " << ss.str() << "\n";
#endif

    return ss.str();
}

} // namespace hash
