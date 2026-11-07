#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include <vector>
#include <cstdint>
#include <cstddef>  

uint32_t decode_symbol(const std::vector<uint8_t>& data, std::map<uint32_t, uint32_t> table, size_t& bit_pos);
std::map<uint32_t, uint32_t> build_huffman_table(const std::vector<uint8_t>& lengths, size_t size);

#endif