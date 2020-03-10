#ifndef REND_OBJPARSER_H_
#define REND_OBJPARSER_H_

#include "math.h"

Model* parse_obj_file(char* filename);
void free_model(Model* m);

void flip_vertices_vertically(Model* m);
void flip_texture_vertices_vertically(Model* m);

#endif // REND_OBJPARSER_H_
