
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

dcl_s2_tex	s0
dcl_f4_col	v1.x
dcl_f2_tex_c	v0.x

label start
label main_
texld r0.xyzw, v0.xyzw, s0	# tex=s0
mul_sat oColor.xyzw, r0.xyzw, v1.xyzw	# gl_FragColor=oColor.xyzw, col=v1.xyzw
label main_end
ret
