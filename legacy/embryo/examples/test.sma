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
   printf("Testing switch and case statements...\n");
   
   new var = 4;
   
   switch (var) {
    case 0:
      printf("It's 0\n");
    case 1:
      printf("It's 1\n");
    case 2:
      printf("It's 2\n");
    case 3:
      printf("It's 3\n");
    case 4:
      printf("It's 4\n");
    case 5:
      printf("It's 5\n");
    case 6:
      printf("It's 6\n");
    case 7:
      printf("It's 7\n");
    default:
      printf("It's something else\n");
   }
   
   
   printf("\n\n");
   printf("The printf() call is a native exported function. This should work\n");
   printf("Calling testfn()...\n");
   
   new ret;
   ret = testfn(12345, "A Test String", 7);
   printf("Done. Return value of testfn() was %i\n", ret);
   
   printf("\n\n");
   
   return 7;
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
