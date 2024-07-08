#include "ArduinoUZlib.h"
#define OUT_CHUNK_SIZE 1
ArduinoUZlib::ArduinoUZlib(){}

void exit_error(const char *what)
{
    printf("ERROR: %s\n", what);
    exit(1);
}


// https://github.com/pfalcon/uzlib/blob/master/examples/tgunzip/tgunzip.c
int32_t ArduinoUZlib::decompress(uint8_t *in_buf, uint32_t in_size, uint8_t *&dest, uint32_t &out_size) {
	if (in_size < 4) exit_error("file too small");
	unsigned int len,dlen, outlen;
	unsigned char *source =(unsigned char*)in_buf;
	
	int res;
	len=in_size;
    dlen =            source[len - 1];
    dlen = 256*dlen + source[len - 2];
    dlen = 256*dlen + source[len - 3];
    dlen = 256*dlen + source[len - 4];

    outlen = dlen;
	dlen++;
	dest = (unsigned char *)malloc(dlen);
    if (dest == NULL) exit_error("memory");

  	struct uzlib_uncomp d;
	//    uzlib_uncompress_init(&d, malloc(32768), 32768);
	uzlib_uncompress_init(&d, NULL, 0);
	d.source = source;
    d.source_limit = source + len - 4;
    d.source_read_cb = NULL;

    res = uzlib_gzip_parse_header(&d);
    if (res != TINF_OK) {
        printf("Error parsing header: %d\n", res);
        exit(1);
    }

    d.dest_start = d.dest = dest;

    while (dlen) {
        unsigned int chunk_len = dlen < OUT_CHUNK_SIZE ? dlen : OUT_CHUNK_SIZE;
        d.dest_limit = d.dest + chunk_len;
        res = uzlib_uncompress_chksum(&d);
        dlen -= chunk_len;
        if (res != TINF_OK) {
            break;
        }
    }

    if (res != TINF_DONE) {
        printf("Error during decompression: %d\n", res);
        exit(-res);
    }

    printf("decompressed %lu bytes\n", d.dest - dest);
	out_size= d.dest - dest;
	return  out_size;

} 
