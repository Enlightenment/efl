typedef Eo Elm_Atspi_Object;

typedef uint64_t Elm_Atspi_State;

#define BIT_FLAG_SET(mask, bit) (mask |= (1 << bit))
#define BIT_FLAG_UNSET(mask, bit) (mask &= ~(1 << bit))
#define BIT_FLAG_GET(mask, bit) (mask & (1 << bit))

/*
 * @brief Gets root (application) type atspi-object.
 */
Elm_Atspi_Object * _elm_atspi_root_object_get(void);

/*
 * @brief Constructs atspi-object for evas_object
 */
Elm_Atspi_Object * _elm_atspi_factory_construct(Evas_Object *obj);

void _elm_atspi_object_init(void);

void _elm_atspi_object_shutdown(void);

