/**
* Eolian_Info is a tool aimed to show specific Eolian information about Eo classes.
* It enables easily graphical/text representation of chosen Eo classes.
* Eolian is used to get this information.
* The output is a .dot file, convertible to other formats with Graphviz.
* To get help about the supported commands run "$ eolian_info -h".
*
* Examples:
*
* To Get dot representation of inheritance tree of elm_layout.eo till depth 3.
* Using system Eo files (used when no include is defined).
* $ eolian_info -d 3 -o test.dot elm_layout.eo
*
* To Get dot representation of inheritors tree of elm_layout.eo and elm_button.eo till depth 3.
* $ eolian_info -d 3 -n -i -o test.dot elm_layout.eo Elm.Button
*
* To Get dot representation of inheritance tree of elm_layout.eo till depth 3.
* Using user defined Eo files dirs.
* $ eolian_info -d 3 -o test.dot -I /home/avilog/git/efl/efl/src/lib/ -I /home/avilog/git/efl/elementary/src/lib/ elm_layout.eo
*
* Other:
*
* If you have Graphviz installed, you can convert the dot file to many other formats:
* SVG:
*  dot -Tsvg test.dot -o test.svg
* PNG:
*   dot -Tpng test.dot -o test.png
*
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <getopt.h>
#include <Eina.h>
#include <Eolian.h>

int _eolian_info_log_dom;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eolian_info_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eolian_info_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eolian_info_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eolian_info_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_eolian_info_log_dom, __VA_ARGS__)

#define MAX_CHARS_FILTER 45

static void _file_filter(const Eolian_Class *ekl,  Eina_Strbuf *buf);
static void _event_filter(const Eolian_Class *ekl, Eina_Strbuf *buf);

FILE* _file = NULL;
Eina_Inlist *classes_edges_list = NULL;

typedef struct
{
   const char* name;
   void (*func)(const Eolian_Class*, Eina_Strbuf *);
   Eina_Bool active;
} _filter;

static _filter _filters[] = {
       {"events", _event_filter, EINA_FALSE},
       {"file", _file_filter, EINA_FALSE},
       {NULL, NULL, EINA_FALSE}
};

typedef struct
{
   EINA_INLIST;
   const char *class, *parent;
} _Class_Node;

static void
_file_filter(const Eolian_Class *ekl, Eina_Strbuf *buf)
{
   Eina_Stringshare* class_file  = eolian_class_file_get(ekl);

   eina_strbuf_append(buf, "Class file: ");
   eina_strbuf_append(buf, class_file);
}

static void
_event_filter(const Eolian_Class *ekl, Eina_Strbuf *buf)
{
   Eina_Iterator *lst_events = eolian_class_events_get(ekl);
   const Eolian_Event* event;
   const char* event_title = "Events: ";
   int dist_start = strlen(event_title)+3;

   if(lst_events){
        eina_strbuf_append(buf, event_title);
   }

   int chars = 0;
   EINA_ITERATOR_FOREACH(lst_events, event){
        const char* event_name = eolian_event_name_get(event);
        chars += strlen(event_name)+1;
        if(chars-3 + dist_start > MAX_CHARS_FILTER)
          {
             /* willl write in a new line */
             chars = strlen(event_name)+1;
             eina_strbuf_append(buf, "\\l");
             for (int i = 0;  i < dist_start;  i++, eina_strbuf_append(buf, "&nbsp;"));
          }
        eina_strbuf_append(buf, event_name);
        eina_strbuf_append(buf, " ");
   }

   eina_iterator_free(lst_events);
}

static void
_print_class(const char *class_name)
{
   const Eolian_Class *ekl = eolian_class_get_by_name(class_name);
   const char* desc = eolian_class_description_get(ekl);
   fprintf(_file, "\"%s\" [tooltip= \"%s\" ",
         class_name, ( desc ? eina_str_escape(desc) : class_name));

   _filter *current_f =  _filters;
   int i = 0;

   fprintf(_file, "label=\"{");

/* Allways print class name */
   fprintf(_file, "<f%d> ", i);
   i++;
   fprintf(_file, "%s\n", class_name);


   Eina_Strbuf *buf = eina_strbuf_new(); 

   while(current_f->name != NULL )
     {
        if(current_f->active)
          {
             current_f->func(ekl, buf);
             if(eina_strbuf_length_get(buf)>0){
                  if(i>0)
                     fprintf(_file, "|");
                  fflush(_file);
                  fprintf(_file, "<f%d> ", i);
                  i++;

                  fprintf(_file, "%s\\l", eina_strbuf_string_get(buf));
                  eina_strbuf_reset(buf);
             }

          }

        current_f++;
     }
   eina_strbuf_free(buf);
   fprintf(_file, "}\"]\n");
}

static void
_print_graph()
{
   _Class_Node *node = NULL;
   Eina_Inlist *nnode;
   Eina_Hash *classes_hash = NULL;
   classes_hash  = eina_hash_string_superfast_new(NULL);

   fprintf(_file, "digraph \"Best Eo Graph Ever\" { \n\
         fontname = \"Bitstream Vera Sans\" \n\
         fontsize = 8 \n\
         \n\
         node [ \n\
         fontname = \"Bitstream Vera Sans\"\n \
         fontsize = 8\n \
         shape = \"record\"\n \
         ]\n \
         \n\
         edge [\n\
         arrowtail = \"empty\" \n\
         ]\n");

   /* print classes info */
   EINA_INLIST_FOREACH_SAFE(classes_edges_list, nnode, node)
     {
        if(node->class && !eina_hash_find(classes_hash, node->class))
          {
             _print_class(node->class);
             eina_hash_add(classes_hash, node->class, node->class);
          }
        if(node->parent && !eina_hash_find(classes_hash, node->parent))
          {
             _print_class(node->parent);
             eina_hash_add(classes_hash, node->parent, node->parent);
          }
     }
   eina_hash_free(classes_hash);

   /* mark the roots */
   fprintf(_file, "{rank=same; ");
   EINA_INLIST_FOREACH_SAFE(classes_edges_list, nnode, node)
     {
        if(!node->parent)
          {
             fprintf(_file, "\"%s\" ", node->class);
          }
     }
    fprintf(_file, "}\n");

/*print classes edges */
   EINA_INLIST_FOREACH_SAFE(classes_edges_list, nnode, node)
     {
        if(node->parent && node->class)
          fprintf(_file, "\"%s\" -> \"%s\"\n [dir=back]",  node->parent, node->class);
     }

   fprintf(_file, "}\n");

   /* Delete the inlist */
   EINA_INLIST_FOREACH_SAFE(classes_edges_list, nnode, node)
     {
        classes_edges_list = eina_inlist_remove(classes_edges_list,
              EINA_INLIST_GET(node));

        free(node);
     }
}

static void
_add_class_edge(const char *class_name, const char *parent_name)
{
   _Class_Node* d;
   _Class_Node *node = NULL;
   Eina_Inlist *nnode;

   /* dont add double edges */
   if(parent_name)
      EINA_INLIST_FOREACH_SAFE(classes_edges_list, nnode, node)
        {
           if((node->parent && !strcmp(parent_name, node->parent)) &&
                 !strcmp(class_name, node->class))
              return;
        }

   d = malloc(sizeof(*d));
   d->parent = parent_name;
   d->class = class_name;

   classes_edges_list = eina_inlist_append(classes_edges_list, EINA_INLIST_GET(d));
}

static void
_inherit_tree_scope_rec(const Eolian_Class *ekl, int depth)
{
   if(depth<= 0 )
      return;

   Eina_Iterator *lst = eolian_class_inherits_get(ekl);
   const char* class_name = eolian_class_full_name_get(ekl);
   const char* ekl_name;

   EINA_ITERATOR_FOREACH(lst, ekl_name)
     {
        ekl=eolian_class_get_by_name(ekl_name);

        _add_class_edge(
              class_name, eolian_class_full_name_get(ekl));

        _inherit_tree_scope_rec(ekl, depth - 1);
     }
   eina_iterator_free(lst);
}

static void
_inherit_tree_scope(const Eolian_Class *ekl, int depth_max)
{
   if(depth_max<= 0 )
      return;

   _add_class_edge(
         eolian_class_full_name_get(ekl), NULL);

   _inherit_tree_scope_rec(ekl, depth_max);
}

static int
_inherits_from_scope_recur(const Eolian_Class *ekl,
      Eina_Hash *classes_hash, const char *des_class_name, int depth_max)
{
   const char *class_name = eolian_class_full_name_get(ekl);
   if(class_name == des_class_name)
      return 1;

   int min_found_depth = -1;
   Eina_Iterator *lst_i = eolian_class_inherits_get(ekl);
   const char *ekl_i;

   if(!lst_i) return -1;

   EINA_ITERATOR_FOREACH(lst_i, ekl_i)
     {
        ekl = eolian_class_get_by_name(ekl_i);
        void *hash = eina_hash_find(classes_hash, ekl);
        int depth_class = -1;
        if(!hash)
          {
             depth_class = _inherits_from_scope_recur(
                   ekl, classes_hash, des_class_name, depth_max);
             eina_hash_add(classes_hash, ekl, (void*)(uintptr_t)depth_class );

          }
        else  depth_class = (int)(uintptr_t)hash;

        if(depth_class>=0){
             if(depth_class<=depth_max){
                  _add_class_edge(
                        class_name, ekl_i);
             }
             if((min_found_depth == -1) ||(min_found_depth>depth_class))
                min_found_depth = depth_class;
        }
     }
   eina_iterator_free(lst_i);

   return min_found_depth>=0 ?  min_found_depth+1 : -1;
}

static void
_inherits_from_scope(const Eolian_Class *ekl, int depth_max)
{
   const char* class_name = eolian_class_full_name_get(ekl);
   if(depth_max<= 0 )
      return;

   Eina_Hash *classes_hash = eina_hash_string_superfast_new(NULL);

   _add_class_edge(
         class_name, NULL);

   Eina_Iterator *lst = eolian_all_classes_get();

   EINA_ITERATOR_FOREACH(lst, ekl)
     {
        if(!eina_hash_find(classes_hash, ekl))
          {
             _inherits_from_scope_recur(
                   ekl, classes_hash, class_name, depth_max);
          }
     }

   eina_iterator_free(lst);
}

static void
_print_help(const char *program)
{
   printf("Usage: %s [options] class_path/class_name...\n", program);
   printf("Options:\n");
   printf("  --help/-h                  Display this information\n");
   printf("  --no-inheritance/-n        By default, we create the Inheritance tree, this flag disables it\n");
   printf("  --inheritors/-i            Add the Inheritors tree (classes that inherit from this class)\n");
   printf("  --output/-o <outFile>      Set output filename to <outFile>\n");
   printf("  --depth/-d <deptth>        Set <depth> to be the the max depth (up and down from the class of the printed tree)\n");
   printf("  --include/-I <includeDir>  Add <includeDir> to the list of include\n" \
          "                             dirs for scanning. If no directory included we scan the system dir.\n");
   printf("  --filters/-f <filter1,...> Set filters (the info of the class that will be printed)\n" \
          "                             as a comma-separated list of:\n" \
          "                                ");
   _filter *current_f =  _filters;
   while(current_f->name != NULL ) printf("%s, ", current_f++->name);
   printf("\b\b  \n");

   printf("\nFor example:\n");
   printf("eo_info_cmd -d 3 -o test.dot -f events,file elm_layout.eo Elm.Button\n");
}

int main(int argc, char **argv)
{
   int ret = 1;
   int max_depth = 0;
   char *output = NULL, *filter = NULL;
   Eina_Bool help = EINA_FALSE, includes = EINA_FALSE;
   Eina_Bool no_inheritance = EINA_FALSE, inheritors = EINA_FALSE;
   Eina_Bool no_input = EINA_TRUE;

   eina_init();
   eolian_init();

   const char *log_dom = "eolian_info";
   _eolian_info_log_dom = eina_log_domain_register(log_dom, EINA_COLOR_GREEN);
   if (_eolian_info_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: %s", log_dom);
        goto end;
     }

   static struct option long_options[] =
     {
        /* These options set a flag. */
          {"help",       no_argument,         0, 'h'},
          {"no-inheritance", no_argument,   0, 'n'},
          {"inheritors", no_argument,   0, 'i'},
          {"output",    required_argument,   0, 'o'},
          {"depth",    required_argument,   0, 'd'},
          {"filters",    required_argument,   0, 'f'},
          {"Include",    required_argument,   0, 'I'},
          {0, 0, 0, 0}
     };
   int long_index = 0, opt;
   while ((opt = getopt_long(argc, argv,"hnio:d:f:I:", long_options, &long_index )) != -1)
     {
        switch (opt) {
           case 0: break;
           case 'h': help = EINA_TRUE; break;
           case 'o':
                     {
                        output = optarg;

                        break;
                     }
           case 'd':
                     {
                        max_depth = atoi(optarg);
                        break;
                     }
           case 'n':
                     {
                        no_inheritance = EINA_TRUE;

                        break;
                     }
           case 'i':
                     {
                        inheritors = EINA_TRUE;

                        break;
                     }
           case 'I':
                     {
                        includes = EINA_TRUE;
                        const char *dir = optarg;
                        if (!eolian_directory_scan(dir))//todo works even if dir dont exist
                          {
                             ERR("Failed to scan %s", dir);
                             goto end;
                          }
                        break;
                     }
           case 'f':
                     {
                        filter = optarg;

                        break;
                     }
           default: {
                         ERR("Unrecognized option %s\n", argv[optind-1]);
                         help = EINA_TRUE;
                    }
        }
     }
   int name_start = optind++, name_end = argc;

   if (help)
     {
        _print_help(argv[0]);
        goto end;
     }

   if(filter)
     {
        unsigned int elements = 0, i;
        char **f_names = eina_str_split_full(filter, ",", 100, &elements);

        for(i = 0; i < elements; i++)/* go over all given filter names*/
          {

             _filter *current_f =  _filters;

             while(current_f->name != NULL )
               {

                  if(!strcmp(current_f->name, f_names[i]))
                    {
                       current_f->active = EINA_TRUE;
                       break;
                    }

                  current_f++;
               }
          }
     }

   if(!includes) eolian_system_directory_scan();
   eolian_all_eo_files_parse();
   while(name_start < name_end)
     {
        char *class_path = argv[name_start++];
        const Eolian_Class *ekl = NULL;
        if(eina_str_has_extension(class_path, ".eo"))
          {
             ekl =  eolian_class_get_by_file(class_path);
          }
        else
          {
             ekl = eolian_class_get_by_name(class_path);
          }
        if(!ekl)
          {
             ERR("Class %s not found", class_path);
             goto end;
          }
        no_input = EINA_FALSE;

        if(!no_inheritance)
          {
             _inherit_tree_scope(ekl, max_depth);
          }
        if(inheritors)
          {
             _inherits_from_scope(ekl, max_depth);
          }

     }
   if (no_input)
     {
        ERR("No input files given");
        _print_help(argv[0]);
        goto end;
     }
   _file = stdout;
   if(output)
     {
        _file = fopen(output, "w");
        if (_file == NULL)
          {
             ERR("Error opening file!\n");
             goto end;
          }
     }
   _print_graph();
   if(output)
      fclose(_file);

end:

   eina_log_timing(_eolian_info_log_dom,
         EINA_LOG_STATE_START,
         EINA_LOG_STATE_SHUTDOWN);
   eina_log_domain_unregister(_eolian_info_log_dom);
   _eolian_info_log_dom = -1;

   eolian_shutdown();
   eina_shutdown();
   return ret;
}
