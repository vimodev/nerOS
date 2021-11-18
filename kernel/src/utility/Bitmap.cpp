#include "Bitmap.h"

// Get the i'th bit using the [] operator
bool Bitmap::operator[](uint64_t index){
    return get(index);
}

// Get the value at the given index
bool Bitmap::get(uint64_t index){
    // Out of bounds
    if (index > size * 8) return false;
    // Split byte and bit index
    uint64_t byte_index = index / 8;
    uint8_t bit_index = index % 8;
    // Create a mask
    uint8_t bit_indexer = 0b10000000 >> bit_index;
    // Apply mask
    if ((buffer[byte_index] & bit_indexer) > 0){
        return true;
    }
    return false;
}

// Set the i'th bit to value
bool Bitmap::set(uint64_t index, bool value){
    // Out of bounds
    if (index > size * 8) return false;
    // Split byte and bit index
    uint64_t byte_index = index / 8;
    uint8_t bit_index = index % 8;
    // Create mask
    uint8_t bit_indexer = 0b10000000 >> bit_index;
    // Set target bit to 0
    buffer[byte_index] &= ~bit_indexer;
    // And if value = true set it to 1 again
    if (value){
        buffer[byte_index] |= bit_indexer;
    }
    return true;
}