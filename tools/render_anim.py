#!/usr/bin/env python3
"""Render Claw'd animations to GIFs + key-frame PNGs.

This is a faithful 1:1 port of the sprite/pose/prop math in
claude_mascot/claude_mascot.ino, so the docs match what runs on the device.
"""
import math, os
from PIL import Image, ImageDraw, ImageFont

# ---- palette (8-bit RGB; device stores these as RGB565) ----
BG        = (0x00, 0x00, 0x00)
CLAY      = (0xD3, 0x7A, 0x59)
DIM       = (0x6B, 0x40, 0x2E)
POLE      = (0xBB, 0xBB, 0xBB)
APPLE_RED = (0xD8, 0x36, 0x2A)
LEAF      = (0x5C, 0xA8, 0x4E)
CONF = [(0xD3,0x7A,0x59),(0xF2,0xC9,0x4C),(0x5A,0xC8,0xC8),(0xFF,0xFF,0xFF),(0xE0,0x6C,0x9A)]

# ---- sprite ----
BODY = [
  "...XXXXXXXXXX...", "...XXXXXXXXXX...",
  "...XXXXXXXXXX...", "...XXXXXXXXXX...",
  ".XXXXXXXXXXXXXX.", ".XXXXXXXXXXXXXX.",
  "...XXXXXXXXXX...", "...XXXXXXXXXX...", "...XXXXXXXXXX...",
  "................", "................",
]
COLS, ROWS, SCALE = 16, 11, 10
FEET_Y = (135 - ROWS * SCALE) / 2.0 + ROWS * SCALE
CX = 120.0
W, H, S = 240, 135, 2          # device size and upscale factor

def lr(v):                      # lroundf: round half away from zero
    return int(math.floor(v + 0.5)) if v >= 0 else -int(math.floor(-v + 0.5))
def ceil(v): return int(math.ceil(v))
def easeIO(u): return u * u * (3 - 2 * u)

def isLeg(c, s):
    if s == 1: return c in (3, 5, 10, 12)
    return c in (4, 6, 9, 11)
def bodyCell(r, c, legSet):
    if r <= 8: return BODY[r][c] == 'X'
    if legSet == 2 and r == 10: return False
    return isLeg(c, legSet)

class Pose:
    def __init__(s): s.ox=s.oy=0.0; s.sx=s.sy=1.0; s.lean=0.0; s.eyeDX=0; s.eyesClosed=False; s.legSet=0

def spriteBox(p):
    cw, ch = SCALE * p.sx, SCALE * p.sy
    left = CX - COLS * cw / 2 + p.ox
    top  = FEET_Y - ROWS * ch + p.oy
    return left, top, cw, ch

def rect(d, x, y, w, h, col):
    d.rectangle([x*S, y*S, (x+w)*S-1, (y+h)*S-1], fill=col)
def disc(d, cx, cy, r, col):
    d.ellipse([(cx-r)*S, (cy-r)*S, (cx+r+1)*S-1, (cy+r+1)*S-1], fill=col)

def draw_clawd(d, p):
    left, top, cw, ch = spriteBox(p)
    for r in range(ROWS):
        lf = (ROWS - 1 - r) / (ROWS - 1)
        for c in range(COLS):
            if not bodyCell(r, c, p.legSet): continue
            rect(d, lr(left + c*cw + p.lean*lf), lr(top + r*ch), ceil(cw), ceil(ch), CLAY)
    if not p.eyesClosed:
        lf = (ROWS - 1 - 2.5) / (ROWS - 1)
        for ec in (5 + p.eyeDX, 10 + p.eyeDX):
            rect(d, lr(left + ec*cw + p.lean*lf), lr(top + 2*ch), ceil(cw), ceil(2*ch), BG)
    else:
        lf = (ROWS - 1 - 3) / (ROWS - 1)
        for ec in (5 + p.eyeDX, 10 + p.eyeDX):
            d.rectangle([(lr(left+ec*cw+p.lean*lf))*S, (lr(top+3*ch-2))*S,
                         (lr(left+ec*cw+p.lean*lf)+ceil(cw))*S-1, (lr(top+3*ch-2)+3)*S-1], fill=BG)

# ---- pose builders (ported) ----
def animWalk(t):
    P, A = 3.2, 36.0; u = math.fmod(t, P) / P; p = Pose()
    if u < 0.20: p.ox=-A; p.oy=2*math.sin(t*4); p.eyeDX=lr(math.sin(t*3))
    elif u < 0.27: k=(u-0.20)/0.07; p.ox=-A; p.sy=1-0.25*k; p.sx=1+0.18*k
    elif u < 0.45: v=(u-0.27)/0.18; e=easeIO(v); p.ox=-A+2*A*e; p.oy=-48*math.sin(math.pi*v); p.sy=1.12; p.sx=0.92; p.lean=9; p.legSet=2
    elif u < 0.52: k=1-(u-0.45)/0.07; p.ox=A; p.sy=1-0.25*k; p.sx=1+0.18*k
    elif u < 0.70: p.ox=A; p.oy=2*math.sin(t*4); p.eyeDX=lr(math.sin(t*3))
    elif u < 0.77: k=(u-0.70)/0.07; p.ox=A; p.sy=1-0.25*k; p.sx=1+0.18*k
    elif u < 0.95: v=(u-0.77)/0.18; e=easeIO(v); p.ox=A-2*A*e; p.oy=-48*math.sin(math.pi*v); p.sy=1.12; p.sx=0.92; p.lean=-9; p.legSet=2
    else: k=1-(u-0.95)/0.05; p.ox=-A; p.sy=1-0.25*k; p.sx=1+0.18*k
    return p
def animStomp(t):
    beat=t*3.0; n=int(beat); fb=beat-n; dirn=1 if (n&1) else -1; p=Pose()
    p.lean=dirn*15*math.sin(math.pi*fb); p.oy=-9*math.sin(math.pi*fb)
    land=1-math.sin(math.pi*fb); p.sy=1-0.18*land; p.sx=1+0.14*land; p.legSet=(n&1)
    return p
def animFlag(t):
    p=Pose(); s=math.sin(t*3.0); p.ox=-4*s; p.lean=-10*s; p.oy=2*abs(math.sin(t*3)); return p
def animGym(t):
    p=Pose(); c=0.5-0.5*math.cos(t*3.2); p.oy=3*(1-c); p.sy=1-0.06*(1-c); p.sx=1+0.05*(1-c); return p
def animLook(t):
    p=Pose(); ph=math.fmod(t,4.0)
    p.eyeDX=1 if ph<1 else (0 if ph<2 else (-1 if ph<3 else 0))
    p.oy=2*math.sin(t*2); p.eyesClosed=math.fmod(t,2.6)<0.12; return p
def animNap(t):
    p=Pose(); p.eyesClosed=True; p.sy=1+0.03*math.sin(t*1.4); p.oy=2*math.sin(t*1.4); return p
def animWave(t):
    p=Pose(); p.oy=2*math.sin(t*2.5); p.lean=4*math.sin(t*2.5); p.eyeDX=1; return p
def applePlan(t):
    P=3.2; u=math.fmod(t,P)/P; dirn=-1 if (int(t/P)&1) else 1; return u, dirn, -dirn*40, dirn*52
def animApple(t):
    u,dirn,startX,appleX=applePlan(t); p=Pose(); p.eyeDX=dirn
    if u<0.30: p.ox=startX; p.oy=2*math.sin(t*4)
    elif u<0.62: v=(u-0.30)/0.32; p.ox=startX+(appleX-startX)*easeIO(v); p.legSet=int(t*8)&1; p.oy=-2*abs(math.sin(t*8))
    elif u<0.72: p.ox=appleX
    else: p.ox=appleX; p.oy=-9*abs(math.sin((u-0.72)*32))
    return p
def animSpin(t):
    p=Pose(); c=math.cos(t*3.0); p.sx=max(0.12,abs(c)); p.eyesClosed=(c<0); p.oy=2*math.sin(t*2); return p

# ---- props ----
def drawConfetti(d, t):
    beat=t*3.0; cur=int(beat)
    for b in (cur-1, cur):
        if b < 0: continue
        tau=t-b/3.0
        if tau<0 or tau>0.8: continue
        for i in range(14):
            ang=i*(2*math.pi/14)+b*0.9; spd=70+(i%3)*22
            x=int(CX+math.cos(ang)*spd*tau); y=int(26-math.sin(ang)*spd*tau+150*tau*tau)
            if y>134 or x<0 or x>239: continue
            rect(d, x, y, 4, 4, CONF[(i+b)%5])
def drawFlag(d, t):
    p=animFlag(t); left,top,cw,ch=spriteBox(p)
    hx=lr(left+14*cw); hy=lr(top+4*ch); poleTop=hy-46
    rect(d, hx, poleTop, 3, hy-poleTop, POLE)
    for row in range(14):
        ph=t*6-row*0.45; length=30+int(7*math.sin(ph)); off=int(4*math.sin(ph))
        rect(d, hx+3, poleTop+row+off, length, 1, CLAY)
def drawDumbbell(d, t):
    p=animGym(t); left,top,cw,ch=spriteBox(p); c=0.5-0.5*math.cos(t*3.2)
    cx=lr(left+8*cw); y=lr(top-6+(1-c)*34)
    rect(d, cx-26, y, 8, 16, DIM); rect(d, cx+18, y, 8, 16, DIM); rect(d, cx-18, y+6, 36, 4, POLE)
def drawHand(d, t):
    p=animWave(t); left,top,cw,ch=spriteBox(p)
    hx=left+15*cw; hy=top+4*ch; ang=-0.5+0.7*math.sin(t*5.0); L=34
    s=0.0
    while s<=1.0:
        disc(d, lr(hx+L*s*math.sin(ang)), lr(hy-L*s*math.cos(ang)), 4, CLAY); s+=0.12
    disc(d, lr(hx+L*math.sin(ang)), lr(hy-L*math.cos(ang)), 7, CLAY)
def drawApple(d, t):
    u,dirn,startX,appleX=applePlan(t)
    if u>=0.70: return
    groundY=FEET_Y-16
    ay=(-12+(groundY+12)*easeIO(u/0.30)) if u<0.30 else groundY
    ax=lr(CX+appleX)
    disc(d, ax, lr(ay), 7, APPLE_RED); rect(d, ax-1, lr(ay)-11, 2, 5, DIM); rect(d, ax+1, lr(ay)-11, 4, 3, LEAF)

# ---- animation table: (name, frames, pose_fn, prop_fn) ----
ANIMS = [
    ("WALK", 32, animWalk, None),
    ("STOMP",20, animStomp, drawConfetti),
    ("FLAG", 21, animFlag, drawFlag),
    ("GYM",  20, animGym, drawDumbbell),
    ("LOOK", 40, animLook, None),
    ("NAP",  45, animNap, None),
    ("WAVE", 25, animWave, drawHand),
    ("APPLE",32, animApple, drawApple),
    ("SPIN", 21, animSpin, None),
]

FONT = ImageFont.load_default(size=8*S)
OUT = os.path.join(os.path.dirname(__file__), "..", "assets")
os.makedirs(os.path.join(OUT, "keys"), exist_ok=True)

def render_frame(name, pose_fn, prop_fn, t):
    img = Image.new("RGB", (W*S, H*S), BG); d = ImageDraw.Draw(img)
    if prop_fn is drawFlag: prop_fn(d, t)             # flag sits behind body
    draw_clawd(d, pose_fn(t))
    if prop_fn and prop_fn is not drawFlag: prop_fn(d, t)
    d.text((4*S, 3*S), name, fill=DIM, font=FONT)
    return img

for name, n, pose_fn, prop_fn in ANIMS:
    frames = [render_frame(name, pose_fn, prop_fn, i*0.10) for i in range(n)]
    gif = os.path.join(OUT, f"{name.lower()}.gif")
    frames[0].save(gif, save_all=True, append_images=frames[1:], duration=100, loop=0,
                   optimize=True, disposal=2)
    for k, idx in enumerate([0, n//4, n//2, (3*n)//4]):
        frames[idx].save(os.path.join(OUT, "keys", f"{name.lower()}_k{k}.png"))
    print(f"{name}: {n} frames -> {os.path.getsize(gif)//1024} KB")
print("done")
