//
//  JPEGENC test app in pure C
//
//  written by Larry Bank (bitbank@pobox.com)
//  Project started July 2021
//  Copyright (c) 2021 BitBank Software, Inc.
//  

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#include "rgb565.h"
#include "JPEGENC.h"

#define OUTPUT_SIZE 65536

//
// Read a Windows BMP file into memory
// For this demo, the only supported files are 24 or 32-bits per pixel
//
uint8_t * ReadBMP(const char *fname, int *width, int *height, int *bpp, unsigned char *pPal)
{
    int y, w, h, bits, offset;
    uint8_t *s, *d, *pTemp, *pBitmap;
    int pitch, bytewidth;
    int iSize, iDelta;
    FILE *infile;

    infile = fopen(fname, "r+b");
    if (infile == NULL) {
        printf("Error opening input file %s\n", fname);
        return NULL;
    }
    // Read the bitmap into RAM
    fseek(infile, 0, SEEK_END);
    iSize = (int)ftell(infile);
    fseek(infile, 0, SEEK_SET);
    pBitmap = (uint8_t *)malloc(iSize);
    pTemp = (uint8_t *)malloc(iSize);
    fread(pTemp, 1, iSize, infile);
    fclose(infile);

    if (pTemp[0] != 'B' || pTemp[1] != 'M' || pTemp[14] < 0x28) {
        free(pBitmap);
        free(pTemp);
        printf("Not a Windows BMP file!\n");
        return NULL;
    }
    w = *(int32_t *)&pTemp[18];
    h = *(int32_t *)&pTemp[22];
    bits = *(int16_t *)&pTemp[26] * *(int16_t *)&pTemp[28];
    if (bits <= 8) { // it has a palette, copy it
        free(pBitmap);
        free(pTemp);
        return NULL; // only support 24/32-bpp for now
//        uint8_t *p = pPal;
//        for (int i=0; i<(1<<bits); i++)
//        {
//           *p++ = pTemp[54+i*4];
//           *p++ = pTemp[55+i*4];
//           *p++ = pTemp[56+i*4];
//        }
    }
    offset = *(int32_t *)&pTemp[10]; // offset to bits
    bytewidth = (w * bits) >> 3;
    pitch = (bytewidth + 3) & 0xfffc; // DWORD aligned
// move up the pixels
    d = pBitmap;
    s = &pTemp[offset];
    iDelta = pitch;
    if (h > 0) {
        iDelta = -pitch;
        s = &pTemp[offset + (h-1) * pitch];
    } else {
        h = -h;
    }
    for (y=0; y<h; y++) {
        if (bits == 32) {// need to swap red and blue
            for (int i=0; i<bytewidth; i+=4) {
                d[i] = s[i+2];
                d[i+1] = s[i+1];
                d[i+2] = s[i];
                d[i+3] = s[i+3];
            }
        } else {
            memcpy(d, s, bytewidth);
        }
        d += bytewidth;
        s += iDelta;
    }
    *width = w;
    *height = h;
    *bpp = bits;
    free(pTemp);
    return pBitmap;
    
} /* ReadBMP() */

int process_bmp_file(const char *input_path, const char *output_path)
{
    int rc, w, h, bpp, pitch;
    uint8_t *bitmap;
    uint8_t *buffer;
    int buffer_size;
    JPEGE_IMAGE jpeg;
    JPEGENCODE jpe;
    FILE *fp;

    bitmap = ReadBMP(input_path, &w, &h, &bpp, NULL);
    if (bitmap == NULL)
    {
        fprintf(stderr, "Unable to open file\n");
        return -1; // bad filename passed?
    }
    pitch = bpp / 8 * w;
    printf("%s, w : %d, h : %d, pitch : %d\n", __func__, w, h, pitch);

    buffer_size = (w * h * 3) / 4;
    buffer = (uint8_t *)malloc(buffer_size);

    memset(&jpeg, 0, sizeof(JPEGE_IMAGE));
    jpeg.pOutput = buffer;
    jpeg.iBufferSize = buffer_size;
    jpeg.pHighWater = &jpeg.pOutput[jpeg.iBufferSize - 512];

    rc = JPEGEncodeBegin(&jpeg, &jpe, w, h, JPEGE_PIXEL_RGB565, JPEGE_SUBSAMPLE_420, JPEGE_Q_HIGH);
    if (rc == JPEGE_SUCCESS)
        JPEGAddFrame(&jpeg, &jpe, bitmap, pitch);

    JPEGEncodeEnd(&jpeg);
    printf("%s, jpeg size : %d\n", __func__, jpeg.iDataSize);

    printf("%s, write to %s\n", __func__, output_path);
    fp = fopen(output_path, "wb");
    if (fp) {
        fwrite(buffer, 1, jpeg.iDataSize, fp);
        fclose(fp);
    }
    free(buffer);

    if (bitmap)
        free(bitmap);

    return 0;
}


int process_bmp_data(uint8_t *bmp, size_t len, const char *output_path)
{
    int rc, y, w, h, bits, offset;
    int pitch, bytewidth, delta;
    uint8_t *buffer, *bmp_tmp;
    size_t buffer_size;
    uint8_t *src, *dst;
    JPEGE_IMAGE jpeg;
    JPEGENCODE jpe;
    FILE *fp;

    /* Sanity check */
    if (bmp[0] != 'B' || bmp[1] != 'M' || bmp[14] < 0x28) {
        printf("Not a BMP file\n");
        return -EINVAL;
    }

    w = *(int32_t *)&bmp[18];
    h = *(int32_t *)&bmp[22];

    bits = *(int16_t *)&bmp[26] * *(int16_t *)&bmp[28];
    if (bits != 16) {
        printf("Not a 16-bit BMP file\n");
        return -EINVAL;
    }

    offset = *(int32_t *)&bmp[10];
    bytewidth = (w * bits) >> 3;
    pitch = (bytewidth + 3) & 0xfffc;
    printf("%s, w : %d, h : %d, pitch : %d\n", __func__, w, h, pitch);

    buffer_size = len;
    buffer = (uint8_t *)malloc(buffer_size);
    bmp_tmp = (uint8_t *)malloc(buffer_size);

    dst = bmp_tmp;
    src = &bmp[offset];
    delta = pitch;
    if (h > 0) {
        delta = -pitch;
        src = &bmp[offset + (h-1) * pitch];
    } else {
        h = -h;
    }

    for (y = 0; y < h; y++) {
        memcpy(dst, src, bytewidth);
        dst += bytewidth;
        src += delta;
    }

    memset(&jpeg, 0, sizeof(JPEGE_IMAGE));
    jpeg.pOutput = buffer;
    jpeg.iBufferSize = buffer_size;
    jpeg.pHighWater = &jpeg.pOutput[jpeg.iBufferSize - 512];

    rc = JPEGEncodeBegin(&jpeg, &jpe, w, h, JPEGE_PIXEL_RGB565, JPEGE_SUBSAMPLE_420, JPEGE_Q_HIGH);
    if (rc == JPEGE_SUCCESS)
        JPEGAddFrame(&jpeg, &jpe, bmp_tmp, pitch);

    JPEGEncodeEnd(&jpeg);
    printf("%s, jpeg size : %d\n", __func__, jpeg.iDataSize);

    printf("%s, write to %s\n", __func__, output_path);
    fp = fopen(output_path, "wb");
    if (fp) {
        fwrite(buffer, 1, jpeg.iDataSize, fp);
        fclose(fp);
    }
    free(buffer);
    free(bmp_tmp);

    return 0;
}

int main(int argc, char *argv[])
{
    int rc;

    printf("JPEGENC linux test app in pure C\n");

    rc = process_bmp_file("../rgb565.bmp", "../output.jpg");
    if (rc) {
        printf("process bmp file failed, %d\n", rc);
        return -1;
    }

    rc = process_bmp_data(rgb565, sizeof(rgb565), "../output2.jpg");
    if (rc) {
        printf("process bmp data failed, %d\n", rc);
        return -1;
    }

    return 0;
}