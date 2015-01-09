#ifndef _ELDBUS_TEST_ELDBUS_MODEL_H
#define _ELDBUS_TEST_ELDBUS_MODEL_H

#include <Eldbus_Model.h>
#include <Efl.h>
#include <Eo.h>

void check_init(void);
void check_shutdown(void);
Eo *create_connection(void);
Eo *create_and_load_connection(void);
Eo *create_object(void);
Eo *create_and_load_object(void);

void efl_model_wait_for_event(Eo *obj, const Eo_Event_Description *event);
void efl_model_wait_for_load_status(Eo *obj, Efl_Model_Load_Status expected_status);
void efl_model_load_and_wait_for_load_status(Eo *obj, Efl_Model_Load_Status expected_status);
Efl_Model_Base *efl_model_nth_child_get(Efl_Model_Base *obj, unsigned int n);
Efl_Model_Base *efl_model_first_child_get(Efl_Model_Base *efl_model);

void check_efl_model_load_status_get(Efl_Model_Base *obj, Efl_Model_Load_Status expected_load_status);
void check_efl_model_children_count_eq(Efl_Model_Base *obj, unsigned int expected_children_count);
void check_efl_model_children_count_ge(Efl_Model_Base *obj, unsigned int minimum_children_count);
void check_efl_model_children_slice_get(Efl_Model_Base *efl_model);
void check_efl_model_property_int_eq(Efl_Model_Base *obj, const char *property, int expected_value);
void check_efl_model_property_int_set(Efl_Model_Base *obj, const char *property, int value);

Eldbus_Model_Proxy *eldbus_model_proxy_from_object_get(Eldbus_Model_Object *object, const char *interface_name);
Eldbus_Model_Method *eldbus_model_method_from_proxy_get(Eldbus_Model_Proxy *proxy, const char *method_name);
Eldbus_Model_Signal *eldbus_model_signal_from_proxy_get(Eldbus_Model_Proxy *proxy, const char *signal_name);

#endif
