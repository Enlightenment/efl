CC       = gcc
CFLAGS   = -O2 -std=gnu99 -Wall -Wextra
INCLUDE  = -I. -I../ -I../src/lib/eo/
CONFIG   = -DHAVE_CONFIG_H=1
EINA_CFLAGS = `pkg-config --cflags eina`
EINA_LIBS   = `pkg-config --libs eina`
SO_OBJS  = eo_ptr_indirection.o eo.o eo_base_class.o
BENCH_OBJS  = eo2_simple.o eo_simple.o eo2-bench.o

all: eo2-bench test

eo.o: ../src/lib/eo/eo.c ../src/lib/eo/Eo.h
	$(CC) $(CFLAGS) -fPIC -c $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $< -o $@

eo_ptr_indirection.o: ../src/lib/eo/eo_ptr_indirection.c ../src/lib/eo/Eo.h
	$(CC) $(CFLAGS) -fPIC -c $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $< -o $@

eo_base_class.o: ../src/lib/eo/eo_base_class.c ../src/lib/eo/Eo.h
	$(CC) $(CFLAGS) -fPIC -c $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $< -o $@

libeo2.so: $(SO_OBJS)
	$(CC) $(CFLAGS) -shared -Wl,-soname,libeo2.so $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $(SO_OBJS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $< -o $@

eo2-bench: libeo2.so $(BENCH_OBJS) ../src/lib/eo/Eo.h
	$(CC) $(CFLAGS) $(INCLUDE) $(CONFIG) $(EINA_LIBS) -L. -leo2 $(BENCH_OBJS) -o $@

.cc.o:
	g++ -O2 -std=c++0x -c $(INCLUDE) $(CONFIG) $(EINA_CFLAGS) $< -o $@

test: libeo2.so simplesignal.o ../src/lib/eo/Eo.h
	g++ -O2 -std=c++0x $(INCLUDE) $(CONFIG) $(EINA_LIBS) -L. -leo2 simplesignal.o -o $@

clean:
	rm -f *~ *.o eo2-bench test libeo2.so
