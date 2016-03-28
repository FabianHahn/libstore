#ifndef LIBSTORE_ENCODING
#define LIBSTORE_ENCODING

#include <stdint.h> // uint32_t

#include "api.h"
#include "types.h"

/**
 * Converts a Unicode codepoint (UCS character) into an UTF-8 encoded string
 *
 * @param codepoint	the codepoint of the character to convert
 * @result			the resulting UTF-8 string, must be freed by the caller
 */
LIBSTORE_NO_EXPORT StoreDynamicString StoreConvertUnicodeToUtf8(uint32_t codepoint);

#endif
