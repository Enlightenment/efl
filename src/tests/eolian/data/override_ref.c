EOAPI EO_VOID_FUNC_BODY(override_a_set);
EOAPI EO_VOID_FUNC_BODY(override_foo);

static void __eolian_override_b_set(Eo *obj EINA_UNUSED, Override_Data *pd, int idx EINA_UNUSED, float a, char b, int c)
{
   a = pd->a;
   b = pd->b;
   c = pd->c;
}

EOAPI EO_VOID_FUNC_BODYV(override_b_set, EO_FUNC_CALL(idx, a, b, c), int idx, float a, char b, int c);

static void __eolian_override_bar(Eo *obj EINA_UNUSED, Override_Data *pd EINA_UNUSED, int idx EINA_UNUSED, int *a, char **str)
{
   if (a) *a = 250;
   if (str) *str = NULL;
}

EOAPI EO_VOID_FUNC_BODYV(override_bar, EO_FUNC_CALL(idx, a, str), int idx, int *a, char **str);

static int __eolian_override_c_get(Eo *obj EINA_UNUSED, Override_Data *pd EINA_UNUSED, int idx EINA_UNUSED)
{
   return 50;
}

EOAPI EO_FUNC_BODYV(override_c_get, int, 50, EO_FUNC_CALL(idx), int idx);

void _override_a_get(Eo *obj, Override_Data *pd);

EOAPI EO_VOID_FUNC_BODY(override_a_get);

void _override_b_get(Eo *obj, Override_Data *pd, int idx, float *a, char *b, int *c);

EOAPI EO_VOID_FUNC_BODYV(override_b_get, EO_FUNC_CALL(idx, a, b, c), int idx, float *a, char *b, int *c);

void _override_c_set(Eo *obj, Override_Data *pd, int idx, int c);

EOAPI EO_VOID_FUNC_BODYV(override_c_set, EO_FUNC_CALL(idx, c), int idx, int c);

void _override_base_constructor(Eo *obj, Override_Data *pd);


static void __eolian_override_base_z_get(Eo *obj EINA_UNUSED, Override_Data *pd, int *a, char *b, float *c)
{
   if (a) *a = pd->a;
   if (b) *b = pd->b;
   if (c) *c = pd->c;
}


static void __eolian_override_base_z_set(Eo *obj EINA_UNUSED, Override_Data *pd EINA_UNUSED, int a EINA_UNUSED, char b EINA_UNUSED, float c EINA_UNUSED)
{
}


static Eo_Op_Description _override_op_desc[] = {
     EO_OP_FUNC_OVERRIDE(base_constructor, _override_base_constructor),
     EO_OP_FUNC_OVERRIDE(base_z_get, __eolian_override_base_z_get),
     EO_OP_FUNC_OVERRIDE(base_z_set, __eolian_override_base_z_set),
     EO_OP_FUNC(override_a_set, NULL, ""),
     EO_OP_FUNC(override_foo, NULL, ""),
     EO_OP_FUNC(override_b_set, __eolian_override_b_set, ""),
     EO_OP_FUNC(override_bar, __eolian_override_bar, ""),
     EO_OP_FUNC(override_c_get, __eolian_override_c_get, ""),
     EO_OP_FUNC(override_a_get, _override_a_get, ""),
     EO_OP_FUNC(override_b_get, _override_b_get, ""),
     EO_OP_FUNC(override_c_set, _override_c_set, ""),
     EO_OP_SENTINEL
};

static const Eo_Class_Description _override_class_desc = {
     EO_VERSION,
     "Override",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(_override_op_desc),
     NULL,
     sizeof(Override_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(override_class_get, &_override_class_desc, BASE_CLASS, NULL);