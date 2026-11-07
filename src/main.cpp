#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>

#include "Inflate.hpp"

// Funktion, um 4-Byte-Wert von Big-Endian nach Little-Endian zu konvertieren
uint32_t to_little_endian(uint8_t* bytes) {
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

// Funktion, um den PNG-Header zu prüfen
bool check_png_header(uint8_t* header) {
    // PNG-Dateien haben immer diese spezielle Signatur als Header
    uint8_t png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    return std::memcmp(header, png_signature, 8) == 0;
}

int main() {
    std::ifstream file("./data/test2.png", std::ios::binary);
    std::vector<uint8_t> compressed_data;
    
    if (!file.is_open()) {
        std::cerr << "Fehler beim Öffnen der Datei!" << std::endl;
        return 1;
    }

    // PNG-Header (8 Bytes) lesen
    uint8_t header[8];
    file.read(reinterpret_cast<char*>(header), 8);
    
    if (!check_png_header(header)) {
        std::cerr << "Die Datei ist keine gültige PNG-Datei!" << std::endl;
        return 1;
    }


    
    // Jetzt beginnen die Chunks (erstes Chunk sollte IHDR sein)
    // hier werden die einzelnen chunks ausgelesen und zugeordnet
    while (!file.eof()) {
        uint8_t length_bytes[4];
        uint8_t type_bytes[4];

        // Länge des Chunks (4 Bytes, Big-Endian)
        file.read(reinterpret_cast<char*>(length_bytes), 4);
        uint32_t length = to_little_endian(length_bytes);


        // Typ des Chunks (4 Bytes, ASCII)
        file.read(reinterpret_cast<char*>(type_bytes), 4);
        std::string chunk_type(reinterpret_cast<char*>(type_bytes), 4);

        std::cout << "Chunk-Typ: " << chunk_type << ", Länge: " << length << std::endl;

        // IHDR-Chunk behandeln (enthält Bildinformationen)
        if (chunk_type == "IHDR") {
            uint8_t ihdr_data[13];  // IHDR-Chunk hat immer 13 Bytes
            file.read(reinterpret_cast<char*>(ihdr_data), 13);

            // Breite und Höhe (jeweils 4 Bytes, Big-Endian)
            uint32_t width = to_little_endian(ihdr_data);
            uint32_t height = to_little_endian(ihdr_data + 4);

            // Farbtiefe (1 Byte) und andere Informationen
            uint8_t bit_depth = ihdr_data[8];
            uint8_t color_type = ihdr_data[9];

            std::cout << "Bildbreite: " << width << ", Bildhöhe: " << height << std::endl;
            std::cout << "Farbtiefe: " << (int)bit_depth << ", Farbtyp: " << (int)color_type << std::endl;
            file.seekg(4, std::ios::cur);
            continue;
        }

        // PLTE-Chunk behandeln (weis noch nicht ob nötig)
        if (chunk_type == "PLTE"){
            std::vector<uint8_t> palette;
            palette.resize(length);
            file.read(reinterpret_cast<char*>(palette.data()), length);
            file.seekg(4, std::ios::cur); 
            continue;
        }

        // IDAT-Chunk behandeln (enthält das Bild)  
        if (chunk_type == "IDAT") {
            std::vector<uint8_t> idat_data(length);
            file.read(reinterpret_cast<char*>(idat_data.data()), length);
            compressed_data.insert(compressed_data.end(), idat_data.begin(), idat_data.end());
            file.seekg(4, std::ios::cur);
            continue;
        }
        
        // Wenn das Ende (IEND) erreicht ist, abbrechen
        if (chunk_type == "IEND") {
            std::cout << "Ende der PNG-Datei erreicht." << std::endl;
            break;
        }

        // Daten des Chunks überspringen (plus 4 Bytes für CRC)
        file.seekg(length + 4, std::ios::cur);
    }
    
    // IDAT-Chunk compressed_data behandeln (enthält das Bild)  
    std::vector<uint8_t> data = zlib_decompress(compressed_data);




    // TODO build huffman tree

    // Verarbeite den dynamischen Huffman-Block
    // std::vector<uint8_t> compressed_data2 = {0x78, 0x9C, 0x63, 0x60, 0x18, 0x05, 0xA3, 0x60, 0x14, 0x8C, 0x8C, 0xAC, 0xC0, 0x00, 0x00, 0xFF, 0xFF};
    //bit_pos = 16;

    std::vector<uint8_t> decoded_data;
    // bit_pos = 0;
    
    //decoded_data = sh_zlib_decompress(compressed_data);
    // process_dynamic_huffman_block(compressed_data, bit_pos, decoded_data);



    // TODO decode huffman codes
    // TODO LZ77 decode
    // TODO apply filter
    // TODO create image / vector
    // TODO unkomprimiertes bild anzeigen

    file.close();
    return 0;
}
