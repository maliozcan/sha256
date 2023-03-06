#ifndef HASH_H
#define HASH_H

#include <string>
#include <iosfwd>

namespace hash
{
    /**
     * @brief Calculates hash for SHA-256.
     *
     * @param message
     * @return std::string
     */
    std::string sha256(std::string& message);

    /**
     * @brief Calculates hash for SHA-256.
     *
     * @param stream
     * @return std::string
     *
     * @note the stream may a file stream or string stream or any stream that is derived from std::istream.
     *
     */
    std::string sha256(std::istream& stream);
} // namespace hash




#endif // HASH_H
