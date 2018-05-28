#include "util.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>

void int2bin6(int value, bool * bin)
{
    int resul;

    memset( bin, 0, 6 );

    for (int i = 5; i >= 0; i--) {
        bin[i] = 1;
        resul = bin[5]*32+bin[4]*16+bin[3]*8+bin[2]*4+bin[1]*2+bin[0]*1;

        if (resul == value) {
            return;
        }
        if (resul > value) {
            bin[i] = 0;
        }
    }
}

uint32_t str2uint( char *s )
{
    uint32_t result = 0;

    if (s == 0) {
        return 0;
    }

    while ( (*s >= '0') && (*s <='9') ) {
        result *= 10;
        result += *s - '0';
        s++;
    }

    return result;
}

void int2str( char *s, int num )
{
    short k = 0;
    char aux[11];

    if (num == 0) {
        aux[k] = 0;
        k++;
    }

    while (num) {
        aux[k] = num%10;
        num = num/10;
        k++;
    }

    while (k>0) {
        *s = (aux[k-1] + '0');
        s++;
        k--;
    }

    *s = 0;
}

bool version_str2int( char *s, uint8_t *maj, uint8_t *min, uint8_t *pat )
{
    if( s[0] == 'V' && isdigit(s[1]) && isdigit(s[3]) && isdigit(s[5]) ) {
        *maj = s[1]-'0';
        *min = s[3]-'0';
        *pat = s[5]-'0';
        return true;
    }
    /* Bad formatted string */
    return false;
}

int file_rename( const char *oldfname, const char *newfname, const char *prefix = NULL )
{
    int retval = 0;
    int ch;

    char oldf[20], newf[20];

    if (prefix) {
        strcpy(oldf, prefix);
        strcpy(newf, prefix);
    }

    strcat(oldf, oldfname);
    strcat(newf, newfname);

    FILE *fpold = fopen(oldf, "r");   // src file
    FILE *fpnew = fopen(newf, "w");   // dest file

    while (1) {                   // Copy src to dest
        ch = fgetc(fpold);        // until src EOF read.
        if (ch == EOF) break;
        fputc(ch, fpnew);
    }

    fclose(fpnew);
    fclose(fpold);

    fpnew = fopen(newf, "r"); // Reopen dest to insure
    if(fpnew == NULL) {           // that it was created.
        retval = (-1);            // Return Error.
    }
    else {
        fclose(fpnew);
        remove(oldf);         // Remove original file.
        retval = (0);             // Return Success.
    }
    return (retval);
}
