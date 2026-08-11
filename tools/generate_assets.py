from PIL import Image, ImageDraw, ImageFont, ImageFilter
import random, math, os, wave, struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
AS = ROOT / 'assets'
random.seed(42)

# ---------- helpers ----------
def lerp(a,b,t): return int(a+(b-a)*t)
def gradient(size, top, bottom):
    w,h=size
    im=Image.new('RGB', size)
    p=im.load()
    for y in range(h):
        t=y/max(1,h-1)
        c=tuple(lerp(top[i],bottom[i],t) for i in range(3))
        for x in range(w): p[x,y]=c
    return im

def add_noise(im, amount=10, density=0.12):
    px=im.load(); w,h=im.size
    for _ in range(int(w*h*density)):
        x=random.randrange(w); y=random.randrange(h)
        r,g,b=px[x,y][:3]
        d=random.randint(-amount,amount)
        px[x,y]=(max(0,min(255,r+d)),max(0,min(255,g+d)),max(0,min(255,b+d)))

def glow_layer(size, center, radius, color, alpha=180):
    layer=Image.new('RGBA',size,(0,0,0,0)); d=ImageDraw.Draw(layer)
    cx,cy=center
    for r in range(radius,0,-3):
        a=int(alpha*(1-r/radius)**1.8)
        d.ellipse((cx-r,cy-r,cx+r,cy+r),fill=(*color,a))
    return layer.filter(ImageFilter.GaussianBlur(radius/7))

def aa_canvas(size, scale=3, bg=(0,0,0,0)):
    return Image.new('RGBA',(size[0]*scale,size[1]*scale),bg),scale

def downsample(im,size):
    return im.resize(size,Image.Resampling.LANCZOS)

# ---------- map backgrounds ----------
MAPS = {
    'verdant': {
        'base': ((28,58,45),(16,38,33)),
        'road': (103,92,73), 'edge': (55,48,39), 'accent': (92,196,139),
        'path': [(0,145),(180,145),(180,330),(500,330),(500,160),(780,160),(780,560),(1060,560),(1060,760),(1200,760)]
    },
    'frost': {
        'base': ((63,83,102),(27,44,61)),
        'road': (122,133,143), 'edge': (60,73,84), 'accent': (110,221,255),
        'path': [(0,690),(210,690),(210,480),(420,480),(420,720),(690,720),(690,400),(920,400),(920,190),(1200,190)]
    },
    'ember': {
        'base': ((72,43,36),(34,25,28)),
        'road': (93,75,69), 'edge': (43,32,31), 'accent': (255,130,70),
        'path': [(0,240),(230,240),(230,600),(430,600),(430,400),(690,400),(690,190),(900,190),(900,690),(1200,690)]
    }
}

def draw_map(name,cfg):
    W,H=1200,900
    im=gradient((W,H),*cfg['base'])
    add_noise(im,12,0.08)
    d=ImageDraw.Draw(im,'RGBA')
    # subtle hex/grid-ish ground markings
    for y in range(0,H,55):
        off=25 if (y//55)%2 else 0
        for x in range(-60+off,W,90):
            d.line((x,y,x+45,y+26,x+90,y),fill=(255,255,255,8),width=1)
    # decorations
    if name=='verdant':
        for _ in range(110):
            x=random.randint(20,W-20); y=random.randint(20,H-20)
            r=random.randint(3,10)
            d.ellipse((x-r,y-r,x+r,y+r),fill=(18,78+random.randint(0,28),54,120))
            if random.random()<.25:
                d.ellipse((x-r//2,y-r*2,x+r//2,y),fill=(70,130,80,90))
    elif name=='frost':
        for _ in range(170):
            x=random.randint(0,W); y=random.randint(0,H); r=random.randint(1,4)
            d.ellipse((x-r,y-r,x+r,y+r),fill=(210,240,255,random.randint(20,80)))
        for _ in range(24):
            x=random.randint(20,W-20); y=random.randint(20,H-20); r=random.randint(10,26)
            d.polygon([(x-r,y+r//2),(x,y-r),(x+r,y+r//2)],fill=(76,96,118,150))
            d.polygon([(x-r//2,y+r//4),(x,y-r+4),(x+r//2,y+r//4)],fill=(180,210,225,70))
    else:
        for _ in range(48):
            x=random.randint(20,W-20); y=random.randint(20,H-20); r=random.randint(7,20)
            d.ellipse((x-r,y-r//2,x+r,y+r//2),fill=(33,28,30,180))
        # lava cracks
        for _ in range(18):
            x=random.randint(0,W); y=random.randint(0,H)
            pts=[(x,y)]
            for k in range(random.randint(3,6)):
                x+=random.randint(-25,25); y+=random.randint(15,40); pts.append((x,y))
            d.line(pts,fill=(255,82,36,65),width=3)
            d.line(pts,fill=(255,166,76,95),width=1)
    # road shadow, border, body
    pts=cfg['path']
    d.line(pts,fill=(0,0,0,80),width=116,joint='curve')
    d.line(pts,fill=(*cfg['edge'],255),width=102,joint='curve')
    d.line(pts,fill=(*cfg['road'],255),width=84,joint='curve')
    # road texture + center marking
    for i in range(len(pts)-1):
        x1,y1=pts[i]; x2,y2=pts[i+1]
        dx,dy=x2-x1,y2-y1; length=max(1,math.hypot(dx,dy)); steps=int(length//24)
        nx,ny=-dy/length,dx/length
        for s in range(steps):
            t=(s+.5)/max(1,steps); cx=x1+dx*t; cy=y1+dy*t
            if s%2==0:
                d.line((cx-nx*6,cy-ny*6,cx+nx*6,cy+ny*6),fill=(235,220,185,70),width=2)
        for _ in range(max(1,int(length/50))):
            t=random.random(); cx=x1+dx*t+nx*random.uniform(-26,26); cy=y1+dy*t+ny*random.uniform(-26,26)
            r=random.randint(1,3); d.ellipse((cx-r,cy-r,cx+r,cy+r),fill=(40,35,32,70))
    # spawn portal and base terminal
    sx,sy=pts[0]; ex,ey=pts[-1]
    d.ellipse((sx-42,sy-42,sx+42,sy+42),outline=(*cfg['accent'],180),width=7)
    d.ellipse((sx-25,sy-25,sx+25,sy+25),fill=(*cfg['accent'],35),outline=(*cfg['accent'],210),width=3)
    d.rounded_rectangle((ex-43,ey-43,ex+43,ey+43),12,fill=(20,27,33,220),outline=(*cfg['accent'],210),width=5)
    d.rectangle((ex-10,ey-26,ex+10,ey+26),fill=(*cfg['accent'],120))
    # vignette
    vig=Image.new('L',(W,H),0); vd=ImageDraw.Draw(vig)
    vd.ellipse((-W*.18,-H*.18,W*1.18,H*1.18),fill=210)
    vig=vig.filter(ImageFilter.GaussianBlur(120))
    dark=Image.new('RGBA',(W,H),(0,0,0,115)); dark.putalpha(Image.eval(vig,lambda x:255-x))
    out=Image.alpha_composite(im.convert('RGBA'),dark)
    out.save(AS/'maps'/f'{name}.png')

for name,cfg in MAPS.items(): draw_map(name,cfg)

# ---------- menu background ----------
W,H=1600,900
im=gradient((W,H),(9,19,31),(3,8,16)).convert('RGBA')
d=ImageDraw.Draw(im,'RGBA')
for x in range(0,W,64): d.line((x,0,x,H),fill=(80,165,220,14),width=1)
for y in range(0,H,64): d.line((0,y,W,y),fill=(80,165,220,14),width=1)
for _ in range(140):
    x=random.randint(0,W); y=random.randint(0,H); r=random.randint(1,3)
    d.ellipse((x-r,y-r,x+r,y+r),fill=(110,205,255,random.randint(25,80)))
# horizon forms
for i in range(18):
    x=i*100-80; peak=random.randint(440,590); ww=random.randint(170,300)
    d.polygon([(x,H),(x+ww//2,peak),(x+ww,H)],fill=(15,32,48,180))
im=Image.alpha_composite(im,glow_layer((W,H),(1180,220),360,(38,154,255),110))
im.save(AS/'ui'/'menu_background.png')

# logo using available font (raster only)
logo=Image.new('RGBA',(900,240),(0,0,0,0)); ld=ImageDraw.Draw(logo)
font_candidates = [
    AS / 'fonts' / 'NotoSans-Bold.ttf',
    Path('/usr/share/fonts/google-noto/NotoSans-Bold.ttf'),
    Path('/usr/share/fonts/dejavu-sans-fonts/DejaVuSans-Bold.ttf'),
    Path('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'),
]
font_path = next((path for path in font_candidates if path.exists()), None)
font = ImageFont.truetype(str(font_path), 86) if font_path else ImageFont.load_default()
sub = ImageFont.truetype(str(font_path), 28) if font_path else ImageFont.load_default()
for off,a in [(8,30),(4,80),(0,255)]:
    ld.text((450+off,70+off),'AEGIS DOMINION',font=font,anchor='mm',fill=(91,210,255,a),stroke_width=2,stroke_fill=(2,9,18,a))
ld.text((450,157),'TACTICAL TOWER DEFENSE',font=sub,anchor='mm',fill=(180,221,240,220))
logo.save(AS/'ui'/'logo.png')

# ---------- tower sprites ----------
TOWERS={
'pulse': ((42,111,146),(89,225,255)),
'cannon':((93,83,73),(255,184,82)),
'frost': ((57,94,130),(132,238,255)),
'sniper':((73,66,94),(214,142,255)),
'tesla': ((48,88,85),(105,255,203)),
'missile':((102,66,57),(255,113,81)),
}

def tower_base(name,base,accent):
    S=128; im,sc=aa_canvas((S,S),3); d=ImageDraw.Draw(im,'RGBA'); c=lambda v:int(v*sc)
    # shadow
    d.ellipse((c(22),c(78),c(106),c(112)),fill=(0,0,0,90))
    # foundation rings
    d.ellipse((c(18),c(32),c(110),c(112)),fill=(*base,255),outline=(20,28,34,255),width=c(3))
    d.ellipse((c(30),c(43),c(98),c(101)),fill=(23,34,42,255),outline=(*accent,170),width=c(3))
    d.ellipse((c(42),c(54),c(86),c(90)),fill=(*base,255))
    # bolts
    for a in range(0,360,45):
        x=64+math.cos(math.radians(a))*34; y=72+math.sin(math.radians(a))*27
        d.ellipse((c(x-3),c(y-3),c(x+3),c(y+3)),fill=(*accent,200))
    return downsample(im,(S,S))

def tower_turret(name,base,accent):
    S=128; im,sc=aa_canvas((S,S),3); d=ImageDraw.Draw(im,'RGBA'); c=lambda v:int(v*sc)
    # oriented up
    if name=='pulse':
        d.rounded_rectangle((c(48),c(32),c(80),c(82)),c(10),fill=(*base,255),outline=(16,24,31,255),width=c(3))
        d.rounded_rectangle((c(57),c(8),c(71),c(47)),c(6),fill=(*accent,240))
        d.ellipse((c(51),c(51),c(77),c(77)),fill=(20,31,39,255),outline=(*accent,230),width=c(3))
    elif name=='cannon':
        d.ellipse((c(40),c(43),c(88),c(91)),fill=(*base,255),outline=(20,24,29,255),width=c(4))
        d.rounded_rectangle((c(53),c(7),c(75),c(60)),c(7),fill=(72,67,61,255),outline=(*accent,210),width=c(3))
        d.rectangle((c(48),c(5),c(80),c(17)),fill=(35,38,41,255))
    elif name=='frost':
        d.polygon([(c(64),c(10)),(c(82),c(45)),(c(75),c(86)),(c(53),c(86)),(c(46),c(45))],fill=(*base,255),outline=(*accent,230))
        d.polygon([(c(64),c(14)),(c(74),c(48)),(c(64),c(71)),(c(54),c(48))],fill=(*accent,190))
    elif name=='sniper':
        d.ellipse((c(43),c(48),c(85),c(90)),fill=(*base,255),outline=(21,23,31,255),width=c(3))
        d.rounded_rectangle((c(58),c(2),c(70),c(62)),c(5),fill=(31,34,44,255),outline=(*accent,240),width=c(3))
        d.rectangle((c(52),c(16),c(76),c(27)),fill=(*accent,180))
    elif name=='tesla':
        d.ellipse((c(43),c(48),c(85),c(91)),fill=(*base,255),outline=(18,26,30,255),width=c(3))
        d.line((c(50),c(55),c(43),c(24)),fill=(*accent,230),width=c(5)); d.line((c(78),c(55),c(85),c(24)),fill=(*accent,230),width=c(5))
        d.ellipse((c(36),c(14),c(50),c(28)),fill=(*accent,240)); d.ellipse((c(78),c(14),c(92),c(28)),fill=(*accent,240))
        d.ellipse((c(55),c(42),c(73),c(60)),fill=(220,255,245,245))
    else: # missile
        d.ellipse((c(42),c(48),c(86),c(92)),fill=(*base,255),outline=(22,25,29,255),width=c(3))
        for x in [49,67]:
            d.rounded_rectangle((c(x),c(10),c(x+12),c(60)),c(5),fill=(74,76,78,255),outline=(*accent,210),width=c(2))
            d.polygon([(c(x),c(10)),(c(x+6),c(1)),(c(x+12),c(10))],fill=(*accent,235))
    return downsample(im,(S,S))

for n,(b,a) in TOWERS.items():
    tower_base(n,b,a).save(AS/'towers'/f'{n}_base.png')
    tower_turret(n,b,a).save(AS/'towers'/f'{n}_turret.png')

# ---------- enemy sprites ----------
ENEMIES={
'raider':((95,107,120),(242,88,88)),
'runner':((64,109,93),(255,205,86)),
'tank':((74,72,73),(255,130,78)),
'shield':((64,83,125),(108,189,255)),
'regen':((66,107,72),(113,255,154)),
'splitter':((95,67,110),(222,120,255)),
'boss':((82,59,66),(255,75,94)),
}

def enemy_sprite(name,body,accent):
    S=96; im,sc=aa_canvas((S,S),4); d=ImageDraw.Draw(im,'RGBA'); c=lambda v:int(v*sc)
    d.ellipse((c(14),c(60),c(82),c(86)),fill=(0,0,0,90))
    if name=='runner':
        d.polygon([(c(48),c(7)),(c(78),c(51)),(c(62),c(82)),(c(48),c(70)),(c(34),c(82)),(c(18),c(51))],fill=(*body,255),outline=(20,25,28,255))
    elif name=='tank':
        d.rounded_rectangle((c(18),c(18),c(78),c(80)),c(14),fill=(*body,255),outline=(18,22,26,255),width=c(4))
        d.rectangle((c(28),c(24),c(68),c(38)),fill=(36,38,39,255)); d.rectangle((c(43),c(4),c(53),c(33)),fill=(*accent,220))
    elif name=='shield':
        d.ellipse((c(20),c(17),c(76),c(81)),fill=(*body,255),outline=(20,24,32,255),width=c(4))
        d.arc((c(8),c(5),c(88),c(90)),190,350,fill=(*accent,235),width=c(5))
        d.arc((c(8),c(5),c(88),c(90)),10,170,fill=(*accent,120),width=c(2))
    elif name=='regen':
        d.ellipse((c(22),c(18),c(74),c(80)),fill=(*body,255),outline=(17,26,21,255),width=c(4))
        d.line((c(48),c(25),c(48),c(68)),fill=(*accent,240),width=c(7)); d.line((c(33),c(46),c(63),c(46)),fill=(*accent,240),width=c(7))
    elif name=='splitter':
        d.polygon([(c(48),c(7)),(c(79),c(30)),(c(68),c(77)),(c(48),c(84)),(c(28),c(77)),(c(17),c(30))],fill=(*body,255),outline=(24,20,28,255))
        d.line((c(48),c(16),c(33),c(70)),fill=(*accent,210),width=c(4)); d.line((c(48),c(16),c(63),c(70)),fill=(*accent,210),width=c(4))
    elif name=='boss':
        d.rounded_rectangle((c(10),c(10),c(86),c(86)),c(18),fill=(*body,255),outline=(19,20,24,255),width=c(5))
        for a in range(0,360,45):
            x=48+math.cos(math.radians(a))*28; y=48+math.sin(math.radians(a))*28
            d.polygon([(c(x),c(y)),(c(x+math.cos(math.radians(a))*14),c(y+math.sin(math.radians(a))*14)),(c(x+math.cos(math.radians(a+90))*5),c(y+math.sin(math.radians(a+90))*5))],fill=(*accent,220))
        d.ellipse((c(31),c(31),c(65),c(65)),fill=(28,24,30,255),outline=(*accent,255),width=c(4))
    else:
        d.polygon([(c(48),c(9)),(c(76),c(28)),(c(72),c(70)),(c(48),c(84)),(c(24),c(70)),(c(20),c(28))],fill=(*body,255),outline=(20,23,27,255))
        d.ellipse((c(36),c(34),c(60),c(58)),fill=(25,30,34,255),outline=(*accent,230),width=c(3))
    # front indicator
    d.polygon([(c(48),c(4)),(c(42),c(15)),(c(54),c(15))],fill=(*accent,245))
    return downsample(im,(S,S))

for n,(b,a) in ENEMIES.items(): enemy_sprite(n,b,a).save(AS/'enemies'/f'{n}.png')

# ---------- small UI icons ----------
def icon(name,kind,color):
    S=64; im,sc=aa_canvas((S,S),4); d=ImageDraw.Draw(im,'RGBA'); c=lambda v:int(v*sc)
    if kind=='coin':
        d.ellipse((c(10),c(10),c(54),c(54)),fill=(*color,255),outline=(120,77,5,255),width=c(4)); d.ellipse((c(18),c(18),c(46),c(46)),outline=(255,241,170,200),width=c(3))
    elif kind=='heart':
        d.polygon([(c(32),c(54)),(c(11),c(32)),(c(10),c(20)),(c(18),c(11)),(c(29),c(12)),(c(32),c(18)),(c(35),c(12)),(c(46),c(11)),(c(54),c(20)),(c(53),c(32))],fill=(*color,255))
    elif kind=='wave':
        for i in range(3): d.arc((c(8+i*7),c(10+i*7),c(58-i*7),c(54-i*7)),200,340,fill=(*color,255),width=c(5))
    elif kind=='star':
        pts=[]
        for i in range(10):
            a=-math.pi/2+i*math.pi/5; r=25 if i%2==0 else 11
            pts.append((c(32+math.cos(a)*r),c(32+math.sin(a)*r)))
        d.polygon(pts,fill=(*color,255))
    return downsample(im,(S,S))
icon('credits','coin',(255,199,69)).save(AS/'ui'/'credits.png')
icon('core','heart',(255,91,106)).save(AS/'ui'/'core.png')
icon('wave','wave',(98,207,255)).save(AS/'ui'/'wave.png')
icon('score','star',(196,141,255)).save(AS/'ui'/'score.png')

# ---------- simple SFX WAV generator ----------
def write_wav(path, freqs, dur=.13, volume=.28, noise=0.0, decay=8.0):
    rate=44100; n=int(rate*dur)
    with wave.open(str(path),'w') as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(rate)
        frames=[]
        phase=0
        for i in range(n):
            t=i/rate; env=math.exp(-decay*t)
            val=0.0
            for f,amp in freqs:
                val+=math.sin(2*math.pi*f*t)*amp
            val += (random.random()*2-1)*noise
            val*=env*volume
            frames.append(struct.pack('<h',max(-32767,min(32767,int(val*32767)))))
        w.writeframes(b''.join(frames))

write_wav(AS/'sfx'/'click.wav',[(780,.7),(1160,.25)],.07,.20,0.01,18)
write_wav(AS/'sfx'/'build.wav',[(210,.6),(420,.45),(840,.2)],.18,.25,0.015,8)
write_wav(AS/'sfx'/'upgrade.wav',[(440,.45),(660,.45),(880,.35)],.32,.23,0.005,3.8)
write_wav(AS/'sfx'/'shoot.wav',[(240,.9),(120,.3)],.09,.22,0.06,16)
write_wav(AS/'sfx'/'laser.wav',[(920,.75),(1350,.25)],.10,.17,0.015,13)
write_wav(AS/'sfx'/'explosion.wav',[(90,.45),(140,.3)],.28,.34,0.55,10)
write_wav(AS/'sfx'/'wave.wav',[(330,.4),(440,.4),(660,.4)],.45,.22,0.01,3)
write_wav(AS/'sfx'/'gameover.wav',[(180,.5),(135,.45),(90,.4)],.8,.24,0.01,1.8)
print('Assets generated in',AS)
