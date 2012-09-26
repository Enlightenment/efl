#ifndef SIMPLE_H
#define SIMPLE_H

extern EAPI Eo_Op SIMPLE_BASE_ID;

enum {
     SIMPLE_SUB_ID_A_SET,
     SIMPLE_SUB_ID_A_GET,
     SIMPLE_SUB_ID_B_SET,
     SIMPLE_SUB_ID_B_GET,
     SIMPLE_SUB_ID_LAST
};

#define SIMPLE_ID(sub_id) (SIMPLE_BASE_ID + sub_id)

/**
 * @def simple_a_set(a)
 * @brief Set value to a-property
 * @param[in] a integer value to set
 */
#define simple_a_set(a) SIMPLE_ID(SIMPLE_SUB_ID_A_SET), EO_TYPECHECK(int, a)

/**
 * @def simple_a_get(a)
 * @brief Get value of a-property
 * @param[out] integer pointer to a-value
 */
#define simple_a_get(a) SIMPLE_ID(SIMPLE_SUB_ID_A_GET), EO_TYPECHECK(int *, a)

/**
 * @def simple_b_set(b)
 * @brief Set value to b-property
 * @param[in] a integer value to set
 */
#define simple_b_set(b) SIMPLE_ID(SIMPLE_SUB_ID_B_SET), EO_TYPECHECK(int, b)

/**
 * @def simple_b_get(b)
 * @brief Get value of b-property
 * @param[out] integer pointer to b-value
 */
#define simple_b_get(b) SIMPLE_ID(SIMPLE_SUB_ID_B_GET), EO_TYPECHECK(int *, b)

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
