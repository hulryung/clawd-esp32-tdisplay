#pragma once

// Transform applied to the Claw'd sprite for a given animation frame.
struct Pose {
  float ox = 0, oy = 0;     // whole-sprite pixel offset
  float sx = 1, sy = 1;     // squash / stretch
  float lean = 0;           // top horizontal lean (px)
  int   eyeDX = 0;          // eye column shift (cells)
  bool  eyesClosed = false;
  int   legSet = 0;
};
