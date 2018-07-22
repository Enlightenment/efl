#ifndef _ELDBUS_TEST_ELDBUS_MODEL_H
#define _ELDBUS_TEST_ELDBUS_MODEL_H

#include <Eo.h>
#include <Eldbus_Model.h>
#include <Efl.h>

void check_init(void);
void check_shutdown(void);
Eo *create_connection(void);
Eo *create_object(void);

void efl_model_wait_for_event(Eo *obj, const Efl_Event_Description *event);
Efl_Model *efl_model_nth_child_get(Efl_Model *obj, unsigned int n);
Efl_Model *efl_model_first_child_get(Efl_Model *efl_model);

void check_efl_model_children_count_eq(Efl_Model *obj, unsigned int expected_children_count);
void check_efl_model_children_count_ge(Efl_Model *obj, unsigned int minimum_children_count);
void check_efl_model_children_slice_get(Efl_Model *efl_model);
void check_efl_model_property_int_eq(Efl_Model *obj, const char *property, int expected_value);
void check_efl_model_property_int_set(Efl_Model *obj, const char *property, int value);
void check_efl_model_future_error(Eina_Future *future, Eina_Error *err);

Eina_Value *efl_model_future_then(Eina_Future *future);
int efl_model_future_then_u(Eina_Future *future);

Eldbus_Model_Proxy *eldbus_model_proxy_from_object_get(Eldbus_Model_Object *object, const char *interface_name);
Eldbus_Model_Method *eldbus_model_method_from_proxy_get(Eldbus_Model_Proxy *proxy, const char *method_name);
Eldbus_Model_Signal *eldbus_model_signal_from_proxy_get(Eldbus_Model_Proxy *proxy, const char *signal_name);

#endif
