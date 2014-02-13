TEXT="Bump"
FONT="Sans:style=bold"
SIZE=64
FILTER="
buffer : a (alpha);
blur (5, dst = a);
bump (map = a, compensate = yes, color = cyan, specular = 10.0);
"
