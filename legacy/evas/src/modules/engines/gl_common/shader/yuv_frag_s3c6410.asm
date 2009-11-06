
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
dcl_s2_texu	s1
dcl_s2_texv	s2
dcl_f4_col	v3.x
dcl_f2_tex_c	v0.x
dcl_f2_tex_c2	v1.x
dcl_f2_tex_c3	v2.x

def c2, 1.164000, 1.164000, 1.164000, 0.000000
def c3, 0.000000, -0.344100, 1.772000, 0.000000
def c4, 1.402000, -0.714100, 0.000000, 0.000000
def c5, -0.773800, 0.456300, -0.958800, 1.000000
def c6, 1.000000, 0.000000, 0.000000, 0.000000

label start
label main_
mul r0.xyzw, c6.xxyy, v0.xyyy	# tex_c=v0.xyyy
texld r0.xyzw, r0.xyzw, s0	# tex=s0
mul r1.xyzw, c6.xxyy, v1.xyyy	# tex_c2=v1.xyyy
texld r1.xyzw, r1.xyzw, s1	# texu=s1
mul r2.xyzw, c6.xxyy, v2.xyyy	# tex_c3=v2.xyyy
texld r2.xyzw, r2.xyzw, s2	# texv=s2
mul r3.xyzw, c2.xyzw, r0.xxxx	# yuv=r0.xxxx
mad r3.xyzw, c3.xyzw, r1.xxxx, r3.xyzw	# yuv=r1.xxxx
mad r3.xyzw, c4.xyzw, r2.xxxx, r3.xyzw	# yuv=r2.xxxx
mov r4.xyzw, c5.xyzw
mad r3.xyzw, r4.xyzw, c6.xxxx, r3.xyzw	# yuv=c6.xxxx
mul_sat oColor.xyzw, r3.xyzw, v3.xyzw	# gl_FragColor=oColor.xyzw, col=v3.xyzw
label main_end
ret
