# Assignment 1c
This is a program to render a scene containing ellipsoids and spheres in 3D space.

# Building
`$ make`

# Usage
#### Notes:

- The `-s` flag is optional. If included, the scene will render using soft shadows instead of hard shadows. _This will take a lot longer._

To run individual files:

`$ ./raytracer1c [-s:soft shadows] <path/to/input_file>`

To run all the provided examples in the `tests/` directory:

`$ ./raytracer1c.sh`

# Input File Format
Sample input files are provided in the `tests/` directory this repo.

### General Notes
- Input files must be `.txt` files.
- The RGB scale is from 0-1.
- Horizontal field of view accepts the value v in degrees, not radians.
- Parallel is an optional parameter that accepts frustum width as its value v.
- Lights are optional, if none are specified then the scene will render without lighting.
- All entries in the header, including lights, _must come before_ entries in the body.

### Header
This section defines the scene's general viewing parameters. The individual values can appear in any order, but this section must be at the top.
```
imsize width height
eye x y z
viewdir x y z
hfov v
updir x y z
bkgcolor x y z
parallel v
light x y z w i
attlight x y z w i c1 c2 c3
depthcueing dcr dcg dcb αmin αmax distmin distmax
```
### Body
This section defines the objects within the scene and their colors. `mtlcolor` must come first followed by the objects that are that color in any order. If you'd like you can also add textures or normal maps via the `texture` and `bump` keywords. This section must come last. Spheres and ellipsoids are defines with center position and radius size, while more complex shapes are defined with a list of vertices, vertex normals, and faces that map to them, similar to the .obj file format.
```
mtlcolor Dr Dg Db Sr Sg Sb ka kd ks n α η
sphere x y z r
ellipse x y z rx ry rz

mtlcolor Dr Dg Db Sr Sg Sb ka kd ks n α η
texture mytexture.ppm
bump mynormalmap.ppm
sphere x y z r
ellipse x y z rx ry rz
v -1 1 -4
v -1 -1 -4
v 1 -1 -4
v 1 1 -4

vn -1 1 1
vn -1 -1 1
vn 1 -1 1
vn 1 1 1

vt 0 0
vt 0 1
vt 1 1
vt 1 0

f 1/1/1 2/2/2 3/3/3
f 1/1/1 3/3/3 4/4/4
...
```
