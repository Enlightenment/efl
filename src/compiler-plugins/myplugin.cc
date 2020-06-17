#include <stdio.h>
#include "gcc-common.h"

/* TODO:
 * Find all efl_super calls
 * Traverse back to find function calling it (foo)
 * Calculate the private data pointer diff
 * Replace foo(efl_super(obj, MY_CLASS),...) calls with the next lower level function for foo and
 * adjust private data with diff
 * Make eolian more expressive (We need the implemented methods of classes, accross the tree)
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
};

typedef struct _Fetch_Result {
  bool valid;
  const char *api_name;
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

   //check that we have efl_super as the first argument
   Fetch_Result first = _fetch_first_argument(stmt, 0);
   if (!first.valid) return ret;
   tree called_api_tree = gimple_call_fndecl(first.first_argument);
   ret.called_api = IDENTIFIER_POINTER(DECL_NAME(called_api_tree));
   if (!!strncmp(first.api_name, "efl_super", 9)) return ret;

   //copy the name we have inside efl_super
   Fetch_Result argument_efl_super = _fetch_first_argument(first.first_argument, 1);
   if (!argument_efl_super.valid) return ret;
   ret.klass = argument_efl_super.api_name;
   ret.valid = true;

   return ret;
}

static unsigned int eo_execute(void)
{
  basic_block bb, entry_bb;
  gimple_stmt_iterator gsi;

  //fprintf(stderr, "Function %s eval\n", get_name(cfun->decl));

  gcc_assert(single_succ_p(ENTRY_BLOCK_PTR_FOR_FN(cfun)));
  entry_bb = single_succ(ENTRY_BLOCK_PTR_FOR_FN(cfun));

  FOR_EACH_BB_FN(bb, cfun) {
    const_gimple stmt;
    for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
      {
        stmt = gsi_stmt(gsi);
        if (!stmt || !is_gimple_call(stmt)) continue;

        struct Caller c = fetch_efl_super_class(stmt);

        if (!c.klass)
          continue;

        fprintf(stderr, "Found call of %s as super of %s\n", c.called_api, c.klass);
        //FIXME work
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
	register_callback(plugin_name, PLUGIN_START_UNIT,
					&super2_start_unit, NULL);

        register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &eo_pass_info);

	return 0;
}
