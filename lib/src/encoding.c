#include <stdbool.h> // bool
#include <stddef.h> // NULL

#include "store/encoding.h"

GString *storeConvertUnicodeToUtf8(uint32_t codepoint)
{
	GString *utf8 = g_string_new("");

	// UTF-8 conversion table:
	//
	// codepoint				binary					byte1		byte2		byte3		byte4
	// 0000 - 007F				00000000000000xxxxxxx	0xxxxxxx
	// 0080 - 07FF				0000000000yyyyyxxxxxx	110yyyyy	10xxxxxx
	// 0800 - D7FF, E000 - FFFF	00000zzzzyyyyyyxxxxxx	1110zzzz	10yyyyyy	10xxxxxx
	// 10000 - 10FFFF			uuuzzzzzzyyyyyyxxxxxx	11110uuu	10zzzzzz	10yyyyyy	10xxxxxx

	if(codepoint <= 0x7f) {
		g_string_append_printf(utf8, "%c", (char) codepoint);
	} else if(codepoint <= 0x7ff) {
		g_string_append_printf(utf8, "%c", (char) (codepoint >> 6) + 0xc0); // shift 6 bits to the right, then prepend 110
		g_string_append_printf(utf8, "%c", (char) (codepoint & 0x3f) + 0x80); // only use lowest 6 bits, then prepend 10
	} else if(codepoint <= 0xd7ff || (codepoint >= 0xe000 && codepoint <= 0xffff)) {
		g_string_append_printf(utf8, "%c", (char) (codepoint >> 12) + 0xe0); // shift 12 bits to the right, then prepend 1110
		g_string_append_printf(utf8, "%c", (char) ((codepoint >> 6) & 0x3f) + 0x80); // shift 6 bits to the right, then only use 6 lowest bits, then prepend 10
		g_string_append_printf(utf8, "%c", (char) (codepoint & 0x3f) + 0x80); // only use lowest 6 bits, then prepend 10
	} else if(codepoint >= 0x10000 && codepoint <= 0x10ffff) {
		g_string_append_printf(utf8, "%c", (char) ((codepoint >> 18) & 0x7) + 0xf0); // shift 18 bits to the right, then only use 3 lowest bits, then prepend 11110
		g_string_append_printf(utf8, "%c", (char) ((codepoint >> 12) & 0x3f) + 0x80); // shift 12 bits to the right, then only use 6 lowest bits, then prepend 10
		g_string_append_printf(utf8, "%c", (char) ((codepoint >> 6) & 0x3f) + 0x80); // shift 6 bits to the right, then only use 6 lowest bits, then prepend 10
		g_string_append_printf(utf8, "%c", (char) (codepoint & 0x3f) + 0x80); // only use lowest 6 bits, then prepend 10
	} else { // invalid code point
		g_string_free(utf8, true);
		return NULL;
	}

	return utf8;
}
