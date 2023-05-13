#include <zlib-ng.h>

Z_EXTERN Z_EXPORT
int32_t zng_compress(uint8_t *dest, size_t *destLen, const uint8_t *source, size_t sourceLen, handle_match_func handle_match, void* handle_match_userdata);

Z_EXTERN Z_EXPORT
int32_t zng_compress2(uint8_t *dest, size_t *destLen, const uint8_t *source, size_t sourceLen, int32_t level, handle_match_func handle_match, void* handle_match_userdata);
