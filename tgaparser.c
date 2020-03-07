#include <stdio.h>
#include <stdlib.h>
#include "tgaparser.h"
#include "file.h"
#include "rend.h"
#include "math.h"

#define loghex(x) printf(#x" = %02X\n", x);
#define dumpheader(h) {\
    unsigned char *dump = (unsigned char*)&h; \
    for (size_t i = 0; i < sizeof(h); i++) printf("%02X ", dump[i]); \
    printf("\n"); }

void logheader(Tga_Header* h)
{
    printf(
        "id_length: %02X, color_map_type: %02X, data_type_code: %02X ,\n"
        "color_map_origin: %i, color_map_length: %i, color_map_depth: %i, \n"
        "x_origin: %i, y_origin: %i, width: %i, height: %i, \n"
        "bits_per_pixel: %i, image_descriptor: %02X\n",
        h->id_length, h->color_map_type, h->data_type_code, h->color_map_origin,
        h->color_map_length, h->color_map_depth, h->x_origin, h->y_origin,
        h->width, h->height, h->bits_per_pixel, h->image_descriptor);
}

/* Can't use memcpy for struct because of misalignment */
void load_header(Tga_Header* header, char* tga_string)
{
    header->id_length = tga_string[0];
    header->color_map_type = tga_string[1];
    header->data_type_code = tga_string[2];
    header->color_map_origin = (tga_string[3] >> 8) | (tga_string[4] << 8);
    header->color_map_length = (tga_string[5] >> 8) | (tga_string[6] << 8);
    header->color_map_depth = tga_string[7];
    header->x_origin = (tga_string[8] >> 8) | (tga_string[9] << 8);
    header->y_origin = (tga_string[10] >> 8) | (tga_string[11] << 8);
    header->width = (tga_string[12] >> 8) | (tga_string[13] << 8);
    header->height = (tga_string[14] >> 8) | (tga_string[15] << 8);
    header->bits_per_pixel = tga_string[16];
    header->image_descriptor = tga_string[17];
}

/* Bitmap format 'R G B R G B ...' where each pixel has 3 colors (3 bytes).
   NOTE: only works RGB and RGBA formats
   TODO: add grayscale format compatibility
   TODO: take x_origin and y_origin into account
 */
Bitmap* parse_tga_file(char* file_path)
{
    size_t tga_strlen;
    char* tga_string = load_file(file_path, &tga_strlen);
    char* tga_string_for_free = tga_string;

    /* Get header */
    Tga_Header header;
    load_header(&header, tga_string);
    logheader(&header);

    /*  */
    int bytes_per_pixel = header.bits_per_pixel / 8;
    int alpha_included = (bytes_per_pixel > 3);

    /* Create bitmap struct */
    Bitmap* bmp = malloc(sizeof(Bitmap));
    bmp->width = header.width;
    bmp->height = header.height;
    size_t bitmap_length = bmp->width * bmp->height * bytes_per_pixel;
    bmp->data = malloc(bitmap_length);

    tga_string += TGA_HEADER_SIZE;
    size_t i = 0;
    while (i < bitmap_length) {
        uchar packet_header = (uchar) *tga_string++;
        if (packet_header < 128) {
            /* Raw packet */
            int body_size = packet_header + 1;
            for (int j = 0; j < body_size; j++) {
                char b = *tga_string++;
                char g = *tga_string++;
                char r = *tga_string++;
                /* Ignore alpha channel if included */
                if (alpha_included)
                    tga_string++;
                bmp->data[i++] = r;
                bmp->data[i++] = g;
                bmp->data[i++] = b;
            }
        } else {
            /* Run-length packet */
            int run_length = packet_header - 128 + 1;
            char b = *tga_string++;
            char g = *tga_string++;
            char r = *tga_string++;
            /* Ignore alpha channel if included */
            if (alpha_included)
                tga_string++;
            /* NOTE: we reverse BGR order so we get RGB */
            for (int j = 0; j < run_length; j++) {
                bmp->data[i++] = r;
                bmp->data[i++] = g;
                bmp->data[i++] = b;
            }            
        }
    }

    free(tga_string_for_free);

    return bmp;
}

