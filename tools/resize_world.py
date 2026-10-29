#!/usr/bin/env python3
"""Resize the walls of a roomshape world file in place.

Rectangular rooms (small_square, large_square, rectangle, hallway):
    resize_world.py rect <world.sdf> <width_x> <depth_y>
Triangular room (equilateral_triangle):
    resize_world.py triangle <world.sdf> <side>

All sizes are INTERNAL sizes in metres (inside face to inside face), the same
convention the classifier uses. The room stays centred on the world origin.
Update the 'Internal size' comment in the file by hand afterwards.
"""
import math
import re
import sys

WALL_THICKNESS = 0.05
WALL_HEIGHT = 0.50
WALL_CENTRE_Z = WALL_HEIGHT / 2.0


def fmt(value):
    return f"{value:.6f}".rstrip("0").rstrip(".")


def set_link(text, link, pose, size):
    """Replace the pose of one link and every <size> inside it."""
    pattern = re.compile(
        r'(<link name="%s">\s*)<pose>[^<]*</pose>(.*?)</link>' % link, re.S)

    def replace(match):
        body = re.sub(r"<size>[^<]*</size>", f"<size>{size}</size>", match.group(2))
        return f"{match.group(1)}<pose>{pose}</pose>{body}</link>"

    text, count = pattern.subn(replace, text)
    if count != 1:
        sys.exit(f"error: expected exactly one link named {link}, found {count}")
    return text


def resize_rect(text, width, depth):
    t, h, z = WALL_THICKNESS, WALL_HEIGHT, WALL_CENTRE_Z
    ns_size = f"{fmt(width + 2 * t)} {fmt(t)} {fmt(h)}"
    ew_size = f"{fmt(t)} {fmt(depth)} {fmt(h)}"
    half_y = depth / 2.0 + t / 2.0
    half_x = width / 2.0 + t / 2.0
    text = set_link(text, "north_wall", f"0 {fmt(half_y)} {fmt(z)} 0 0 0", ns_size)
    text = set_link(text, "south_wall", f"0 {fmt(-half_y)} {fmt(z)} 0 0 0", ns_size)
    text = set_link(text, "east_wall", f"{fmt(half_x)} 0 {fmt(z)} 0 0 0", ew_size)
    text = set_link(text, "west_wall", f"{fmt(-half_x)} 0 {fmt(z)} 0 0 0", ew_size)
    return text


def resize_triangle(text, side):
    t, h, z = WALL_THICKNESS, WALL_HEIGHT, WALL_CENTRE_Z
    # Wall centre lines sit one inradius plus half a wall thickness from the centroid.
    d = side * math.sqrt(3.0) / 6.0 + t / 2.0
    size = f"{fmt(side + t * math.sqrt(3.0))} {fmt(t)} {fmt(h)}"
    text = set_link(text, "base_wall", f"0 {fmt(-d)} {fmt(z)} 0 0 0", size)
    text = set_link(text, "left_wall",
                    f"{fmt(-d * math.sqrt(3.0) / 2.0)} {fmt(d / 2.0)} {fmt(z)} 0 0 {fmt(math.pi / 3.0)}", size)
    text = set_link(text, "right_wall",
                    f"{fmt(d * math.sqrt(3.0) / 2.0)} {fmt(d / 2.0)} {fmt(z)} 0 0 {fmt(2.0 * math.pi / 3.0)}", size)
    return text


def main(argv):
    if len(argv) == 5 and argv[1] == "rect":
        resize = lambda text: resize_rect(text, float(argv[3]), float(argv[4]))
    elif len(argv) == 4 and argv[1] == "triangle":
        resize = lambda text: resize_triangle(text, float(argv[3]))
    else:
        sys.exit(__doc__)
    path = argv[2]
    with open(path) as handle:
        text = handle.read()
    with open(path, "w") as handle:
        handle.write(resize(text))
    print(f"resized {path}")


if __name__ == "__main__":
    main(sys.argv)