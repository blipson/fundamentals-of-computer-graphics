# Assignment 2B

# Building
```
$ cmake .
$ make
```


# Usage
`$ ./HW2b`
- Use WASD to move the eye position around.
- Use the arrow keys to change the viewing direction.
- Use `[` and `]` to move the eye position up or down.
- Resizing the window updates the projection.

# Showcase
### Base assignment

### Extra credit - translating up and down & rotating up and down

# Grading Criteria
- The student has submitted a program that compiles and runs. The program contains at
  least one meaningful change beyond what is done in the template. (5 pts)
  - Not much to say here. Just run it.


- When the program launches, the user is inside of the cathedral model from an initial
  viewing position and initial direction of view that are appropriate for a first-person virtual “walk
  through” of the provided 3D model. Specifically, the user should be facing towards an open area
  and the camera height should approximately match the eye height of a person standing on the
  floor of the model. (The initial eye/camera position and initial viewing direction would typically
  be included as part of the scene description, but for this assignment they can be hard-coded into
  the program.) In the code, it can be seen that the camera parameters (view position and view
  direction) are used to define an appropriate viewing transformation matrix that has the potential
  to later be interactively modified by the user (the updating of this matrix is credited elsewhere).
  (10 pts)
  - The eye position, viewing direction, and up direction are initialized such that the viewer is placed inside the model, facing towards the entrance.
  ```c
  Vec3f eye = Vec3f(-5.0f, -10.0f, 0.0f);
  Vec3f viewDir = Vec3f(-1.0f, 0.0f, 0.0f);
  Vec3f upDir = Vec3f(0.0f, 1.0f, 0.0f);
  ```
  - This function is called on every frame.
  ```c
  void updateViewTransform() {
      Globals::n = normalize(Globals::viewDir * -1.0f);
      Globals::u = normalize(Globals::upDir.cross(Globals::n));
      Globals::v = normalize(Globals::n.cross(Globals::u));
      float dx = -(Globals::eye.dot(Globals::u));
      float dy = -(Globals::eye.dot(Globals::v));
      float dz = -(Globals::eye.dot(Globals::n));
      Globals::view.m[0] = Globals::u[0];
      Globals::view.m[1] = Globals::v[0];
      Globals::view.m[2] = Globals::n[0];
      Globals::view.m[4] = Globals::u[1];
      Globals::view.m[5] = Globals::v[1];
      Globals::view.m[6] = Globals::n[1];
      Globals::view.m[8] = Globals::u[2];
      Globals::view.m[9] = Globals::v[2];
      Globals::view.m[10] = Globals::n[2];
      Globals::view.m[12] = dx;
      Globals::view.m[13] = dy;
      Globals::view.m[14] = dz;
  }
  ```

- When the program launches, the initial view of the cathedral feels reasonably “lifelike”,
  without excessive apparent perspective distortion, and with a visibility of the user’s surroundings
  that is similar to what one would expect if the building model were large with respect to the
  viewer’s eyes. In the code, it can be seen that appropriately-defined parameters have been used
  to construct the perspective projection matrix that is applied to each vertex in the vertex shader.
  There is no excessive clipping of the scene by the near clipping plane. (15 pts)
  - The parameters are initialized to create a reasonable viewing frustum.
  ```c
  #define FRUSTUM_WIDTH 0.5
  #define FRUSTUM_HEIGHT 0.5
  ...
  namespace Globals {
    float near = 0.5f;
    float far = 100.0f;
    float left = -FRUSTUM_WIDTH;
    float right = FRUSTUM_WIDTH;
    float bottom = -FRUSTUM_HEIGHT;
    float top = FRUSTUM_HEIGHT;
    ...
  }
  ```
  - This method is called on initialization of the program to create a perspective projection transform.
  ```c
  static void updateProjectionTransform() {
    Globals::projection.m[0] = 2 * Globals::near / (Globals::right - Globals::left);
    Globals::projection.m[5] = 2 * Globals::near / (Globals::top - Globals::bottom);
    Globals::projection.m[8] = (Globals::right + Globals::left) / (Globals::right - Globals::left);
    Globals::projection.m[9] = (Globals::top + Globals::bottom) / (Globals::top - Globals::bottom);
    Globals::projection.m[10] = -(Globals::far + Globals::near) / (Globals::far - Globals::near);
    Globals::projection.m[11] = -1;
    Globals::projection.m[14] = -2 * Globals::far * Globals::near / (Globals::far - Globals::near);
    Globals::projection.m[15] = 0;
  }
  ```

- The program allows the user to translate the camera location (eye) forward and
  backwards by an appropriate amount within the scene by pressing the ‘w’ and ‘s’ keys.
  Specifically, pressing the ‘w’ key causes the eye position to move forward by about one step
  (~0.75m) in the direction of view, and pressing the ‘s’ key causes the camera to move backwards
  (in the opposite direction to the direction of view) at a similar rate. (15 pts)
  ```c
  case GLFW_KEY_W:
    Globals::eye += Globals::viewDir * speed;
    break;
  case GLFW_KEY_S:
    Globals::eye += Globals::viewDir * -speed;
    break;
  ```
  
- The program allows the user to translate the camera location (eye) left and right by an
  appropriate amount, relative to the direction of view, by pressing the ‘a’ and ‘d’ keys.
  Specifically, pressing the ‘a’ key causes the eye position to sidestep to the left by about one step
  (~0.75m), or slightly less, in a direction that is orthogonal to the current direction of view, and
  pressing the ‘d’ key causes the camera to move to the right in a similar way. (10 pts)
  ```c
  case GLFW_KEY_A:
    Globals::eye += Globals::u * -speed;
    break;
  case GLFW_KEY_D:
    Globals::eye += Globals::u * speed;
    break;
  ```
  
- The program allows the user to spin the direction of view to the left and right around their
  current position, within a plane that is parallel to the groundplane of the building model, by
  pressing the left and right arrow keys. Pressing the left arrow key causes the viewing direction to
  rotate by a small amount to the left, and pressing the right arrow key causes similar changes to
  the right. The camera position does not move as the view direction changes – i.e. the user is able
  to rotate a full 360 ̊ via continuous key presses and end up with the exact same view they started
  with. (20 pts)
  ```c
  static Mat4x4 rotation_y(float angle) {
    Mat4x4 rot;
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);
    rot.m[0] = cosTheta;
    rot.m[2] = -sinTheta;
    rot.m[8] = sinTheta;
    rot.m[10] = cosTheta;
    return rot;
  }  
  ...
  case GLFW_KEY_RIGHT:
      Globals::viewDir = rotation_y(-speed) * Globals::viewDir;
      break;
  case GLFW_KEY_LEFT:
      Globals::viewDir = rotation_y(speed) * Globals::viewDir;
      break;
  ```
  
- The rendered image always fills the entire viewing window and does not appear squashed
  or stretched after the user re-sizes or re-shapes the window. This outcome is achieved by
  adjusting the dimensions of the viewport and the aspect ratio of the viewing frustum
  appropriately in response to a window resizing event. If the window becomes uniformly larger
  or smaller, without a change in aspect ratio, then the same scene contents can remain visible
  within the larger or smaller window. But if the window is reshaped to be wider-than-tall or
  taller-than-wide, the new view should show more of the scene in the elongated dimension than
  was visible before the resizing event. (20 pts)
  ```c
  static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Globals::win_width = float(width);
    Globals::win_height = float(height);
    Globals::aspect = Globals::win_width / Globals::win_height;
    glViewport(0, 0, width, height);

    if (Globals::aspect > 1) {
        float newBoundary = (Globals::aspect * (Globals::top - Globals::bottom)) / 2.0f;
        Globals::left = -newBoundary;
        Globals::right = newBoundary;
    } else {
        float newBoundary = ((Globals::right - Globals::left) / Globals::aspect) / 2.0f;
        Globals::bottom = -newBoundary;
        Globals::top = newBoundary;
    }

    updateProjectionTransform();
  }
  ```
  
- The student has turned in all of their source code, along with any special instructions
  needed to compile and run their program. The code that the student has provided is written in a
  platform-independent manner and is reasonably straightforward to compile and run. (5 pts)
  - Not much to say here. Just run it.

#### Extra Credit:

- The program allows the user to rotate the viewing direction upwards and downwards by
  pressing the up and down arrow keys. Pressing the up arrow key causes the viewing direction to
  rotate by a small amount directly upwards without any tilting to the left or right (e.g. within the
  plane defined by the viewing and ‘up’ directions) regardless of the direction of view, and
  pressing the down arrow key causes the viewing direction to rotate downward in a similar
  manner. There is no shift in the camera position as the user rotates, and the rotation is
  accomplished in such a way that the user is able to rotate a full 360 ̊ (e.g. a somersault) via
  continuous key presses, so as to end up with the exact same view of the scene that they started
  from. In particular, nothing prevents the user from looking straight up or straight down. (10 pts)
  ```c
  case GLFW_KEY_UP:
  {
      Globals::viewDir = rotation(Globals::u, -speed) * Globals::viewDir;
      Globals::upDir = rotation(Globals::u, -speed) * Globals::upDir;
      break;
  }
  case GLFW_KEY_DOWN:
  {
      Globals::viewDir = rotation(Globals::u, speed) * Globals::viewDir;
      Globals::upDir = rotation(Globals::u, speed) * Globals::upDir;
      break;
  }
  ```

- The program allows the user to translate the camera location (eye) by an appropriate
  amount upwards or downwards in the vertical direction with respect to the model (i.e. parallel to
  the world coordinate y axis), by pressing the left and right bracket keys, (‘[‘ and ‘]’) respectively.
  Specifically, pressing the ‘[’ key causes the camera position to move a small amount towards the
  floor of the cathedral, and pressing the ‘]’ key causes the camera position to move a small
  amount towards the ceiling of the cathedral. (2 pts)
  ```c
  case GLFW_KEY_LEFT_BRACKET:
      Globals::eye[1] -= speed;
      break;
  case GLFW_KEY_RIGHT_BRACKET:
      Globals::eye[1] += speed;
      break;
  ```
