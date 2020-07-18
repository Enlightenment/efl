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
	cgraph_create_edge(cgraph_node::get(current_function_decl), node,
			super2_func, bb->count, frequency);
}

struct Caller {
  bool valid;
  const char *called_api;
  const char *klass;
  tree klass_decl;
  tree called_api_decl;
  tree klass_call;
  gimple efl_super_call;
  gimple class_get_call;
};

typedef struct _Fetch_Result {
  bool valid;
  const char *api_name;
  tree api;
  gimple first_argument;
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
   ret.efl_super_call = first.first_argument;
   if (!!strncmp(first.api_name, "efl_super", 9)) return ret;

   //copy the name we have inside efl_super
   Fetch_Result argument_efl_super = _fetch_first_argument(first.first_argument, 1);
   if (!argument_efl_super.valid) return ret;
   ret.klass = argument_efl_super.api_name;
   ret.klass_decl = argument_efl_super.api;
   ret.klass_call = gimple_call_arg(first.first_argument, 0);
   ret.class_get_call = argument_efl_super.first_argument;

   ret.valid = true;

   return ret;
}

struct Registered_Api{
  bool valid;
  tree call;
  tree pd_minus_field;
  tree pd_plus_field;
  tree replacement_candidate;

  bool is_mixin;
};

static Registered_Api
fetch_replacement_api(tree klass_decl, const char* called_api)
{
   Registered_Api result = {false, 0, 0};
   for (tree attribute = DECL_ATTRIBUTES(klass_decl); attribute != NULL_TREE; attribute = TREE_CHAIN(attribute))
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
        result.call =                  TREE_VALUE(attribute_arguments);
        result.pd_minus_field =        TREE_VALUE(TREE_CHAIN(attribute_arguments));
        result.pd_plus_field =         TREE_VALUE(TREE_CHAIN(TREE_CHAIN(attribute_arguments)));
        result.replacement_candidate = TREE_VALUE(TREE_CHAIN(TREE_CHAIN(TREE_CHAIN(attribute_arguments))));
        //result.is_mixin = !!(TREE_VALUE(TREE_CHAIN(TREE_CHAIN(TREE_CHAIN(TREE_CHAIN(attribute_arguments))))));
        if (!!strncmp(TREE_STRING_POINTER(result.call), called_api, strlen(called_api))) continue;
        result.valid = true;
        break;
     }
   return result;
}

static unsigned int eo_execute(void)
{
  basic_block bb, entry_bb;
  gimple_stmt_iterator gsi;
  unsigned int length = 0;

  //fprintf(stderr, "Function %s eval\n", get_name(cfun->decl));

  //FIXME check if this is a real eo op, with Eo *obj, and *pd as next agument
  //FIXME therefore we need *something* to check if this is really

  gcc_assert(single_succ_p(ENTRY_BLOCK_PTR_FOR_FN(cfun)));
  entry_bb = single_succ(ENTRY_BLOCK_PTR_FOR_FN(cfun));

  //FIXME this is jerky but we are checking if args are more than 2, if so we might be a eo op, and thats fine
  for (tree argument = DECL_ARGUMENTS(cfun->decl); argument; argument = DECL_CHAIN(argument))
    {
       length ++;
    }
  if (length < 2) return 0; //not considering functions that are invalid here

  FOR_EACH_BB_FN(bb, cfun) {
    gimple stmt;
    for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
      {
        stmt = gsi_stmt(gsi);

        if (!stmt || !is_gimple_call(stmt)) continue;

        struct Caller c = fetch_efl_super_class(stmt);

        if (!c.klass)
          continue;

        Registered_Api api = fetch_replacement_api(c.klass_decl, c.called_api);

        if (!api.valid) continue;

        fprintf(stderr, "Replace! %s %s\n", c.called_api, TREE_STRING_POINTER(api.replacement_candidate));

        //Create a new call to the found replacement candidate
        /*
         * Create function declaration
         */
        vec<tree> argument_types;
        unsigned int i = 0;
        argument_types.create(10);
        for (tree argument = DECL_ARGUMENTS(c.called_api_decl); argument; argument = TREE_CHAIN(argument), i++)
          {
             argument_types.safe_push(argument);
             if (i == 0)
               argument_types.safe_push(void_type_node);
          }

        tree pd_type = void_type_node;//the pd argument is interpreted as void pointer here.
        argument_types[1] = pd_type;
        tree result = DECL_RESULT(c.called_api_decl);
        if (!result)
          result = void_type_node;
        tree implementation_function_types = build_function_type_array(result, i + 1, &argument_types.address()[0]);
        tree implementation_function_declaration = build_fn_decl(TREE_STRING_POINTER(api.replacement_candidate), implementation_function_types);
        /**
         * Create function call
         */
        vec<tree> new_arguments;
        new_arguments.create(gimple_call_num_args(stmt) + 1);
        new_arguments.safe_push(DECL_ARGUMENTS(cfun->decl));

        tree field1 = build_decl(UNKNOWN_LOCATION, VAR_DECL, get_identifier(TREE_STRING_POINTER(api.pd_minus_field)), integer_type_node);
        DECL_EXTERNAL(field1) = 1;
        TREE_PUBLIC(field1) = 1;
        tree field2 = build_decl(UNKNOWN_LOCATION, VAR_DECL, get_identifier(TREE_STRING_POINTER(api.pd_plus_field)), integer_type_node);
        DECL_EXTERNAL(field2) = 1;
        TREE_PUBLIC(field2) = 1;
        tree tmp1 = build2(MINUS_EXPR, integer_type_node, DECL_CHAIN(DECL_ARGUMENTS(cfun->decl)), field1);
        tree tmp2 = build2(PLUS_EXPR, integer_type_node, tmp1, field2);
        new_arguments.safe_push(tmp2);
        for (unsigned int i = 1; i < gimple_call_num_args(stmt); ++i)
          {
             new_arguments.safe_push(gimple_call_arg(stmt, i));
          }

        gcall *new_call = gimple_build_call_vec(implementation_function_declaration, new_arguments);

        const greturn *ret = as_a <const greturn *> (stmt);
        tree return_val = gimple_return_retval(ret);
        gimple_return_set_retval(as_a<greturn *>(new_call), return_val);

        gsi_replace(&gsi, new_call, true);

        auto removal = gsi_for_stmt(c.efl_super_call);
        gsi_remove(&removal, false);
        removal = gsi_for_stmt(c.class_get_call);
        gsi_remove(&removal, false);

      }
    }
  return 0;
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
  register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &eo_pass_info);
  register_callback (plugin_name, PLUGIN_ATTRIBUTES, register_next_hop_attribute, NULL);
  return 0;
}
