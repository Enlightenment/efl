#include <float>
#include "test.inc"

/* This is ugly - please ignore this code! */

/* To compile: 
 * embryo_cc ./test.sma -otest.amx
 * 
 * To test run:
 * embryo ./test.amx
 */

public global1 = 99;
public global2 = 77;

main()
{
   call("Main Test 1", "meh", 1);
   call("Main Test 2", "fkhwefkjwe", 7);
   call("Main Test 3", "ikkie", 88);
   call("Main Test 4", "MAH", 123);
   
   testfn(12345, "Panties!!!!", 7);
   
   return 77;
}

tester(arg1=0, str[]="", arg2=0)
{
   if (arg1 == 7) printf("arg1 == 7!!!\n");
   printf("  TESTER: arg1=%i str='%s' arg2=%i\n", arg1, str, arg2);
}

public testfn(arg1, str[], arg2)
{
   new a = 10;
   new Float:b = 20.0;
   
   printf("float test %i %f\n", a, b);

   new val;
   
   tester(77, "blahdieblah", 1024);
   if (arg1 == 7) printf("arg1 == 7!!!\n");
   printf("arg1=%i str='%s' arg2=%i\n", arg1, str, arg2);
   
   val = call("Test Test 1", "bim", 77);
   if (val == 10)
     call("Val is 10", "pfft", 111);
   else
     call("Val is NOT 10", "meh", 101010);
   return 5;
}

