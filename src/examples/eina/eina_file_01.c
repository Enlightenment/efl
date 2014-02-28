//Compile with:
//gcc -g eina_file_01.c -o eina_file_01 `pkg-config --cflags --libs eina`

#include <stdio.h>
#include <Eina.h>

static void
_print_cb(const char *name, const char *path, void *data EINA_UNUSED)
{
   printf("file %s in %s\n", name, path);
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Eina_Iterator *it;
   const char *f_name;
   const Eina_File_Direct_Info *f_info;

   eina_init();

   eina_file_dir_list("/home/", EINA_FALSE, _print_cb, NULL);

   it = eina_file_ls("/home/");
   EINA_ITERATOR_FOREACH(it, f_name)
     {
        printf("%s\n", f_name);
        eina_stringshare_del(f_name);
     }
   eina_iterator_free(it);

   it = eina_file_stat_ls("/home/");
   EINA_ITERATOR_FOREACH(it, f_info)
     printf("%s if of type %d\n", f_info->path, f_info->type);
   eina_iterator_free(it);

   it = eina_file_direct_ls("/home/");
   EINA_ITERATOR_FOREACH(it, f_info)
     printf("%s if of type %d\n", f_info->path, f_info->type);
   eina_iterator_free(it);

   eina_shutdown();

   return 0;
}
