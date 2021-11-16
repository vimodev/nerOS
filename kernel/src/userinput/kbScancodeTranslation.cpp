#include "kbScancodeTranslation.h"

namespace QWERTYKeyboard {

    // Lookup table for scan_code -> ascii
    const char ascii_table[] {
        0 ,  0 , '1', '2',
        '3', '4', '5', '6',
        '7', '8', '9', '0',
        '-', '=',  0 ,  0 ,
        'q', 'w', 'e', 'r',
        't', 'y', 'u', 'i',
        'o', 'p', '[', ']',
         0 ,  0 , 'a', 's',
        'd', 'f', 'g', 'h',
        'j', 'k', 'l', ';',
        '\'','`',  0 , '\\',
        'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',',
        '.', '/',  0 , '*',
         0 , ' '
    };

    // Look up the scan code in the table
    char translate(uint8_t scan_code, bool upper_case) {
        // Out of bounds
        if (scan_code > 58) return 0;
        // Upper case lookup
        if (upper_case) return ascii_table[scan_code] - 32;
        // otherwise return lookup
        return ascii_table[scan_code];
    }

}