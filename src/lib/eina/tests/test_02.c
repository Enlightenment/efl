
FUNC {
  Eina_List *list = NULL;
  list = eina_list_append(list, (void *)123L);
  FAIL_IF(eina_list_data_find_list(list, (void *)120L) != list);
}

