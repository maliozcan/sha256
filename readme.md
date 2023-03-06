# An Implementation of the SHA-256 algorithm

Reference Resource: [FIPS PUB 180-4](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf)

Example hash values: [Examples with Intermediate Values for SHA256](https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Standards-and-Guidelines/documents/examples/SHA256.pdf) (This is very helpful for debugging)

## Compilation And Running

### Prerequisites

curl is necessary for test build, or you can download [doctest.h](https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h).

### Test Build

```shell
$ ./build.sh test
$ g++ --std=c++17 -g -Wall -Wextra -Wpedantic -Werror hash.h hash.cpp test.cpp -o test
$ ./test
```
If you want to open debug logs, you should define `DEBUG_LOGS` macro such that

```shell
$ g++ --std=c++17 -g -Wall -Wextra -Wpedantic -Werror -DDEBUG_LOGS hash.h hash.cpp test.cpp -o test
```

### CLI Application Build

```shell
$ ./build.sh app
$ g++ --std=c++17 -O3 -Wall -Wextra -Wpedantic -Werror -DNDEBUG hash.h hash.cpp sha256.cpp -o sha256
$ ./sha256 sha256.cpp 
f843d418d1f3546ab63b6196c1a6225475a4c5c168e66bcdb0e90e322d7bf4fa  sha256.cpp
$ cat sha256.cpp | ./sha256
f843d418d1f3546ab63b6196c1a6225475a4c5c168e66bcdb0e90e322d7bf4fa  -
```
