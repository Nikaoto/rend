#ifndef REND_OBJPARSER_H_
#define REND_OBJPARSER_H_

#include "math.h"

Model* parse_obj_file(char* filename);
void free_model(Model* m);

#endif // REND_OBJPARSER_H_
