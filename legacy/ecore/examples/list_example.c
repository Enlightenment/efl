#include <stdio.h>
#include <Ecore_Data.h>

void print_list (Ecore_List *list) {
  char *list_item;

  printf("--- Current List ---\n");
  ecore_list_goto_first(list);
  while((list_item = (char*)ecore_list_next(list)) != NULL) {
    printf("\t%s\n", list_item);
  }
}

int main (int argc, char *argv[]) {
  Ecore_List *list;
  char *first  = "first";
  char *second = "second";
  char *last   = "last";

  list = ecore_list_new();

  ecore_list_append(list, last);    // Insert
  ecore_list_prepend(list, first);  // Add to front
  ecore_list_goto_index(list, 1);   // counted from 0
  ecore_list_insert(list, second);  // Insert before item at index 2
  print_list(list);

  ecore_list_destroy(list);

  return 0;
}
