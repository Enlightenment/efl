
struct _Elm_Atspi_Action
{
   const char *name;
   const char *action;
   const char *param;
   Eina_Bool (*func)(Evas_Object *obj, const char *params);
};

typedef struct _Elm_Atspi_Action Elm_Atspi_Action;
