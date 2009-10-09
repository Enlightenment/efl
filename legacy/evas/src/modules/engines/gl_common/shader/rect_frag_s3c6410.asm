
#-------------------------------------------------
# ORION - OpenGL ES 2.0 Shading Language Compiler
# SAMSUNG INDIA SOFTWARE OPERATIONS PVT. LTD.
# Compiler Version	: v04.00.09
# Release Date		: 19.01.2009
# FIMG VERSION      : FIMGv1.5
# Optimizer Options :  -O --nolodcalc
#-------------------------------------------------

# hand optimised - removed useless ops

ps_3_0

fimg_version	0x01020000

dcl_f4_col	v0.x

label start
label main_
label main_end
mov_sat oColor.xyzw, v0.xyzw
ret

