#include "custom-zlib.h"

#include "zbuild.h"

int Z_EXPORT PREFIX(compress)(unsigned char *dest, z_uintmax_t *destLen, const unsigned char *source, z_uintmax_t sourceLen, handle_match_func handle_match, void* handle_match_userdata) {
    return PREFIX(compress2)(dest, destLen, source, sourceLen, Z_DEFAULT_COMPRESSION, handle_match, handle_match_userdata);
}

int Z_EXPORT PREFIX(compress2)(unsigned char *dest, z_uintmax_t *destLen, const unsigned char *source, z_uintmax_t sourceLen, int level, handle_match_func handle_match, void* handle_match_userdata) {
    PREFIX3(stream) stream;
    int err;
    const unsigned int max = (unsigned int)-1;
    z_size_t left;

    left = *destLen;
    *destLen = 0;

    stream.zalloc = NULL;
    stream.zfree = NULL;
    stream.opaque = NULL;
    stream.handle_match = handle_match;
    stream.handle_match_userdata = handle_match_userdata;

    err = PREFIX(deflateInit)(&stream, level);
    if (err != Z_OK)
        return err;

    stream.next_out = dest;
    stream.avail_out = 0;
    stream.next_in = (z_const unsigned char *)source;
    stream.avail_in = 0;

    do {
        if (stream.avail_out == 0) {
            stream.avail_out = left > (unsigned long)max ? max : (unsigned int)left;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = sourceLen > (unsigned long)max ? max : (unsigned int)sourceLen;
            sourceLen -= stream.avail_in;
        }
        err = PREFIX(deflate)(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
    } while (err == Z_OK);

    *destLen = stream.total_out;
    PREFIX(deflateEnd)(&stream);
    return err == Z_STREAM_END ? Z_OK : err;
}
