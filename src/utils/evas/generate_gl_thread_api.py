#
# GL threaded API generate tool
#
import os, sys, re
import argparse

global g_verbose
global g_print_err
global g_debug_lvl
global g_no_indent

global g_internal_total
global g_evgl_total

api_dic_list = list()

_NEWLINE = "\n"
_MAX_COLUMN = 10

################################################################################
# api_def data format:
#  1.| {INTERNAL|EVAS GL} | : Wrapper API types
#  2.| {return type} |
#  3.| {api name} |
#  4.| {gl api} |
#  5.| {paramter1, 2,..} |
#  6.| {enqueue|flush|finish} | : queue operation
#  7.| {ext|noext} | : extension API or not
#  8.| {memory pool name, size, value | ""} |
#  9.| {GL| GLES} | ""} | : GL types
# 10.| {"NeedMacro" | ""} | : user macro need or not

#--------------------------------------------------------------
# parse API data (txt -> dictionary)
# input  : a line of data file, it contains attributes for warpper API generate
def parse_apidef(api):
	api_dic = {}
	parse_result = { 'parse_fail':True, 'type':"", 'queue_opr':"", 'ext':False,
					 'warning':list(),
		             'needCopy':False, 'needMacro':False, 'gl_type':False }

	L = api.split('|')

	if len(L) < _MAX_COLUMN :
		print "*warning*\nfew column, check format,..(%s)" %(api)
		return parse_result # few column, invalid format

	api_dic['type'] = L[1].strip()
	api_dic['macro'] = "GL_TH_FN"
	api_dic['qtype'] = "EVAS_GL_THREAD_TYPE_GL"
	api_dic['env_check'] = "!evas_gl_thread_enabled(thread_type)"

	parse_result['type'] = L[1].strip()

	api_dic['return_type'] = L[2].strip()
	api_dic['api_name'] = L[3].strip()

	# make parameter list and name list
	para_list = list()
	para_name_list = list()

	p = re.compile('[a-zA-Z_+][_a-zA-Z0-9]*') # variable pattern

	for para in L[4].split(','):
		para_list.append(para.strip())

		match_list = p.findall(para.strip())
		if len(match_list) < 2 :
			pass # invalid type or variable name OR 'void'
		else :
			para_name_list.append(match_list[len(match_list) -1])

	api_dic['para_list'] = para_list
	api_dic['para_name_list'] = para_name_list

	api_dic['queue_opr'] = L[5].strip()
	parse_result['queue_opr'] = L[5].strip()

	api_dic['extYN'] = L[6].strip()

	if len(L[7].strip()) > 0 and api_dic['queue_opr'] == "flush": # need copy
		parse_result['needCopy'] = True
		copy_info = L[7].split(',')

		if len(copy_info) is 3 : # {memory pool name, size, val}
			api_dic['mp_name'] = copy_info[0].strip()
			api_dic['copy_size'] = copy_info[1].strip()
			api_dic['copy_val'] = copy_info[2].strip()
		else :
			parse_result['warning'].append("invalid copy info,..(%s)" %(api_dic['api_name']))

	if len(L[8].strip()) > 0 : #if defiend gl_type
		api_dic['gl_type'] = L[8].strip()
		parse_result['gl_type'] = L[8].strip()

	if len(L[9].strip()) > 0 : #if defined userMacro
		if api_dic['queue_opr'] == "finish" :
			parse_result['warning'].append("user macro defined at aync operation only,..(%s)" %(api_dic['api_name']))
		else :
			api_dic['needMacro'] = True
			parse_result['needMacro'] = True

	api_dic_list.append(api_dic)

	parse_result['parse_fail'] = False

	return parse_result

#--------------------------------------------------------------
def add_result(parse_result, result) :

	if parse_result['parse_fail'] is True:
		result['fail'] += 1
	else :
		result['success'] += 1

	if parse_result['queue_opr'] == "flush" :
		result['flush'] += 1
	elif parse_result['queue_opr'] == "finish" :
		result['finish'] += 1
	elif parse_result['queue_opr'] == "enqueue" :
		result['enqueue'] += 1

	if parse_result['ext'] is True:
		result['ext'] += 1
	if parse_result['needCopy'] is True:
		result['needCopy'] += 1
	if parse_result['needMacro'] is True:
		result['needMacro'] += 1
	if parse_result['gl_type']  == "GL" :
		result['gl'] += 1
	if len(parse_result['warning']) > 0:
		result['warning'].append(parse_result['warning'])


def get_api_def(api_list_file) :

	warning_msgs = list()
	result  = list()
	result_evas = { 'success':0, 'fail':0, 'warning':warning_msgs,
		          'flush':0, 'finish':0, 'enqueue':0,
		          'ext':0, 'needCopy':0, 'needMacro':0, 'gl':0 }
	result_evgl = { 'success':0, 'fail':0, 'warning':warning_msgs,
		            'flush':0, 'finish':0, 'enqueue':0,
		            'ext':0, 'needCopy':0, 'needMacro':0, 'gl':0 }
	result_evgl_api = { 'success':0, 'fail':0, 'warning':warning_msgs,
		                'flush':0, 'finish':0, 'enqueue':0,
		                'ext':0, 'needCopy':0, 'needMacro':0, 'gl':0 }
	f = open(api_list_file, 'r')

	lines = f.readlines()
	f.close()

	for line in lines :
		api = line.strip()

		if len(api) is 0 : #empty line
			continue

		if api[0] is '/' and api[1] is '*' : # "/*" comment start
			continue
		if api[0] is '*' or api[0] is '/' :
			continue

		if api[0] is not '#' : # if not comment line
			parse_result = parse_apidef(api)
			add_result(parse_result, result_evas)

	result.append(result_evas)
	result.append(result_evgl)
	result.append(result_evgl_api)

	return result

#------------------------------------------------------
# Utility functions,...
#------------------------------------------------------
def list_to_str (name_list) :
	name_str = ""
	for p in name_list:
		if (name_list.index(p) > 0) :
			name_str += ", "
		name_str += p

	return name_str

def LOG (buf) :
	if g_verbose: print buf

def FWRITE (fp, buf) :
	fp.write(buf)

# check gl_type, if gl_type is "GL" need "#ifndef GL_GLES ~ #endif"
def ifdef_stmt (api_dic) :
	if 'gl_type' in api_dic and api_dic['gl_type'] == "GL" :
		return "\n#ifndef GL_GLES"
	return ""

def endif_stmt (api_dic) :
	if 'gl_type' in api_dic and api_dic['gl_type'] == "GL" :
		return "\n#endif"
	return ""

# debug lvl is 1 ----------------------------------------------
def define_counter(api_dic) :

	counter = ifdef_stmt(api_dic)
	counter += "\nint cb_%s_counter = 1;" %(api_dic['api_name'])
	counter += "\nint evas_%s_counter = 1;" %(api_dic['api_name'])

	counter += endif_stmt(api_dic)

	return counter

def logging_and_counter (func_name) :
	func_body = _NEWLINE
	if g_print_err : func_body += "ERR(\"(counter:%d)\\n\", "
	else           : func_body += "fprintf(stderr,\" %s(%d)\\n\", __func__, "
	func_body += "%s_counter++);" %(func_name)
	func_body += "\n"

	return func_body

# debug lvl is 2 ----------------------------------------------
def define_timeval() :
	func_body = _NEWLINE
	func_body += "struct timeval tv1, tv2; /* time value 1, 2 */"
	func_body += _NEWLINE
	func_body += "double et;               /* elapsed time */"
	func_body += _NEWLINE

	return func_body

def elapsed_time (tv1, tv2) :
	func_body = _NEWLINE
	func_body += "et = (%s.tv_sec - %s.tv_sec) * 1000.0;\n" %(tv1, tv2)
	func_body += "et += (%s.tv_usec - %s.tv_usec) / 1000.0;\n" %(tv1, tv2)
	func_body += _NEWLINE

	return func_body

# debug lvl is 3 ----------------------------------------------
def check_glError() :
	func_body = _NEWLINE
	func_body += "/* for debug, check glError */\n"
	func_body += "GLenum err = glGetError();\n"
	func_body += "if (err != GL_NO_ERROR)\n"
	func_body += "ERR(\"glGetError(%x)\", err);\n"
	func_body += _NEWLINE

	return func_body

################################################################################

#--------------------------------------------------------------
def gen_structure(api_def):

	return_type = api_def['return_type']
	api_name = api_def['api_name'].strip()
	para_list = api_def['para_list']
	para_name_list = api_def['para_name_list']
	extYN = api_def['extYN']
	queue_opr = api_def['queue_opr']

	prototype = "typedef %s(*GL_TH_FNTYPE(%s))(%s);" % (return_type, api_name, list_to_str(para_list))

	s_member = ""

	s_member += _NEWLINE
	s_member += "GL_TH_FNTYPE(%s) orig_func;" % (api_name)

	if g_debug_lvl is 2:
		s_member += _NEWLINE
		s_member += "struct timeval tv;"

	if return_type != "void" :
		s_member += _NEWLINE
		s_member += return_type + " return_value;"
	for p in para_list :
		if p ==  "void":
			pass
		else :
			s_member += _NEWLINE
			s_member += p + ";"

	if 'copy_val' in api_def :
		s_member += _NEWLINE
		s_member += "void *%s_copied; /* COPIED */" %(api_def['copy_val'])

	if 'needMacro' in api_def :
		s_member += _NEWLINE
		s_member += "%s_VARIABLE_DECLARE /* TODO */" %(api_name.upper())
	s_member += _NEWLINE

	struct_data = """
%s

typedef struct
{%s
} GL_TH_ST(%s);
""" %(prototype, s_member, api_name)

	return struct_data

#--------------------------------------------------------------
def gen_function_pointer(api_def_dic) :

	return_type = api_def_dic['return_type']
	api_name = api_def_dic['api_name']
	para_data = list_to_str(api_def_dic['para_list'])
	macro = api_def_dic['macro']

	# function pointer name
	func_body = "\n"
	fp_name = "ORIG_GL(%s)" %(api_name)

	func_body += "%s (*%s)(%s);\n" %(return_type, fp_name, para_data)

	if api_def_dic['extYN'] == "ext" :
		func_body += """
void
%s_SET(%s)(void *func)
{
%s = func;
}

void *
%s_GET(%s)(void)
{
return %s;
}
""" %(macro, api_name, fp_name,
	  macro, api_name, fp_name)

	return func_body

#--------------------------------------------------------------
def gen_gl_thread_api(api_def_dic) :

	api_name = api_def_dic['api_name']
	return_type = api_def_dic['return_type']
	para_name_list = api_def_dic['para_name_list']
	extYN = api_def_dic['extYN']
	queue_opr = api_def_dic['queue_opr']

	gl_call = "((GL_TH_FNTYPE(%s))thread_data->orig_func)\n" % (api_name)
	para_data = "void *data"
	func_body = ""

	# logging & increase counter
	if g_debug_lvl is 1 :
		func_body += logging_and_counter("cb_" + api_name)
	elif g_debug_lvl is 2 :
		func_body += define_timeval()

	func_body += _NEWLINE
	func_body += "GL_TH_ST(%s) *thread_data =\n" %(api_name)
	func_body += "*(void **)data;\n"

	func_body += _NEWLINE

	# get time of callback function call,..
	if g_debug_lvl is 2:
		func_body += "gettimeofday(&tv1, NULL);\n"
		func_body += elapsed_time("tv1", "(thread_data->tv)")
		func_body += "printf(\"[%f] callback(%s)\\n\", et, __func__);"
		func_body += "\n\n"

	# if needMacro
	if 'needMacro' in api_def_dic :
		func_body += "\n"
		func_body += "%s_GLCALL_BEFORE; /* TODO */\n\n" %(api_name.upper())

	func_body += _NEWLINE

	if return_type != "void" :
		if g_debug_lvl is 3 :
			func_body += "%s return_value = " %(return_type)
		else :
			func_body += "thread_data->return_value = "
		func_body += "%s(" %(gl_call) #GL API or function pointer call
	else :
		func_body += "%s(" %(gl_call) #GL API or function pointer call

	for p in para_name_list:
		if (para_name_list.index(p) > 0) :
			func_body += ","
			func_body += _NEWLINE
		func_body += "thread_data->%s" %(p)
	func_body += ");\n"

	# get time of GL API done,..
	if g_debug_lvl is 2:
		func_body += _NEWLINE
		func_body += "gettimeofday(&tv2, NULL);\n"
		func_body += elapsed_time("tv2", "tv1")
		func_body += "printf(\"\\t[%f] GL done.\\n\", et);"
		func_body += _NEWLINE
		func_body += "memcpy(&(thread_data->tv), &tv2, sizeof(struct timeval));\n"

	if g_debug_lvl is 3 :
		func_body += _NEWLINE
		func_body += check_glError()
		if return_type != "void" :
			func_body += "thread_data->return_value = return_value;\n"

	# if needMacro
	if 'needMacro' in api_def_dic :
		func_body += "\n"
		func_body += "%s_GLCALL_AFTER; /* TODO */\n" %(api_name.upper())

	# if copy variable, free
	if 'copy_val' in api_def_dic:
		func_body += "\n"
		func_body += _NEWLINE
		func_body += "if (thread_data->%s_copied)" %(api_def_dic['copy_val'])
		func_body += _NEWLINE
		func_body += "eina_mempool_free(%s, thread_data->%s_copied);\n" %(api_def_dic['mp_name'],
                                                                   api_def_dic['copy_val'])

	return """
static void
GL_TH_CB(%s)(%s)
{%s
}
""" %(api_name, para_data, func_body)

#--------------------------------------------------------------
def gen_thread_cmd_api(api_def_dic) :

	api_name = api_def_dic['api_name']
	return_type = api_def_dic['return_type']
	macro = api_def_dic['macro']
	para_list = api_def_dic['para_list']
	para_name_list = api_def_dic['para_name_list']
	queue_opr = api_def_dic['queue_opr']
#	if queue_opr == "flush" or queue_opr == "enqueue" :
#		print "(%s)%s:%s" %(api_type, api_name, queue_opr)
	extYN = api_def_dic['extYN']
	env_check = api_def_dic['env_check']
	queue_type = api_def_dic['qtype']

	gl_call = "((GL_TH_FNTYPE(%s))orig_func)\n" % (api_name)

	if 'copy_val' in api_def_dic :
		mp_name = api_def_dic['mp_name']
		copy_val = api_def_dic['copy_val']
		copy_size = api_def_dic['copy_size']

	para_data = ""
	func_body = ""

	if len(para_name_list) == 0 :
		para_data += "GL_TH_DP"
	else :
		para_data += "GL_TH_DP, "
		para_data += list_to_str(para_list)

	para_val = "NULL"

	# thread_mode
	func_body += _NEWLINE
	func_body += "int thread_mode = EVAS_GL_THREAD_MODE_FINISH;\n"

	# Has structure
	func_body += _NEWLINE
	func_body += "GL_TH_ST(%s) thread_data_local, *thread_data = &thread_data_local, **thread_data_ptr;\n" %(api_name)
	func_body += "void *thcmd_ref;\n"
	func_body += _NEWLINE

	# logging & increase counter
	if g_debug_lvl is 1 :
		fp_name = "evas_"
		func_body += logging_and_counter( fp_name + api_name )
	# if debug level 2, define timeval
	elif g_debug_lvl is 2 :
		func_body += _NEWLINE
		func_body += "printf(\"%s\\n\", __func__);\n"
		func_body += define_timeval()

	# check use_gl_thread_cmd()
	func_body += _NEWLINE
	func_body += "if (%s)" %(env_check)
	func_body += _NEWLINE
	func_body += "{"
	func_body += _NEWLINE

	if g_debug_lvl is 2: # if debug level 2, check elapsed time
		func_body += _NEWLINE
		func_body += "gettimeofday(&tv1, NULL);\n"
		func_body += _NEWLINE
		if return_type != "void" :
			func_body += "%s rc;" %(return_type)
			func_body += _NEWLINE
			func_body += "rc = %s(%s);" %(gl_call, list_to_str(para_name_list))
		else :
			func_body += "%s(%s);" %(gl_call, list_to_str(para_name_list))

		func_body += _NEWLINE
		func_body += "gettimeofday(&tv2, NULL);\n"
		func_body += elapsed_time("tv2", "tv1")
		func_body += "printf(\"\\t[%f] GL done. RENDER_THREAD_OFF\\n\", et);\n"

		if return_type != "void" :
			func_body += "return rc;"
			func_body += _NEWLINE
		else :
			func_body += "return;"
			func_body += _NEWLINE

	else : # not debug mode
		if return_type == "void" :
			func_body += "%s(%s);" %(gl_call, list_to_str(para_name_list))
			func_body += _NEWLINE
			func_body += "return;"

		else :
			func_body += "return %s(%s);" %(gl_call, list_to_str(para_name_list))
		func_body += _NEWLINE

	func_body += "}"
	func_body += _NEWLINE

	# Has structure
	#if (return_type != "void") or (len(para_name_list) > 0) or (g_debug_lvl is 2):
	# check force_finish() before malloc
	func_body += "thread_data_ptr =\n"
	if queue_opr == "finish" :
		func_body += "evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(%s) *), &thcmd_ref);\n" %(api_name)
		func_body += "*thread_data_ptr = thread_data;\n"
	else :
		func_body += "evas_gl_thread_cmd_create(thread_type, sizeof(GL_TH_ST(%s) *) + sizeof(GL_TH_ST(%s)), &thcmd_ref);\n" %(api_name, api_name)
		func_body += "*thread_data_ptr = (void *)((char *)thread_data_ptr + sizeof(GL_TH_ST(%s) *));\n" % (api_name)
		func_body += "thread_data = *thread_data_ptr;\n"

	if queue_opr != "finish" :
		func_body += _NEWLINE
		func_body += "if (!evas_gl_thread_force_finish())\n"
		func_body += "   thread_mode = EVAS_GL_THREAD_MODE_%s;\n" %(queue_opr.upper())

	# assign parameter to thread_data
	for p in para_name_list :
		func_body += _NEWLINE

		idx = para_list[para_name_list.index(p)].find('[') # if parameter is array
		if idx > 0 :
			p_dat = para_list[para_name_list.index(p)].split() # type name[size]
			p_type = p_dat[0]               # get type
			p_temp = p_dat[1].split('[')    # get size
			p_size = p_temp[1].split(']')
			func_body += "memcpy(thread_data->%s, &%s, sizeof(%s) * %s);" %(p, p, p_type, p_size[0])
		else :
			func_body += "thread_data->%s = %s;" %(p, p)

	func_body += _NEWLINE
	func_body += "thread_data->orig_func = orig_func;"

	func_body += _NEWLINE
	# end of assign

	if 'copy_val' in api_def_dic :
		func_body += _NEWLINE
		func_body += "thread_data->%s_copied = NULL;" %(copy_val)

	if 'needMacro' in api_def_dic :
		func_body += _NEWLINE
		func_body += "%s_VARIABLE_INIT; /* TODO */\n" %(api_name.upper())

	if queue_opr != "finish" and ('copy_val' in api_def_dic or 'needMacro' in api_def_dic):
		func_body += _NEWLINE
		func_body += "if (thread_mode == EVAS_GL_THREAD_MODE_FINISH)\n"
		func_body += "goto finish;\n"

	# if copy_val defined, copy to given memory pool
	if 'copy_val' in api_def_dic :
		param = para_list[para_name_list.index(copy_val)]
		idx = param.find('*')
		p_type = param[:idx]

		func_body += _NEWLINE
		func_body += "/* copy variable */\n"
		func_body += "if (%s)\n" %(copy_val)
		func_body += "{\n"

		func_body += "/* 1. check memory size */\n"
		func_body += "unsigned int copy_size = %s;\n" %(copy_size)
		func_body += "if (copy_size > %s_memory_size)\n" %(mp_name)
		func_body += "{\n"
		func_body += "thread_mode = EVAS_GL_THREAD_MODE_FINISH;\n"
		func_body += "goto finish;\n"
		func_body += "}\n"

		func_body += "/* 2. malloc & copy */\n"
		func_body += "thread_data->%s_copied = " %(copy_val)
		func_body += "eina_mempool_malloc(%s, copy_size);\n" %(mp_name)
		func_body += "if (thread_data->%s_copied)\n" %(copy_val)
		func_body += "{\n"
		func_body += "memcpy(thread_data->%s_copied, %s, copy_size);\n" %(copy_val, copy_val)
		func_body += "}\n"
		func_body += "else\n"
		func_body += "{\n"
		func_body += "thread_mode = EVAS_GL_THREAD_MODE_FINISH;\n"
		func_body += "goto finish;\n"
		func_body += "}\n"

		func_body += "/* 3. replace */\n"
		func_body += "thread_data->%s = (%s *)thread_data->%s_copied;\n" %(copy_val, p_type, copy_val)
		func_body += "}\n"

		func_body += "/* end of copy variable */"
		func_body += _NEWLINE

	# get time of enqueue
	if g_debug_lvl is 2:
		func_body += "gettimeofday(&tv1, NULL);\n"
		func_body += "memcpy(&(thread_data->tv), &tv1, sizeof(struct timeval));"
		func_body += _NEWLINE

	if 'needMacro' in api_def_dic :
		func_body += _NEWLINE
		func_body += "%s_ASYNC_PREPARE; /* TODO */\n" %(api_name.upper())

	if queue_opr != "finish" and ('copy_val' in api_def_dic or 'needMacro' in api_def_dic):
		func_body += _NEWLINE
		func_body += "finish:"

	if 'needMacro' in api_def_dic :
		func_body += _NEWLINE
		func_body += "%s_ENQUEUE_BEFORE; /* TODO */\n" %(api_name.upper())

	func_body += _NEWLINE
	func_body += "evas_gl_thread_cmd_enqueue(thcmd_ref,\n"
	func_body += "GL_TH_CB(%s),\n" %(api_name)
	func_body += "thread_mode);"
	func_body += _NEWLINE

	if 'needMacro' in api_def_dic :
		func_body += _NEWLINE
		func_body += "%s_ENQUEUE_AFTER; /* TODO */\n" %(api_name.upper())

	if g_debug_lvl is 2:
		func_body += _NEWLINE
		func_body += "gettimeofday(&tv2, NULL);\n"
		func_body += elapsed_time("tv2", "(thread_data->tv)")
		func_body += "printf(\"\\t\\t[%f] return\\n\", et);"
		func_body += _NEWLINE
		func_body += "gettimeofday(&tv2, NULL);\n"
		func_body += elapsed_time("tv2", "tv1")
		func_body += "printf(\"\\t\\t\\t[%f]\\n\", et);"
		func_body += _NEWLINE

	# Has return
	if return_type != "void" :
		func_body += _NEWLINE
		func_body += "return thread_data->return_value;"
		func_body += _NEWLINE

	return """
%s
%s(%s)(%s)
{%s}
""" %(return_type, macro, api_name, para_data, func_body)

#--------------------------------------------------------------
def gen_wrapper_api_header(macro, need_ext, api_def_dic) :

	api_name = api_def_dic['api_name']
	return_type = api_def_dic['return_type']
	para_data = list_to_str(api_def_dic['para_list'])
	extYN = api_def_dic['extYN']

	header_data = _NEWLINE

	func_name = "%s(%s)" %(macro, api_name)

	if para_data == "void" :
		para_data = "GL_TH_DP"
	else :
		para_data = "GL_TH_DP, " + para_data

	header_data += "extern %s %s(%s);" %(return_type, func_name, para_data)

	return header_data

#--------------------------------------------------------------
# for debug
def api_define_print(api) :

	return """
/*
* %s
* %s(%s);
*/
""" %(api['return_type'], api['api_name'], list_to_str(api['para_list']))

################################################################################
def function_pointer_structure(macro, need_ext, api_dic):

	api_name = api_dic['api_name']
	return_type = api_dic['return_type']
	para_data = list_to_str(api_dic['para_list'])
	extYN = api_dic['extYN']

	mem_data = ifdef_stmt(api_dic)

	if para_data == "void" :
		para_data = "GL_TH_DP"
	else :
		para_data = "GL_TH_DP, " + para_data

	mem_data += "\n%s (*%s(%s))(%s);" %(return_type, macro, api_name, para_data)

	mem_data += endif_stmt(api_dic)

	return mem_data
#--------------------------------------------------------------
def func_get_header(gl_type) :

	return """
static Evas_GL_Thread_%s_Generated_Func th_%s_gen_func;
Eina_Bool th_%s_gen_func_initialized = EINA_FALSE;

void *
%s_generated_func_get()
{
   if (!th_%s_gen_func_initialized)
     {
#define THREAD_FUNCTION_ASSIGN(func) th_%s_gen_func.func = func;
""" %(gl_type.upper(), gl_type, gl_type,
	  gl_type, gl_type, gl_type)

def function_pointer_assign(macro, need_ext, api_dic):

	api_name = api_dic['api_name']
	return_type = api_dic['return_type']
	extYN = api_dic['extYN']

	func_body = ifdef_stmt(api_dic)

	func_body += "\nTHREAD_FUNCTION_ASSIGN(%s(%s));" %(macro, api_name)
	func_body += endif_stmt(api_dic)

	return func_body

def func_get_tail(api_type) :

	return """

#undef THREAD_FUNCTION_ASSIGN

        th_%s_gen_func_initialized = EINA_TRUE;
     }

   return &th_%s_gen_func;
}
""" %(api_type, api_type)

#--------------------------------------------------------------
# for _link_generate files
def function_pointer_init(macro, need_ext, api_dic):

	api_name = api_dic['api_name']
	return_type = api_dic['return_type']
	para_data = list_to_str(api_dic['para_list'])
	extYN = api_dic['extYN']

	fp_init = ifdef_stmt(api_dic)

	if para_data == "void" :
		para_data = "GL_TH_DP"
	else :
		para_data = "GL_TH_DP, " + para_data

	fp_init += "\n%s (*%s(%s))(%s) = NULL;" %(return_type, macro, api_name, para_data)

	fp_init += endif_stmt(api_dic)

	return fp_init

def function_pointer_extern(macro, need_ext, api_dic):

	api_name = api_dic['api_name']
	return_type = api_dic['return_type']
	para_data = list_to_str(api_dic['para_list'])
	extYN = api_dic['extYN']

	fp_extern = ifdef_stmt(api_dic)

	if para_data == "void" :
		para_data = "GL_TH_DP"
	else :
		para_data = "GL_TH_DP, " + para_data

	fp_extern += "\nextern %s (*%s(%s))(%s);" %(return_type, macro, api_name, para_data)
	fp_extern += endif_stmt(api_dic)

	return fp_extern

def init_function_extern(api_type) :

	return """

extern void _gl_thread_link_%s_generated_init(void *func_ptr);
""" %(api_type)

#--------------------------------------------------------------
def func_init_header(api_type) :

	return """
void
_gl_thread_link_%s_generated_init(void *func_ptr)
{
   const Evas_GL_Thread_%s_Generated_Func *th_%s_gen_func = func_ptr;

   if (!th_%s_gen_func)
     {
        ERR("Thread functions (%s-Generated BASE) are not exist");
        return;
     }

#define THREAD_FUNCTION_ASSIGN(func) func = th_%s_gen_func->func;

""" %(api_type, api_type.upper(), api_type, api_type, api_type.upper(), api_type)

def func_init_tail() :

	return """
#undef THREAD_FUNCTION_ASSIGN

}
"""

###############################################################################
def generate_files(out_fname) :

	gen_files = {} # {file name : file pointer}

	gl_src = out_fname + "_gl_generated.c"
	gl_hdr = out_fname + "_gl_generated.h"

	gen_files[gl_src] = open(gl_src, 'w')
	gen_files[gl_hdr] = open(gl_hdr, 'w')

	gl_link_src = out_fname + "_gl_link_generated.c"
	gl_link_hdr = out_fname + "_gl_link_generated.h"

	gen_files[gl_link_src] = open(gl_link_src, 'w')
	gen_files[gl_link_hdr] = open(gl_link_hdr, 'w')

	# file truncate & write auto gen message
	auto_gen_msg = """/*
 * ------------------------------------------------------------------------------
 * This is an automatically generated file using a python script.
 * ($EFL_HOME/src/utils/evas/generate_gl_thread_api.py)
 * Recommend that you modify data files ($EFL_HOME/src/utils/evas/gl_api_def.txt)
 * and make use of scripts if you need to fix them.
 * ------------------------------------------------------------------------------
 */
"""
	files = gen_files.keys()

	for f in files :
		gen_files[f].truncate()
		gen_files[f].write(auto_gen_msg)

	return gen_files
#--------------------------------------------------------------
def generate_code(out_files) :

	files = out_files.keys()
	# get file pointer,...
	for f in files :

		# EVAS
		if f.find("_gl_generated.c") > 0 :
			gl_source_file = out_files[f]
			if g_debug_lvl is 2 : gl_source_file.write("#include <sys/time.h>\n")
		if f.find("_gl_generated.h") > 0 :
			gl_header_file = out_files[f]
		if f.find("_gl_link_generated.c") > 0 :
			gl_link_source_file = out_files[f]
		if f.find("_gl_link_generated.h") > 0 :
			gl_link_header_file = out_files[f]

	# include headers
	evas_gl_include = """
#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

"""
	gl_header_file.write(evas_gl_include)
	#if g_internal_total > 0 	: gl_header_file.write(evas_gl_include)

	#print ">>> code generate START >>>"
	#if debug mode, define counter variable
	if g_debug_lvl is 1 :
		for api_dic in api_dic_list :

			fp_src = gl_source_file

			def_counter = define_counter(api_dic)
			LOG(def_counter)
			FWRITE(fp_src, def_counter)

	### generate *_gl_thread_{gl|evgl|evgl_api}_generated files >>>>
	for api_dic in api_dic_list :

		fp_src = gl_source_file
		fp_hdr = gl_header_file

		### ifndef,...
		LOG(ifdef_stmt(api_dic))
		FWRITE(fp_src, ifdef_stmt(api_dic))

		### API prototype (for debug)
		debug_msg = api_define_print(api_dic)
		LOG(debug_msg)
		FWRITE(fp_src, debug_msg)

		### data structure
		struct_data = gen_structure(api_dic)
		if len(struct_data) > 0 :
			LOG(struct_data)
			FWRITE(fp_src, struct_data)

		# generate call-back function
		api_data = gen_gl_thread_api(api_dic)
		LOG(api_data)
		FWRITE(fp_src, api_data)

		# generate wrapper function
		api_data = gen_thread_cmd_api(api_dic)
		LOG(api_data)
		FWRITE(fp_src, api_data)

		###  endif
		LOG(endif_stmt(api_dic))
		FWRITE(fp_src, endif_stmt(api_dic))

		### header
		header_data = ifdef_stmt(api_dic)
		header_data += gen_wrapper_api_header("GL_TH_FN", True, api_dic)
		header_data += endif_stmt(api_dic)

		LOG(header_data)
		FWRITE(fp_hdr, header_data)

	### structure Evas_Gl_Thread_{GL|EVGL}_Generated_Func >>
	FWRITE(gl_source_file, "\n\ntypedef struct\n{")

	for api_dic in api_dic_list :

		struct_member = function_pointer_structure("GL_TH_FN", True, api_dic)
		LOG(struct_member)
		FWRITE(gl_source_file, struct_member)

	FWRITE(gl_source_file, "\n} Evas_GL_Thread_GL_Generated_Func;\n\n")

	#### generate gl_generated_func_get() >>
	FWRITE(gl_source_file, func_get_header("gl"))

	for api_dic in api_dic_list :

		macro_data = function_pointer_assign("GL_TH_FN", True, api_dic)
		LOG(macro_data)
		FWRITE(gl_source_file, macro_data)

	FWRITE(gl_source_file, func_get_tail("gl"))


	### generate *_gl_thread_{gl|evgl}_link_generated files >>>>

	## function pointer init. & extern
	for api_dic in api_dic_list :

		func_init_data = function_pointer_init("GL_TH_FN", True, api_dic)
		LOG(func_init_data)
		FWRITE(gl_link_source_file, func_init_data)

		func_ptr_extern = function_pointer_extern("GL_TH_FN", True, api_dic)
		LOG(func_ptr_extern)
		FWRITE(gl_link_header_file, func_ptr_extern)

	FWRITE(gl_link_header_file, init_function_extern("gl"))

	### structure Evas_Gl_Thread_{GL|EVGL}_Generated_Func >>
	FWRITE(gl_link_source_file, "\n\ntypedef struct\n{")

	for api_dic in api_dic_list :

		struct_member = function_pointer_structure("GL_TH_FN", True, api_dic)
		LOG(struct_member)
		FWRITE(gl_link_source_file, struct_member)

	FWRITE(gl_link_source_file, "\n} Evas_GL_Thread_GL_Generated_Func;\n\n")

	#### generate _gl_thread_link_{gl|evgl}_generated_init() >>
	FWRITE(gl_link_source_file, func_init_header("gl"))

	for api_dic in api_dic_list :

		macro_data = function_pointer_assign("GL_TH_FN", True, api_dic)
		LOG(macro_data)
		FWRITE(gl_link_source_file, macro_data)

	FWRITE(gl_link_source_file, func_init_tail())

	for f in files :
		out_files[f].close()

#############################################################
def print_result(result):

	print "* data parsing success: %d, failed: %d" %(result['success'], result['fail'])
	if len(result['warning']) > 0 :
		for msg in result['warning'] :
			print "- warning: %s" %(msg)
	print "- enqueue: %d, flush: %d, finish: %d" %(result['enqueue'],
			                                 result['flush'], result['finish'])
	print "- extension API: %d" %(result['ext'])
	print "- need Copy variable: %d, userMacro: %d" %(result['needCopy'], result['needMacro'])
	print "- GL types : %d" %(result['gl'])

#-----------------------------------------------------------#
if __name__ == '__main__':

	parser = argparse.ArgumentParser()
	parser.add_argument('-i',
						required=True,
						dest='input_data',
						action='store',
						help='API data file name')
	parser.add_argument('-o',
						dest='out_file_name',
						action='store',
						default='evas_gl_thread',
						help='output (generated file) name prefix')
	parser.add_argument('-d', '--debug',
						type=int,
						choices=range(0,4),
						dest='debug_level',
						action='store',
						default='0',
						help='set debug level')
	parser.add_argument('--print-err',
						dest='print_err',
						action='store_true',
						default=False,
						help='use ERR(), when debug level 1')
	parser.add_argument('--no-indent',
						dest='no_indent',
						action='store_true',
						default=False,
						help='No indentation process to make faster')
	parser.add_argument('--deploy',
						dest='deploy',
						action='store_true',
						default=False,
						help='copy output (generated) file to evas engines')
	parser.add_argument('--verbose',
						dest='verbose',
						action='store_true',
						default=False)
	parser.add_argument('--version',
						action='version',
						version='%(prog)s 2.02')

	args = parser.parse_args()
	g_verbose = args.verbose
	g_debug_lvl = args.debug_level
	g_print_err = args.print_err
	g_no_indent = args.no_indent
	g_internal_total = 0
	g_evgl_total = 0

	# parsing data file
	result = get_api_def(args.input_data)

	res_evas = result[0]
	res_evgl = result[1]
	res_evgl_api = result[2]

	g_internal_total = res_evas['success'] + res_evas['fail']
	g_evgl_total = res_evgl['success'] + res_evgl['fail']


	# parsing result >>>
	print "[INTERNAL:%d]" %(g_internal_total)
	if g_internal_total > 0 : print_result(res_evas)

	print "[EVAS GL:%d]" %(g_evgl_total)
	if g_evgl_total > 0 : print_result(res_evgl)

	print "* Total APIs: %d" %(g_internal_total + g_evgl_total)

	# generate files
	gen_files = generate_files(args.out_file_name)

	# generate codes
	generate_code(gen_files)

	print "* generated files>>"
	for f in gen_files :
		print f

	#eflindent
	if args.no_indent == False :
		indent = """vim -c "set ts=8" -c "set sw=3" -c "set sts=3" -c "set expandtab" """
		indent += """ -c "set cino=>5n-3f0^-2{2(0W1st0" -c "normal gg=G" -c "wq" """
		for f in gen_files :
			os.system(indent + f)

		print "* eflindent done"

	if args.deploy is True:
		command = "cp %s*_generated.[ch] " %(args.out_file_name)
		command += "./../../modules/evas/engines/gl_common"
		os.system(command)
		print command
		print "* deploy done."

