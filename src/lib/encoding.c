#include <stddef.h> // NULL

#include "encoding.h"
#include "types.h"

StoreDynamicString StoreConvertUnicodeToUtf8(uint32_t codepoint)
{
	StoreDynamicString utf8 = StoreCreateDynamicString();

	// UTF-8 conversion table:
	//
	// codepoint				binary					byte1		byte2		byte3		byte4
	// 0000 - 007F				00000000000000xxxxxxx	0xxxxxxx
	// 0080 - 07FF				0000000000yyyyyxxxxxx	110yyyyy	10xxxxxx
	// 0800 - D7FF, E000 - FFFF	00000zzzzyyyyyyxxxxxx	1110zzzz	10yyyyyy	10xxxxxx
	// 10000 - 10FFFF			uuuzzzzzzyyyyyyxxxxxx	11110uuu	10zzzzzz	10yyyyyy	10xxxxxx

	if(codepoint <= 0x7f) {
		StoreAppendDynamicString(utf8, "%c", (char) codepoint);
	} else if(codepoint <= 0x7ff) {
		StoreAppendDynamicString(utf8, "%c", (char) (codepoint >> 6) + 0xc0); // shift 6 bits to the right, then prepend 110
		StoreAppendDynamicString(utf8, "%c", (char) (codepoint & 0x3f) + 0x80); // only use lowest 6 bits, then prepend 10
	} else if(codepoint <= 0xd7ff || (codepoint >= 0xe000 && codepoint <= 0xffff)) {
		StoreAppendDynamicString(utf8, "%c", (char) (codepoint >> 12) + 0xe0); // shift 12 bits to the right, then prepend 1110
		StoreAppendDynamicString(utf8, "%c", (char) ((codepoint >> 6) & 0x3f) + 0x80); // shift 6 bits to the right, then only use 6 lowest bits, then prepend 10
		StoreAppendDynamicString(utf8, "%c", (char) (codepoint & 0x3f) + 0x80); // only use lowest 6 bits, then prepend 10
	} else if(codepoint >= 0x10000 && codepoint <= 0x10ffff) {
		StoreAppendDynamicString(utf8, "%c", (char) ((codepoint >> 18) & 0x7) + 0xf0); // shift 18 bits to the right, then only use 3 lowest bits, then prepend 11110
		StoreAppendDynamicString(utf8, "%c", (char) ((codepoint >> 12) & 0x3f) + 0x80); // shift 12 bits to the right, then only use 6 lowest bits, then prepend 10
		StoreAppendDynamicString(utf8, "%c", (char) ((codepoint >> 6) & 0x3f) + 0x80); // shift 6 bits to the right, then only use 6 lowest bits, then prepend 10
		StoreAppendDynamicString(utf8, "%c", (char) (codepoint & 0x3f) + 0x80); // only use lowest 6 bits, then prepend 10
	} else { // invalid code point
		StoreFreeDynamicString(utf8);
		return NULL;
	}

	return utf8;
}
