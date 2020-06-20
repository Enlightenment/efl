#ifndef ECORE_EVAS_EXTN_H_
# define ECORE_EVAS_EXTN_H_

typedef struct _Ecore_Evas_Interface_Extn Ecore_Evas_Interface_Extn;

struct _Ecore_Evas_Interface_Extn
{
   Ecore_Evas_Interface base;

   void            (*data_lock)(Ecore_Evas *ee);
   void            (*data_unlock)(Ecore_Evas *ee);
   Eina_Bool       (*connect)(Ecore_Evas *ee, const char *svcname, int svcnum, Eina_Bool svcsys);
   Eina_Bool       (*listen)(Ecore_Evas *ee, const char *svcname, int svcnum, Eina_Bool svcsys);
};

#endif
