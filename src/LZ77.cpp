#include "LZ77.hpp"

#include "BitReader.hpp"

#include <vector>
#include <iostream>
#include <cstdint>


size_t decode_length(const std::vector<uint8_t>& data, size_t& bit_pos, uint32_t length){
    if(length > 256 && length < 265){
        return length - 254;

    }else if(length > 264 && length < 285){
        size_t numBits = (length - 261) / 4;
        size_t baseLength = 11 + ((length - 265) % 4) * (1 << numBits);
        size_t extraLength = readBits(data, bit_pos, numBits);
        return baseLength + extraLength;

    }else if(length == 285){
        return 258;
    }

    std::cerr << "Ungültige length!" << std::endl;
    return -1;
}

size_t decode_distance(const std::vector<uint8_t>& data, size_t& bit_pos, uint32_t distance){
    if(distance < 4){
        return distance + 1;

    }else if(distance < 30){
        size_t numBits = (distance - 2) / 2;
        size_t baseDistance = (4 + ((distance - 4) % 2) * (1 << numBits));
        size_t extraDistance = readBits(data, bit_pos, numBits);
        return baseDistance + extraDistance;
    }

    std::cerr << "Ungültige distance!" << std::endl;
    return -1;
}

void copy_sequence(std::vector<uint8_t>& decoded_data, size_t length, size_t distance) {
    size_t start_pos = decoded_data.size() - distance;  // Bestimme, wo die zu kopierende Sequenz beginnt
    for (size_t i = 0; i < length; ++i) {
        decoded_data.push_back(decoded_data[start_pos + i % distance]);
    }
}