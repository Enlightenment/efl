AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = edje_cc
EDJE_FLAGS = -v -id $(top_srcdir)/data/themes -fd $(top_srcdir)/data/themes

filesdir = $(datadir)/elementary/themes
files_DATA = default.edj

EXTRA_DIST = \
default.edc \
arrow_down.png \
arrow_up.png \
bt_base1.png \
bt_base2.png \
bt_bases.png \
bt_basew.png \
bt_glow.png \
bt_hilight.png \
bt_hilightw.png \
bt_shine.png \
bt_sm_base1.png \
bt_sm_base2.png \
bt_sm_hilight.png \
bt_sm_shine.png \
dia_botshad.png \
dia_grad.png \
dia_topshad.png \
frame_1.png \
frame_2.png \
flip_0b.png \
flip_0t.png \
flip_1b.png \
flip_1t.png \
flip_2b.png \
flip_2t.png \
flip_3b.png \
flip_3t.png \
flip_4b.png \
flip_4t.png \
flip_5b.png \
flip_5t.png \
flip_6b.png \
flip_6t.png \
flip_7b.png \
flip_7t.png \
flip_8b.png \
flip_8t.png \
flip_9b.png \
flip_9t.png \
flip_amb.png \
flip_amt.png \
flip_base.png \
flip_base_shad.png \
flip_pmb.png \
flip_pmt.png \
flip_shad.png \
head.png \
sb_runnerh.png \
sb_runnerv.png \
shelf_inset.png \
tog_base.png \
shad_circ.png \
bt_dis_base.png \
bt_dis_hilight.png \
outdent-top.png \
outdent-bottom.png \
updown.png \
leftright.png \
hoversel_entry_bg.png

default.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/themes/default.edc \
	$(top_builddir)/data/themes/default.edj

clean-local:
	rm -f *.edj
