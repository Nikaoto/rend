#ifndef REND_H_
#define REND_H_

#define logf(f) printf(#f " = %g\n", f);
#define logi(i) printf(#i " = %i\n", i);
#define logvec(v) printf(#v " = (%g, %g, %g)\n", v.x, v.y, v.z);
#define logvec2i(v) printf(#v " = (%i, %i)\n", v.x, v.y);

#endif // REND_H_
