# rank-cpp-experiments

## Build

__Note__: you need to have Boost _headers_ available on your system path.  On
Linux, installation from a package manager will suffice.  On macOS it is
simplest to use Homebrew: `brew install boost`.

```shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j${nproc}
```

## Applications

These are `rank` and `sim-cpa`.

### `sim-cpa`

`sim-cpa` generates a set of distinguishing vectors for a simulated CPA on a
configurable number of AES SubBytes targets.  The scores are written to disk
in a binary format (see below) and the key used is written as a hexadecimal
string to a second file.

Example use:

```shell
./src/sim-cpa -f scores.bin -k key.txt -t 1000 -s 0.0125 -r 1234
```

This generates 1000 traces with an SNR of 0.0125 (assuming standard DPA model,
leakage of the Hamming weight of an 8-bit value etc).  The key, plaintext and
randomness are generated with the seed 1234.  The program is deterministic in
the seed, so re-using the same seed will produce the same output.

Use `./src/sim-cpa -h` to see all arguments.

### `rank`

`rank` reads a set of distinguishing vectors and key from disk and estimates the
rank of the key.

Example use:

```shell
$ ./src/rank -f scores.bin -k key.txt -p 16
keyWeight: 32470
rank: 2154623129
rank (log2): 31.0048
duration (s): 0.0869005
```

This ranks at 16 bits of precision, outputting the integer weight for the
correct key, the rank and log rank, and the runtime of the rank algorithm in
seconds.

Use `./src/rank -h` to see all arguments.

## Binary format

All distinguishing vectors for an attack are stored as simple binary blobs.
The bytes corresponding to the first distinguishing vector appear first, the
bytes corresponding to the second appear next, and so on.

Distinguishing scores _MUST_ be serialized as double values (e.g. consuming
8 bytes per score on most platforms).

## Changing attack dimensions

The `rank-cpp` library must be told the "dimensions" of the attack at compile
time.  Dimensions consist of the number of distinguishing vectors
(`VectorCount` in the code) and the width of each distinguishing vector in bits.

This means that if you change any of these values you must re-compile the
binary.

If you look at the main method of `rank.cpp` you will see the following lines of
code:

```cpp
constexpr std::size_t const ByteCount = 16;
constexpr std::size_t const KeyLenBits = ByteCount * 8;
rankcpp::Dimensions dimensions(ByteCount, 8);
rankcpp::estimateRank<KeyLenBits>(dimensions, precision, filepath,
                                  keyFilepath);
```

In this case, we're assuming a standard attack on AES, with 16 attacks on each
of the 8-bit SubBytes outputs.

To handle an 16-bit subkey attacks on a 4096-bit RSA key, you'll need:

```cpp
constexpr std::size_t const BitWidth = 16;
constexpr std::size_t const KeyLenBits = 4096;
constexpr std::size_t const ByteCount = 4096 / BitWidth;
rankcpp::Dimensions dimensions(ByteCount, BitWidth);
rankcpp::estimateRank<KeyLenBits>(dimensions, precision, filepath,
                                  keyFilepath);
```

The library can also handle "uneven" attacks; e.g. to estimate the rank with
8-bit attacks on the first four subkey and 32-bit attacks on the remaining:

```cpp
constexpr std::size_t const KeyLenBits = 128;
constexpr std::size_t const ByteCount = 7;
rankcpp::Dimensions dimensions({8, 8, 8, 8, 32, 32, 32});
rankcpp::estimateRank<KeyLenBits>(dimensions, precision, filepath,
                                  keyFilepath);
```