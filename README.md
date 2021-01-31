# rend
Software renderer I wrote from complete scratch in C99 based on [https://github.com/ssloy/tinyrenderer](https://github.com/ssloy/tinyrenderer).

The `tgaview/` directory contains a TGA image parser also written from scratch in C99.

![rendering the 3D model with and without wireframe with rend](./rend_preview.gif)

Uses X11, only works on Linux with X11 or Mac OS with Xquartz.

This code has two dependencies, mainly:

* X11, which is called from `gfx.h` to create windows and blit pixels.
* libc, especially `sqrt` from the math library. Will be replaced with own implementation.

## Building
```
git clone
cd rend
make
```

## Usage
`./rend path_to_obj_file.obj path_to_texture.tga [f [f]]`

Set the third argument to `f` if you wish to flip the vertices of the obj file vertically.

Set the fourth argument to `f` to flip the texture vertically.

Namely, run
```
./rend african_head.obj african_head_diffuse.tga f f
```
to see it render the test model.

## License
All source code is GPL2 unless it says otherwise in the comments.

`african_head.obj`, `diablo_pose.obj`, and `african_head_diffuse.tga` are taken from https://github.com/ssloy/tinyrenderer.
