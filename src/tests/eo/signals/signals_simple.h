#ifndef SIMPLE_H
#define SIMPLE_H

extern EAPI Eo_Op SIMPLE_BASE_ID;

enum {
     SIMPLE_SUB_ID_A_SET,
     SIMPLE_SUB_ID_LAST
};

typedef struct
{
   int cb_count;
} Simple_Public_Data;

#define SIMPLE_ID(sub_id) (SIMPLE_BASE_ID + sub_id)

/**
 * @def simple_a_set(a)
 * @brief Set value to a - property
 * @param[in] a integer value to set
 */
#define simple_a_set(a) SIMPLE_ID(SIMPLE_SUB_ID_A_SET), EO_TYPECHECK(int, a)

extern const Eo_Event_Description _EV_A_CHANGED;
#define EV_A_CHANGED (&(_EV_A_CHANGED))

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
