#ifndef REND_H_
#define REND_H_

#define logf(f) printf(#f " = %g\n", f);
#define logi(i) printf(#i " = %i\n", i);
#define logvec(v) printf(#v " = (%g, %g, %g)\n", v.x, v.y, v.z);
#define logvec2i(v) printf(#v " = (%i, %i)\n", v.x, v.y);

#define WIREFRAME     0
#define RENDER_MODEL  1
#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 1024

#endif // REND_H_
