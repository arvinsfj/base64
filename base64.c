//
//  main.c
//  testbase64
//
//  Created by cz on 4/24/16.
//  Copyright © 2016 arvin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char base64digits[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define BAD	-1
static const char base64val[] = {
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD, 62, BAD,BAD,BAD, 63,
    52, 53, 54, 55,  56, 57, 58, 59,  60, 61,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
    15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25,BAD, BAD,BAD,BAD,BAD,
    BAD, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
    41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51,BAD, BAD,BAD,BAD,BAD
};

//raw data to base 64 string
void to_base64(char *out, const char *in, unsigned long inlen)
{
    while(inlen >= 3){
        *out++ = base64digits[((in[0] >> 2) & 0x3f)];
        *out++ = base64digits[((in[0] << 4) & 0x30) | ((in[1] >> 4) & 0x0f)];
        *out++ = base64digits[((in[1] << 2) & 0x3c) | ((in[2] >> 6) & 0x03)];
        *out++ = base64digits[in[2] & 0x3f];
        in += 3;
        inlen -= 3;
    }
    
    if (inlen > 0){
        unsigned char fragment;
        *out++ = base64digits[((in[0] >> 2) & 0x3f)];
        fragment = (in[0] << 4) & 0x30;
        if (inlen > 1){
            fragment |= (in[1] >> 4) & 0x0f;
        }
        *out++ = base64digits[fragment];
        *out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
        *out++ = '=';
    }
    
    *out = '\0';
}

unsigned long from_base64(char *out, const char *in)
{
    unsigned long len = 0;
    unsigned char d1, d2, d3, d4;
    
    do{
        d1 = in[0];
        if (d1 > 127 || base64val[d1] == BAD)
            return -1;
        d2 = in[1];
        if (d2 > 127 || base64val[d2] == BAD)
            return -1;
        d3 = in[2];
        if (d3 > 127 || (d3 != '=' && base64val[d3] == BAD))
            return -1;
        d4 = in[3];
        if (d4 > 127 || (d4 != '=' && base64val[d4] == BAD))
            return -1;
        in += 4;
        
        *out++ = ((base64val[d1] << 2) & 0xfc) | ((base64val[d2] >> 4) & 0x0f);
        len++;
        
        if (d3 != '='){
            *out++ = ((base64val[d2] << 4) & 0xf0) | ((base64val[d3]) >> 2 & 0x0f);
            len++;
            
            if (d4 != '='){
                *out++ = ((base64val[d3] << 6) & 0xc0) | (base64val[d4] & 0x3f);
                len++;
            }
        }
    }while(*in && d4 != '=');
    
    return len;
}

char* piperead(unsigned long *length)
{
    *length = 0;
    const unsigned long addsize = sizeof(char) * 1024 * 1024; // 1mb
    unsigned long size = addsize;
    char *ctt = malloc(size);
    memset(ctt, '\0', size);
    char c;
    char *cttp = ctt;
    while ((c = getchar()) != EOF) {
        if (*length >= size) {
            size += addsize;
            char *cttr = realloc(ctt, size);
            if (cttr == NULL) {
                return ctt;
            }
            if (ctt != cttr) {
                cttp = cttr + (size - addsize);
                ctt = cttr;
            }
        }
        (*length)++;
        *cttp++ = c;
    }
    return ctt;
}

char* fileread(const char *filename, unsigned long *length)
{
    *length = 0;
    FILE *fd;
    if ((fd = fopen(filename, "rb")) == NULL) {
        printf("file %s can not open\n", filename);
        exit(1);
    }
    fseek(fd, 0, SEEK_END);
    *length = ftell(fd);
    rewind(fd);
    char *ctt = malloc(*length);
    memset(ctt, '\0', *length);
    
    *length = fread(ctt, 1, *length, fd);
    
    fclose(fd);
    
    return ctt;
}

void filewrite(const char *filename,const char *data, const unsigned long length)
{
    FILE *fd;
    if ((fd = fopen(filename, "wb")) == NULL) {
        printf("file %s can not open\n", filename);
        exit(1);
    }
    
    fwrite(data, 1, length, fd);
    
    fclose(fd);
}

int main(int ac, char **av)
{
    char *arg;
    char *filename = NULL;
    char *ctt;
    
    if (ac != 3) {
        printf("usage: base64 -e [filename] > [filename]\n");
        printf("usage: cat [filename] | base64 -d [filename]\n");
        printf("usage: base64 -e [filename] | base64 -d [filename]\n");
        exit(1);
    }
    
    arg = av[1];
    filename = av[2];
    
    if (strcmp("-e", arg) && strcmp("-d", arg)) {
        printf("usage: base64 -e [filename] > [filename]\n");
        printf("usage: cat [filename] | base64 -d [filename]\n");
        printf("usage: base64 -e [filename] | base64 -d [filename]\n");
        exit(1);
    }
    
    if (!strcmp("-e", arg)) {
        unsigned long inlen;
        ctt = fileread(filename, &inlen);
        unsigned long outlen = (inlen/3)*4 + ((inlen%3) ? 4 : 0) + 1;
        char *outstr = malloc(sizeof(char)*outlen);
        memset(outstr, '\0', outlen);
        to_base64(outstr, ctt, inlen);
        printf("%s",outstr);
    }
    
    if (!strcmp("-d", arg)) {
        unsigned long inlen;
        ctt = piperead(&inlen);
        char *outstr = malloc(sizeof(char)*inlen);
        memset(outstr, '\0', inlen);
        unsigned long outlen = from_base64(outstr, ctt);
        filewrite(filename, outstr, outlen);
    }
    
    return 0;
}

