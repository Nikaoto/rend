// Parses .obj files into a struct

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include "rend.h"
#include "math.h"

#define READ_BUF_SIZE 256

char* read_obj_file(char* filename, int* size)
{
    char buf[READ_BUF_SIZE];

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printf("Couldn't open %s\n", filename);
        return 0;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        printf("fstat failed\n");
        return 0;
    }

    size_t file_size = (size_t) st.st_size;
    *size = file_size;
    size_t bytes_read = read(fd, buf, READ_BUF_SIZE);
    if (bytes_read == 0) {
        printf("obj file %s empty\n", filename);
        return 0;
    }
    if (bytes_read < 0) {
        printf("Couldn't read %s\n", filename);
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

int get_vert_coord_len(char *s, int off)
{
    int len = 0;
    while (s[off + len] != ' ' && s[off + len] != '\n')
        len++;
    return len;
}

int get_num_len(char* s, int off, char* delimiters)
{
    int len = 0;
    char* delimp;
    while (len < INT_MAX) {
        delimp = delimiters;
        while (*delimp != '\0'){
            if (s[off + len] == *delimp) {
                return len;
            }
            delimp++;
        }
        len++;
    }
    return -1;
}

int get_next_uint(char* s, int off, int len)
{
    int num = 0;
    for (int i = 0; i < len; i++) {
        if (is_num(s[off + i])) {
            num += ((int) power(10, len - i - 1)) * (s[off + i] - '0');
        }
    }
    return num;
}

double string_to_double(char* s, int off, int len)
{
    // Get sign
    int sign;
    if (s[off] == '-') {
        sign = 1;
        off++;
        len--;
    } else {
        sign = 0;
    }

    // If 'e' used, assign mult and deduct length
    double mult = 1;
    for (int i = 0; i < len; i++) {
        if (s[off + i] == 'e') {
            i++;
            double d = string_to_double(s, off + i, len - i);
            mult = power(10, d);
            len = i - 1;
            break;
        }
    }
    
    // Get whole part length
    int wlen = 0;
    while (s[off + wlen] != '.' && wlen != len) {
        wlen++;
    }

    // Get whole part
    int whole = 0;
    for (int i = 0; i < wlen; i++) {
        if (is_num(s[off + i])) {
            whole += ((int) power(10, wlen - i - 1)) * (s[off + i] - '0');
        }
    }

    // If point not in number
    if (wlen == len) {
        return sign ? -whole * mult : whole * mult;
    }

    // Get fractional part
    double frac = 0.0;
    int flen = len - wlen - 1;
    int foff = off + wlen + 1; // frac part offset
    for (int i = 0; i < flen; i++) {
        if (is_num(s[foff + i])) {
            frac += power(0.1, i + 1) * (s[foff + i] - '0');
        }
    }

    double absolute = mult * (frac + whole);
    return sign ? -absolute : absolute;
}

Model* parse_obj_file(char* filename)
{
    int file_length = 0;
    char* obj_string = read_obj_file(filename, &file_length);
    if (obj_string == NULL) {
        printf("obj_string is null\n");
        return NULL;
    }
    Model* m = malloc(sizeof(Model));
    m->vertex_count = 0;
    m->vertices = malloc(0);
    m->face_count = 0;
    m->faces = malloc(0);

    // Populate vertices and tmp faces
    size_t row = 0;
    size_t col = 0;
    while (row + col < file_length) {
        if (obj_string[row + col] == 'v') {
            col++;
            if (obj_string[row + col] == ' ') {
                col++;

                // Add a vertex
                m->vertex_count++;
                m->vertices = realloc(m->vertices, sizeof(Vec3) * m->vertex_count);
                int vindex = m->vertex_count - 1;
                // X
                int len = get_vert_coord_len(obj_string, col + row);
                m->vertices[vindex].x = string_to_double(obj_string, col + row, len);
                col += len + 1;
                // Y
                len = get_vert_coord_len(obj_string, col + row);
                m->vertices[vindex].y = string_to_double(obj_string, col + row, len);
                col += len + 1;
                // Z
                len = get_vert_coord_len(obj_string, col + row);
                m->vertices[vindex].z = string_to_double(obj_string, col + row, len);
                col += len + 1;
            }
        } else if (obj_string[row + col] == 'f')  {
            col++;
            if (obj_string[row + col] == ' ') {
                col++;

                // Add face
                m->face_count++;
                m->faces = realloc(m->faces, sizeof(size_t) * m->face_count);
                int findex = m->face_count - 1;
                m->faces[findex] = malloc(sizeof(int[3]));
                // Vert1 index
                int len = get_num_len(obj_string, col + row, "/ ");
                m->faces[findex][0] = get_next_uint(obj_string, col + row, len) - 1;
                col += len + 1;
                // Move to next triplet
                col += get_num_len(obj_string, col + row, " ") + 1;
                // Vert2 index
                len = get_num_len(obj_string, col + row, "/ ");
                m->faces[findex][1] = get_next_uint(obj_string, col + row, len) - 1;
                col += len + 1;
                // Move to next triplet
                col += get_num_len(obj_string, col + row, " ") + 1;
                // Vert3 index
                len = get_num_len(obj_string, col + row, "/ ");
                m->faces[findex][2] = get_next_uint(obj_string, col + row, len) - 1;
                col += len + 1;
            }
        } else {
            col++;
        }
    }

/*
    for (int i = 0; i < m->vertex_count; i++) {
       printf("v %g %g %g\n", m->vertices[i].x, m->vertices[i].y, m->vertices[i].z);
     }
*/

/*    for (int i = 0; i < m->face_count; i++) {
        printf("f %i %i %i\n", m->faces[i][0], m->faces[i][1], m->faces[i][2]);
    }
*/
    free(obj_string);
    return m;
}
