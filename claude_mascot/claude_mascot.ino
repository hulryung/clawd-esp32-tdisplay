#include <Arduino_GFX_Library.h>
#include <math.h>
#include "pose.h"

// ---- TTGO T-Display (ST7789V) pin map ----
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS    5
#define TFT_DC   16
#define TFT_RST  23
#define TFT_BL    4
#define BTN_L 35   // input-only (board has external pull-up)
#define BTN_R  0   // BOOT button

Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, GFX_NOT_DEFINED);
Arduino_GFX *panel = new Arduino_ST7789(bus, TFT_RST, 1, true, 135, 240, 52, 40, 53, 40);
Arduino_Canvas *gfx = new Arduino_Canvas(240, 135, panel);

uint16_t BG, CLAY, DIM, POLE, APPLE_RED, LEAF;
uint16_t CONF[5];   // confetti palette

// ---- Claw'd solid body (16 x 11); eyes punched separately ----
#define COLS 16
#define ROWS 11
const char *BODY[ROWS] = {
  "...XXXXXXXXXX...", "...XXXXXXXXXX...",
  "...XXXXXXXXXX...", "...XXXXXXXXXX...",   // eye band (rows 2-3)
  ".XXXXXXXXXXXXXX.", ".XXXXXXXXXXXXXX.",   // side stubs / hands
  "...XXXXXXXXXX...", "...XXXXXXXXXX...", "...XXXXXXXXXX...",
  "................", "................",   // legs in code (rows 9-10)
};

const int SCALE = 10;
const float FEET_Y = (135 - ROWS * SCALE) / 2.0f + ROWS * SCALE;
const float CX = 120.0f;

bool isLeg(int c, int set) {
  if (set == 1) return c == 3 || c == 5 || c == 10 || c == 12;   // walk-alt
  return c == 4 || c == 6 || c == 9 || c == 11;                  // base
}
bool bodyCell(int r, int c, int legSet) {
  if (r <= 8) return BODY[r][c] == 'X';
  if (legSet == 2 && r == 10) return false;     // air: legs retracted
  return isLeg(c, legSet);
}

// sprite bounding box in pixels for the given pose (feet anchored)
void spriteBox(const Pose &p, float &left, float &top, float &cw, float &ch) {
  cw = SCALE * p.sx; ch = SCALE * p.sy;
  left = CX - COLS * cw / 2 + p.ox;
  top  = FEET_Y - ROWS * ch + p.oy;
}

void drawClawd(const Pose &p) {
  float left, top, cw, ch; spriteBox(p, left, top, cw, ch);
  for (int r = 0; r < ROWS; r++) {
    float lf = (float)(ROWS - 1 - r) / (ROWS - 1);
    for (int c = 0; c < COLS; c++) {
      if (!bodyCell(r, c, p.legSet)) continue;
      gfx->fillRect(lroundf(left + c * cw + p.lean * lf), lroundf(top + r * ch),
                    ceilf(cw), ceilf(ch), CLAY);
    }
  }
  if (!p.eyesClosed) {
    float lf = (float)(ROWS - 1 - 2.5f) / (ROWS - 1);
    int ec[2] = {5 + p.eyeDX, 10 + p.eyeDX};
    for (int e = 0; e < 2; e++)
      gfx->fillRect(lroundf(left + ec[e] * cw + p.lean * lf), lroundf(top + 2 * ch),
                    ceilf(cw), ceilf(2 * ch), BG);
  } else {  // closed: thin lids
    float lf = (float)(ROWS - 1 - 3) / (ROWS - 1);
    int ec[2] = {5 + p.eyeDX, 10 + p.eyeDX};
    for (int e = 0; e < 2; e++)
      gfx->fillRect(lroundf(left + ec[e] * cw + p.lean * lf), lroundf(top + 3 * ch - 2),
                    ceilf(cw), 3, BG);
  }
}

static inline float easeIO(float u) { return u * u * (3 - 2 * u); }

// ---------- pose builders (faithful to the official animations) ----------
Pose animWalk(float t) {                 // look -> crouch -> arc jump -> land, ping-pong
  const float P = 3.2f, A = 36.0f;
  float u = fmodf(t, P) / P;
  Pose p;
  if (u < 0.20f) { p.ox = -A; p.oy = 2 * sinf(t * 4); p.eyeDX = (int)lroundf(sinf(t * 3)); }
  else if (u < 0.27f) { float k = (u - 0.20f) / 0.07f; p.ox = -A; p.sy = 1 - 0.25f * k; p.sx = 1 + 0.18f * k; }
  else if (u < 0.45f) { float v = (u - 0.27f) / 0.18f, e = easeIO(v);
                        p.ox = -A + 2 * A * e; p.oy = -48 * sinf(PI * v); p.sy = 1.12f; p.sx = 0.92f; p.lean = 9; p.legSet = 2; }
  else if (u < 0.52f) { float k = 1 - (u - 0.45f) / 0.07f; p.ox = A; p.sy = 1 - 0.25f * k; p.sx = 1 + 0.18f * k; }
  else if (u < 0.70f) { p.ox = A; p.oy = 2 * sinf(t * 4); p.eyeDX = (int)lroundf(sinf(t * 3)); }
  else if (u < 0.77f) { float k = (u - 0.70f) / 0.07f; p.ox = A; p.sy = 1 - 0.25f * k; p.sx = 1 + 0.18f * k; }
  else if (u < 0.95f) { float v = (u - 0.77f) / 0.18f, e = easeIO(v);
                        p.ox = A - 2 * A * e; p.oy = -48 * sinf(PI * v); p.sy = 1.12f; p.sx = 0.92f; p.lean = -9; p.legSet = 2; }
  else { float k = 1 - (u - 0.95f) / 0.05f; p.ox = -A; p.sy = 1 - 0.25f * k; p.sx = 1 + 0.18f * k; }
  return p;
}
Pose animStomp(float t) {                // tilt-stomp left/right (confetti drawn in loop)
  float beat = t * 3.0f; int n = (int)beat; float fb = beat - n;
  int dir = (n & 1) ? 1 : -1;
  Pose p;
  p.lean = dir * 15 * sinf(PI * fb);
  p.oy = -9 * sinf(PI * fb);
  float land = 1 - sinf(PI * fb);        // squashed when foot down
  p.sy = 1 - 0.18f * land; p.sx = 1 + 0.14f * land;
  p.legSet = (n & 1);
  return p;
}
Pose animFlag(float t) {                 // body sways opposite the flag
  Pose p; float s = sinf(t * 3.0f);
  p.ox = -4 * s; p.lean = -10 * s; p.oy = 2 * fabsf(sinf(t * 3));
  return p;
}
Pose animGym(float t) {                  // dumbbell curl effort
  Pose p; float c = 0.5f - 0.5f * cosf(t * 3.2f);   // 0 down .. 1 up
  p.oy = 3 * (1 - c); p.sy = 1 - 0.06f * (1 - c); p.sx = 1 + 0.05f * (1 - c);
  return p;
}
Pose animLook(float t) {
  Pose p; float ph = fmodf(t, 4.0f);
  p.eyeDX = ph < 1 ? 1 : (ph < 2 ? 0 : (ph < 3 ? -1 : 0));
  p.oy = 2 * sinf(t * 2); p.eyesClosed = fmodf(t, 2.6f) < 0.12f;
  return p;
}
Pose animNap(float t) {
  Pose p; p.eyesClosed = true; p.sy = 1 + 0.03f * sinf(t * 1.4f); p.oy = 2 * sinf(t * 1.4f);
  return p;
}
Pose animWave(float t) {                 // waves hello, looking toward the hand
  Pose p; p.oy = 2 * sinf(t * 2.5f); p.lean = 4 * sinf(t * 2.5f); p.eyeDX = 1;
  return p;
}
// shared apple choreography: which side, where, and progress
void applePlan(float t, float &u, int &dir, float &startX, float &appleX) {
  const float P = 3.2f;
  u = fmodf(t, P) / P;
  dir = (((int)(t / P)) & 1) ? -1 : 1;
  appleX = dir * 52; startX = -dir * 40;
}
Pose animApple(float t) {                // scuttles over and eats a dropped apple
  float u, startX, appleX; int dir; applePlan(t, u, dir, startX, appleX);
  Pose p; p.eyeDX = dir;
  if (u < 0.30f) { p.ox = startX; p.oy = 2 * sinf(t * 4); }
  else if (u < 0.62f) { float v = (u - 0.30f) / 0.32f; p.ox = startX + (appleX - startX) * easeIO(v);
                        p.legSet = ((int)(t * 8)) & 1; p.oy = -2 * fabsf(sinf(t * 8)); }
  else if (u < 0.72f) { p.ox = appleX; }
  else { p.ox = appleX; p.oy = -9 * fabsf(sinf((u - 0.72f) * 32)); }   // happy hops
  return p;
}
Pose animSpin(float t) {                 // turns around in place
  Pose p; float c = cosf(t * 3.0f);
  p.sx = fmaxf(0.12f, fabsf(c)); p.eyesClosed = (c < 0); p.oy = 2 * sinf(t * 2);
  return p;
}

const int NUM = 9;
const char *NAMES[NUM] = {"WALK", "STOMP", "FLAG", "GYM", "LOOK", "NAP", "WAVE", "APPLE", "SPIN"};
Pose buildPose(int i, float t) {
  switch (i) { case 0: return animWalk(t); case 1: return animStomp(t); case 2: return animFlag(t);
               case 3: return animGym(t);  case 4: return animLook(t);  case 5: return animNap(t);
               case 6: return animWave(t); case 7: return animApple(t); default: return animSpin(t); }
}

// ---------- props ----------
void drawConfetti(float t) {
  float beat = t * 3.0f; int cur = (int)beat;
  for (int b = cur - 1; b <= cur; b++) {            // last two bursts linger
    if (b < 0) continue;
    float tau = t - b / 3.0f;
    if (tau < 0 || tau > 0.8f) continue;
    for (int i = 0; i < 14; i++) {
      float ang = i * (6.2832f / 14) + b * 0.9f;
      float spd = 70 + (i % 3) * 22;
      int x = (int)(CX + cosf(ang) * spd * tau);
      int y = (int)(26 - sinf(ang) * spd * tau + 150 * tau * tau);
      if (y > 134 || x < 0 || x > 239) continue;
      gfx->fillRect(x, y, 4, 4, CONF[(i + b) % 5]);
    }
  }
}
void drawFlag(float t) {
  // pole rises from the right hand; flag waves
  Pose p = animFlag(t); float left, top, cw, ch; spriteBox(p, left, top, cw, ch);
  int hx = lroundf(left + 14 * cw);          // right stub
  int hy = lroundf(top + 4 * ch);
  int poleTop = hy - 46;
  gfx->fillRect(hx, poleTop, 3, hy - poleTop, POLE);     // pole
  for (int row = 0; row < 14; row++) {                   // waving banner
    float ph = t * 6 - row * 0.45f;
    int len = 30 + (int)(7 * sinf(ph));
    int off = (int)(4 * sinf(ph));
    gfx->drawFastHLine(hx + 3, poleTop + row + off, len, CLAY);
  }
}
void drawDumbbell(float t) {
  Pose p = animGym(t); float left, top, cw, ch; spriteBox(p, left, top, cw, ch);
  float c = 0.5f - 0.5f * cosf(t * 3.2f);
  int cx = lroundf(left + 8 * cw);
  int y = lroundf(top - 6 + (1 - c) * 34);   // up near body top when curled
  gfx->fillRect(cx - 26, y, 8, 16, DIM);     // weights + bar
  gfx->fillRect(cx + 18, y, 8, 16, DIM);
  gfx->fillRect(cx - 18, y + 6, 36, 4, POLE);
}
void drawHand(float t) {                      // waving hand on a swinging arm
  Pose p = animWave(t); float left, top, cw, ch; spriteBox(p, left, top, cw, ch);
  float hx = left + 15 * cw, hy = top + 4 * ch;
  float ang = -0.5f + 0.7f * sinf(t * 5.0f);  // swing from the shoulder
  float L = 34;
  for (float s = 0; s <= 1.0f; s += 0.12f) {  // arm
    float x = hx + L * s * sinf(ang), y = hy - L * s * cosf(ang);
    gfx->fillCircle(lroundf(x), lroundf(y), 4, CLAY);
  }
  float tx = hx + L * sinf(ang), ty = hy - L * cosf(ang);
  gfx->fillCircle(lroundf(tx), lroundf(ty), 7, CLAY);   // hand
}
void drawApple(float t) {
  float u, startX, appleX; int dir; applePlan(t, u, dir, startX, appleX);
  if (u >= 0.70f) return;                      // eaten
  float groundY = FEET_Y - 16;
  float ay = (u < 0.30f) ? (-12 + (groundY + 12) * easeIO(u / 0.30f)) : groundY;  // fall then rest
  int ax = lroundf(CX + appleX);
  gfx->fillCircle(ax, lroundf(ay), 7, APPLE_RED);
  gfx->fillRect(ax - 1, lroundf(ay) - 11, 2, 5, DIM);   // stem
  gfx->fillRect(ax + 1, lroundf(ay) - 11, 4, 3, LEAF);  // leaf
}

int animIdx = 0;
float tAnim = 0, tSwitch = 0;
bool prevL = true, prevR = true;

void setup() {
  Serial.begin(115200); delay(400);
  pinMode(TFT_BL, OUTPUT); digitalWrite(TFT_BL, HIGH);
  pinMode(BTN_L, INPUT); pinMode(BTN_R, INPUT_PULLUP);
  if (!gfx->begin()) Serial.println("canvas begin FAILED");

  BG   = gfx->color565(0x00, 0x00, 0x00);
  CLAY = gfx->color565(0xD3, 0x7A, 0x59);
  DIM  = gfx->color565(0x6B, 0x40, 0x2E);
  POLE = gfx->color565(0xBB, 0xBB, 0xBB);
  CONF[0] = gfx->color565(0xD3, 0x7A, 0x59);  // clay
  CONF[1] = gfx->color565(0xF2, 0xC9, 0x4C);  // yellow
  CONF[2] = gfx->color565(0x5A, 0xC8, 0xC8);  // teal
  CONF[3] = gfx->color565(0xFF, 0xFF, 0xFF);  // white
  CONF[4] = gfx->color565(0xE0, 0x6C, 0x9A);  // pink
  APPLE_RED = gfx->color565(0xD8, 0x36, 0x2A);
  LEAF      = gfx->color565(0x5C, 0xA8, 0x4E);

  Serial.println("clawd official-anim ready");
}

void loop() {
  tAnim += 0.10f; tSwitch += 0.10f;

  bool curL = digitalRead(BTN_L), curR = digitalRead(BTN_R);
  if (prevL && !curL) { animIdx = (animIdx + NUM - 1) % NUM; tAnim = tSwitch = 0; }
  if (prevR && !curR) { animIdx = (animIdx + 1) % NUM;       tAnim = tSwitch = 0; }
  prevL = curL; prevR = curR;
  if (tSwitch > 7.0f) { animIdx = (animIdx + 1) % NUM; tAnim = tSwitch = 0; }

  gfx->fillScreen(BG);
  if (animIdx == 2) drawFlag(tAnim);          // flag behind/around body
  drawClawd(buildPose(animIdx, tAnim));
  if (animIdx == 1) drawConfetti(tAnim);
  if (animIdx == 3) drawDumbbell(tAnim);
  if (animIdx == 6) drawHand(tAnim);
  if (animIdx == 7) drawApple(tAnim);

  gfx->setTextSize(1); gfx->setTextColor(DIM); gfx->setCursor(4, 4);
  gfx->print(NAMES[animIdx]);
  gfx->flush();
}
