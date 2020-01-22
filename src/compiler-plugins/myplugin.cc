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

static unsigned int eo_execute(void)
{
  basic_block bb, entry_bb;
  gimple_stmt_iterator gsi;

  //fprintf(stderr, "Function %s called\n", get_name(cfun->decl));

  gcc_assert(single_succ_p(ENTRY_BLOCK_PTR_FOR_FN(cfun)));
  entry_bb = single_succ(ENTRY_BLOCK_PTR_FOR_FN(cfun));

  FOR_EACH_BB_FN(bb, cfun) {
    const_gimple stmt;
    for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
      {
        stmt = gsi_stmt(gsi);
        if (is_gimple_call(stmt)){
	  tree current_fn_decl = gimple_call_fndecl(stmt);
	  if (!current_fn_decl)
	     continue;
          const char* name = IDENTIFIER_POINTER(DECL_NAME(current_fn_decl));
          if (!strncmp(name, "efl_super", 9)) {
            fprintf(stderr, "Found function %s\n", name);
            super2_add(&gsi);
	  }
        }
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
