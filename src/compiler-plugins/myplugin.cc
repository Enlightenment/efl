#include <stdio.h>
#include "gcc-common.h"

/*
 * Why all this ?
 *
 * In eo we have super calls, these super calls already validated to have a valid obj pointer, and the pd pointer is already enough to calculate one, *without* the need to call eo again.
 *
 * How:
 *
 * - First of all, in eolian we are calculating the next call candidates for a overriden function, for them we are storing the name of the symbol *and* and class  providing it.
 * - Additionally, eolian generates more symbols:
 *    a) One symbol containing the offset over logical 0 address in pd's
 *    b) For each mixin in the inheritance of the class, the offset to reach exactly *that* mixin
 * - In the plugin here we are fetching all the calls of foo(efl_super(obj, XXX), aaa, bbb, ccc); then we are fetching from the eolian output what the actaul implementation for foo is, after the class XXX
 * - Then we are replacing the foo call with the actaul implementation, 2 arguments for the calls are preprended (obj, pd - A + B) where A is the offset from XXX to logical 0, and B is the offset from B to logical 0.
 *
 */

/* TODO:
 * Replace foo(efl_super(obj, MY_CLASS),...) calls with the next lower level function for foo and
 * adjust private data with diff
 */

__visible int plugin_is_GPL_compatible;

static const char super2_function[] = "efl_super2";
static GTY(()) tree super2_function_decl;

static void super2_add(gimple_stmt_iterator *gsi)
{
	gimple stmt;
	gcall *super2_func;
	cgraph_node_ptr node;
	int frequency;
	basic_block bb;


	stmt = gimple_build_call(super2_function_decl, 0);
	super2_func = as_a_gcall(stmt);
	gsi_insert_after(gsi, super2_func, GSI_CONTINUE_LINKING);

	/* Update the cgraph */
	bb = gimple_bb(super2_func);
	node = cgraph_get_create_node(super2_function_decl);
	gcc_assert(node);
	frequency = compute_call_stmt_bb_frequency(current_function_decl, bb);
	cgraph_create_edge(cgraph_get_node(current_function_decl), node,
			super2_func, bb->count, frequency);
}

struct Caller {
  bool valid;
  const char *called_api;
  const char *klass;
  tree klass_decl;
  tree called_api_decl;
};

typedef struct _Fetch_Result {
  bool valid;
  const char *api_name;
  tree api;
  const_gimple first_argument;
} Fetch_Result;

static Fetch_Result
_fetch_first_argument(const_gimple arg, unsigned int narg)
{
   Fetch_Result ret = {false, NULL, {}};

   if (gimple_call_num_args(arg) < narg + 1) return ret;
   tree first_argument = gimple_call_arg(arg, narg);

   if (!first_argument) return ret;
   if (TREE_CODE(first_argument) != SSA_NAME) return ret;
   ret.first_argument = SSA_NAME_DEF_STMT(first_argument);
   if (!ret.first_argument) return ret;
   if (!is_gimple_call(ret.first_argument)) return ret;
   tree tmp = gimple_call_fndecl(ret.first_argument);

   if (!tmp) return ret;
   ret.api_name = IDENTIFIER_POINTER(DECL_NAME(tmp));
   ret.api = tmp;
   ret.valid = true;
   return ret;
}

/**
 * Check that the first argument is a call to efl_super, and fetch the class function from it.
 * As a result, this gives a boolean flag if the result is valid or not.
 * If false, called_api and klass are invalid
 * If true, klass is the class this is supering on, and called_api contains the API call
 *
 */
static Caller fetch_efl_super_class(const_gimple stmt)
{
   Caller ret = {false, NULL, NULL};

   tree called_api_tree = gimple_call_fndecl(stmt);
   if (!called_api_tree) return ret;

   //check that we have efl_super as the first argument
   Fetch_Result first = _fetch_first_argument(stmt, 0);
   if (!first.valid) return ret;
   ret.called_api = IDENTIFIER_POINTER(DECL_NAME(called_api_tree));
   ret.called_api_decl = called_api_tree;
   if (!!strncmp(first.api_name, "efl_super", 9)) return ret;

   //copy the name we have inside efl_super
   Fetch_Result argument_efl_super = _fetch_first_argument(first.first_argument, 1);
   if (!argument_efl_super.valid) return ret;
   ret.klass = argument_efl_super.api_name;
   ret.klass_decl = argument_efl_super.api;

   ret.valid = true;

   return ret;
}

static unsigned int eo_execute(void)
{
  basic_block bb, entry_bb;
  gimple_stmt_iterator gsi;

  //fprintf(stderr, "Function %s eval\n", get_name(cfun->decl));

  //FIXME check if this is a real eo op, with Eo *obj, and *pd as next agument
  //FIXME therefore we need *something* to check if this is really

  gcc_assert(single_succ_p(ENTRY_BLOCK_PTR_FOR_FN(cfun)));
  entry_bb = single_succ(ENTRY_BLOCK_PTR_FOR_FN(cfun));

  FOR_EACH_BB_FN(bb, cfun) {
    const_gimple stmt;
    for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
      {
        stmt = gsi_stmt(gsi);
        if (!stmt || !is_gimple_call(stmt)) continue;

        struct Caller c = fetch_efl_super_class(stmt);
        tree replacement_candidate = NULL;
        tree provider_of_replacement_candidate = NULL;

        if (!c.klass)
          continue;

        for (tree attribute = DECL_ATTRIBUTES(c.klass_decl); attribute != NULL_TREE; attribute = TREE_CHAIN(attribute))
          {
             tree attribute_name = TREE_PURPOSE(attribute);
             if (!attribute_name)
               {
                  fprintf(stderr, "Error, expected name\n");
                  continue;
               }
             if (!!strncmp(IDENTIFIER_POINTER(attribute_name), "register_next", strlen("register_next")))
               continue;

             //this here assumes a special tree_list structure
             tree attribute_arguments = TREE_VALUE(attribute);
             tree call = TREE_VALUE(attribute_arguments);
             tree providing_class = TREE_VALUE(TREE_CHAIN(attribute_arguments));
             tree implementation = TREE_VALUE(TREE_CHAIN(TREE_CHAIN(attribute_arguments)));

             if (!!strncmp(TREE_STRING_POINTER(call), c.called_api, strlen(c.called_api))) continue;

             provider_of_replacement_candidate = providing_class;
             replacement_candidate = implementation;
          }
        if (!replacement_candidate) continue;
        fprintf(stderr, "Replace! %s %s\n", c.called_api, TREE_STRING_POINTER(replacement_candidate));

        //Create a new call to the found replacement candidate

#if 1
        //FIXME we need here:
        //add another argument "pd - <my_class>_pd_offset + <providing_class>_pd_offset" (TODO check if these are mixins)
        /*
         * Create function declaration
         */
        vec<tree> argument_types;
        unsigned int i = 0;
        argument_types.create(10);
        for (tree argument = DECL_ARGUMENTS(c.called_api_decl); argument; argument = TREE_CHAIN(argument), i++)
          {
             if (!argument_types.space(i + 1))
               argument_types.safe_grow(i + 1);
             if (i == 0) {
               argument_types[i] = argument;
             } else {
               argument_types[i + 1] = argument;
             }
             fprintf(stderr, "COPY\n");
          }

        tree pd_type = void_type_node;//the pd argument is interpreted as void pointer here.
        argument_types[1] = pd_type;
        tree result = DECL_RESULT(c.called_api_decl);
        if (!result)
          result = void_type_node;
        fprintf(stderr, "----> %d\n", i);
        tree implementation_function_types = build_function_type_array(result, i + 1, argument_types.address());
        tree implementation_function_declaration = build_fn_decl(TREE_STRING_POINTER(replacement_candidate), implementation_function_types);

        /**
         * Create function call
         */
        vec<tree> new_arguments;
        new_arguments.create(gimple_call_num_args(stmt) + 1);
        new_arguments[0] = gimple_call_arg(stmt, 0);
        tree pd_value = new_arguments[0]; //FIXME create calls to second argument of cfun pd + and -
        new_arguments[1] = pd_value;
        for (unsigned int i = 1; i < gimple_call_num_args(stmt); ++i)
          {
             new_arguments[i + 1] = gimple_call_arg(stmt, i);
             fprintf(stderr, "COPY2\n");
          }

        gcall *new_call = gimple_build_call_vec(implementation_function_declaration, new_arguments);
        const greturn *ret = as_a <const greturn *> (stmt);
        tree return_val = gimple_return_retval(ret);
        gimple_return_set_retval(as_a<greturn *>(new_call), return_val);
        gsi_replace(&gsi, new_call, true);
#endif
      }
    }
  return 0;
}

static void super2_start_unit(void *gcc_data __unused,
				 void *user_data __unused)
{
	/* void efl_super2(void) */
	tree fntype = build_function_type_list(void_type_node, NULL_TREE);
	super2_function_decl = build_fn_decl(super2_function, fntype);
	DECL_ASSEMBLER_NAME(super2_function_decl); /* for LTO */
	TREE_PUBLIC(super2_function_decl) = 1;
	TREE_USED(super2_function_decl) = 1;
	DECL_EXTERNAL(super2_function_decl) = 1;
	DECL_ARTIFICIAL(super2_function_decl) = 1;
	DECL_PRESERVE_P(super2_function_decl) = 1;
}

static tree
handle_user_attribute (tree *node, tree name, tree args,
                      int flags, bool *no_add_attrs)
{
  *no_add_attrs = 0;

  //FIXME we should validate that in some way
/*
  fprintf(stderr, "%p\n", node);

  for (tree arg = args; arg; arg = TREE_CHAIN(arg))
    {
       //tree value = TREE_VALUE(arg);
       //fprintf(stderr, "%s\n", get_tree_code_name(TREE_CODE(value)));

       //tree value = TREE_VALUE(arg);
       //fprintf(stderr, "----> %s\n", TREE_STRING_POINTER(value));
    }
*/
  return NULL_TREE;
}

static struct attribute_spec next_hop_attr =
      { "register_next", 4, 4, true,  false, false,  true, handle_user_attribute, NULL};

static void
register_next_hop_attribute (void *event_data, void *data)
{
  register_attribute (&next_hop_attr);
}

#define PASS_NAME eo
#define NO_GATE
#include "gcc-generate-gimple-pass.h"

__visible int plugin_init(struct plugin_name_args *plugin_info,
    struct plugin_gcc_version *version)
{
  const char *plugin_name = plugin_info->base_name;

  if (!plugin_default_version_check (version, &gcc_version))
    return 1;

  PASS_INFO(eo, "cfg", 1, PASS_POS_INSERT_AFTER);

  /* Register to be called before processing a translation unit */
  register_callback(plugin_name, PLUGIN_START_UNIT, &super2_start_unit, NULL);
  register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &eo_pass_info);
  register_callback (plugin_name, PLUGIN_ATTRIBUTES, register_next_hop_attribute, NULL);
  return 0;
}
