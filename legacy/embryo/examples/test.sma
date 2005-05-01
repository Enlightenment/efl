#include "test.inc"

/* This is ugly - please ignore this code! */

/* To compile: 
 * embryo_cc ./test.sma -o test.amx
 * 
 * To test run:
 * embryo ./test.amx
 */

public global1 = 99;
public global2 = 77;

main()
{
   printf("Testing switch and case statements...\n");
   
   new var;

   for (var = 1; var < 5; var++)
     {
	switch (var)
	  {
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
     }
   printf("\n\n");
   printf("Testing for loops...\n");
   for (var = 0; var < 10; var++)
     {
	printf("Var = %i\n", var);
     }
   
   printf("\n\n");
   printf("Testing recursion...\n");
   var = recurse(3);
   printf("var = %i\n", var);
   
   printf("\n\n");
   printf("Testing while loops...\n");
   var = 7;
   while (var > 1)
     {
	printf("var = %i\n", var);
	var--;
     }
   
   printf("\n\n");
   printf("Testing Float Math...\n");
   new Float:a;
   new Float:b;
   new Float:c;
   
   c = 10.5;
   printf("c = %f (should be 10.5)\n", c);
   a = 5.0;
   b = 2.345;
   c = a + b;
   printf("a = %f (should be 5.0)\n", a);
   printf("b = %f (should be 2.345)\n", b);
   printf("a + b = %f (should be 7.345)\n", c);
   a = 2.5;
   b = 3.5;
   c = a * b;
   printf("a = %f (should be 2.5)\n", a);
   printf("b = %f (should be 3.5)\n", b);
   printf("a 8 b = %f (should be 8.75)\n", c);
   a = 5.5;
   b = 1.5;
   c = a / b;
   printf("a = %f (should be 5.5)\n", a);
   printf("b = %f (should be 1.5)\n", b);
   printf("a / b = %f (should be 3.666666667)\n", c);
   
   printf("The printf() call is a native exported function. This should work\n");
   printf("Calling testfn()...\n");
   
   new ret;
   ret = testfn(12345, "A Test String", 7);
   printf("Done. Return value of testfn() was %i\n", ret);
   
   printf("Test varargs...\n");
   vargs(1, 2, "hello", "there", 8, 77, 5.0, 7.77);
   
   printf("\n\n");
   
   printf("native_rec(1, 77)\n");
   native_rec(1, "Smelly", 77);
   printf("native_rec() done\n");
   
   printf("atoi(\"7\") = %i\n", atoi("7"));
   
   return 7;
}

public rec1(arg1, str[], arg2)
{
  printf("arg1=%i str=%s arg2=%i\n", arg1, str, arg2);
  return native_rec(arg1 + 1, str, arg2);
}

vargs(a, b, ...)
{
   printf("ARGS...\n");
   printf("  a = %i\n", a);
   printf("  b = %i\n", b);
   for (new i = 2; i < numargs(); i++)
     {
	new val;
	new Float:fval;
	new str[100];
	
	printf("  GET ARG... %i\n", i);
	if (i < 4)
	  {
	     getsarg(i, str, sizeof(str));
	     printf("    ARG: %s [max %i]\n", str, sizeof(str));
	  }
	else if (i < 6)
	  {
	     val = getarg(i);
	     printf("    ARG: %i\n", val);
	  }
	else if (i < 8)
	  {
	     fval = getfarg(i);
	     printf("    FARG: %f\n", fval);
	  }
     }
   printf("ARGS DONE.\n");
}

recurse(val)
{
   printf("Recurse: val = %i\n", val);
   if (val >= 10) return val;
   return recurse(val + 1);
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
