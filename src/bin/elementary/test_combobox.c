#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_combobox_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   printf("Hover button is clicked and 'clicked' callback is called.\n");
}

static void
_combobox_item_selected_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                           void *event_info)
{
   const char *txt = elm_object_item_text_get(event_info);
   printf("'item,selected' callback is called. (selected item : %s)\n", txt);
}

static void
_combobox_dismissed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   printf("'dismissed' callback is called.\n");
}

static void
_combobox_expanded_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   printf("'expanded' callback is called.\n");
}

static void
_combobox_item_pressed_cb(void *data EINA_UNUSED, Evas_Object *obj,
                      void *event_info)
{
   const char *txt = elm_object_item_text_get(event_info);
   printf("'item,pressed' callback is called. (selected item : %s)\n", txt);
   elm_object_text_set(obj, txt);
   elm_combobox_hover_end(obj);
   elm_entry_cursor_end_set(obj);
}

static char *
gl_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item # %i", (int)(uintptr_t)data);
   return strdup(buf);
}

static char *
gl2_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "%s", (char*)data);
   return strdup(buf);
}

static Evas_Object *gl_content_get(void *data EINA_UNUSED, Evas_Object *obj,
                                    const char *part)
{
   char buf[PATH_MAX];
   Evas_Object *ic = elm_icon_add(obj);
   if (!strcmp(part, "elm.swallow.end"))
     snprintf(buf, sizeof(buf), "%s/images/bubble.png", elm_app_data_dir_get());
   else
     snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}

static Eina_Bool gl_state_get(void *data EINA_UNUSED,
                               Evas_Object *obj EINA_UNUSED,
                               const char *part EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
gl_filter_get(void *data, Evas_Object *obj EINA_UNUSED, void *key)
{
   char buf[256];
   // if the key is empty/NULL, return true for item
   if (!strlen((char *)key)) return EINA_TRUE;
   snprintf(buf, sizeof(buf), "Item # %i", (int)(uintptr_t)data);
   if (strcasestr(buf, (char *)key))
     return EINA_TRUE;
   // Default case should return false (item fails filter hence will be hidden)
   return EINA_FALSE;
}

static void
_gl_filter_restart_cb(void *data EINA_UNUSED,
                       Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
   elm_genlist_filter_set(obj, (void *)elm_object_text_get(obj));
}

static void
_gl2_filter_restart_cb(void *data EINA_UNUSED,
                       Evas_Object *obj,
                       void *event_info EINA_UNUSED)
{
   elm_genlist_filter_set(obj, (void *)elm_object_text_get(elm_multibuttonentry_entry_get(obj)));
}

static Eina_Bool
gl2_filter_get(void *data, Evas_Object *obj EINA_UNUSED, void *key)
{
   char buf[256];
   // if the key is empty/NULL, return true for item
   if (!strlen((char *)key)) return EINA_TRUE;
   snprintf(buf, sizeof(buf), "%s", (char*)data);
   if (!strncmp(buf, (char *)key, strlen((char*)key)))
     return EINA_TRUE;
   // Default case should return false (item fails filter hence will be hidden)
   return EINA_FALSE;
}

static void
_gl_filter_finished_cb(void *data EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   printf("Filter finished\n");
}

void
test_combobox(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *combobox;
   Elm_Genlist_Item_Class *itc;
   win = elm_win_util_standard_add("combobox", "Combobox");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.text_get = gl_text_get;
   itc->func.content_get = gl_content_get;
   itc->func.state_get = gl_state_get;
   itc->func.filter_get = gl_filter_get;
   itc->func.del = NULL;

   combobox = elm_combobox_add(win);
   evas_object_size_hint_weight_set(combobox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(combobox, EVAS_HINT_FILL, 0);
   elm_object_part_text_set(combobox, "guide", "A long list");
   for (int i = 0; i < 1000; i++)
     elm_genlist_item_append(combobox, itc, (void *)(uintptr_t)i,
                                   NULL, ELM_GENLIST_ITEM_NONE, NULL,
                                   (void*)(uintptr_t)(i * 10));
   evas_object_smart_callback_add(combobox, "clicked",
                                  _combobox_clicked_cb, NULL);
   evas_object_smart_callback_add(combobox, "dismissed",
                                  _combobox_dismissed_cb, NULL);
   evas_object_smart_callback_add(combobox, "expanded",
                                  _combobox_expanded_cb, NULL);
   evas_object_smart_callback_add(combobox, "item,selected",
                                  _combobox_item_selected_cb, NULL);
   evas_object_smart_callback_add(combobox, "item,pressed",
                                  _combobox_item_pressed_cb, NULL);
   evas_object_smart_callback_add(combobox, "filter,done",
                                  _gl_filter_finished_cb, NULL);
   evas_object_smart_callback_add(combobox, "changed",
                                  _gl_filter_restart_cb, NULL);
   elm_box_pack_end(bx, combobox);
   evas_object_show(combobox);

   combobox = elm_combobox_add(win);
   evas_object_size_hint_weight_set(combobox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(combobox, EVAS_HINT_FILL, 0);
   elm_object_text_set(combobox, "Disabled Combobox");
   for (int i = 0; i < 1000; i++)
     elm_genlist_item_append(combobox, itc, (void *)(uintptr_t)i,
                                   NULL, ELM_GENLIST_ITEM_NONE, NULL,
                                   (void*)(uintptr_t)(i * 10));
   evas_object_smart_callback_add(combobox, "clicked",
                                  _combobox_clicked_cb, NULL);
   evas_object_smart_callback_add(combobox, "item,selected",
                                  _combobox_item_selected_cb, NULL);
   evas_object_smart_callback_add(combobox, "item,pressed",
                                  _combobox_item_pressed_cb, NULL);
   evas_object_smart_callback_add(combobox, "dismissed",
                                  _combobox_dismissed_cb, NULL);
   evas_object_smart_callback_add(combobox, "expanded",
                                  _combobox_expanded_cb, NULL);
   evas_object_smart_callback_add(combobox, "filter,done",
                                  _gl_filter_finished_cb, NULL);
   elm_object_disabled_set(combobox, EINA_TRUE);
   elm_box_pack_end(bx, combobox);
   evas_object_show(combobox);

   evas_object_resize(win, 320, 500);
   evas_object_show(win);

   elm_genlist_item_class_free(itc);
}

static void
_combobox2_item_pressed_cb(void *data EINA_UNUSED, Evas_Object *obj,
                      void *event_info)
{
   const char *txt = elm_object_item_text_get(event_info);
   printf("'item,pressed' callback is called. (selected item : %s)\n", txt);
   if (elm_combobox_multiple_selection_get(obj))
     elm_multibuttonentry_item_append(obj, txt, NULL, NULL);
   else
      elm_object_text_set(obj, txt);
   elm_combobox_hover_end(obj);
}
// Combobox with multiple selection
void
test_combobox2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   Evas_Object *win, *combobox, *ly;
   Elm_Genlist_Item_Class *itc;
   char buf[128];
   char* email_address[300] = {"augue.ut.lacus@Suspendisse.com","egestas.Aliquam.nec@Vivamusmolestiedapibus.edu","urna.et@purusNullam.co.uk","elit@Sedid.net","cursus@malesuadafringillaest.net","Lorem@Cras.org","risus@sedhendrerita.co.uk","auctor.nunc.nulla@utsemNulla.com","nunc.nulla@nonenim.org","egestas@egetipsum.co.uk","sed.tortor@tempusmauris.edu","rutrum@gravida.org","nunc@acsemut.com","lobortis.quam@eratvolutpatNulla.net","fames@ullamcorperDuisat.co.uk","pede.et.risus@necurna.edu","semper.cursus.Integer@justo.com","sem.molestie@orciPhasellusdapibus.com","quam.a@tinciduntvehicula.co.uk","ullamcorper.Duis@odioEtiam.com","enim.Sed.nulla@Etiamligulatortor.net","molestie.pharetra.nibh@velfaucibus.com","morbi@neceuismodin.com","egestas.hendrerit.neque@nequetellus.com","Etiam.gravida.molestie@purusDuis.org","metus.vitae@risus.net","tincidunt.tempus.risus@nonummyFusce.co.uk","Curabitur.ut.odio@in.com","rutrum@utnisi.ca","iaculis.nec@ultriciesdignissimlacus.ca","consectetuer.ipsum.nunc@Cumsociisnatoque.ca","non.arcu.Vivamus@ornare.com","arcu.Vestibulum@interdumligula.edu","sem.magna@urnasuscipit.ca","eu.euismod@urnaconvalliserat.net","Maecenas.mi.felis@mattisInteger.ca","magna.Suspendisse@Donecdignissimmagna.ca","cursus.Integer.mollis@fringillaeuismodenim.ca","tellus.eu@molestie.co.uk","metus.In@egestas.co.uk","Cum@nisl.com","Phasellus@atarcu.net","nec.quam.Curabitur@tortorInteger.org","condimentum.Donec@egetodioAliquam.edu","orci.luctus@magnatellus.co.uk","adipiscing@noncursusnon.co.uk","Sed.eget.lacus@Nullam.edu","felis.ullamcorper@ornare.co.uk","Nunc.quis.arcu@Proineget.edu","in@lobortis.co.uk","leo.Vivamus.nibh@seddui.com","eu.tellus.Phasellus@natoque.ca","ultrices.iaculis@Aliquam.net","purus.ac@feugiatLorem.net","tincidunt.neque@ut.net","odio.sagittis.semper@nibhdolornonummy.org","Sed@eunequepellentesque.com","elementum@sempererat.co.uk","Curae.Donec.tincidunt@neque.net","rhoncus@erat.edu","mauris.elit@Donec.ca","metus.vitae.velit@ad.edu","scelerisque.scelerisque@etmalesuadafames.net","velit.in@convallis.co.uk","ridiculus@laoreetlectusquis.org","tincidunt.orci.quis@musDonec.net","tempor@orciconsectetuereuismod.co.uk","Duis.sit@eratSednunc.com","elit.sed.consequat@nuncinterdum.edu","lorem@Pellentesqueultricies.org","ornare.placerat.orci@pretiumnequeMorbi.com","euismod.enim@primisinfaucibus.ca","a.scelerisque.sed@sapienCras.com","Aliquam@Vestibulum.net","nec@at.ca","quis.diam.luctus@atauctor.ca","nec@purusin.org","montes.nascetur.ridiculus@viverraMaecenas.co.uk","elementum@amet.edu","fringilla.cursus.purus@velarcuCurabitur.co.uk","et.rutrum@consectetuerrhoncus.edu","Aenean@maurissapiencursus.com","interdum@vehiculaaliquet.co.uk","orci.quis.lectus@facilisisfacilisis.org","et.netus.et@arcu.net","ipsum.porta.elit@sapienNunc.edu","libero.Morbi@ipsumCurabiturconsequat.ca","libero@sitamet.com","porta@penatibus.org","nec.enim.Nunc@egetmetusIn.edu","Nunc.sollicitudin.commodo@porttitorinterdum.org","Phasellus.at.augue@dolor.org","nec.ante@etlibero.com","diam@gravida.co.uk","laoreet@malesuada.co.uk","in.lobortis@blanditenim.edu","ante@ipsumnon.net","in@odio.org","Quisque.tincidunt@risus.co.uk","lacus.varius@Vestibulum.com","eu.eros.Nam@arcuNunc.org","pellentesque@Vivamuseuismodurna.org","Cum.sociis@eleifendCras.com","neque@vulputate.org","imperdiet.dictum.magna@risus.org","sagittis@enimcondimentum.edu","hendrerit@maurisMorbi.org","suscipit.nonummy@disparturientmontes.org","Vivamus.non.lorem@fermentummetusAenean.net","In.mi@maurisaliquam.com","est@massanonante.org","molestie@a.co.uk","sit@acturpis.org","diam@felisorci.edu","dolor.nonummy.ac@elitsedconsequat.co.uk","justo@Praesentinterdum.co.uk","Quisque.varius@enimMaurisquis.ca","nibh.enim.gravida@ut.ca","arcu.Vivamus@orciquis.co.uk","sociis@Sedeget.net","risus@egetvolutpatornare.com","vel.est.tempor@ipsum.net","ipsum@dolordapibusgravida.edu","sem.egestas@quamelementum.co.uk","ipsum@Duisatlacus.co.uk","facilisis.vitae@acturpisegestas.net","Nam@aliquetlobortis.net","ipsum.dolor.sit@nuncInat.net","gravida.sagittis@et.org","mauris@magnatellus.edu","sed@adipiscingenimmi.org","sed@ipsumportaelit.com","malesuada.vel.convallis@amet.net","Praesent.interdum@dictumeu.co.uk","nunc.In.at@ornare.co.uk","Phasellus.fermentum.convallis@ipsum.net","sed.libero.Proin@Aliquameratvolutpat.org","aliquet.libero@telluslorem.net","lectus.pede.ultrices@Maurisquisturpis.edu","blandit.at.nisi@ut.net","erat@convallisdolor.net","ante.Nunc.mauris@vehiculaetrutrum.ca","vel.quam@egestas.edu","non@justofaucibuslectus.co.uk","sem.ut.dolor@odioNaminterdum.org","et.ipsum@malesuada.net","non@Nulladignissim.com","ullamcorper.nisl@iaculisodio.com","neque.sed@necurna.ca","in.cursus.et@fermentumvelmauris.co.uk","magna.sed@eteuismodet.co.uk","a@Crasvehicula.com","tortor.Nunc.commodo@velmauris.net","dignissim.pharetra@Aeneaneuismodmauris.org","egestas.urna.justo@acorci.org","iaculis.enim.sit@maurisIntegersem.com","malesuada@imperdietnec.com","erat.volutpat.Nulla@ipsum.org","Aliquam@IntegerurnaVivamus.co.uk","Nunc.sollicitudin@ipsumCurabitur.net","nibh.enim@quam.co.uk","pede@quismassa.com","vel.nisl@fringillacursuspurus.co.uk","vel@auctorvelitAliquam.org","auctor.quis.tristique@fringillamilacinia.org","nisl.elementum@amagna.com","facilisis@feugiattelluslorem.co.uk","eleifend.Cras@Vestibulumante.net","Integer.eu.lacus@ipsumCurabiturconsequat.com","Donec.porttitor@Etiamvestibulummassa.ca","montes@auctorquistristique.net","Nunc.ullamcorper.velit@Vivamusnon.co.uk","dictum.cursus@sed.org","Aliquam.erat.volutpat@nonummyut.org","ac.mattis@ligulaNullamenim.net","id.sapien.Cras@Proin.ca","Phasellus.dolor@fermentumfermentum.edu","in@odio.edu","non.luctus@pedeNuncsed.com","per.conubia@euismodacfermentum.com","luctus.aliquet@venenatisvelfaucibus.ca","nulla.Cras@purusaccumsaninterdum.ca","aliquet.vel.vulputate@pedesagittis.edu","rutrum@pedeultrices.co.uk","Nullam.lobortis@hendrerit.ca","nonummy.ac.feugiat@Sedmalesuadaaugue.edu","nibh@ipsum.com","in.faucibus.orci@vehicula.com","odio.vel.est@in.edu","amet.ornare.lectus@Suspendisse.co.uk","Maecenas.malesuada.fringilla@at.co.uk","Aliquam@aceleifendvitae.org","Nullam.feugiat.placerat@massaQuisque.ca","urna@tempor.org","magnis.dis.parturient@arcuCurabitur.edu","erat.vel@In.ca","rutrum@Integervitae.ca","metus.In@odio.co.uk","nec.imperdiet@tellus.ca","dui.semper.et@at.org","sit.amet@quisarcu.org","ante@Donecsollicitudinadipiscing.edu","turpis.egestas.Aliquam@egestasnunc.edu","posuere@quismassaMauris.co.uk","Nulla.dignissim@nibhAliquamornare.com","facilisis.vitae.orci@estmaurisrhoncus.net","vitae.aliquet.nec@nostraper.co.uk","lorem@enimsitamet.co.uk","pellentesque@acipsum.org","pede.et.risus@nonvestibulum.org","sed@Nuncsollicitudin.com","erat@Maurisvestibulum.org","scelerisque@tortorNunc.org","metus@idsapien.org","dignissim@Duis.ca","Duis.at.lacus@egestaslaciniaSed.com","auctor.velit@dapibus.co.uk","Curae.Phasellus.ornare@eudolor.net","arcu@metusfacilisis.ca","laoreet@dictummagna.net","tristique.neque@auctorvitaealiquet.ca","nunc.interdum.feugiat@primisinfaucibus.edu","elit.pede.malesuada@quam.net","semper.et.lacinia@ornareliberoat.ca","magna.Praesent.interdum@elit.net","consequat@loremDonec.ca","Vivamus@nisiMauris.edu","feugiat.tellus@sociisnatoquepenatibus.co.uk","scelerisque.mollis.Phasellus@facilisis.edu","rhoncus.Proin@enimEtiam.com","amet.consectetuer.adipiscing@lacusNullatincidunt.edu","aliquet.lobortis.nisi@leo.com","magna@purus.org","a@etmalesuadafames.com","Nunc.commodo@vulputatenisisem.net","et.rutrum.non@imperdieteratnonummy.com","consectetuer@mauris.net","iaculis.lacus@Proinvelarcu.ca","tincidunt.Donec.vitae@habitant.net","et.ultrices@nequesed.org","Lorem.ipsum@nonante.edu","Vestibulum.ut@sed.co.uk","fermentum.arcu@Duis.com","Morbi.quis.urna@vulputate.org","Sed.eget@liberolacusvarius.net","amet.lorem@tincidunt.co.uk","morbi@Classaptenttaciti.com","nisl.Quisque.fringilla@ut.ca","Aenean.egestas.hendrerit@eleifendnec.co.uk","elit@odio.net","sodales.Mauris.blandit@fermentumfermentumarcu.com","massa.non@Nuncsollicitudin.com","quam@sit.co.uk","consectetuer@quispedePraesent.co.uk","erat.eget@aliquetPhasellusfermentum.ca","libero@convalliserat.net","dui@arcu.ca","Curabitur@ascelerisquesed.org","amet@sed.com","in.aliquet.lobortis@acipsum.net","Donec.non@feugiatnec.com","Suspendisse.dui.Fusce@musProin.com","congue@loremauctor.co.uk","magna@Morbi.com","sit@dolor.edu","Praesent.eu.nulla@parturientmontes.com","eu.dui.Cum@arcuvelquam.org","leo.elementum@aliquet.edu","aliquam@urna.org","congue@nonummy.ca","urna.Nullam@atauctor.ca","natoque.penatibus@id.co.uk","aliquam.arcu@risusQuisque.com","ultrices.iaculis@liberoet.com","mollis.Integer.tincidunt@auctorvelit.org","sit@mus.org","est.mollis@orci.net","gravida@eunullaat.co.uk","varius.ultrices@Intinciduntcongue.org","Duis.cursus@nuncnulla.org","eu.turpis@Cumsociis.com","metus.In@sapiencursusin.org","a.feugiat.tellus@velitjusto.co.uk","nibh.lacinia.orci@mifelis.org","tincidunt.neque.vitae@Sed.ca","convallis.est.vitae@Donec.org","mauris@semelit.co.uk","Nam.interdum@Morbiquis.ca","vel.arcu.Curabitur@ullamcorperDuisat.net","dolor@mauris.com","Suspendisse@ipsum.org","Vivamus@dui.edu","condimentum.eget.volutpat@lobortisultrices.ca","commodo@et.edu","ut.ipsum@MorbimetusVivamus.co.uk","ut@feugiatnecdiam.org","Nam@ultrices.co.uk","orci.Donec@turpis.org","semper.tellus@venenatislacus.com","elit.elit@arcuimperdietullamcorper.edu"};
   win = elm_win_util_standard_add("combobox", "Combobox");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/combobox_multiple.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "combobox_multiple_test");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.text_get = gl2_text_get;
   itc->func.content_get = gl_content_get;
   itc->func.state_get = gl_state_get;
   itc->func.filter_get = gl2_filter_get;
   itc->func.del = NULL;

   combobox = elm_combobox_add(win);
   evas_object_size_hint_weight_set(combobox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(combobox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(combobox, "To:");
   elm_object_part_text_set(combobox, "guide", "Tap to add recipient");
   elm_combobox_multiple_selection_set(combobox, EINA_TRUE);
   for (int i = 0; i < 300; i++)
     elm_genlist_item_append(combobox, itc, (void *)email_address[i],
                                   NULL, ELM_GENLIST_ITEM_NONE, NULL,
                                   (void*)(uintptr_t)(i * 10));

   evas_object_smart_callback_add(combobox, "clicked",
                                  _combobox_clicked_cb, NULL);
   evas_object_smart_callback_add(combobox, "item,selected",
                                  _combobox_item_selected_cb, NULL);
   evas_object_smart_callback_add(combobox, "dismissed",
                                  _combobox_dismissed_cb, NULL);
   evas_object_smart_callback_add(combobox, "expanded",
                                  _combobox_expanded_cb, NULL);
   evas_object_smart_callback_add(combobox, "item,pressed",
                                  _combobox2_item_pressed_cb, NULL);
   evas_object_smart_callback_add(combobox, "filter,done",
                                  _gl_filter_finished_cb, NULL);
   evas_object_smart_callback_add(combobox, "changed",
                                  _gl2_filter_restart_cb, NULL);
   elm_object_part_content_set(ly, "combobox", combobox);
   evas_object_show(combobox);

   evas_object_resize(win, 640, 600);
   evas_object_show(win);

   elm_genlist_item_class_free(itc);
}
