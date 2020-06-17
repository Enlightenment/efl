#include <stdio.h>

void *empty_class_get(void){
   return NULL;
}

void* efl_super(void *v){
  printf("asdfasdf %p\n", v);
  return v;
}
void efl_super2(void *v, const char *asdf){
  printf("asdfasdfasdf222 %p %s\n", v, asdf);
}

int main(void)
{
	efl_super2(efl_super(empty_class_get()), "asdf");
	return 0;
}
