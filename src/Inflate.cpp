#include "Inflate.hpp"

#include "BitReader.hpp"
#include "Huffman.hpp"
#include "LZ77.hpp"

#include <vector>
#include <map>
#include <cstdint>

static void align_to_byte_boundary(size_t& bit_pos) {
    if (bit_pos % 8 != 0) {
        bit_pos += 8 - (bit_pos % 8);  // Verschiebe bit_pos zur nächsten Byte-Grenze
    }
}

// Funktion zum Lesen der ersten Bits des DEFLATE-Streams, um den Blocktyp zu bestimmen
static int get_block_type(const std::vector<uint8_t>& data, size_t& bit_pos) {
    size_t byte_pos = bit_pos / 8;  // Berechne das aktuelle Byte
    size_t bit_offset = bit_pos % 8;  // Berechne die Bit-Position innerhalb des Bytes

    uint8_t first_byte = data[byte_pos];  // Hol das aktuelle Byte
    // BFINAL ist das erste Bit
    int bfinal = (first_byte >> bit_offset) & 1;

    // BTYPE sind die nächsten 2 Bits
    int btype = (first_byte >> (bit_offset + 1)) & 3;

    // Wenn das Bit-Offset nicht bei 0 liegt, müssen wir die Bits aus dem nächsten Byte holen
    if(bit_offset <= 5) {
        // Der normale Fall: BTYPE passt vollständig in das aktuelle Byte
        btype = (first_byte >> (bit_offset + 1)) & 3;
    
    }else if(bit_offset == 6) {
        // Spezialfall: BFINAL im aktuellen Byte, 1 Bit von BTYPE im aktuellen und 1 Bit im nächsten Byte
        uint8_t next_byte = data[byte_pos + 1];
        btype = ((first_byte >> 7) & 1) | ((next_byte & 1) << 1);
    
    }else if(bit_offset == 7) {
        // Spezialfall: BFINAL im aktuellen Byte, beide BTYPE-Bits im nächsten Byte
        uint8_t next_byte = data[byte_pos + 1];
        btype = next_byte & 3;  // Die ersten beiden Bits des nächsten Bytes
    }

    // Update der Bit-Position um 3
    bit_pos += 3;

    std::cout << "BFINAL: " << bfinal << ", BTYPE: " << btype << std::endl;
    return btype;
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

static void process_dynamic_huffman_block(const std::vector<uint8_t>& data, size_t& bit_pos, std::vector<uint8_t>& decoded_data) {
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

int zlib_decompress(std::vector<uint8_t> compressed_data){
    size_t bit_pos = 16;  // Starte bei der ersten Bit-Position, 16 um den zlib header zu überspringen
    int blockType = get_block_type(compressed_data, bit_pos);
    if(blockType != 2){
        std::cerr << "No implementation for such Block Types! " << blockType << std::endl;
        return -1;
    }else{
        std::cout << "Block Type is 2!" << std::endl;
    }
}