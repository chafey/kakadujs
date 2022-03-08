// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

#include <fstream>
#include <iostream>
#include <vector>
#include <iterator>
#include <time.h>
#include <algorithm>

#include "../../src/HTJ2KDecoder.hpp"
#include "../../src/HTJ2KEncoder.hpp"

void readFile(std::string fileName, std::vector<uint8_t> &vec)
{
    // open the file:
    std::ifstream file(fileName, std::ios::in | std::ios::binary);
    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);

    // get its size:
    int fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve capacity
    vec.resize(0);
    vec.reserve(fileSize);

    // read the data:
    vec.insert(vec.begin(),
               std::istream_iterator<uint8_t>(file),
               std::istream_iterator<uint8_t>());
}

void writeFile(std::string fileName, const std::vector<uint8_t> &vec)
{
    std::ofstream file(fileName, std::ios::out | std::ofstream::binary);
    std::copy(vec.begin(), vec.end(), std::ostreambuf_iterator<char>(file));
}

enum
{
    NS_PER_SECOND = 1000000000
};

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

void decodeFile(const char *path, size_t iterations = 1)
{
    HTJ2KDecoder decoder;
    std::vector<uint8_t> &encodedBytes = decoder.getEncodedBytes();
    readFile(path, encodedBytes);

    timespec start, finish, delta;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    decoder.readHeader();

    for (int i = 0; i < iterations; i++)
    {
        decoder.decode();
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &finish);
    sub_timespec(start, finish, &delta);
    auto frameInfo = decoder.getFrameInfo();

    auto ns = delta.tv_sec * 1000000000.0 + delta.tv_nsec;
    auto totalTimeMS = ns / 1000000.0;
    auto timePerFrameMS = ns / 1000000.0 / (double)iterations;
    auto pixels = (frameInfo.width * frameInfo.height);
    auto megaPixels = (double)pixels / (1024.0 * 1024.0);
    auto fps = 1000 / timePerFrameMS;
    auto mps = (double)(megaPixels)*fps;

    printf("Native-decode %s Pixels=%d megaPixels=%f TotalTime= %.2f ms TPF=%.2f ms (%.2f MP/s, %.2f FPS)\n", path, pixels, megaPixels, totalTimeMS, timePerFrameMS, mps, fps);
}

void encodeFile(const char *inPath, const FrameInfo frameInfo, const char *outPath = NULL, size_t iterations = 1)
{
    HTJ2KEncoder encoder;
    // encoder.setDecompositions(2);
    // encoder.setQuality(false, 0.001f);
    // encoder.setBlockDimensions(Size(16, 16));
    // encoder.setProgressionOrder(0);
    std::vector<uint8_t> &rawBytes = encoder.getDecodedBytes(frameInfo);

    readFile(inPath, rawBytes);

    timespec start, finish, delta;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

    for (int i = 0; i < iterations; i++)
    {
        encoder.encode();
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &finish);
    sub_timespec(start, finish, &delta);

    auto ns = delta.tv_sec * 1000000000.0 + delta.tv_nsec;
    auto totalTimeMS = ns / 1000000.0;
    auto timePerFrameMS = ns / 1000000.0 / (double)iterations;
    auto pixels = (frameInfo.width * frameInfo.height);
    auto megaPixels = (double)pixels / (1024.0 * 1024.0);
    auto fps = 1000 / timePerFrameMS;
    auto mps = (double)(megaPixels)*fps;

    printf("Native-encode %s Pixels=%d megaPixels=%f TotalTime= %.2f ms TPF=%.2f ms (%.2f MP/s, %.2f FPS)\n", inPath, pixels, megaPixels, totalTimeMS, timePerFrameMS, mps, fps);

    if (outPath)
    {
        const std::vector<uint8_t> &encodedBytes = encoder.getEncodedBytes();
        // printf("encodedBytes.size() = %lu\n", encodedBytes.size());
        writeFile(outPath, encodedBytes);
    }
}

int main(int argc, char **argv)
{
    // warm up the decoder and encoder
    decodeFile("test/fixtures/j2c/CT1.j2c", 1);
    encodeFile("test/fixtures/raw/CT1.RAW", {.width = 512, .height = 512, .bitsPerSample = 16, .componentCount = 1, .isSigned = true});

    const size_t iterations = (argc > 1) ? atoi(argv[1]) : 1;
    decodeFile("test/fixtures/j2c/CT1.j2c", iterations);
    decodeFile("test/fixtures/j2c/MG1.j2c", iterations);
    //   decodeFile("test/fixtures/j2c/CT2.j2c");
    //   decodeFile("test/fixtures/j2c/MG1.j2c");

    encodeFile("test/fixtures/raw/CT1.RAW", {.width = 512, .height = 512, .bitsPerSample = 16, .componentCount = 1, .isSigned = true}, "test/fixtures/j2c/ignore.j2c", iterations);
    return 0;
}
