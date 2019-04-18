#ifndef EO_TEST_REFLECTION_COMPLEX_CLASS_STRCUTURE_H
#define EO_TEST_REFLECTION_COMPLEX_CLASS_STRCUTURE_H

typedef Eo Complex_Mixin;
#define COMPLEX_MIXIN_MIXIN complex_mixin_mixin_get()
EWAPI const Efl_Class *complex_mixin_mixin_get(void);
EOAPI void complex_mixin_m_test_set(Eo *obj, int i);
EOAPI int complex_mixin_m_test_get(const Eo *obj);

typedef Eo Complex_Interface;
#define COMPLEX_INTERFACE_INTERFACE complex_interface_interface_get()
EWAPI const Efl_Class *complex_interface_interface_get(void);
EOAPI void complex_interface_i_test_set(Eo *obj, int i);
EOAPI int complex_interface_i_test_get(const Eo *obj);

typedef Eo Complex_Class;
#define COMPLEX_CLASS_CLASS complex_class_class_get()
EWAPI const Efl_Class *complex_class_class_get(void);

#endif
