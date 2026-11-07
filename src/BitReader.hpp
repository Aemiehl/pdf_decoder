#ifndef BITREADER_HPP
#define BITREADER_HPP

#include <vector>
#include <cstdint>
#include <cstddef>  

uint32_t readBits(const std::vector<uint8_t>& data, size_t& startBit, size_t numBits);

#endif