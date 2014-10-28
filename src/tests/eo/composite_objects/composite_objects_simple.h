#ifndef SIMPLE_H
#define SIMPLE_H

typedef struct
{
   int a;
} Simple_Public_Data;

EAPI void simple_a_set(int a);
EAPI int simple_a_get(void);

EAPI void simple_a_set1(int a);
EAPI void simple_a_set2(int a);
EAPI void simple_a_set3(int a);
EAPI void simple_a_set4(int a);
EAPI void simple_a_set5(int a);
EAPI void simple_a_set6(int a);
EAPI void simple_a_set7(int a);
EAPI void simple_a_set8(int a);
EAPI void simple_a_set9(int a);
EAPI void simple_a_set10(int a);
EAPI void simple_a_set11(int a);
EAPI void simple_a_set12(int a);
EAPI void simple_a_set13(int a);
EAPI void simple_a_set14(int a);
EAPI void simple_a_set15(int a);
EAPI void simple_a_set16(int a);
EAPI void simple_a_set17(int a);
EAPI void simple_a_set18(int a);
EAPI void simple_a_set19(int a);
EAPI void simple_a_set20(int a);
EAPI void simple_a_set21(int a);
EAPI void simple_a_set22(int a);
EAPI void simple_a_set23(int a);
EAPI void simple_a_set24(int a);
EAPI void simple_a_set25(int a);
EAPI void simple_a_set26(int a);
EAPI void simple_a_set27(int a);
EAPI void simple_a_set28(int a);
EAPI void simple_a_set29(int a);
EAPI void simple_a_set30(int a);
EAPI void simple_a_set31(int a);
EAPI void simple_a_set32(int a);
EAPI void simple_a_get1(int a);
EAPI void simple_a_get2(int a);
EAPI void simple_a_get3(int a);
EAPI void simple_a_get4(int a);
EAPI void simple_a_get5(int a);
EAPI void simple_a_get6(int a);
EAPI void simple_a_get7(int a);
EAPI void simple_a_get8(int a);
EAPI void simple_a_get9(int a);
EAPI void simple_a_get10(int a);
EAPI void simple_a_get11(int a);
EAPI void simple_a_get12(int a);
EAPI void simple_a_get13(int a);
EAPI void simple_a_get14(int a);
EAPI void simple_a_get15(int a);
EAPI void simple_a_get16(int a);
EAPI void simple_a_get17(int a);
EAPI void simple_a_get18(int a);
EAPI void simple_a_get19(int a);
EAPI void simple_a_get20(int a);
EAPI void simple_a_get21(int a);
EAPI void simple_a_get22(int a);
EAPI void simple_a_get23(int a);
EAPI void simple_a_get24(int a);
EAPI void simple_a_get25(int a);
EAPI void simple_a_get26(int a);
EAPI void simple_a_get27(int a);
EAPI void simple_a_get28(int a);
EAPI void simple_a_get29(int a);
EAPI void simple_a_get30(int a);
EAPI void simple_a_get31(int a);
EAPI void simple_a_get32(int a);

extern const Eo_Event_Description _EV_A_CHANGED;
#define EV_A_CHANGED (&(_EV_A_CHANGED))

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
