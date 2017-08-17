#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_WIN_PROTECTED
#define EFL_UI_WIN_SOCKET_PROTECTED

#include <Elementary.h>

#define MY_CLASS EFL_UI_WIN_SOCKET_CLASS
#define MY_CLASS_NAME "Efl.Ui.Win_Socket"

typedef struct
{
} Efl_Ui_Win_Socket_Data;

EOLIAN static Efl_Object *
_efl_ui_win_socket_efl_object_finalize(Eo *obj, Efl_Ui_Win_Socket_Data *pd EINA_UNUSED)
{
   efl_ui_win_type_set(obj, ELM_WIN_SOCKET_IMAGE);
   obj = efl_finalize(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static Eina_Bool
_efl_ui_win_socket_socket_listen(Eo *obj, Efl_Ui_Win_Socket_Data *pd EINA_UNUSED, const char *svcname, int svcnum, Eina_Bool svcsys)
{
   Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));

   if (!ee) return EINA_FALSE;
   return ecore_evas_extn_socket_listen(ee, svcname, svcnum, svcsys);
}

#include "efl_ui_win_socket.eo.c"
