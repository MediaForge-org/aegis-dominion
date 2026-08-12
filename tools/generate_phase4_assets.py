#!/usr/bin/env python3
"""Generate deterministic, replaceable Phase-4 runtime art with Pillow."""

from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter
import math
import random

ROOT = Path(__file__).resolve().parents[1] / "assets"


def terrain(name, base, accent, seed):
    rng = random.Random(seed)
    size = 256
    image = Image.new("RGB", (size, size))
    pixels = image.load()
    for y in range(size):
        for x in range(size):
            periodic = math.sin(x * math.tau / 64) * 3 + math.cos(y * math.tau / 71) * 3
            grain = rng.randint(-9, 9) + periodic
            pixels[x, y] = tuple(max(0, min(255, int(c + grain))) for c in base)
    detail = Image.new("RGBA", image.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(detail)
    for _ in range(42):
        x, y = rng.randrange(size), rng.randrange(size)
        r = rng.randrange(3, 18)
        color = (*accent, rng.randrange(10, 35))
        d.ellipse((x-r*2, y-r, x+r*2, y+r), fill=color)
    if name in ("industrial", "rock"):
        for x in range(0, size, 64):
            d.line((x, 0, x, size), fill=(*accent, 45), width=2)
        for y in range(0, size, 64):
            d.line((0, y, size, y), fill=(*accent, 38), width=2)
    if name in ("ash", "rock"):
        for _ in range(18):
            x, y = rng.randrange(size), rng.randrange(size)
            d.line((x, y, x+rng.randrange(-18, 19), y+rng.randrange(8, 30)), fill=(*accent, 62), width=2)
    image = Image.alpha_composite(image.convert("RGBA"), detail.filter(ImageFilter.GaussianBlur(1.2)))
    (ROOT / "terrain").mkdir(parents=True, exist_ok=True)
    image.save(ROOT / "terrain" / f"{name}.png")


def canvas(size=128, scale=3):
    return Image.new("RGBA", (size*scale, size*scale), (0, 0, 0, 0)), scale


def finish(image, path, size=128):
    image = image.resize((size, size), Image.Resampling.LANCZOS)
    path.parent.mkdir(parents=True, exist_ok=True)
    image.save(path)


def prop(name):
    image, s = canvas()
    d = ImageDraw.Draw(image)
    def poly(points, **kw): d.polygon([(x*s, y*s) for x, y in points], **kw)
    def ellipse(box, **kw): d.ellipse(tuple(v*s for v in box), **kw)
    def rect(box, **kw): d.rounded_rectangle(tuple(v*s for v in box), radius=4*s, **kw)
    ellipse((27, 92, 108, 113), fill=(0, 0, 0, 65))
    if name == "tree":
        rect((57, 49, 70, 103), fill=(55, 61, 48, 255), outline=(21, 27, 24, 255), width=2*s)
        for box, color in [((23, 24, 74, 78),(31,91,67,255)),((48,14,103,70),(38,112,78,255)),((37,34,91,91),(27,82,61,255))]: ellipse(box, fill=color, outline=(13,43,35,255), width=2*s)
        ellipse((57, 29, 70, 43), fill=(99,194,132,210))
    elif name == "bush":
        for box, color in [((23,52,73,98),(31,99,64,255)),((48,39,105,98),(39,124,77,255)),((32,31,80,88),(46,137,83,255))]: ellipse(box, fill=color, outline=(16,55,40,255), width=2*s)
    elif name.startswith("rock"):
        pts = [(25,91),(34,49),(61,28),(96,44),(108,84),(86,103),(43,102)] if name == "rock_large" else [(37,94),(44,62),(66,48),(91,65),(94,91),(75,104),(48,102)]
        poly(pts, fill=(77,88,91,255), outline=(30,39,44,255))
        poly([(42,61),(62,39),(87,51),(71,64)], fill=(126,139,139,190))
    elif name == "crate":
        rect((28,38,101,104), fill=(79,78,65,255), outline=(26,34,37,255), width=4*s)
        d.line((35*s,45*s,94*s,97*s), fill=(142,118,70,255), width=7*s); d.line((94*s,45*s,35*s,97*s), fill=(142,118,70,255), width=7*s)
        rect((50,31,78,43), fill=(49,59,61,255))
    elif name == "ruin":
        poly([(25,104),(25,49),(42,35),(58,48),(75,28),(101,42),(101,104)], fill=(69,74,76,255), outline=(25,32,35,255))
        rect((42,64,60,102), fill=(18,29,34,255)); d.line((27*s,82*s,99*s,82*s), fill=(119,100,72,170), width=3*s)
    elif name == "lamp":
        rect((59,36,68,105), fill=(48,58,62,255)); rect((43,24,84,43), fill=(41,49,54,255), outline=(12,20,25,255), width=2*s)
        ellipse((49,28,78,39), fill=(125,229,255,245)); ellipse((39,18,88,49), fill=(68,207,255,45))
    elif name == "antenna":
        poly([(37,105),(55,76),(73,76),(92,105)], fill=(55,65,69,255), outline=(19,29,34,255)); rect((61,31,67,82), fill=(97,112,118,255))
        d.arc((42*s,18*s,86*s,61*s), 205, 335, fill=(90,224,255,255), width=3*s); ellipse((58,23,70,35), fill=(255,153,70,255))
    elif name == "scrap":
        poly([(23,92),(39,56),(65,70),(83,39),(108,89),(91,105),(45,103)], fill=(79,67,58,255), outline=(29,31,31,255))
        d.line((34*s,94*s,93*s,51*s), fill=(156,83,48,255), width=6*s); ellipse((70,76,93,99), fill=(35,44,47,255))
    elif name == "barricade":
        rect((19,54,109,78), fill=(58,67,70,255), outline=(19,27,31,255), width=3*s)
        for x in (31, 70): poly([(x,78),(x+12,78),(x+22,107),(x+12,107)], fill=(45,52,55,255))
        for x in range(23, 101, 24): poly([(x,57),(x+11,57),(x+22,75),(x+11,75)], fill=(229,145,54,255))
    finish(image, ROOT / "environment" / f"{name}.png")


def world_asset(name):
    image, s = canvas(192, 3)
    d = ImageDraw.Draw(image)
    def ellipse(box, **kw): d.ellipse(tuple(v*s for v in box), **kw)
    def poly(points, **kw): d.polygon([(x*s, y*s) for x, y in points], **kw)
    ellipse((24,132,168,169), fill=(0,0,0,65))
    if name == "spawn_gate":
        for r, col, width in [(72,(24,32,38,255),18),(57,(58,78,84,255),12),(41,(66,226,236,210),7)]:
            d.arc(((96-r)*s,(96-r)*s,(96+r)*s,(96+r)*s), 195, 525, fill=col, width=width*s)
        for x in (34,139): d.rounded_rectangle((x*s,82*s,(x+20)*s,153*s),radius=5*s,fill=(45,54,58,255),outline=(99,129,132,255),width=3*s)
        ellipse((68,68,124,124), fill=(43,223,241,38), outline=(90,240,255,220), width=4*s)
    elif name == "aegis_core":
        poly([(24,146),(38,75),(69,47),(123,47),(154,75),(168,146),(137,165),(55,165)], fill=(40,48,55,255), outline=(93,111,117,255))
        poly([(49,133),(57,79),(79,61),(113,61),(135,79),(143,133),(125,148),(67,148)], fill=(20,29,36,255), outline=(64,198,220,180))
        for x in (49,135): ellipse((x-7,112,x+7,126), fill=(255,151,58,255))
    else:
        ellipse((58,58,134,134), fill=(52,222,245,42), outline=(102,237,255,230), width=5*s)
        poly([(96,57),(119,83),(110,122),(96,139),(82,122),(73,83)], fill=(119,241,255,220), outline=(224,255,255,255))
        ellipse((79,79,113,113), fill=(230,255,255,235))
    finish(image, ROOT / "world" / f"{name}.png", 192)


def enemy(name):
    image = Image.new("RGBA", (288, 288), (0, 0, 0, 0)); d = ImageDraw.Draw(image); s = 3
    def poly(points, **kw): d.polygon([(x*s, y*s) for x, y in points], **kw)
    def line(points, **kw): d.line([(x*s, y*s) for x, y in points], **kw)
    def ellipse(box, **kw): d.ellipse(tuple(v*s for v in box), **kw)
    def rect(box, radius=5, **kw): d.rounded_rectangle(tuple(v*s for v in box), radius=radius*s, **kw)
    outline=(10,17,23,255); metal=(48,61,71,255); light=(94,116,126,255)
    ellipse((17,72,79,92),fill=(0,0,0,70))
    if name=="raider":
        poly([(48,9),(73,24),(86,52),(73,79),(48,89),(23,79),(10,52),(23,24)],fill=metal,outline=outline)
        poly([(48,16),(66,30),(71,61),(48,75),(25,61),(30,30)],fill=(70,80,86,255),outline=outline)
        for x in (19,70): rect((x,35,x+9,68),fill=(25,32,38,255),outline=outline)
        ellipse((38,37,58,57),fill=(9,21,28,255),outline=(255,89,95,255),width=3*s);ellipse((44,43,52,51),fill=(255,136,94,255))
    elif name=="runner":
        poly([(48,5),(65,25),(75,66),(58,89),(48,78),(38,89),(21,66),(31,25)],fill=(61,78,90,255),outline=outline)
        poly([(48,12),(58,31),(57,66),(48,75),(39,66),(38,31)],fill=(104,126,134,255),outline=outline)
        poly([(31,43),(8,67),(37,60)],fill=(50,67,78,255),outline=outline);poly([(65,43),(88,67),(59,60)],fill=(50,67,78,255),outline=outline)
        line([(48,17),(48,58)],fill=(255,105,108,255),width=3*s)
    elif name=="tank":
        for x in (9,70): rect((x,21,x+17,81),radius=7,fill=(29,37,43,255),outline=outline,width=2*s)
        rect((20,13,76,86),radius=10,fill=(57,66,70,255),outline=outline,width=3*s)
        poly([(48,19),(68,35),(64,66),(48,79),(32,66),(28,35)],fill=(91,96,91,255),outline=outline)
        rect((43,2,53,49),radius=3,fill=(108,105,87,255),outline=outline,width=2*s);rect((39,2,57,13),radius=3,fill=(32,39,42,255))
        for y in (30,48,66): line([(12,y),(23,y)],fill=light,width=2*s);line([(73,y),(84,y)],fill=light,width=2*s)
    elif name=="shield":
        poly([(48,15),(72,27),(82,53),(69,80),(48,90),(27,80),(14,53),(24,27)],fill=(49,65,83,255),outline=outline)
        rect((34,28,62,76),radius=10,fill=(67,84,111,255),outline=outline,width=3*s)
        d.arc((8*s,8*s,88*s,88*s),195,525,fill=(105,205,255,255),width=5*s)
        for angle in (20,110,200,290):
            x=48+math.cos(math.radians(angle))*39;y=48+math.sin(math.radians(angle))*39;ellipse((x-3,y-3,x+3,y+3),fill=(211,248,255,255))
    elif name=="regen":
        poly([(48,9),(66,23),(82,46),(74,78),(48,90),(22,78),(14,46),(30,23)],fill=(40,75,63,255),outline=outline)
        for x in (22,64): rect((x,26,x+10,72),fill=(51,112,79,255),outline=outline)
        ellipse((29,29,67,67),fill=(22,45,39,255),outline=(89,232,135,255),width=3*s)
        line([(48,36),(48,60)],fill=(112,255,161,255),width=6*s);line([(36,48),(60,48)],fill=(112,255,161,255),width=6*s)
        for p in ((27,22),(69,22),(18,52),(78,52)):ellipse((p[0]-3,p[1]-3,p[0]+3,p[1]+3),fill=(95,255,157,255))
    elif name=="splitter":
        poly([(48,5),(63,29),(89,33),(69,51),(78,83),(48,66),(18,83),(27,51),(7,33),(33,29)],fill=(67,48,83,255),outline=outline)
        poly([(48,13),(59,35),(48,61),(37,35)],fill=(134,80,166,255),outline=outline)
        line([(48,17),(48,63)],fill=(234,136,255,255),width=3*s);line([(20,37),(39,49)],fill=(183,93,222,255),width=3*s);line([(76,37),(57,49)],fill=(183,93,222,255),width=3*s)
    else:
        poly([(48,3),(72,13),(90,34),(88,65),(70,88),(48,95),(26,88),(8,65),(6,34),(24,13)],fill=(47,47,50,255),outline=outline)
        for x in (8,76): rect((x,25,x+12,74),fill=(29,33,37,255),outline=outline)
        poly([(48,12),(68,27),(73,61),(58,81),(38,81),(23,61),(28,27)],fill=(82,73,68,255),outline=outline)
        for p in ((25,23),(71,23),(18,63),(78,63)):ellipse((p[0]-4,p[1]-4,p[0]+4,p[1]+4),fill=(255,89,78,255),outline=outline)
        ellipse((34,33,62,61),fill=(20,23,28,255),outline=(255,102,92,255),width=4*s);ellipse((42,41,54,53),fill=(255,176,92,255))
    finish(image, ROOT / "enemies" / f"{name}.png", 96)


def main():
    palettes = {
        "grass": ((24,67,52),(74,139,88),11), "dirt": ((79,62,43),(132,102,62),12),
        "rock": ((61,69,70),(124,133,130),13), "snow": ((154,176,180),(224,239,238),14),
        "ice": ((71,125,145),(151,225,238),15), "sand": ((139,116,70),(205,178,105),16),
        "ash": ((57,47,44),(133,74,51),17), "industrial": ((45,54,58),(101,129,133),18)}
    for name, (base, accent, seed) in palettes.items(): terrain(name, base, accent, seed)
    for name in ("tree","bush","rock_small","rock_large","crate","ruin","lamp","antenna","scrap","barricade"): prop(name)
    for name in ("spawn_gate","aegis_core","aegis_core_energy"): world_asset(name)
    for name in ("raider","runner","tank","shield","regen","splitter","boss"): enemy(name)


if __name__ == "__main__": main()
