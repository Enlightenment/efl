TEXT="Grow - Contour"
FONT="Sans:style=bold"
SIZE=32
FILTER="
buffer : fat (alpha);
grow (4, dst = fat);
blend (src = fat, color = black);
blend (color = white);
"
