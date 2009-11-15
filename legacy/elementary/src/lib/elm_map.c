/*
 *
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Map Map
 *
 * This is a widget specifically for displaying the free map OpenStreetMap.
 *
 * Signals that you can add callbacks for are:
 *
 * clicked - This is called when a user has clicked the map without dragging
 * around.
 *
 * press - This is called when a user has pressed down on the map.
 *
 * longpressed - This is called when a user has pressed down on the map for
 * a long time without dragging around.
 *
 * clicked,double - This is called when a user has double-clicked the photo.
 *
 * load,details - Map detailed data load begins.
 *
 * loaded,details - This is called when all parts of the map are loaded.
 *
 * zoom,start - Zoom animation started.
 *
 * zoom,stop - Zoom animation stopped.
 *
 * zoom,change - Zoom changed when using an auto zoom mode.
 *
 * scroll - the content has been scrolled (moved)
 *
 * scroll,anim,start - scrolling animation has started
 *
 * scroll,anim,stop - scrolling animation has stopped
 *
 * scroll,drag,start - dragging the contents around has started
 *
 * scroll,drag,stop - dragging the contents around has stopped
 *
 */


typedef struct _Widget_Data Widget_Data;
typedef struct _Pan Pan;
typedef struct _Grid Grid;
typedef struct _Grid_Item Grid_Item;
typedef struct _Marker_Group Marker_Group;


#define SOURCE_PATH "http://tile.openstreetmap.org/%d/%d/%d.png"
#define DEST_DIR_ZOOM_PATH "/tmp/elm_map/%d/"
#define DEST_DIR_PATH DEST_DIR_ZOOM_PATH"%d/"
#define DEST_FILE_PATH "%s%d.png"

struct _Elm_Map_Marker
{
   Widget_Data *wd;
   Elm_Map_Marker_Class *class;
   double longitude, latitude;

   Evas_Coord map_size;
   Evas_Coord x[19], y[19];
   void *data;

   Evas_Object *content;
};

struct _Marker_Group
{
   Widget_Data *wd;
   Eina_List *markers;
   Evas_Coord x, y;
   Evas_Coord w, h;
   Evas_Object *obj, *bubble, *sc, *bx;
   Eina_Bool open : 1;
};

struct _Grid_Item
{
   Widget_Data *wd;
   Evas_Object *img;
   //Evas_Object *txt;
   const char *file;
   struct
     {
	int x, y, w, h;
     } src, out;
   Eina_Bool want : 1;
   Eina_Bool download : 1;
   Eina_Bool have : 1;
   Ecore_File_Download_Job *job;
   int try_num;
};

struct _Grid
{
   Widget_Data *wd;
   int tsize; // size of tile (tsize x tsize pixels)
   int zoom; // zoom level tiles want for optimal display (1, 2, 4, 8)
   int iw, ih; // size of image in pixels
   int w, h; // size of grid image in pixels (represented by grid)
   int gw, gh; // size of grid in tiles
   Eina_Matrixsparse *grid;
};

struct _Widget_Data
{
   Evas_Object *obj;
   Evas_Object *scr;
   Evas_Object *pan_smart;
   Evas_Object *rect;
   Pan *pan;
   Evas_Coord pan_x, pan_y, minw, minh;

   int zoom;
   Elm_Map_Zoom_Mode mode;

   Ecore_Job *calc_job;
   Ecore_Timer *scr_timer;
   Ecore_Timer *long_timer;
   Ecore_Animator *zoom_animator;
   double t_start, t_end;
   struct
     {
	int w, h;
	int ow, oh, nw, nh;
	struct
	  {
	     double x, y;
	  } spos;
     } size;
   int tsize;
   int nosmooth;
   int preload_num;
   Eina_List *grids;
   Eina_Bool resized : 1;
   Eina_Bool longpressed : 1;
   Eina_Bool on_hold : 1;
   Eina_Bool paused : 1;

   Eina_List *markers[19];
   Evas_Coord marker_w, marker_h;
   Evas_Coord marker_max_w, marker_max_h;
   int marker_zoom;
};

struct _Pan
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Widget_Data *wd;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _show_region_hook(void *data, Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _calc_job(void *data);
static void grid_place(Evas_Object *obj, Grid *g, Evas_Coord px, Evas_Coord py, Evas_Coord ox, Evas_Coord oy, Evas_Coord ow, Evas_Coord oh);
static void grid_clear(Evas_Object *obj, Grid *g);
static Grid *grid_create(Evas_Object *obj);
static void grid_load(Evas_Object *obj, Grid *g);


static void _group_open_cb(void *data, Evas_Object *obj, const char *emission, const char *soure);
static void _group_bubble_create(Marker_Group *group);
static void _group_bubble_free(Marker_Group *group);
static void _group_bubble_place(Marker_Group *group);

static void _group_bubble_content_update(Marker_Group *group);
static void _group_bubble_content_free(Marker_Group *group);
static void marker_place(Evas_Object *obj, Grid *g, Evas_Coord px, Evas_Coord py, Evas_Coord ox, Evas_Coord oy, Evas_Coord ow, Evas_Coord oh);

   static void
rect_place(Evas_Object *obj, Evas_Coord px, Evas_Coord py, Evas_Coord ox, Evas_Coord oy, Evas_Coord ow, Evas_Coord oh)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord ax, ay, gw, gh;
   int x, y;

   ax = 0;
   ay = 0;
   gw = wd->size.w;
   gh = wd->size.h;
   if (ow > gw) ax = (ow - gw) / 2;
   if (oh > gh) ay = (oh - gh) / 2;
   evas_object_move(wd->rect,
	 ox + 0 - px + ax,
	 oy + 0 - py + ay);
   evas_object_resize(wd->rect, gw, gh);
}

   static void
marker_place(Evas_Object *obj, Grid *g, Evas_Coord px, Evas_Coord py, Evas_Coord ox, Evas_Coord oy, Evas_Coord ow, Evas_Coord oh)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord ax, ay, gw, gh, tx, ty;
   Eina_List *l, *l2;
   Marker_Group *group;
   int x, y;
   int xx, yy, ww, hh;
   char buf[PATH_MAX];

   if(g != eina_list_data_get(wd->grids)) return ;

   ax = 0;
   ay = 0;
   gw = wd->size.w;
   gh = wd->size.h;
   if (ow > gw) ax = (ow - gw) / 2;
   if (oh > gh) ay = (oh - gh) / 2;

   if(wd->zoom != wd->marker_zoom)
     {
	EINA_LIST_FOREACH(wd->markers[wd->marker_zoom], l, group)
	  {
	     if(group->obj)
	       {
		  evas_object_del(group->obj);
		  group->obj = NULL;
		  group->open = EINA_FALSE;
		  _group_bubble_free(group);
	       }
	  }
     }
   wd->marker_zoom = wd->zoom;

   EINA_LIST_FOREACH(wd->markers[wd->zoom], l, group)
     {
	if(!group->markers) continue ;

	xx = group->x;
	yy = group->y;
	ww = group->w;
	hh = group->h;

	if(ww<=0) ww = 1;
	if(hh<=0) hh = 1;

	if ((gw != g->w) && (g->w > 0))
	  {
	     tx = xx;
	     xx = ((long long )gw * xx) / g->w;
	     ww = (((long long)gw * (tx + ww)) / g->w) - xx;
	  }
	if ((gh != g->h) && (g->h > 0))
	  {
	     ty = yy;
	     yy = ((long long)gh * yy) / g->h;
	     hh = (((long long)gh * (ty + hh)) / g->h) - yy;
	  }

	if(xx-px+ax >= ox && xx-px+ax<= ox+ow
	      && yy-py+ay >= oy && yy-py+ay<= oy+oh)
	  {
	     if(!group->obj)
	       {
		  group->obj = edje_object_add(evas_object_evas_get(obj));
		  _elm_theme_set(group->obj, "map", "marker", elm_widget_style_get(obj));

		  evas_object_smart_member_add(group->obj,
			wd->pan_smart);
		  elm_widget_sub_object_add(obj, group->obj);

		  edje_object_signal_callback_add(group->obj, "open", "elm", _group_open_cb, group);

		  if(group->open)
		    _group_bubble_create(group);
	       }

	     if(eina_list_count(group->markers) > 1)
	       {
		  snprintf(buf, PATH_MAX, "%d", eina_list_count(group->markers));
		  edje_object_part_text_set(group->obj, "elm.text", buf);
	       }
	     evas_object_move(group->obj,
		   xx - px + ax + ox - ww/2,
		   yy - py + ay + oy - hh/2);
	     evas_object_resize(group->obj, ww, hh);
	     evas_object_raise(group->obj);
	     evas_object_show(group->obj);

	     if(group->bubble)
	       _group_bubble_place(group);
	  }
	else if(group->obj)
	  {
	     evas_object_del(group->obj);
	     group->obj = NULL;
	     _group_bubble_free(group);
	  }
     }
}

   static void
grid_place(Evas_Object *obj, Grid *g, Evas_Coord px, Evas_Coord py, Evas_Coord ox, Evas_Coord oy, Evas_Coord ow, Evas_Coord oh)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord ax, ay, gw, gh, tx, ty;
   int xx, yy, ww, hh;

   ax = 0;
   ay = 0;
   gw = wd->size.w;
   gh = wd->size.h;
   if (ow > gw) ax = (ow - gw) / 2;
   if (oh > gh) ay = (oh - gh) / 2;

   Eina_Iterator *it = eina_matrixsparse_iterator_new(g->grid);
   Eina_Matrixsparse_Cell *cell;

   EINA_ITERATOR_FOREACH(it, cell)
     {
	Grid_Item *gi = eina_matrixsparse_cell_data_get(cell);

	xx = gi->out.x;
	yy = gi->out.y;
	ww = gi->out.w;
	hh = gi->out.h;
	if ((gw != g->w) && (g->w > 0))
	  {
	     tx = xx;
	     xx = ((long long )gw * xx) / g->w;
	     ww = (((long long)gw * (tx + ww)) / g->w) - xx;
	  }
	if ((gh != g->h) && (g->h > 0))
	  {
	     ty = yy;
	     yy = ((long long)gh * yy) / g->h;
	     hh = (((long long)gh * (ty + hh)) / g->h) - yy;
	  }
	evas_object_move(gi->img,
	      xx - px + ax + ox,
	      yy - py + ay + oy);

	evas_object_resize(gi->img, ww, hh);

	/*evas_object_move(gi->txt,
	  xx - px + ax + ox,
	  yy - py + ay + oy);

	  evas_object_resize(gi->txt, ww, hh);
	  */
     }
   eina_iterator_free(it);
}

   static void
grid_clear(Evas_Object *obj, Grid *g)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char buf[PATH_MAX];

   if (!g->grid) return;

   Eina_Iterator *it = eina_matrixsparse_iterator_new(g->grid);
   Eina_Matrixsparse_Cell *cell;

   snprintf(buf, PATH_MAX, DEST_DIR_ZOOM_PATH, g->zoom);
   ecore_file_recursive_rm(buf);

   EINA_ITERATOR_FOREACH(it, cell)
     {
	Grid_Item *gi = eina_matrixsparse_cell_data_get(cell);
	evas_object_del(gi->img);
	//evas_object_del(gi->txt);

	if (gi->want)
	  {
	     gi->want = EINA_FALSE;
	     wd->preload_num--;
	     if (wd->preload_num == 0)
	       {
		  edje_object_signal_emit(elm_smart_scroller_edje_object_get(wd->scr),
			"elm,state,busy,stop", "elm");
		  evas_object_smart_callback_call(obj, "loaded,detail", NULL);
	       }
	  }

	if(gi->job)
	  {
	     DBG("DOWNLOAD abort %s", gi->file);
	     ecore_file_download_abort(gi->job);
	     remove(gi->file);
	     gi->job = NULL;
	  }
	if(gi->file)
	  eina_stringshare_del(gi->file);

	free(gi);
     }
   eina_matrixsparse_free(g->grid);
   eina_iterator_free(it);
   g->grid = NULL;
   g->gw = 0;
   g->gh = 0;
}

   static void
_tile_update(Grid_Item *gi)
{
   gi->want = EINA_FALSE;
   gi->download = EINA_FALSE;
   evas_object_image_file_set(gi->img, gi->file, NULL);
   evas_object_show(gi->img);

   //evas_object_text_text_set(gi->txt, gi->file);
   //evas_object_show(gi->txt);

   gi->have = EINA_TRUE;
   gi->wd->preload_num--;
   if (gi->wd->preload_num == 0)
     {
	edje_object_signal_emit(elm_smart_scroller_edje_object_get(gi->wd->scr),
	      "elm,state,busy,stop", "elm");
	evas_object_smart_callback_call(gi->wd->obj, "loaded,detail", NULL);
     }
}


   static void
_tile_downloaded(void *data, const char *file, int status)
{
   Grid_Item *gi = data;

   gi->download = EINA_FALSE;
   gi->job = NULL;

   DBG("DOWNLOAD done %s", gi->file);
   if (gi->want && !status)
     _tile_update(gi);

   if(status)
     DBG("Download failed (%d) %s", status, gi->file);
}

   static Grid *
grid_create(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int x, y;
   Grid *g;

   g = calloc(1, sizeof(Grid));

   g->zoom = wd->zoom;
   g->tsize = wd->tsize;
   g->wd = wd;

   if (g->zoom > 18) return NULL;

   int size =  pow(2.0, wd->zoom);
   g->gw = size;
   g->gh = size;

   g->w = g->tsize * g->gw;
   g->h = g->tsize * g->gh;

   g->grid = eina_matrixsparse_new(g->gh, g->gw, NULL, NULL);

   return g;
}

   static void
grid_load(Evas_Object *obj, Grid *g)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int x, y;
   int size;
   Evas_Coord ow, oh, tx, ty, gw, gh, xx, yy, ww, hh;
   Eina_Iterator *it;
   Eina_Matrixsparse_Cell *cell;
   Grid_Item *gi;

   elm_smart_scroller_child_viewport_size_get(wd->scr, &ow, &oh);

   gw = wd->size.w;
   gh = wd->size.h;

   if(gw <= 0 || gh <= 0) return ;

   size = g->tsize;
   if ((gw != g->w) && (g->w > 0))
     size = ((long long)gw * size) / g->w;
   if(size < g->tsize / 2) return; // else we will load to much tiles


   it = eina_matrixsparse_iterator_new(g->grid);

   EINA_ITERATOR_FOREACH(it, cell)
     {
	Grid_Item *gi = eina_matrixsparse_cell_data_get(cell);

	xx = gi->out.x;
	yy = gi->out.y;
	ww = gi->out.w;
	hh = gi->out.h;

	if ((gw != g->w) && (g->w > 0))
	  {
	     tx = xx;
	     xx = ((long long )gw * xx) / g->w;
	     ww = (((long long)gw * (tx + ww)) / g->w) - xx;
	  }
	if ((gh != g->h) && (g->h > 0))
	  {
	     ty = yy;
	     yy = ((long long)gh * yy) / g->h;
	     hh = (((long long)gh * (ty + hh)) / g->h) - yy;
	  }

	if(! ELM_RECTS_INTERSECT( wd->pan_x, wd->pan_y, ow, oh,
		 xx, yy, ww, hh) )
	  {
	     if (gi->want)
	       {
		  wd->preload_num--;
		  if (wd->preload_num == 0)
		    {
		       edje_object_signal_emit(elm_smart_scroller_edje_object_get(wd->scr),
			     "elm,state,busy,stop", "elm");
		       evas_object_smart_callback_call(obj, "loaded,detail", NULL);
		    }
		  evas_object_hide(gi->img);
		  //evas_object_hide(gi->txt);
		  evas_object_image_file_set(gi->img, NULL, NULL);
		  gi->want = EINA_FALSE;
		  gi->have = EINA_FALSE;

		  if(gi->job)
		    {
		       DBG("DOWNLOAD abort %s", gi->file);
		       ecore_file_download_abort(gi->job);
		       remove(gi->file);
		       gi->job = NULL;
		    }
		  gi->download = EINA_FALSE;
	       }
	     else if (gi->have)
	       {
		  evas_object_hide(gi->img);
		  //evas_object_hide(gi->txt);
		  evas_object_image_preload(gi->img, 1);
		  evas_object_image_file_set(gi->img, NULL, NULL);
		  gi->have = EINA_FALSE;
		  gi->want = EINA_FALSE;
	       }
	  }
     }
   eina_iterator_free(it);

   xx = wd->pan_x / size - 1;
   if(xx < 0) xx = 0;

   yy = wd->pan_y / size - 1;
   if(yy < 0) yy = 0;

   ww =  ow / size + 2;
   if(xx + ww > g->gw) ww = g->gw - xx;

   hh =  oh / size + 2;
   if(yy + hh > g->gh) hh = g->gh - yy;

   for (y = yy; y < yy + hh; y++)
     {
	for (x = xx; x < xx + ww; x++)
	  {
	     gi = eina_matrixsparse_data_idx_get(g->grid, y, x);

	     if(!gi && g != eina_list_data_get(wd->grids))
	       continue;

	     if(!gi)
	       {
		  gi = calloc(1, sizeof(Grid_Item));
		  gi->src.x = x * g->tsize;
		  gi->src.y = y * g->tsize;
		  gi->src.w = g->tsize;
		  gi->src.h = g->tsize;

		  gi->out.x = gi->src.x;
		  gi->out.y = gi->src.y;
		  gi->out.w = gi->src.w;
		  gi->out.h = gi->src.h;

		  gi->wd = wd;

		  gi->img = evas_object_image_add(evas_object_evas_get(obj));
		  evas_object_image_scale_hint_set
		     (gi->img, EVAS_IMAGE_SCALE_HINT_DYNAMIC);
		  evas_object_image_filled_set(gi->img, 1);

		  evas_object_smart_member_add(gi->img,
			wd->pan_smart);
		  elm_widget_sub_object_add(obj, gi->img);
		  evas_object_pass_events_set(gi->img, 1);

		  /*gi->txt = evas_object_text_add(evas_object_evas_get(obj));
		    evas_object_text_font_set(gi->txt, "Vera", 12);
		    evas_object_color_set(gi->txt, 100, 100, 100, 255);
		    evas_object_smart_member_add(gi->txt,
		    wd->pan_smart);
		    elm_widget_sub_object_add(obj, gi->txt);
		    evas_object_pass_events_set(gi->txt, 1);
		    */
		  eina_matrixsparse_data_idx_set(g->grid, y, x, gi);
	       }

	     if (!gi->have && !gi->download)
	       {
		  char buf[PATH_MAX], buf2[PATH_MAX];

		  gi->want = EINA_TRUE;

		  snprintf(buf, PATH_MAX, DEST_DIR_PATH, g->zoom, x);
		  if(!ecore_file_exists(buf))
		    ecore_file_mkpath(buf);

		  snprintf(buf2, PATH_MAX, DEST_FILE_PATH, buf, y);

		  snprintf(buf, PATH_MAX, SOURCE_PATH,
			g->zoom, x, y);

		  if(gi->file)
		    eina_stringshare_del(gi->file);
		  gi->file = eina_stringshare_add(buf2);

		  if(ecore_file_exists(buf2) || g == eina_list_data_get(wd->grids))
		    {
		       gi->download = EINA_TRUE;
		       wd->preload_num++;
		       if (wd->preload_num == 1)
			 {
			    edje_object_signal_emit(elm_smart_scroller_edje_object_get(wd->scr),
				  "elm,state,busy,start", "elm");
			    evas_object_smart_callback_call(obj, "load,detail", NULL);
			 }

		       if(ecore_file_exists(buf2))
			 _tile_update(gi);
		       else
			 {
			    DBG("DOWNLOAD %s \t in %s", buf, buf2);
			    ecore_file_download(buf, buf2, _tile_downloaded, NULL, gi, &(gi->job));
			    if(!gi->job)
			      DBG("Can't start to download %s", buf);
			 }
		    }
	       }
	     else if(gi->have)
	       evas_object_show(gi->img);
	  }
     }
}

   static void
grid_clearall(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Grid *g;

   EINA_LIST_FREE(wd->grids, g)
     {
	grid_clear(obj, g);
	free(g);
     }
}

   static void
_smooth_update(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Grid *g;

   EINA_LIST_FOREACH(wd->grids, l, g)
     {
	Eina_Iterator *it = eina_matrixsparse_iterator_new(g->grid);
	Eina_Matrixsparse_Cell *cell;

	EINA_ITERATOR_FOREACH(it, cell)
	  {
	     Grid_Item *gi = eina_matrixsparse_cell_data_get(cell);
	     evas_object_image_smooth_scale_set(gi->img, (wd->nosmooth == 0));
	  }
	eina_iterator_free(it);
     }
}

   static void
_grid_raise(Grid *g)
{
   Eina_Iterator *it = eina_matrixsparse_iterator_new(g->grid);
   Eina_Matrixsparse_Cell *cell;

   g->wd->size.w = g->w;
   g->wd->size.h = g->h;

   EINA_ITERATOR_FOREACH(it, cell)
     {
	Grid_Item *gi = eina_matrixsparse_cell_data_get(cell);
	evas_object_raise(gi->img);
	//evas_object_raise(gi->txt);
     }
   eina_iterator_free(it);
}

   static int
_scr_timeout(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->nosmooth--;
   if (wd->nosmooth == 0) _smooth_update(data);
   wd->scr_timer = NULL;
   return 0;
}

   static void
_scr(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd->scr_timer)
     {
	wd->nosmooth++;
	if (wd->nosmooth == 1) _smooth_update(data);
     }
   if (wd->scr_timer) ecore_timer_del(wd->scr_timer);
   wd->scr_timer = ecore_timer_add(0.5, _scr_timeout, data);
}

   static int
zoom_do(Evas_Object *obj, double t)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord xx, yy, ow, oh;

   wd->size.w = (wd->size.ow * (1.0 - t)) + (wd->size.nw * t);
   wd->size.h = (wd->size.oh * (1.0 - t)) + (wd->size.nh * t);

   elm_smart_scroller_child_viewport_size_get(wd->scr, &ow, &oh);

   xx = (wd->size.spos.x * wd->size.w) - (ow / 2);
   yy = (wd->size.spos.y * wd->size.h) - (oh / 2);
   if (xx < 0) xx = 0;
   else if (xx > (wd->size.w - ow)) xx = wd->size.w - ow;
   if (yy < 0) yy = 0;
   else if (yy > (wd->size.h - oh)) yy = wd->size.h - oh;

   elm_smart_scroller_child_region_show(wd->scr, xx, yy, ow, oh);
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
   if (t >= 1.0)
     return 0;
   return 1;
}


   static int
_zoom_anim(void *data)
{
   Evas_Object *obj = data;
   Widget_Data *wd = elm_widget_data_get(obj);
   double t;
   int go;

   t = ecore_loop_time_get();
   if (t >= wd->t_end)
     t = 1.0;
   else if (wd->t_end > wd->t_start)
     t = (t - wd->t_start) / (wd->t_end - wd->t_start);
   else
     t = 1.0;
   t = 1.0 - t;
   t = 1.0 - (t * t);
   go = zoom_do(obj, t);
   if (!go)
     {
	wd->nosmooth--;
	if (wd->nosmooth == 0) _smooth_update(data);
	wd->zoom_animator = NULL;
	evas_object_smart_callback_call(obj, "zoom,stop", NULL);
     }
   return go;
}

   static void
_mouse_move(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Move *ev = event_info;
}

   static int
_long_press(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->long_timer = NULL;
   wd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(data, "longpressed", NULL);
   return 0;
}

   static void
_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Down *ev = event_info;
   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) wd->on_hold = EINA_TRUE;
   else wd->on_hold = EINA_FALSE;
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(data, "clicked,double", NULL);
   else
     evas_object_smart_callback_call(data, "press", NULL);
   wd->longpressed = EINA_FALSE;
   if (wd->long_timer) ecore_timer_del(wd->long_timer);
   wd->long_timer = ecore_timer_add(1.0, _long_press, data);
}

   static void
_mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Up *ev = event_info;
   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) wd->on_hold = EINA_TRUE;
   else wd->on_hold = EINA_FALSE;
   if (wd->long_timer)
     {
	ecore_timer_del(wd->long_timer);
	wd->long_timer = NULL;
     }
   if (!wd->on_hold)
     evas_object_smart_callback_call(data, "clicked", NULL);
   wd->on_hold = EINA_FALSE;
}

static Evas_Smart_Class _pan_sc = EVAS_SMART_CLASS_INIT_NULL;

   static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (wd->calc_job) ecore_job_del(wd->calc_job);
   if (wd->scr_timer) ecore_timer_del(wd->scr_timer);
   if (wd->zoom_animator) ecore_animator_del(wd->zoom_animator);
   if (wd->long_timer) ecore_timer_del(wd->long_timer);

   free(wd);
}

   static void
_del_pre_hook(Evas_Object *obj)
{
   Marker_Group *group;
   Elm_Map_Marker *marker;
   int i;
   Eina_Bool free_marker = EINA_TRUE;

   Widget_Data *wd = elm_widget_data_get(obj);
   grid_clearall(obj);
   
   for(i=0; i<19; i++)
     {
	EINA_LIST_FREE(wd->markers[i], group)
	  {
	     EINA_LIST_FREE(group->markers, marker)
	       {
		  if(free_marker)
		    free(marker);
	       }
	     free(group);
	  }
	free_marker = EINA_FALSE;
     }


   evas_object_del(wd->pan_smart);
   wd->pan_smart = NULL;
}

   static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_smart_scroller_theme_set(wd->scr, "map", "base", elm_widget_style_get(obj));
   edje_object_scale_set(wd->scr, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

   static void
_show_region_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord x, y, w, h;
   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

   static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   //   evas_object_size_hint_min_get(wd->scr, &minw, &minh);
   evas_object_size_hint_max_get(wd->scr, &maxw, &maxh);
   //   minw = -1;
   //   minh = -1;
   //   if (wd->mode != ELM_LIST_LIMIT) minw = -1;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

   static void
_calc_job(void *data)
{
   Widget_Data *wd = data;
   Evas_Coord minw, minh;

   minw = wd->size.w;
   minh = wd->size.h;

   if (wd->resized)
     {
	wd->resized = 0;
	if (wd->mode != ELM_PHOTOCAM_ZOOM_MODE_MANUAL)
	  {
	     double tz = wd->zoom;
	     wd->zoom = 0.0;
	     elm_photocam_zoom_set(wd->obj, tz);
	  }
     }
   if ((minw != wd->minw) || (minh != wd->minh))
     {
	wd->minw = minw;
	wd->minh = minh;
	evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
	_sizing_eval(wd->obj);
     }
   wd->calc_job = NULL;
   evas_object_smart_changed(wd->pan_smart);
}

   static void
_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if ((x == sd->wd->pan_x) && (y == sd->wd->pan_y)) return;
   sd->wd->pan_x = x;
   sd->wd->pan_y = y;
   evas_object_smart_changed(obj);
}

   static void
_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (x) *x = sd->wd->pan_x;
   if (y) *y = sd->wd->pan_y;
}

   static void
_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = sd->wd->minw - ow;
   if (ow < 0) ow = 0;
   oh = sd->wd->minh - oh;
   if (oh < 0) oh = 0;
   if (x) *x = ow;
   if (y) *y = oh;
}

   static void
_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (w) *w = sd->wd->minw;
   if (h) *h = sd->wd->minh;
}

   static void
_pan_add(Evas_Object *obj)
{
   Pan *sd;
   Evas_Object_Smart_Clipped_Data *cd;

   _pan_sc.add(obj);
   cd = evas_object_smart_data_get(obj);
   sd = calloc(1, sizeof(Pan));
   if (!sd) return;
   sd->__clipped_data = *cd;
   free(cd);
   evas_object_smart_data_set(obj, sd);
}

   static void
_pan_del(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   _pan_sc.del(obj);
}

   static void
_pan_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   sd->wd->resized = 1;
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = ecore_job_add(_calc_job, sd->wd);
}

   static void
_pan_calculate(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ox, oy, ow, oh;
   Eina_List *l;
   Grid *g;

   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   rect_place(sd->wd->obj, sd->wd->pan_x, sd->wd->pan_y, ox, oy, ow, oh);
   EINA_LIST_FOREACH(sd->wd->grids, l, g)
     {
	grid_load(sd->wd->obj, g);
	grid_place(sd->wd->obj, g, sd->wd->pan_x, sd->wd->pan_y, ox, oy, ow, oh);
	marker_place(sd->wd->obj, g, sd->wd->pan_x, sd->wd->pan_y, ox, oy, ow, oh);
     }
}

   static void
_hold_on(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 1);
}

   static void
_hold_off(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 0);
}

   static void
_freeze_on(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 1);
}

   static void
_freeze_off(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 0);
}

   static void
_scr_anim_start(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "scroll,anim,start", NULL);
}

   static void
_scr_anim_stop(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "scroll,anim,stop", NULL);
}

   static void
_scr_drag_start(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "scroll,drag,start", NULL);
}

   static void
_scr_drag_stop(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "scroll,drag,stop", NULL);
}

   static void
_scr_scroll(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "scroll", NULL);
}

   static void
_group_bubble_create(Marker_Group *group)
{
   if(group->bubble) return ;

   group->bubble = edje_object_add(evas_object_evas_get(group->obj));
   _elm_theme_set(group->bubble, "map", "marker_bubble",
	 elm_widget_style_get(group->wd->obj));
   evas_object_smart_member_add(group->bubble,
	 group->wd->obj);
   elm_widget_sub_object_add(group->wd->obj, group->bubble);

   _group_bubble_content_update(group);

   _group_bubble_place(group);
}

   static void
_group_bubble_mouse_in_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Marker_Group *group = data;
   evas_object_raise(group->bubble);
   evas_object_raise(group->sc);
}

   static void
_group_bubble_content_update(Marker_Group *group)
{
   Eina_List *l;
   Elm_Map_Marker *marker;

   if(!group->bubble) return ;

   _group_bubble_content_free(group);

   if(!group->sc)
     {
	group->sc = elm_scroller_add(group->bubble);
	elm_scroller_content_min_limit(group->sc, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(group->sc, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_bounce_set(group->sc, EINA_TRUE, EINA_FALSE);
	edje_object_part_swallow(group->bubble, "elm.swallow.content", group->sc);
	evas_object_show(group->sc);
	evas_object_smart_member_add(group->sc,
	      group->wd->obj);
	elm_widget_sub_object_add(group->wd->obj, group->sc);

	group->bx = elm_box_add(group->bubble);
	evas_object_size_hint_align_set(group->bx, -1, -1);
	evas_object_size_hint_weight_set(group->bx, 0.5, 0.5);
	elm_box_horizontal_set(group->bx, EINA_TRUE);
	evas_object_show(group->bx);

	evas_object_event_callback_add(group->bx, EVAS_CALLBACK_MOUSE_IN, _group_bubble_mouse_in_cb, group);
	evas_object_event_callback_add(group->bx, EVAS_CALLBACK_MOUSE_DOWN, _group_bubble_mouse_in_cb, group);

	elm_scroller_content_set(group->sc, group->bx);
     }

   EINA_LIST_FOREACH(group->markers, l, marker)
     {
	if(marker->class->func.get)
	  marker->content = marker->class->func.get(group->wd->obj, marker, marker->data);

	if(marker->content)
	   elm_box_pack_end(group->bx, marker->content);
     }
}

   static void
_group_bubble_content_free(Marker_Group *group)
{
   Eina_List *l;
   Elm_Map_Marker *marker;

   if(!group->sc) return ;

   EINA_LIST_FOREACH(group->markers, l, marker)
     {
	if(marker->content && marker->class->func.del)
	  marker->class->func.del(group->wd->obj, marker, marker->data, marker->content);
	else if(marker->content)
	  evas_object_del(marker->content);
     }

   evas_object_del(group->sc);
   group->sc = NULL;
}

   static void
_group_bubble_free(Marker_Group *group)
{
   if(!group->bubble) return ;
   evas_object_del(group->bubble);
   group->bubble = NULL;
   _group_bubble_content_free(group);
}

   static void
_group_bubble_place(Marker_Group *group)
{
   Evas_Coord x, y, w, h;
   Evas_Coord xx, yy, ww, hh;
   const char *s;

   if(!group->bubble) return ;

   evas_object_geometry_get(group->obj, &x, &y, &w, &h);
   s = edje_object_data_get(group->bubble, "size_w");
   ww = atoi(s);
   s = edje_object_data_get(group->bubble, "size_h");
   hh = atoi(s);
   xx = x+w/2-ww/2;
   yy = y-hh;

   evas_object_move(group->bubble, xx, yy);
   evas_object_resize(group->bubble, ww, hh);
   evas_object_show(group->bubble);
}


static void
_group_open_cb(void *data, Evas_Object *obj, const char *emission, const char *soure)
{
   Marker_Group *group = data;

   if(group->bubble)
     {
	group->open = EINA_FALSE;
	_group_bubble_free(group);
	return ;
     }
   group->open = EINA_TRUE;
   _group_bubble_create(group);
}


/**
 * Add a new Map object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Map
 */
   EAPI Evas_Object *
elm_map_add(Evas_Object *parent)
{
   Evas_Object *obj, *o;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord minw, minh;
   static Evas_Smart *smart = NULL;
   int i;
   const char *s;
   Grid *g;


   if(!ecore_file_download_protocol_available("http://"))
     {
	ERR("Ecore must be built with the support of HTTP for the widget map !");
	return NULL;
     }

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "map");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_theme_set(wd->scr, "map", "base", "default");
   evas_object_smart_callback_add(wd->scr, "scroll", _scr, obj);
   evas_object_smart_callback_add(wd->scr, "drag", _scr, obj);
   elm_widget_resize_object_set(obj, wd->scr);

   evas_object_smart_callback_add(wd->scr, "animate,start", _scr_anim_start, obj);
   evas_object_smart_callback_add(wd->scr, "animate,stop", _scr_anim_stop, obj);
   evas_object_smart_callback_add(wd->scr, "drag,start", _scr_drag_start, obj);
   evas_object_smart_callback_add(wd->scr, "drag,stop", _scr_drag_stop, obj);
   evas_object_smart_callback_add(wd->scr, "scroll", _scr_scroll, obj);

   elm_smart_scroller_bounce_allow_set(wd->scr, 1, 1);

   wd->obj = obj;

   //load the minimum size of the marker group
   o = edje_object_add(e);
   _elm_theme_set(o, "map", "marker", elm_widget_style_get(obj));
   s = edje_object_data_get(o, "size_w");
   wd->marker_w = atoi(s);
   s = edje_object_data_get(o, "size_h");
   wd->marker_h = atoi(s);
   s = edje_object_data_get(o, "size_max_w");
   wd->marker_max_w = atoi(s);
   s = edje_object_data_get(o, "size_max_h");
   wd->marker_max_h = atoi(s);
   evas_object_del(o);

   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);

   if (!smart)
     {
	static Evas_Smart_Class sc;

	evas_object_smart_clipped_smart_set(&_pan_sc);
	sc = _pan_sc;
	sc.name = "elm_map_pan";
	sc.version = EVAS_SMART_CLASS_VERSION;
	sc.add = _pan_add;
	sc.del = _pan_del;
	sc.resize = _pan_resize;
	sc.calculate = _pan_calculate;
	smart = evas_smart_class_new(&sc);
     }
   if (smart)
     {
	wd->pan_smart = evas_object_smart_add(e, smart);
	wd->pan = evas_object_smart_data_get(wd->pan_smart);
	wd->pan->wd = wd;
     }

   elm_smart_scroller_extern_pan_set(wd->scr, wd->pan_smart,
	 _pan_set, _pan_get,
	 _pan_max_get, _pan_child_size_get);

   wd->rect = evas_object_rectangle_add(e);
   evas_object_event_callback_add(wd->rect, EVAS_CALLBACK_MOUSE_DOWN,
	 _mouse_down, obj);
   evas_object_event_callback_add(wd->rect, EVAS_CALLBACK_MOUSE_UP,
	 _mouse_up, obj);
   evas_object_event_callback_add(wd->rect, EVAS_CALLBACK_MOUSE_MOVE,
	 _mouse_move, obj);
   evas_object_smart_member_add(wd->rect, wd->pan_smart);
   elm_widget_sub_object_add(obj, wd->rect);
   evas_object_show(wd->rect);
   evas_object_color_set(wd->rect, 0, 0, 0, 0);



   wd->zoom = -1;
   wd->mode = ELM_PHOTOCAM_ZOOM_MODE_MANUAL;

   wd->tsize = 256;

   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr),
	 &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   wd->paused = EINA_TRUE;
   elm_map_zoom_set(obj, 0);
   wd->paused = EINA_FALSE;

   _sizing_eval(obj);

   wd->calc_job = ecore_job_add(_calc_job, wd);

   return obj;
}

/**
 * Set the zoom level of the map
 *
 * This sets the zoom level. 0 is the world map and 18 is the maximum zoom.
 *
 * @param obj The map object
 * @param zoom The zoom level to set
 *
 * @ingroup Map
 */
   EAPI void
elm_map_zoom_set(Evas_Object *obj, int zoom)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Grid *g, *g_zoom = NULL;
   Evas_Coord pw, ph, rx, ry, rw, rh;
   int z;
   int zoom_changed = 0, started = 0;

   if (zoom < 0 ) zoom = 0;
   if (zoom > 18) zoom = 18;
   if (zoom == wd->zoom) return;

   wd->zoom = zoom;
   wd->size.ow = wd->size.w;
   wd->size.oh = wd->size.h;
   elm_smart_scroller_child_pos_get(wd->scr, &rx, &ry);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &rw, &rh);

   if (wd->mode == ELM_MAP_ZOOM_MODE_MANUAL)
     {
	wd->size.nw = pow(2.0, wd->zoom) * wd->tsize;
	wd->size.nh = pow(2.0, wd->zoom) * wd->tsize;
     }
   else if (wd->mode == ELM_MAP_ZOOM_MODE_AUTO_FIT)
     {
	int p2w, p2h;
	int cumulw, cumulh;

	cumulw = wd->tsize;
	p2w = 0;
	while(cumulw <= rw)
	  {
	     p2w++;
	     cumulw*=2;
	  }
	p2w--;

	cumulh = wd->tsize;
	p2h = 0;
	while(cumulh <= rh)
	  {
	     p2h++;
	     cumulh*=2;
	  }
	p2h--;

	if(p2w < p2h)
	  z = p2w;
	else
	  z = p2h;

	wd->zoom = z;
	wd->size.nw = pow(2.0, wd->zoom) * wd->tsize;
	wd->size.nh = pow(2.0, wd->zoom) * wd->tsize;
     }
   else if (wd->mode == ELM_MAP_ZOOM_MODE_AUTO_FILL)
     {
	int p2w, p2h;
	int cumulw, cumulh;

	cumulw = wd->tsize;
	p2w = 0;
	while(cumulw <= rw)
	  {
	     p2w++;
	     cumulw*=2;
	  }
	p2w--;

	cumulh = wd->tsize;
	p2h = 0;
	while(cumulh <= rh)
	  {
	     p2h++;
	     cumulh*=2;
	  }
	p2h--;

	if(p2w > p2h)
	  z = p2w;
	else
	  z = p2h;

	wd->zoom = z;
	wd->size.nw = pow(2.0, wd->zoom) * wd->tsize;
	wd->size.nh = pow(2.0, wd->zoom) * wd->tsize;
     }

   if ((wd->size.w > 0) && (wd->size.h > 0))
     {
	wd->size.spos.x = (double)(rx + (rw / 2)) / (double)wd->size.w;
	wd->size.spos.y = (double)(ry + (rh / 2)) / (double)wd->size.h;
     }
   else
     {
	wd->size.spos.x = 0.5;
	wd->size.spos.y = 0.5;
     }
   if (rw > wd->size.w) wd->size.spos.x = 0.5;
   if (rh > wd->size.h) wd->size.spos.y = 0.5;
   if (wd->size.spos.x > 1.0) wd->size.spos.x = 1.0;
   if (wd->size.spos.y > 1.0) wd->size.spos.y = 1.0;

   EINA_LIST_FOREACH(wd->grids, l, g)
     {
	if (g->zoom == wd->zoom)
	  {
	     wd->grids = eina_list_remove(wd->grids, g);
	     wd->grids = eina_list_prepend(wd->grids, g);
	     _grid_raise(g);
	     goto done;
	  }
     }
   g = grid_create(obj);
   if (g)
     {
	if (eina_list_count(wd->grids) > 1)
	  {
	     g_zoom = eina_list_last(wd->grids)->data;
	     wd->grids = eina_list_remove(wd->grids, g_zoom);
	     grid_clear(obj, g_zoom);
	     free(g_zoom);
	  }
	wd->grids = eina_list_prepend(wd->grids, g);
     }
   else
     {
	EINA_LIST_FREE(wd->grids, g)
	  {
	     grid_clear(obj, g);
	     free(g);
	  }
     }
done:

   wd->t_start = ecore_loop_time_get();
   wd->t_end = wd->t_start + _elm_config->zoom_friction;

   if (wd->paused)
     {
	zoom_do(obj, 1.0);
     }
   else
     {
	if (!wd->zoom_animator)
	  {
	     wd->zoom_animator = ecore_animator_add(_zoom_anim, obj);
	     wd->nosmooth++;
	     if (wd->nosmooth == 1) _smooth_update(obj);
	     started = 1;
	  }
     }
   if (wd->zoom_animator)
     {
	if (!_zoom_anim(obj))
	  {
	     ecore_animator_del(wd->zoom_animator);
	     wd->zoom_animator = NULL;
	  }
     }
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
   if (!wd->paused)
     {
	if (started)
	  evas_object_smart_callback_call(obj, "zoom,start", NULL);
	if (!wd->zoom_animator)
	  evas_object_smart_callback_call(obj, "zoom,stop", NULL);
     }

   if (zoom_changed)
     evas_object_smart_callback_call(obj, "zoom,change", NULL);
}

/**
 * Get the zoom level of the photo
 *
 * This returns the current zoom level of the map object. Note that if
 * you set the fill mode to other than ELM_MAP_ZOOM_MODE_MANUAL
 * (which is the default), the zoom level may be changed at any time by the
 * map object itself to account for map size and map viewpoer size
 *
 * @param obj The map object
 * @return The current zoom level
 *
 * @ingroup Map
 */
   EAPI double
elm_map_zoom_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->zoom;
}

/**
 * Set the zoom mode
 *
 * This sets the zoom mode to manual or one of several automatic levels.
 * Manual (ELM_MAP_ZOOM_MODE_MANUAL) means that zoom is set manually by
 * elm_map_zoom_set() and will stay at that level until changed by code
 * or until zoom mode is changed. This is the default mode.
 * The Automatic modes will allow the map object to automatically
 * adjust zoom mode based on properties. ELM_MAP_ZOOM_MODE_AUTO_FIT) will
 * adjust zoom so the photo fits inside the scroll frame with no pixels
 * outside this area. ELM_MAP_ZOOM_MODE_AUTO_FILL will be similar but
 * ensure no pixels within the frame are left unfilled. Do not forget that the valid sizes are 2^zoom, consequently the map may be smaller than the scroller view.
 *
 * @param obj The map object
 * @param mode The desired mode
 *
 * @ingroup Map
 */
   EAPI void
elm_map_zoom_mode_set(Evas_Object *obj, Elm_Map_Zoom_Mode mode)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->mode == mode) return;
   wd->mode = mode;
     {
	double tz = wd->zoom;
	wd->zoom = 0.0;
	elm_map_zoom_set(wd->obj, tz);
     }
}

/**
 * Get the zoom mode
 *
 * This gets the current zoom mode of the map object
 *
 * @param obj The map object
 * @return The current zoom mode
 *
 * @ingroup Map
 */
   EAPI Elm_Map_Zoom_Mode
elm_map_zoom_mode_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->mode;
}

   EAPI void
elm_map_geo_region_bring_in(Evas_Object *obj, double lon, double lat)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int rx, ry, rw, rh;

   elm_map_utils_convert_geo_into_coord(lon, lat, wd->size.w, &rx, &ry);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &rw, &rh);

   rx = rx - rw/2;
   ry = ry - rh/2;

   if (wd->zoom_animator)
     {
	wd->nosmooth--;
	if (wd->nosmooth == 0) _smooth_update(obj);
	ecore_animator_del(wd->zoom_animator);
	wd->zoom_animator = NULL;
	zoom_do(obj, 1.0);
	evas_object_smart_callback_call(obj, "zoom,stop", NULL);
     }
   elm_smart_scroller_region_bring_in(wd->scr, rx, ry, rw, rh);
}

/**
 * Move the map to the current coordinates.
 *
 * This move the map to the current coordinates. The map will be centred on these coordinates.
 *
 * @param obj The map object
 * @param lat The latitude.
 * @param lon The longitude.
 *
 * @ingroup Map
 */
   EAPI void
elm_map_geo_region_show(Evas_Object *obj, double lon, double lat)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int rx, ry, rw, rh;

   elm_map_utils_convert_geo_into_coord(lon, lat, wd->size.w, &rx, &ry);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &rw, &rh);

   rx = rx - rw/2;
   ry = ry - rh/2;

   if (wd->zoom_animator)
     {
	wd->nosmooth--;
	ecore_animator_del(wd->zoom_animator);
	wd->zoom_animator = NULL;
	zoom_do(obj, 1.0);
	evas_object_smart_callback_call(obj, "zoom,stop", NULL);
     }
   elm_smart_scroller_child_region_show(wd->scr, rx, ry, rw, rh);
}

/**
 * Move the map to the current coordinates.
 *
 * This move the map to the current coordinates. The map will be centred on these coordinates.
 *
 * @param obj The map object
 * @param lat The latitude.
 * @param lon The longitude.
 *
 * @ingroup Map
 */
   EAPI void
elm_map_geo_region_get(Evas_Object *obj, double *lon, double *lat)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord sx, sy, sw, sh;
   int iw, ih;

   elm_smart_scroller_child_pos_get(wd->scr, &sx, &sy);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &sw, &sh);
   sx += sw/2;
   sy += sh/2;

   elm_map_utils_convert_coord_into_geo(sx, sy, wd->size.w, lon, lat);
}

/**
 * Set the paused state for map
 *
 * This sets the paused state to on (1) or off (0) for map. The default
 * is off. This will stop zooming using animation change zoom levels and
 * change instantly. This will stop any existing animations that are running.
 *
 * @param obj The map object
 * @param paused The pause state to set
 */
   EAPI void
elm_map_paused_set(Evas_Object *obj, Eina_Bool paused)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->paused == !!paused) return;
   wd->paused = paused;
   if (wd->paused)
     {
	if (wd->zoom_animator)
	  {
	     ecore_animator_del(wd->zoom_animator);
	     wd->zoom_animator = NULL;
	     zoom_do(obj, 1.0);
	     evas_object_smart_callback_call(obj, "zoom,stop", NULL);
	  }
     }
}

/**
 * Get the paused state for map
 *
 * This gets the current paused state for the map object.
 *
 * @param obj The map object
 * @return The current paused state
 */
   EAPI Eina_Bool
elm_map_paused_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->paused;
}


/**
 * Convert a pixel coordinate into a geographic coordinate.
 *
 * Convert a pixel coordinate (x,y) into a geographic coordinate (longitude, latitude).
 *
 * @param obj The map object
 * @param x the coordinate
 * @param y the coordinate
 * @param size the size in pixels of the map. The map is a square and generally his size is : pow(2.0, zoom)*256.
 * @param lon the longitude correspond to x
 * @param lat the latitude correspond to y
 */
   EAPI void
elm_map_utils_convert_coord_into_geo(int x, int y, int size, double *lon, double *lat)
{
   if (lon)
     {
	*lon = x / (double)size * 360.0 - 180;
     }
   if (lat)
     {
	double n = ELM_PI - 2.0 * ELM_PI * y / size;
	*lat = 180.0 / ELM_PI * atan(0.5 * (exp(n) - exp(-n)));
     }
}

/**
 * Convert a geographic coordinate into a pixel coordinate.
 *
 * Convert a geographic coordinate (longitude, latitude) into a pixel coordinate (x, y).
 *
 * @param obj The map object
 * @param lon the longitude
 * @param lat the latitude
 * @param size the size in pixels of the map. The map is a square and generally his size is : pow(2.0, zoom)*256.
 * @param x the coordinate correspond to the longitude
 * @param y the coordinate correspond to the latitude
 */
   EAPI void
elm_map_utils_convert_geo_into_coord(double lon, double lat, int size, int *x, int *y)
{
   if(x)
     *x = floor((lon + 180.0) / 360.0 * size);
   if(y)
     *y = floor((1.0 - log( tan(lat * ELM_PI/180.0) + 1.0 / cos(lat * ELM_PI/180.0)) / ELM_PI) / 2.0 * size);
}



/**
 * Add a marker on the map
 *
 * Add a marker on the map
 *
 * @param obj The map object
 * @param lon the longitude
 * @param lat the latitude
 * @param class the class to use
 * @param data the data passed to the callbacks
 */
   EAPI Elm_Map_Marker *
elm_map_marker_add(Evas_Object *obj, double lon, double lat, Elm_Map_Marker_Class *class, void *data)
{
   int i;
   Eina_List *l, *l2;
   Marker_Group *group;
   Elm_Map_Marker *_marker;
   Widget_Data *wd = elm_widget_data_get(obj);

   Elm_Map_Marker *marker = calloc(1, sizeof(Elm_Map_Marker));

   marker->wd = wd;
   marker->class = class;
   marker->longitude = lon;
   marker->latitude = lat;
   marker->data = data;

   for(i=0; i<=18; i++)
     {
	int sizew = wd->marker_w;;
	int sizeh = wd->marker_h;;
	if(sizew<=0) sizew = 2;
	if(sizeh<=0) sizeh = 2;
	if(sizew > wd->marker_max_w) sizew = wd->marker_max_w;
	if(sizeh > wd->marker_max_h) sizeh = wd->marker_max_h;

	elm_map_utils_convert_geo_into_coord(lon, lat, pow(2.0, i)*wd->tsize,
	      &(marker->x[i]), &(marker->y[i]));

	EINA_LIST_FOREACH(wd->markers[i], l, group)
	  {
	     if(ELM_RECTS_INTERSECT( marker->x[i]-sizew/4, marker->y[i]-sizeh/4, sizew, sizeh,
		      group->x-group->w/4, group->y-group->h/4, group->w, group->h))
	       {
		  group->x = marker->x[i];
		  group->y = marker->y[i];
		  EINA_LIST_FOREACH(group->markers, l2, _marker)
		    {
		       group->x+=_marker->x[i];
		       group->y+=_marker->y[i];
		    }
		  group->markers = eina_list_append(group->markers, marker);
		  group->x = group->x / eina_list_count(group->markers);
		  group->y = group->y / eina_list_count(group->markers);
		  group->w = sizew + sizew/8. * eina_list_count(group->markers);
		  group->h = sizeh + sizew/8. * eina_list_count(group->markers);
		  if(group->w > wd->marker_max_w) group->w = wd->marker_max_w;
		  if(group->h > wd->marker_max_h) group->h = wd->marker_max_h;

		  break;
	       }
	  }
	if(!group)
	  {
	     group = calloc(1, sizeof(Marker_Group));
	     group->wd = wd;
	     group->x = marker->x[i];
	     group->y = marker->y[i];
	     group->w = sizew;
	     group->h = sizeh;
	     group->markers = eina_list_append(group->markers, marker);
	     wd->markers[i] = eina_list_append(wd->markers[i], group);
	  }
     }

   if(wd->grids)
     {
	Evas_Coord ox, oy, ow, oh;
	evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
	marker_place(obj, eina_list_data_get(wd->grids), wd->pan_x, wd->pan_y, ox, oy, ow, oh);
     }

   return marker;
}

