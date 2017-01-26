set(DESCRIPTION "A small virtual machine engine and bytecode compiler")

set(LIBRARIES
  eina
  m
)

set(PUBLIC_HEADERS
  Embryo.h
)

set(SOURCES
  embryo_amx.c
  embryo_args.c
  embryo_float.c
  embryo_main.c
  embryo_private.h
  embryo_rand.c
  embryo_str.c
  embryo_time.c
)
