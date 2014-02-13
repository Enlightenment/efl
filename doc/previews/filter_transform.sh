TEXT="Transform - Mirror effect"
FONT="Sans"
SIZE=50
FILTER="
buffer : t (alpha);
transform (oy = 20, dst = t);
blend (src = t, color = silver);
blend (color = white);
"
