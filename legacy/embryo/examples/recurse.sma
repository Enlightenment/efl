#include "recurse.inc"

/* To compile: 
 * embryo_cc ./recurse.sma -o recurse.amx
 * 
 * To test run:
 * embryo ./recurse.amx
 */

main()
{
   new ret = 0;
   
   ret = rec1(1, "Recurse Test", 123);
   return ret;
}

public rec1(arg1, str[], arg2)
{
   new ret = 0;
   
   printf("rec1(arg1=%i, str='%s', arg2=%i);\n", arg1, str, arg2);
   ret = native_rec(arg1, str, arg2 * 2);
   return ret;
}
