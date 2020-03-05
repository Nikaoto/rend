#ifndef TGA_PARSER_H_
#define TGA_PARSER_H_

typedef unsigned char uchar;

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

typedef struct Bitmap {
    char* data;
    int width;
    int height;
} Bitmap;

/* Returns RGB bitmap */
Bitmap* parse_tga_file(char* file_path);

#endif /* TGA_PARSER_H_ */
