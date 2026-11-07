#include "Huffman.hpp"

#include "BitReader.hpp"

#include <vector>
#include <map>
#include <iostream>
#include <stdexcept>
#include <cstdint>


void align_to_byte_boundary(size_t& bit_pos) {
    if (bit_pos % 8 != 0) {
        bit_pos += 8 - (bit_pos % 8);  // Verschiebe bit_pos zur nächsten Byte-Grenze
    }
}

static uint32_t decode_symbol(const std::vector<uint8_t>& data, std::map<uint32_t, uint32_t> table, size_t& bit_pos){
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

static void readH(const std::vector<uint8_t>& data, std::map<uint32_t, uint32_t> table, std::vector<uint8_t>& codes, size_t& bit_pos, size_t H) {
   
    size_t i = 0;

    while(i < H){
        uint32_t symbol = decode_symbol(data, table, bit_pos);

        if(symbol <= 15){
            codes[i++] = symbol;

        }else if(symbol == 16){
            int repeat = readBits(data, bit_pos, 2) + 3;
            uint8_t previous_lenght = codes[i - 1];
            for(int j = 0; j < repeat && i < H; j++){
                codes[i++] = previous_lenght;
            }

        }else if(symbol == 17){
            int repeat = readBits(data, bit_pos, 3) + 3;
            for(int j = 0; j < repeat && i < H; j++){
                codes[i++] = 0;
            }

        }else if(symbol == 18){
            int repeat = readBits(data, bit_pos, 7) + 11;
            for(int j = 0; j < repeat && i < H; j++){
                codes[i++] = 0;
            }
            
        }else if(symbol > 18) {
            throw std::runtime_error("Invalid symbol in readH");
        }
    }
}

// bauen der Huffman Table | scheint RICHTIG
static std::map<uint32_t, uint32_t> build_huffman_table(const std::vector<uint8_t>& lengths, size_t size) {
    
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

static size_t decode_length(const std::vector<uint8_t>& data, size_t& bit_pos, uint32_t length){
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

static size_t decode_distance(const std::vector<uint8_t>& data, size_t& bit_pos, uint32_t distance){
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

static void copy_sequence(std::vector<uint8_t>& decoded_data, size_t length, size_t distance) {
    size_t start_pos = decoded_data.size() - distance;  // Bestimme, wo die zu kopierende Sequenz beginnt
    for (size_t i = 0; i < length; ++i) {
        decoded_data.push_back(decoded_data[start_pos + i % distance]);
    }
}

void process_dynamic_huffman_block(const std::vector<uint8_t>& data, size_t& bit_pos, std::vector<uint8_t>& decoded_data) {
    // align_to_byte_boundary(bit_pos);
    size_t HLIT = readBits(data, bit_pos, 5) + 257;
    size_t HDIST = readBits(data, bit_pos, 5) + 1;
    size_t HCLEN = readBits(data, bit_pos, 4) + 4;

    // Reihenfolge der Code-Längen-Codes
    std::vector<uint8_t> code_length_order = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    std::vector<uint8_t> code_lengths(19, 0);

    for(size_t i = 0; i < HCLEN; i++){
        code_lengths[code_length_order[i]] = readBits(data, bit_pos, 3);
    }

    std::map<uint32_t, uint32_t> code_length_table = build_huffman_table(code_lengths, 19);

    std::vector<uint8_t> literal_length_codes(HLIT, 0);
    std::vector<uint8_t> distance_codes(HDIST, 0);
    
    readH(data, code_length_table, literal_length_codes, bit_pos, HLIT); // HIER ERROR
    readH(data, code_length_table, distance_codes, bit_pos, HDIST);

    std::map<uint32_t, uint32_t> literal_length_table = build_huffman_table(literal_length_codes, HLIT);
    std::map<uint32_t, uint32_t> distance_table = build_huffman_table(distance_codes, HDIST);

    while(true){
        uint32_t package = decode_symbol(data, literal_length_table, bit_pos);

        // Literal 
        if(package < 256){
            decoded_data.push_back(static_cast<uint8_t>(package)); // nochmal prüfen 

        // End-of-block
        }else if(package == 256){
            break;

        // Längenwert 
        }else if(package >= 257 && package <= 285){
            size_t length = decode_length(data, bit_pos, package);

            uint32_t distance_package = decode_symbol(data, distance_table, bit_pos);
            size_t distance = decode_distance(data, bit_pos, distance_package);

            copy_sequence(decoded_data, length, distance);
        
        }else{
            std::cerr << "Ungültiges package!" << std::endl;
        }

    }

    align_to_byte_boundary(bit_pos);
}
