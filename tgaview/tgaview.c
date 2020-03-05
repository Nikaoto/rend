#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "gfx.h"

typedef unsigned char uchar;

#define loghex(x) printf(#x" = %02X\n", x);
#define dumpheader(h) {\
    unsigned char *dump = (unsigned char*)&h; \
    for (size_t i = 0; i < sizeof(h); i++) printf("%02X ", dump[i]); \
    printf("\n"); }

#define READ_BUF_SIZE 256
#define TGA_HEADER_SIZE   18

typedef struct Tga_Header {
    char id_length;
    char color_map_type;
    char data_type_code;
    short color_map_origin;
    short color_map_length;
    char color_map_depth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char bits_per_pixel;
    char image_descriptor;
} Tga_Header;

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

char* load_file(char* file_path, size_t* size)
{
    char buf[READ_BUF_SIZE];

    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        printf("Couldn't open %s\n", file_path);
        return 0;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        printf("fstat failed\n");
        return 0;
    }

    size_t file_size = (size_t) st.st_size;

    // Set size pointer if given
    if (size) {
        *size = file_size;
    }

    size_t bytes_read = read(fd, buf, READ_BUF_SIZE);
    if (bytes_read == 0) {
        printf("File %s empty\n", file_path);
        return 0;
    }
    if (bytes_read < 0) {
        printf("Couldn't read %s\n", file_path);
        return 0;
    }

    size_t total_read = 0;
    char* file_string = malloc(file_size + 1);

    do {
        for (size_t i = 0; i < bytes_read; i++) {
            file_string[total_read + i] = buf[i];
        }
        total_read += bytes_read;
        bytes_read = read(fd, buf, READ_BUF_SIZE);
    } while(bytes_read > 0);
    file_string[file_size] = '\0';

    return file_string;
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
 */
char* tga_to_bitmap(char* tga_string, size_t str_length, Tga_Header* header)
{
    int bytes_per_pixel = header->bits_per_pixel / 8;
    int alpha_included = (bytes_per_pixel > 3);
    tga_string += TGA_HEADER_SIZE;
    size_t bitmap_length = header->width * header->height * bytes_per_pixel;
    char* bitmap = malloc(bitmap_length);
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
                bitmap[i++] = r;
                bitmap[i++] = g;
                bitmap[i++] = b;
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
                bitmap[i++] = r;
                bitmap[i++] = g;
                bitmap[i++] = b;
            }            
        }
    }

    return bitmap;
}

int main(int argc, char** argv)
{
    char* file_path = "african_head_diffuse.tga";

    if (argc > 1) {
        file_path = argv[1];
    } else {
        printf("usage: tgaview image.tga\n");
    }

    size_t str_length;
    char* tga_string = load_file(file_path, &str_length);

    /* Get header */
    Tga_Header header;
    load_header(&header, tga_string);
    logheader(&header);

    /* Open new window */
    int height = header.height;
    int width = header.width;
    gfx_open(width, height, file_path);

    /* Load image into bitmap */
    char* bitmap = tga_to_bitmap(tga_string, str_length, &header);

    while (1) {
        gfx_color(255, 255, 255);

        /* Draw bitmap
           TODO: take x_origin and y_origin into account */
        for (int y = 0; y < header.height; y++) {
            for (int x = 0; x < header.width; x++) {
                gfx_color(
                    bitmap[(x + y*header.width)*3 ],
                    bitmap[(x + y*header.width)*3 + 1],
                    bitmap[(x + y*header.width)*3 + 2]);
                gfx_point(x, height - y);
            }
        }

        char c = gfx_wait();
        if (c == 'q' || c == '\x1b') break;
    }

    free(tga_string);
    free(bitmap);

    return 0;
}
