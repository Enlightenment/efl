/* Float arithmetic
 *
 * (c) Copyright 1999, Artran, Inc.
 * Written by Greg Garner (gmg@artran.com)
 * Modified in March 2001 to include user defined
 * operators for the floating point functions.
 * (c) Copyright 2004, Carsten Haitzler
 * Modified March 2004 by Carsten Haitzler <raster@rasterman.com> to conform
 * to E coding style
 * Became default include for embryo...
 * Added string functions
 * Added rand functions
 * Added time functions
 *
 * This file is provided as is (no warranties).
 */
#if defined DEFAULT_INC
#endinput
#endif
#define DEFAULT_INC

#pragma rational Float

#define PI  3.1415926535897932384626433832795

/* Different methods of rounding */
enum Float_Round_Method
{
   ROUND, FLOOR, CEIL, TOZERO
};
/* different angle addressing modes (default is radians) */
enum Float_Angle_Mode
{
   RADIAN, DEGREES, GRADES
};

/* varags - get numebr of args to a function */
native numargs();
/* varags - get arg no "arg" */
native getarg(arg, index=0);
native getsarg(arg, buf[], buflen);
native Float:getfarg(arg, index=0);
/* varags - set arg no "arg" */
native setarg(arg, index=0, value);
native setfarg(arg, index=0, Float:value);

/* Convert a string into a floating point value */
native Float:atof(const string[]);
/* Return the fractional part of a float */
native Float:fract(Float:value);
/* Round a float into a integer value */
native       round(Float:value, Float_Round_Method:method=ROUND);
/* Return the square root of value, same as float_power(value, 0.5) */
native Float:sqrt(Float:value);
/* Return the value raised to the power of the exponent */
native Float:pow(Float:value, Float:exponent);
/* Return the logarithm */
native Float:log(Float:value, Float:base=10.0);
/* Return the sine, cosine or tangent. The input angle may be in radian*/
/* degrees or grades. */
native Float:sin(Float:value, Float_Angle_Mode:mode=RADIAN);
native Float:cos(Float:value, Float_Angle_Mode:mode=RADIAN);
native Float:tan(Float:value, Float_Angle_Mode:mode=RADIAN);
/* Return the absolute value */
native Float:abs(Float:value);
/* return integer from string */
native       atoi(str[]);
/* return 0 if string matches glob, non-zero otherwise */
native       fnmatch(glob[], str[]);
/* same as strcmp() */
native       strcmp(str1[], str2[]);
/* same as strncmp */
native       strncmp(str1[], str2[], n);
/* same as strcpy */
native       strcpy(dst[], src[]);
/* same as strncpy  except it nul terminates */
native       strncpy(dst[], src[], n);
/* same as strlen */
native       strlen(str[]);
/* same as strcat */
native       strcat(dst[], src[]);
/* same as strncat except it nul terminates */
native       strncat(dst[], src[], n);
/* prepends src string onto start of dst string */
native       strprep(dst[], src[]);
/* prepends at most n chars from src string onto dst string */
native       strnprep(dst[], src[], n);
/* cuts chars from char n to (not including) n2, and puts them in str */
native       strcut(dst[], str[], n, n2);
/* same as snprintf, except only supports %%, %c, %i, %d, %f, %x, %X, %s, \n and \t */
native       snprintf(dst[], dstn, fmt[], ...);
/* same as strstr */
native       strstr(str[], ndl[]);
/* same as strchr, except ch must be a 1 charater long string, and returns string index */
native       strchr(str[], ch[]);
/* same as strrchr, except ch must be a 1 charater long string and returns string index */
native       strrchr(str[], ch[]);
/* return random number 0 - 65535 */
native       rand();
/* return random number 0.0 - 1.0 */
native Float:randf();
/* return seconds since midnight as a float */
native Float:seconds();
/* return the current date, year, time etc. in the variables provided */
native       date(&year, &month, &day, &yearday, &weekday, &hr, &min, &Float:sec);

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/**************************************************/
/* Hidden calls - all are overloaded on operators */
/**************************************************/

/* Convert an integer into a floating point value */
native Float:float(value);
/* Multiple two floats together */
native Float:float_mul(Float:oper1, Float:oper2);
/* Divide the dividend float by the divisor float */
native Float:float_div(Float:dividend, Float:divisor);
/* Add two floats together */
native Float:float_add(Float:oper1, Float:oper2);
/* Subtract oper2 float from oper1 float */
native Float:float_sub(Float:oper1, Float:oper2);
/* Compare two integers. If the two elements are equal, return 0. */
/* If the first argument is greater than the second argument, return 1, */
/* If the first argument is less than the second argument, return -1. */
native       float_cmp(Float:oper1, Float:oper2);
/* user defined operators */
native Float:operator*(Float:oper1, Float:oper2) = float_mul;
native Float:operator/(Float:oper1, Float:oper2) = float_div;
native Float:operator+(Float:oper1, Float:oper2) = float_add;
native Float:operator-(Float:oper1, Float:oper2) = float_sub;
native Float:operator=(oper) = float;
stock Float:operator++(Float:oper)
    return oper+1.0;
stock Float:operator--(Float:oper)
    return oper-1.0;
stock Float:operator-(Float:oper)
    return oper^Float:0x80000000; /* IEEE values are sign/magnitude */
stock Float:operator*(Float:oper1, oper2)
    return float_mul(oper1, float(oper2)); /* "*" is commutative */
stock Float:operator/(Float:oper1, oper2)
    return float_div(oper1, float(oper2));
stock Float:operator/(oper1, Float:oper2)
    return float_div(float(oper1), oper2);
stock Float:operator+(Float:oper1, oper2)
    return float_add(oper1, float(oper2)); /* "+" is commutative */
stock Float:operator-(Float:oper1, oper2)
    return float_sub(oper1, float(oper2));
stock Float:operator-(oper1, Float:oper2)
    return float_sub(float(oper1), oper2);
stock bool:operator==(Float:oper1, Float:oper2)
    return float_cmp(oper1, oper2) == 0;
stock bool:operator==(Float:oper1, oper2)
    return float_cmp(oper1, float(oper2)) == 0;  /* "==" is commutative */
stock bool:operator!=(Float:oper1, Float:oper2)
    return float_cmp(oper1, oper2) != 0;
stock bool:operator!=(Float:oper1, oper2)
    return float_cmp(oper1, float(oper2)) != 0;  /* "!=" is commutative */
stock bool:operator>(Float:oper1, Float:oper2)
    return float_cmp(oper1, oper2) > 0;
stock bool:operator>(Float:oper1, oper2)
    return float_cmp(oper1, float(oper2)) > 0;
stock bool:operator>(oper1, Float:oper2)
    return float_cmp(float(oper1), oper2) > 0;
stock bool:operator>=(Float:oper1, Float:oper2)
    return float_cmp(oper1, oper2) >= 0;
stock bool:operator>=(Float:oper1, oper2)
    return float_cmp(oper1, float(oper2)) >= 0;
stock bool:operator>=(oper1, Float:oper2)
    return float_cmp(float(oper1), oper2) >= 0;
stock bool:operator<(Float:oper1, Float:oper2)
    return float_cmp(oper1, oper2) < 0;
stock bool:operator<(Float:oper1, oper2)
    return float_cmp(oper1, float(oper2)) < 0;
stock bool:operator<(oper1, Float:oper2)
    return float_cmp(float(oper1), oper2) < 0;
stock bool:operator<=(Float:oper1, Float:oper2)
    return float_cmp(oper1, oper2) <= 0;
stock bool:operator<=(Float:oper1, oper2)
    return float_cmp(oper1, float(oper2)) <= 0;
stock bool:operator<=(oper1, Float:oper2)
    return float_cmp(float(oper1), oper2) <= 0;
stock bool:operator!(Float:oper)
    return (_:oper & 0x7fffffff) == 0;
/* forbidden operations */
forward operator%(Float:oper1, Float:oper2);
forward operator%(Float:oper1, oper2);
forward operator%(oper1, Float:oper2);

/**************************************************************************/
/* ADDED in embryo 1.2                                                    */
/**************************************************************************/
/* use this to determine embryo age */
#define EMBRYO_12 12
/* Return the inverse sine, cosine or tangent. The output may be radians, */
/* degrees or grades. */
native Float:asin(Float:value, Float_Angle_Mode:mode=RADIAN);
native Float:acos(Float:value, Float_Angle_Mode:mode=RADIAN);
native Float:atan(Float:value, Float_Angle_Mode:mode=RADIAN);
native Float:atan2(Float:valuey, Float:valuex, Float_Angle_Mode:mode=RADIAN);
/* same as libc functions */
native Float:log1p(Float:value);
native Float:cbrt(Float:value);
native Float:exp(Float:value);
native Float:exp2(Float:value);
native Float:hypot(Float:valuex, Float:valuey);
