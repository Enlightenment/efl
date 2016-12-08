import gdb

"""
All of this script relies heavily on Eo internals and will break if they
change. Need to make sure this is always in sync.
"""

ptr_size = int(gdb.parse_and_eval('sizeof(void *)'))

if ptr_size == 4:
    # 32 bits
    BITS_MID_TABLE_ID = 5
    BITS_TABLE_ID = 5
    BITS_ENTRY_ID = 11
    BITS_GENERATION_COUNTER = 6
    BITS_DOMAIN = 2
    BITS_CLASS = 1
    REF_TAG_SHIFT = 30
    SUPER_TAG_SHIFT = 31
    DROPPED_TABLES = 0
    DROPPED_ENTRIES = 4
else:
    # 64 bits
    BITS_MID_TABLE_ID = 11
    BITS_TABLE_ID = 11
    BITS_ENTRY_ID = 11
    BITS_GENERATION_COUNTER = 26
    BITS_DOMAIN = 2
    BITS_CLASS = 1
    REF_TAG_SHIFT = 62
    SUPER_TAG_SHIFT = 63
    DROPPED_TABLES = 2
    DROPPED_ENTRIES = 3

# /* Shifts macros to manipulate the Eo id */
SHIFT_DOMAIN = (BITS_MID_TABLE_ID + BITS_TABLE_ID +
                BITS_ENTRY_ID + BITS_GENERATION_COUNTER)
SHIFT_MID_TABLE_ID = (BITS_TABLE_ID +
                      BITS_ENTRY_ID + BITS_GENERATION_COUNTER)
SHIFT_TABLE_ID = (BITS_ENTRY_ID + BITS_GENERATION_COUNTER)
SHIFT_ENTRY_ID = (BITS_GENERATION_COUNTER)

# /* Maximum ranges */
MAX_DOMAIN = (1 << BITS_DOMAIN)
MAX_MID_TABLE_ID = (1 << BITS_MID_TABLE_ID)
MAX_TABLE_ID = ((1 << BITS_TABLE_ID) - DROPPED_TABLES)
MAX_ENTRY_ID = ((1 << BITS_ENTRY_ID) - DROPPED_ENTRIES)
MAX_GENERATIONS = (1 << BITS_GENERATION_COUNTER)

# /* Masks */
MASK_DOMAIN = (MAX_DOMAIN - 1)
MASK_MID_TABLE_ID = (MAX_MID_TABLE_ID - 1)
MASK_TABLE_ID = ((1 << BITS_TABLE_ID) - 1)
MASK_ENTRY_ID = ((1 << BITS_ENTRY_ID) - 1)
MASK_GENERATIONS = (MAX_GENERATIONS - 1)
MASK_OBJ_TAG = (1 << (REF_TAG_SHIFT))


null_void_ptr = gdb.parse_and_eval('(_Eo_Object *) 0')
null_eo_object_ptr = gdb.parse_and_eval('(_Eo_Object *) 0')
zero_uintptr_t = gdb.parse_and_eval('(uintptr_t) 0')


class Eo_resolve(gdb.Function):
    def __init__(self):
        gdb.Function.__init__(self, 'eo_resolve')

    def invoke(self, arg):
        obj_id = int(str(arg.cast(zero_uintptr_t.type)), 0)

        mid_table_id = (obj_id >> SHIFT_MID_TABLE_ID) & MASK_MID_TABLE_ID
        table_id = (obj_id >> SHIFT_TABLE_ID) & MASK_TABLE_ID
        entry_id = (obj_id >> SHIFT_ENTRY_ID) & MASK_ENTRY_ID
        tag_bit = (obj_id) & MASK_OBJ_TAG
        generation = obj_id & MASK_GENERATIONS

        if (obj_id == 0) or (tag_bit == 0):
            gdb.write('Pointer is NULL or not a valid object.\n')
            return null_eo_object_ptr

        entries = gdb.parse_and_eval('_eo_gdb_main_domain->tables[0]->' +
                                     'eo_ids_tables[{0}]'.format(mid_table_id))

        if int(entries) == 0:
            gdb.write('Pointer is not a valid object.\n')
            return null_eo_object_ptr

        entry = entries[table_id]['entries'][entry_id]

        if (not entry['active']) or (int(entry['generation']) != generation):
            gdb.write('Pointer is no longer active.\n')
            return null_eo_object_ptr

        return entry['ptr']


Eo_resolve()


class Eo_data_get(gdb.Function):
    def __init__(self):
        gdb.Function.__init__(self, 'eo_data_get')

    def invoke(self, ptr, kls_name):
        ptr = ptr.cast(null_eo_object_ptr.type)  # Cast to correct type

        if int(ptr) == 0:
            gdb.write('Object is not a valid pointer (NULL).\n')
            return null_void_ptr

        kls_name = kls_name.string()
        extns = ptr['klass']['mro']
        kls = None

        i = 0
        while int(extns[i]) != 0:
            if extns[i]['desc']['name'].string() == kls_name:
                kls = extns[i]
            i += 1

        if kls is None:
            gdb.write('Class "{}" not found in the object mro.\n'
                      .format(kls_name))
            return null_void_ptr

        # Check if not mixin
        if int(kls['desc']['type'].cast(zero_uintptr_t.type)) != 3:
            return gdb.parse_and_eval('(void *) (((char *) {}) + {})'
                                      .format(ptr, kls['data_offset']))
        else:
            extn_off = ptr['klass']['extn_data_off']
            if int(extn_off) == 0:
                return null_void_ptr

            i = 0
            while int(extn_off[i]['klass']) != 0:
                kls = extn_off[i]['klass']
                if kls['desc']['name'].string() == kls_name:
                    return gdb.parse_and_eval('(void *) (((char *) {}) + {})'
                                              .format(ptr, kls['data_offset']))
                i += 1

        return null_void_ptr


Eo_data_get()
