#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include <vector>
#include <cstdint>
#include <cstddef>  

void process_dynamic_huffman_block(const std::vector<uint8_t>& data, size_t& bit_pos, std::vector<uint8_t>& decoded_data);

#endif