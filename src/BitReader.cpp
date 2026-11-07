#include "BitReader.hpp"

#include <vector>
#include <cstdint>
#include <cstddef> 
#include <stdexcept> 

//einlesen von bis zu 32 bits | scheint RICHTIG
uint32_t readBits(const std::vector<uint8_t>& data, size_t& startBit, size_t numBits) {
    uint32_t result = 0;  // Hier speichern wir das Ergebnis
    size_t byteIndex = startBit / 8;  // Der Startbyte-Index
    size_t bitIndex = startBit % 8;   // Der Startbit-Index innerhalb des Bytes

    if(numBits == 0){
        throw std::runtime_error("Invalid Read Size");
    }

    for (size_t i = 0; i < numBits; ++i) {
        // Extrahiere das aktuelle Bit
        uint8_t currentByte = data[byteIndex];
        uint8_t bitValue = (currentByte >> (7 - bitIndex)) & 1;  // Extrahiere das relevante Bit

        // Schiebe das Bit an die richtige Position im Ergebnis
        result = (result << 1) | bitValue;

        // Gehe zum nächsten Bit über
        ++startBit;
        ++bitIndex;
        if (bitIndex == 8) {
            bitIndex = 0;
            ++byteIndex;  // Zum nächsten Byte übergehen
        }
    }

    return result;
}