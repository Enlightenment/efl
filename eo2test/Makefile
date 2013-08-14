CC       = gcc
CFLAGS   = -O2 -std=gnu99 -Wall -Wextra
INCLUDE  = -I. -I../ -I../src/lib/eo/ -I../build
CONFIG   = -DHAVE_CONFIG_H=1
EINA_CFLAGS = `pkg-config --cflags eina`
EINA_LIBS   = `pkg-config --libs eina`
EO_SRC  = ../src/lib/eo/eo_ptr_indirection.c ../src/lib/eo/eo.c ../src/lib/eo/eo_base_class.c ../src/lib/eo/eo2_base_class.c
EO_OBJS  = eo_ptr_indirection.o eo.o eo_base_class.o eo2_base_class.o
BENCH_SRC  = eo2_simple.c eo_simple.c eo2_inherit.c eo_inherit.c eo2-bench.c
BENCH_OBJS  = eo2_simple.o eo_simple.o eo2_inherit.o eo_inherit.o eo2-bench.o

all: eo2-bench eo2-bench-static test

eo.o: ../src/lib/eo/eo.c ../src/lib/eo/Eo.h ../src/lib/eo/eo_private.h
	$(CC) $(CFLAGS) -fPIC -c $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $< -o $@

eo_ptr_indirection.o: ../src/lib/eo/eo_ptr_indirection.c ../src/lib/eo/Eo.h ../src/lib/eo/eo_private.h
	$(CC) $(CFLAGS) -fPIC -c $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $< -o $@

eo_base_class.o: ../src/lib/eo/eo_base_class.c ../src/lib/eo/Eo.h ../src/lib/eo/eo_private.h
	$(CC) $(CFLAGS) -fPIC -c $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $< -o $@

eo2_base_class.o: ../src/lib/eo/eo2_base_class.c ../src/lib/eo/Eo.h ../src/lib/eo/eo_private.h
	$(CC) $(CFLAGS) -fPIC -c $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $< -o $@

libeo2.so: $(EO_OBJS)
	$(CC) $(CFLAGS) -shared -Wl,-soname,libeo2.so $(INCLUDE) $(CONFIG) $(EO_OBJS) $(EINA_LIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $< -o $@

eo2-bench: libeo2.so $(BENCH_OBJS) ../src/lib/eo/Eo.h ../src/lib/eo/eo_private.h
	$(CC) $(CFLAGS) $(INCLUDE) $(CONFIG) $(BENCH_OBJS) $(EINA_LIBS) -L. -leo2  -o $@

eo2-bench-static: $(EO_OBJS) $(BENCH_SRC) ../src/lib/eo/Eo.h ../src/lib/eo/eo_private.h
	$(CC) $(CFLAGS) $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $(EINA_LIBS) $(EO_SRC) $(BENCH_SRC) -o $@

.cc.o:
	g++ -O2 -std=c++0x -c $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $< -o $@

test: libeo2.so simplesignal.o ../src/lib/eo/Eo.h ../src/lib/eo/eo_private.h
	g++ -O2 -std=c++0x $(INCLUDE) $(CONFIG) simplesignal.o $(EINA_LIBS) -L. -leo2 -o $@

clean:
	rm -f *~ *.o eo2-bench eo2-bench-static test libeo2.so
