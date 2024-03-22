# Assignment 1d
This is a program to render a scene containing ellipsoids, spheres, and triangles in 3D space.

# Building
`$ make`

# Usage

#### Grading Criteria:
- At each relevant ray/surface intersection point on a reflective surface, the program accurately
  calculates the direction of reflection R.
  - Handled in [ray.h](ray.h#L401).
    - ```c
      Ray reflectRay(Vector3 intersectionPoint, Vector3 reverseIncidentDirection, Vector3 surfaceNormal) {
          return (Ray) {
              .origin = intersectionPoint,
              .direction = subtract((multiply(surfaceNormal, (2.0f * dot(surfaceNormal, reverseIncidentDirection)))), reverseIncidentDirection)
          };
      }
      ```
- At each relevant ray/surface intersection point on a transparent surface, the program accurately
  calculates the direction of transmission T. In particular, the program always uses the correct direction of
  the surface normal vector N when computing T at a ray/surface intersection point, regardless of whether
  the incident ray is exiting or entering the object. Additionally, the program correctly keeps track of the
  indices of refraction of the materials the incident ray has been traveling through, so that the correct indices
  of refraction are used to calculate the direction of the transmitted ray when that ray is exiting a transparent
  object as well as when entering.
  - Direction calculation handled in [render.h](render.h#L212).
    - ```c
      Ray nextIncident = (Ray) {
          .origin = intersection.intersectionPoint,
          .direction = add(
              multiply(multiply(intersection.surfaceNormal, -1.0f),cosThetaExiting),
              multiply(refractionDirToMultiply, refractionCoefficient)
          )
        };
        ```
  - Surface normal, enter/exit, and indices of refraction calculation handled in [render.h](render.h#L245)
    - ```c
      float currentRefractionIndex = rayState.previousRefractionIndex;
      float nextRefractionIndex = intersection.mtlColor.refractionIndex;
  
      Exclusion newExclusion = rayState.exclusion;
      bool exiting = dot(intersection.surfaceNormal, intersection.incidentDirection) >= 0;
      if (exiting) {
          float tempRefractionIndex = currentRefractionIndex;
          currentRefractionIndex = nextRefractionIndex;
          intersection.surfaceNormal = multiply(intersection.surfaceNormal, -1.0f);
          nextRefractionIndex = tempRefractionIndex;
          newExclusion = (Exclusion) {
              .excludeSphereIdx = -1,
              .excludeEllipsoidIdx = -1,
              .excludeFaceIdx = intersection.exclusion.excludeFaceIdx
          };
      }
      ```
      as well as
    - ```c
      Vector3 transparencyColor = shadeRay(nextIncident, scene, (RayState) {
          .exclusion = rayState.exclusion,
          .shadow = rayState.shadow,
          .reflectionDepth = rayState.reflectionDepth,
          .previousRefractionIndex = currentRefractionIndex
      });
    ```
- At each relevant ray/surface intersection point on a reflective surface, the program recursively
  traces a ray in the reflected direction R and correctly uses Schlick’s method to define an appropriate
  weighting factor to apply to the color returned by the reflected ray, depending on the index of refraction of
  the reflective material and the angle of incidence of the incident ray.
  - Schlick's method calculation handled in [render.h](render.h#L262)
    - ```c
      float F0 = powf(((nextRefractionIndex - currentRefractionIndex) / (nextRefractionIndex + currentRefractionIndex)), 2);
      float Fr = F0 + ((1.0f - F0) * powf(1.0f - dot(multiply(intersection.incidentDirection, -1.0f), intersection.surfaceNormal), 5));
      Reflection reflection = applyReflections(scene, intersection, illumination, rayState, Fr);
      ```
  - Reflection calculation handled in [render.h](render.h#L157)
    - ```c
      reflectionColor = shadeRay(
          reflectRay(
              intersection.intersectionPoint,
                  multiply(intersection.incidentDirection, -1.0f),
                  intersection.surfaceNormal
          ),
          scene,
          (RayState) {
              .exclusion = rayState.exclusion,
              .shadow = rayState.shadow,
              .reflectionDepth = rayState.reflectionDepth + 1,
              .previousRefractionIndex = rayState.previousRefractionIndex
          }
      );
      reflection = multiply(reflectionColor, Fr);
      baseColor = clamp(add(
          multiply(illumination.ambient, 1.0f - Fr),
          multiply(illumination.depthCueingAmbient, 1.0f - Fr)
      ));
      ```
- At each relevant ray/surface intersection point on a transparent surface, the program recursively
  traces a ray in the transmitted direction T as well as in the reflected direction R, and correctly uses
  Schlick’s method to define appropriate weighting factors to apply to the colors returned by the transmitted
  and reflected rays, depending on the index of refraction of material that the incident ray has passed
  through, and the index of refraction of the material that the transmitted ray will be passing through, and
  the angle of incidence of the incident ray. The opacity of the intersected object is also correctly used in
  weighting the contribution of the transmitted ray.
  - Opacity handled in [render.h](render.h#L233). The rest has been described above.
    - ```c
      Vector3 transparency = multiply(attenuatedTransparencyColor, (1.0f - intersectionPointReflectionCoefficient) * (1.0f - intersection.mtlColor.alpha));
      ```
- At each step in the recursion of the reflected and transmitted rays, the traced ray returns an
  intensity that is correctly computed using the Phong illumination model. Specifically, at each step in the
  recursion, the vectors N, L, and H are each correctly defined at the ray/surface intersection point; in
  particular, the vector H is computed as the halfway vector between L and I, where I is the unit vector
  pointing in the opposite direction as the incident ray at the ray/surface intersection point. If non-zero
  weights kd , ks are defined for a transparent material, the diffuse and specular components of the Phong
  illumination are computed using a surface normal direction N that is pointing towards the direction of the
  incoming ray (N × I > 0).
  - H computed in [render.h](render.h#L32). L follows from that. N is described above.
    - ```c
      Vector3 halfwayLightDirection = normalize(add(lightDirection, multiply(intersection.incidentDirection, -1.0f)));
      ```
- The program correctly identifies and appropriately handles total internal reflection.
  - Total internal reflection is handled in [render.h](render.h#L269),
    - ```c
      if (exiting && intersection.mtlColor.alpha >= 1.0f) {
          return reflection.color;
      }
      ```
- The program accounts for the fact that transparent objects do not fully block all incoming light
  by extending the concept of the shadow flag to accommodate fractional values, depending on the
  opacity of any transparent objects encountered between the ray/surface intersection point and the light
  source. Shadows are thus enabled to appear appropriately darker when multiple intervening transparent
  objects are encountered.
  - Shadows handled in [render.h](render.h#L98)
    - ```c
      shadow *= (1.0f - shadowIntersection.mtlColor.alpha);
      ```
  - Soft shadows handled in [render.h](render.h#L84)
    - ```c
      if (centralShadowIntersection.mtlColor.alpha < 1.0f) {
          softShadow *= (1.0f - centralShadowIntersection.mtlColor.alpha);
      }
      shadow = 1.0f - softShadow;
      ```
- The student has provided at least one creative, original scene file along with one or more
  accompanying images to demonstrate the capabilities of their program.
  - Many scenes provided in the `tests/` directory.


#### Extra Credit:
- The program has been extended to model variable visibility attenuation as a ray passes through
    a partially transparent material, so that thicker partially transparent objects block the view of what is
    behind them to a greater extent than thinner partially transparent objects. Specifically, the program
    keeps track of the distance a ray has traveled through a transparent medium and uses that information
    to when computing the colors at the surface boundaries.
  - Visibility attenuation is implemented in [render.h](render.h#L229)
    - ```c
      float attenuationCoefficient = 5;
      float attenuationFactor = expf(-attenuationCoefficient * distanceTraveled);
      Vector3 attenuatedTransparencyColor = multiply(transparencyColor, attenuationFactor);
      Vector3 transparency = multiply(attenuatedTransparencyColor, (1.0f - intersectionPointReflectionCoefficient) * (1.0f - intersection.mtlColor.alpha));
      ```
- The program has been extended to incorporate the use of bounding volumes to reduce the
  number of ray/surface intersection tests required to render a scene.
  - Bound volume hierarchy has been implemented in [render.h](render.h#L278) with the use of `bvhsphere` in your scene description file. If none are included, then the scene will render as normal.
    - ```c
      Intersection bvhIntersection = castBvhRay(ray, scene);
      if (scene->bvhSphereCount > 0 && !intersectionExists(bvhIntersection)) {
          return scene->bkgColor.color;
      }
      ```

#### Notes:
- The program accepts values for both hfov and vfov. If both are provided then hfov will be used.
- The `-s` flag is optional. If included, the scene will render using soft shadows instead of hard shadows. _This will take a lot longer._
- Faces can be given as `f v1 v2 v3` `f v1/vt1 v2/vt2 v3/vt3` or `f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3`.

To run individual files:

`$ ./raytracer1d [-s:soft shadows] <path/to/input_file>`

To run all the provided examples in the `tests/` directory, included all of the samples provided by the TAs:

`$ ./raytracer1d.sh`

# Input File Format
Sample input files are provided in the `tests/` directory this repo.

### General Notes
- Input files must be `.txt` files.
- The RGB scale is from 0-1.
- Field of view accepts the value v in degrees, not radians.
- Parallel is an optional parameter that accepts frustum width as its value v.
- Lights are optional, if none are specified then the scene will render without lighting.
- All entries in the header, including lights, _must come before_ entries in the body.

### Header
This section defines the scene's general viewing parameters. The individual values can appear in any order, but this section must be at the top.
```
imsize w h
eye x y z
viewdir x y z
hfov v
vfov v
updir x y z
bkgcolor x y z η
parallel v
light x y z w i
attlight x y z w i c1 c2 c3
depthcueing dcr dcg dcb αmin αmax distmin distmax
```
### Body
This section defines the objects within the scene and their colors. `mtlcolor` must come first followed by the objects that are that color in any order. This section must come last.
```
bvsphere x y z r

mtlcolor Dr Dg Db Sr Sg Sb ka kd ks n α η
sphere x y z r
ellipse x y z rx ry rz

mtlcolor Dr Dg Db Sr Sg Sb ka kd ks n α η
sphere x y z r
ellipse x y z rx ry rz

mtlcolor Dr Dg Db Sr Sg Sb ka kd ks n α η
v x y z
v x y z
v x y z
...

vt x y
vt x y
vt x y
...

vn x y z
vn x y z
vn x y z
...

texture path/to/texturemap.ppm
bump path/to/normalmap.ppm

f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
...
```
