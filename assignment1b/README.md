# Assignment 1b
This is a program to render a scene containing ellipses and spheres in 3D space.

# Building
`$ make`

# Usage
To run individual files:

`$ ./raytracer1b <path/to/input_file>`

To run all the provided examples:

`$ ./raytracer1b.sh`

# Input File Format
Sample input files are provided in this repo.
- example1.txt

### General Notes
- The RGB scale is from 0-1.
- Horizontal field of view accepts the value v in degrees, not radians.
- Parallel is an optional parameter that accepts frustum width as its value v.

### Header
This section defines the scene's general viewing parameters. The individual values can appear in any order, but this section must be at the top.
```
imsize width height
eye x y z
viewdir x y z
hfov v
updir x y z
bkgcolor R G B
parallel v
```
### Body
This section defines the objects within the scene and their colors. `mtlcolor` must come first followed by the objects that are that color in any order. This section must come last.
```
mtlcolor Dr Dg Db Sr Sg Sb ka kd ks n
sphere x y z r
ellipse x y z rx ry rz

mtlcolor Dr Dg Db Sr Sg Sb ka kd ks n
sphere x y z r
ellipse x y z rx ry rz
```