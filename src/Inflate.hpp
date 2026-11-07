#ifndef INFLATE_HPP
#define INFLATE_HPP

#include <vector>
#include <cstdint>
#include <cstddef>  
 
std::vector<uint8_t> zlib_decompress(const std::vector<uint8_t> compressed_data);

#endif