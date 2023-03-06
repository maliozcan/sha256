#include "hash.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

int main(int argc, char* argv[])
{
    int result = 0;

    if (argc < 2) {
        std::cout << hash::sha256(std::cin) << "  -\n";
    }

    for (int file_index = 1; file_index < argc; ++file_index) {
        const std::string filename{argv[file_index]};
        std::ifstream file{filename, std::ios::in | std::ios::binary};
        if (file.is_open()) {
            std::cout << hash::sha256(file) << "  " << filename << '\n';
        } else {
            std::cout << argv[0] << ": '" << filename << "': " << std::strerror(errno) << '\n';
            result = -1;
        }
    }

    return result;
}
