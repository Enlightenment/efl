#include "example.inc"

/* To compile: 
 * embryo_cc ./example.sma -oexample.amx
 * 
 * To test run:
 * embryo ./example.amx
 */

public global1 = 99;
public global2 = 77;

main()
{
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
   
   printf("arg1=%i str='%s' arg2=%i\n", arg1, str, arg2);
   
   printf("float test %i %f\n", a, b);
   
   tester(77, "blahdieblah", 1024);
   if (arg1 == 7) printf("arg1 == 7!!!\n");
   
   return 5;
}

