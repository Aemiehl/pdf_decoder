#include "Huffman.hpp"

#include "BitReader.hpp"

#include <vector>
#include <map>
#include <iostream>
#include <stdexcept>
#include <cstdint>


uint32_t decode_symbol(const std::vector<uint8_t>& data, std::map<uint32_t, uint32_t> table, size_t& bit_pos){
    uint32_t symbol = 0;
    int symbolSize = 0;

    while(true){
        symbol = (symbol << 1) | readBits(data, bit_pos, 1);
        symbolSize += 1;

        // geht so da symbol / die nodes Präfixfrei sind
        std::map<uint32_t, uint32_t>::iterator finder = table.find(symbol);
        if(finder != table.end()){
            return finder->second;
        }

        if(symbolSize > 15){ // MAX_CODE_LENGTH für DEFLATE ist 15 Bits
            throw std::runtime_error("Invalid Huffman Code");
        }
    }
}

// bauen der Huffman Table | scheint RICHTIG
std::map<uint32_t, uint32_t> build_huffman_table(const std::vector<uint8_t>& lengths, size_t size) {
    
    std::map<uint32_t, uint32_t> table;

    std::vector<uint32_t> countVector(16, 0);
    for(size_t i = 0; i < size; i++){
        countVector[lengths[i]] += 1;
    }

    std::vector<uint32_t> codeVector(16, 0);
    for(size_t i = 1; i < 16; i++){
        codeVector[i] = (codeVector[i-1] + countVector[i-1]) << 1;
    }

    for (size_t i = 0; i < size; ++i) {
        uint8_t len = lengths[i];
        if (len != 0) {
            table[codeVector[len]] = i;
            codeVector[len]++;
        }
    }

    return table;
}


