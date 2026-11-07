#ifndef LZ77_HPP
#define LZ77_HPP

#include <vector>

#include <vector>
#include <iostream>
#include <cstdint>

size_t decode_length(const std::vector<uint8_t>& data, size_t& bit_pos, uint32_t length);
size_t decode_distance(const std::vector<uint8_t>& data, size_t& bit_pos, uint32_t distance);
void copy_sequence(std::vector<uint8_t>& decoded_data, size_t length, size_t distance);

#endif