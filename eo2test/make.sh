gcc $CFLAGS -DHAVE_CONFIG_H=1 -Ibuild `pkg-config --cflags eina` -std=c99 *.c ../src/lib/eo/*.c -I../src/lib/eo -I../build `pkg-config --libs eina` -o eo2test
