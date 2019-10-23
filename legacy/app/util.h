#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>

void int2bin6(int value, bool * bin);
uint32_t str2uint( char *s );
void int2str( char *s, int num );
bool version_str2int( char *s, uint8_t *maj, uint8_t *min, uint8_t *pat );

int file_rename( const char *oldfname, const char *newfname, const char *prefix );
#endif
