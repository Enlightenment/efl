#
# GL threaded API generate tool
#
import os, sys, re
import argparse

global g_verbose
global g_print_err
global g_debug_lvl

global g_evas_total
global g_evgl_total
global g_evgl_api_total

api_dic_list = list()

_NEWLINE = "\n"
_MAX_COLUMN = 9

################################################################################
# api_def data format:
# 1.| {EVAS |EVAS GL |EVAS GL API} | : Wrapper API types
# 2.| {return type} |
# 3.| {api name} |
# 4.| {paramter1, 2,..} |
# 5.| {enqueue|flush|finish} | : queue operation
# 6.| {ext|noext} | : extension API or not
# 7.| {memory pool name, size, value | ""} |
# 8.| {GL| GLES} | ""} | : GL types
# 9.| {"NeedMacro" | ""} | : user macro need or not

def parse_apidef(api):
	api_dic = {}
	parse_result = { 'parse_fail':True, 'type':"", 'queue_opr':"", 'ext':False, 'warning':"",
		             'needCopy':False, 'needMacro':False, 'gl_type':False }

	L = api.split('|')

	if len(L) < _MAX_COLUMN :
		print "*warning*\nfew column, check format,..(%s)" %(api)
		return parse_result # few column, invalid format

	api_dic['type'] = L[1].strip()
	if api_dic['type'] == "EVAS GL API" :
		api_dic['func_ptr'] = "orig_evgl_api"

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
	if L[6].strip() == "ext":
		parse_result['ext'] = True
		if api_dic['type'] == "EVAS" : api_dic['func_ptr'] = "orig_evas"
		elif api_dic['type'] == "EVAS GL" : api_dic['func_ptr'] = "orig_evgl"

	if len(L[7].strip()) > 0 and api_dic['queue_opr'] == "flush": # need copy
		parse_result['needCopy'] = True
		copy_info = L[7].split(',')

		if len(copy_info) is 3 : # {memory pool name, size, val}
			api_dic['mp_name'] = copy_info[0].strip()
			api_dic['copy_size'] = copy_info[1].strip()
			api_dic['copy_val'] = copy_info[2].strip()
		else :
			parse_result['warning'] = "invalid copy info,..(%s)" %(api_dic['api_name'])

	if len(L[8].strip()) > 0 : #if defiend gl_type
		api_dic['gl_type'] = L[8].strip()
		parse_result['gl_type'] = L[8].strip()

	if len(L[9].strip()) > 0 : #if defined userMacro
		if api_dic['queue_opr'] == "finish" :
			parse_result['warning'] = "user macro defined at aync operation only,..(%s)" %(api_dic['api_name'])
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
			if parse_result['type'] == "EVAS" :
				add_result(parse_result, result_evas)

			elif parse_result['type'] == "EVAS GL" :
				add_result(parse_result, result_evgl)

			elif parse_result['type'] == "EVAS GL API" :
				add_result(parse_result, result_evgl_api)


	result.append(result_evas)
	result.append(result_evgl)
	result.append(result_evgl_api)

	return result

#--------------------------------------------------------------
def list_to_str (name_list) :
	name_str = ""
	for p in name_list:
		if (name_list.index(p) > 0) :
			name_str += ", "
		name_str += p

	return name_str

# debug_lvl is 1 or 2
def logging_and_counter (func_name) :

	func_body = _NEWLINE
	if g_print_err : func_body += "ERR(\"(counter:%d)\\n\", "
	else           : func_body += "fprintf(stderr,\" %s(%d)\\n\", __func__, "
	func_body += "%s_counter++);" %(func_name)
	func_body += "\n"

	return func_body

# debug_lvl is 2
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

# debug_lvl is 3
def check_glError() :

	func_body = _NEWLINE
	func_body += "/* for debug, check glError */\n"
	func_body += "GLenum err = glGetError();\n"
	func_body += "if (err != GL_NO_ERROR)\n"
	func_body += "ERR(\"glGetError(%x)\", err);\n"
	func_body += _NEWLINE

	return func_body

def LOG (buf) :
	if g_verbose: print buf

def FWRITE (fp, buf) :
	fp.write(buf)

################################################################################
def gen_structure(api_def_dic):

	api_type = api_def_dic['type']
	return_type = api_def_dic['return_type']
	api_name = api_def_dic['api_name'].strip()
	para_list = api_def_dic['para_list']
	para_name_list = api_def_dic['para_name_list']
	queue_opr = api_def_dic['queue_opr']
	if api_type == "EVAS" :
		struct_name_prefix = "Evas_Thread_Command"
	elif api_type == "EVAS GL" :
		struct_name_prefix = "EVGL_Thread_Command"
	elif api_type == "EVAS GL API" :
		struct_name_prefix = "EVGL_API_Thread_Command"

	s_member = ""
	if (return_type == "void") and (len(para_name_list) == 0) and g_debug_lvl is not 2 :
			return ""

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

	if 'copy_val' in api_def_dic :
		s_member += _NEWLINE
		s_member += "void *%s_copied; /* COPIED */" %(api_def_dic['copy_val'])

	if queue_opr != "finish" :
		s_member += _NEWLINE
		s_member += "int command_allocated;"

	if 'needMacro' in api_def_dic :
		s_member += _NEWLINE
		s_member += "%s_VARIABLE_DECLARE /* TODO */" %(api_name.upper())
	s_member += _NEWLINE

	struct_data = """
typedef struct
{%s
} %s_%s;
""" %(s_member, struct_name_prefix, api_name)

	return struct_data

#--------------------------------------------------------------
def gen_function_pointer(api_def_dic) :

	if ('func_ptr' in api_def_dic) is False :
		return ""

	func_ptr = api_def_dic['func_ptr']
	api_type = api_def_dic['type']
	return_type = api_def_dic['return_type']
	api_name = api_def_dic['api_name']
	para_data = list_to_str(api_def_dic['para_list'])

	fp_name = "%s_%s" %(func_ptr, api_name)
	func_body = "\n%s (*%s)(%s);\n" %(return_type, fp_name, para_data)

	if api_type == "EVAS GL API" :
		return func_body
	else :
		func_body += """
void
%s_%s_set(void *func)
{
%s = func;
}

void *
%s_%s_get(void)
{
return %s;
}
""" %(api_name, func_ptr, fp_name,
	  api_name, func_ptr, fp_name)

	return func_body

#--------------------------------------------------------------
def gen_gl_thread_api(api_def_dic) :

	api_type = api_def_dic['type']
	api_name = api_def_dic['api_name']
	return_type = api_def_dic['return_type']
	para_name_list = api_def_dic['para_name_list']
	extYN = api_def_dic['extYN']
	queue_opr = api_def_dic['queue_opr']
	gl_prefix = ""

	if api_type == "EVAS" :
		struct_name_prefix = "Evas_Thread_Command"
		func_name_prefix = "_gl_thread"
		if extYN == "ext" :	gl_prefix = "orig_evas_" # for extension
	elif api_type == "EVAS GL" :
		struct_name_prefix = "EVGL_Thread_Command"
		func_name_prefix = "_evgl_thread"
		if extYN == "ext" :	gl_prefix = "orig_evgl_" # for extension
	elif api_type == "EVAS GL API" :
		struct_name_prefix = "EVGL_API_Thread_Command"
		func_name_prefix = "_evgl_api_thread"
		gl_prefix = "orig_evgl_api_"

	para_data = "void *data"
	func_body = ""

	# logging & increase counter
	if g_debug_lvl is 1 :
		func_body += logging_and_counter(func_name_prefix + "_" + api_name)
	elif g_debug_lvl is 2 :
		func_body += define_timeval()

	if (return_type == "void") and (len(para_name_list) == 0) and g_debug_lvl is not 2:
			para_data += " EINA_UNUSED"

	else :
		func_body += _NEWLINE
		func_body += "%s_%s *thread_data =\n" %(struct_name_prefix, api_name)
		func_body += "(%s_%s *)data;\n" %(struct_name_prefix, api_name)

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

	# if EVAS GL API, need begin()~end()
	if api_type == "EVAS GL API":
		func_body += "evas_gl_thread_begin();\n"

	if return_type != "void" :
		if g_debug_lvl is 3 :
			func_body += "%s return_value = " %(return_type)
		else :
			func_body += "thread_data->return_value = "

	func_body += "%s%s(" %(gl_prefix, api_name) #GL API or function pointer call

	for p in para_name_list:
		if (para_name_list.index(p) > 0) :
			func_body += ","
			func_body += _NEWLINE
		func_body += "thread_data->%s" %(p)
	func_body += ");\n"

	if api_type == "EVAS GL API":
		func_body += "evas_gl_thread_end();\n"

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

	# thread_data free
	if (queue_opr != "finish") and len(para_name_list) > 0:
		func_body += "\n"
		func_body += "if (thread_data->command_allocated)\n"
		func_body += "eina_mempool_free(_mp_command, thread_data);"

	return """
static void
%s_%s(%s)
{%s
}
""" %(func_name_prefix, api_name, para_data, func_body)

#--------------------------------------------------------------
def gen_thread_cmd_api(api_def_dic) :

	api_type = api_def_dic['type']
	api_name = api_def_dic['api_name']
	return_type = api_def_dic['return_type']
	para_list = api_def_dic['para_list']
	para_name_list = api_def_dic['para_name_list']
	queue_opr = api_def_dic['queue_opr']
#	if queue_opr == "flush" or queue_opr == "enqueue" :
#		print "(%s)%s:%s" %(api_type, api_name, queue_opr)
	extYN = api_def_dic['extYN']
	gl_prefix = ""
	queue_type = ""
	if 'copy_val' in api_def_dic :
		mp_name = api_def_dic['mp_name']
		copy_val = api_def_dic['copy_val']
		copy_size = api_def_dic['copy_size']

	if api_type == "EVAS" :
		env_check = "!evas_gl_thread_enabled()"
		struct_name_prefix = "Evas_Thread_Command"
		func_name_prefix = "_gl_thread"
		func_suffix = "th"
		queue_type = "EVAS_GL_THREAD_TYPE_GL"
		if extYN == "ext" :	gl_prefix = "orig_evas_" # for extension
	elif api_type == "EVAS GL" :
		env_check = "!evas_evgl_thread_enabled()"
		struct_name_prefix = "EVGL_Thread_Command"
		queue_type = "EVAS_GL_THREAD_TYPE_EVGL"
		func_name_prefix = "_evgl_thread"
		func_suffix = "evgl_th"
		if extYN == "ext" :	gl_prefix = "orig_evgl_" # for extension
	elif api_type == "EVAS GL API" :
		env_check = "!evas_evgl_thread_enabled()"
		struct_name_prefix = "EVGL_API_Thread_Command"
		queue_type = "EVAS_GL_THREAD_TYPE_EVGL"
		func_name_prefix = "_evgl_api_thread"
		func_suffix = "evgl_api_th"
		gl_prefix = "orig_evgl_api_"

	para_data = ""
	func_body = ""

	if len(para_name_list) == 0 :
		para_data += "void"
	else :
		para_data += list_to_str(para_list)

	para_val = "NULL"

	# thread_mode
	func_body += _NEWLINE
	func_body += "int thread_mode = EVAS_GL_THREAD_MODE_FINISH;\n"

	# Has structure
	if (return_type != "void") or (len(para_name_list) > 0) or (g_debug_lvl is 2):
		func_body += _NEWLINE
		func_body += "%s_%s thread_data_local;\n" %(struct_name_prefix, api_name)
		func_body += "%s_%s *thread_data = &thread_data_local;\n" %(struct_name_prefix,
																		api_name)


	# logging & increase counter
	if g_debug_lvl is 1 :
		func_body += logging_and_counter("evas_" + api_name + "_" + func_suffix)
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
			func_body += "rc = "
		func_body += "%s%s(%s);\n" %(gl_prefix, api_name, list_to_str(para_name_list))
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
			func_body += "%s%s(%s);" %(gl_prefix, api_name, list_to_str(para_name_list))
			func_body += _NEWLINE
			func_body += "return;"

		else :
			func_body += "return %s%s(%s);" %(gl_prefix, api_name, list_to_str(para_name_list))
		func_body += _NEWLINE

	func_body += "}"
	func_body += _NEWLINE

	# Has structure
	if (return_type != "void") or (len(para_name_list) > 0) or (g_debug_lvl is 2):
		para_val = "thread_data"

		# check force_finish() before malloc
		if queue_opr != "finish" :
			func_body += _NEWLINE
			func_body += "/* command_allocated flag init. */\n"
			func_body += "thread_data->command_allocated = 0;\n"
			func_body += _NEWLINE
			func_body += "if (!evas_gl_thread_force_finish())\n"
			func_body += "{ /* _flush */\n"
			func_body += "%s_%s *thread_data_new;\n" %(struct_name_prefix, api_name)
			func_body += "thread_data_new = eina_mempool_malloc(_mp_command,\n"
			func_body += "sizeof(%s_%s));\n" %(struct_name_prefix, api_name)
			func_body += "if (thread_data_new)\n"
			func_body += "{\n"
			func_body += "thread_data = thread_data_new;\n"
			func_body += "thread_data->command_allocated = 1;\n"
			func_body += "thread_mode = EVAS_GL_THREAD_MODE_%s;\n" %(queue_opr.upper())
			func_body += "}\n"
			func_body += "}\n"

		# assign parameter to thread_data
		for p in para_name_list :
			func_body += _NEWLINE

			idx = para_list[para_name_list.index(p)].find('[') # if parameter is array
			if idx > 0 :
				p_dat = para_list[para_name_list.index(p)].split() # type name[size]
				p_type = p_dat[0]               # get type
				p_temp = p_dat[1].split('[')    # get size
				p_size = p_temp[1].split(']')
				func_body += "memcpy(thread_data->%s, &%s, sizeof(%s) * %s);" %(p, p, p_type,
																					p_size[0])
			else :
				func_body += "thread_data->%s = %s;" %(p, p)

		func_body += _NEWLINE
		# end of assign

		if 'copy_val' in api_def_dic :
			func_body += _NEWLINE
			func_body += "thread_data->%s_copied = NULL;" %(copy_val)

		if 'needMacro' in api_def_dic :
			func_body += _NEWLINE
			func_body += "%s_VARIABLE_INIT; /* TODO */\n" %(api_name.upper())

		if queue_opr != "finish" and ('copy_val' in api_def_dic or
										'needMacro' in api_def_dic):
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
	func_body += "evas_gl_thread_cmd_enqueue(%s,\n" %(queue_type)
	func_body += "%s_%s,\n" %(func_name_prefix, api_name)
	func_body += "%s,\n" %(para_val)
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
EAPI %s
evas_%s_%s(%s)
{%s}
""" %(return_type, api_name, func_suffix, para_data, func_body)

#--------------------------------------------------------------
def gen_wrapper_api_header(api_def_dic) :

	api_type = api_def_dic['type']
	api_name = api_def_dic['api_name']
	return_type = api_def_dic['return_type']
	para_data = list_to_str(api_def_dic['para_list'])
	extYN = api_def_dic['extYN']
	if api_type == "EVAS" : 		 api_suffix = "th"
	elif api_type == "EVAS GL" : 	 api_suffix = "evgl_th"
	elif api_type == "EVAS GL API" : api_suffix = "evgl_api_th"

	header_data = ""

	if 'func_ptr' in api_def_dic:
		func_prefix = api_def_dic['func_ptr']
		if api_type == "EVAS" or api_type == "EVAS GL" :
			header_data += _NEWLINE
			header_data += _NEWLINE
			header_data += "EAPI void %s_%s_set(void *func);" %(api_name, func_prefix)
			header_data += _NEWLINE
			header_data += "EAPI void *%s_%s_get(void);" %(api_name, func_prefix)
		elif api_type == "EVAS GL API" :
			header_data += _NEWLINE
			header_data += _NEWLINE
			header_data += "extern %s (*%s_%s)(%s);" %(return_type, func_prefix,
													api_name, para_data)

	header_data += _NEWLINE
	header_data += "EAPI %s evas_%s_%s(%s);" %(return_type, api_name, api_suffix, para_data)
	if 'gl_type' in api_def_dic and api_def_dic['gl_type'] == "GL" : header_data += _NEWLINE

	return header_data

#--------------------------------------------------------------
def gen_init_func_data(func_type) :

	init_func_data = "\n\nvoid\n_gl_thread_link_%s_generated_init()\n{\n" %(func_type)
	init_func_data += \
"""#define LINK2GENERIC(sym) \\
	sym = dlsym(RTLD_DEFAULT, #sym); \\
	if (!sym) ERR("Could not find function '%s'", #sym);
"""
	return init_func_data

def gen_symload_set_get(api_dic) :

	fp_name = "%s_%s" %(api_dic['api_name'], api_dic['func_ptr'])
	symload_data = "LINK2GENERIC(%s_set);" %(fp_name)
	symload_data += _NEWLINE
	symload_data += "LINK2GENERIC(%s_get);" %(fp_name)
	symload_data += _NEWLINE

	return symload_data

#--------------------------------------------------------------

# for debug
def api_define_print(api) :

	return """
/*
* %s
* %s(%s);
*/
""" %(api['return_type'], api['api_name'], list_to_str(api['para_list']))


###############################################################################
def generate_files(out_fname) :

	gen_files = {} # {file name : file pointer}
	if g_evas_total > 0 :
		gl_src = out_fname + "_gl_generated.c"
		gl_hdr = out_fname + "_gl_generated.h"

		gen_files[gl_src] = open(gl_src, 'w')
		gen_files[gl_hdr] = open(gl_hdr, 'w')

		gl_link_src = out_fname + "_gl_link_generated.c"
		gl_link_hdr = out_fname + "_gl_link_generated.h"

		gen_files[gl_link_src] = open(gl_link_src, 'w')
		gen_files[gl_link_hdr] = open(gl_link_hdr, 'w')

	if g_evgl_total > 0 :
		evgl_src = out_fname + "_evgl_generated.c"
		evgl_hdr = out_fname + "_evgl_generated.h"

		gen_files[evgl_src] = open(evgl_src, 'w')
		gen_files[evgl_hdr] = open(evgl_hdr, 'w')

		evgl_link_src = out_fname + "_evgl_link_generated.c"
		evgl_link_hdr = out_fname + "_evgl_link_generated.h"

		gen_files[evgl_link_src] = open(evgl_link_src, 'w')
		gen_files[evgl_link_hdr] = open(evgl_link_hdr, 'w')

	if g_evgl_api_total > 0 :
		evgl_api_src = out_fname + "_evgl_api_generated.c"
		evgl_api_hdr = out_fname + "_evgl_api_generated.h"

		gen_files[evgl_api_src] = open(evgl_api_src, 'w')
		gen_files[evgl_api_hdr] = open(evgl_api_hdr, 'w')

	return gen_files
#--------------------------------------------------------------
def generate_code(out_files) :

	auto_gen_msg = """/*
 * This is an automatically generated file using a python script.
 * ($EFL_HOME/src/utils/evas/generate_gl_thread_api.py)
 * Recommend that you modify data files ($EFL_HOME/src/utils/evas/gl_api_def.txt)
 * and make use of scripts if you need to fix them.
 */
"""
	evgl_header_file_head = \
"""
#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */
"""

	files = out_files.keys()

	# get file pointer, init. files...
	for f in files :
		out_files[f].truncate()
		out_files[f].write(auto_gen_msg)

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

		# EVAS GL
		if f.find("_evgl_generated.c") > 0 :
			evgl_source_file = out_files[f]
			if g_debug_lvl is 2 : evgl_source_file.write("#include <sys/time.h>\n")
		if f.find("_evgl_generated.h") > 0 :
			evgl_header_file = out_files[f]
			evgl_header_file.write(evgl_header_file_head)
		if f.find("_evgl_link_generated.c") > 0 :
			evgl_link_source_file = out_files[f]
		if f.find("_evgl_link_generated.h") > 0 :
			evgl_link_header_file = out_files[f]

		# EVAS GL API
		if f.find("_evgl_api_generated.c") > 0 :
			evgl_api_source_file = out_files[f]
			if g_debug_lvl is 2 : evgl_api_source_file.write("#include <sys/time.h>\n")
		if f.find("_evgl_api_generated.h") > 0 :
			evgl_api_header_file = out_files[f]
			evgl_api_header_file.write(evgl_header_file_head)

	#print ">>> code generate START >>>"
	#if debug mode, define counter variable
	if g_debug_lvl is 1 :
		if g_evas_total > 0 : gl_source_file.write("\n\n/* DEBUG */\n\n")
		if g_evgl_total > 0 : evgl_source_file.write("\n\n/* DEBUG */\n\n")
		if g_evgl_api_total > 0 : evgl_api_source_file.write("\n\n/* DEBUG */\n\n")

		for api_dic in api_dic_list :
			if api_dic['type'] == "EVAS" :
				api_prefix = "_gl_thread"
				api_suffix = "th"
				fp = gl_source_file
			elif api_dic['type'] == "EVAS GL" :
				api_prefix = "_evgl_thread"
				api_suffix = "evgl_th"
				fp = evgl_source_file
			elif api_dic['type'] == "EVAS GL API" :
				api_prefix = "_evgl_api_thread"
				api_suffix = "evgl_api_th"
				fp = evgl_api_source_file

			# check gl type,..
			ifdef_stmt = ""
			endif_stmt = ""
			if 'gl_type' in api_dic and api_dic['gl_type'] == "GL" :
				ifdef_stmt = "#ifndef GL_GLES\n"
				endif_stmt = "\n#endif\n"

			counter = ifdef_stmt
			counter += "int %s_%s_counter = 1;\n" %(api_prefix, api_dic['api_name'])
			counter += "int evas_%s_%s_counter = 1;\n" %(api_dic['api_name'], api_suffix)
			counter += endif_stmt

			LOG(counter)
			FWRITE(fp, counter)

	# generate wrapper API
	for api_dic in api_dic_list :

		api_name = api_dic['api_name']
		return_type = api_dic['return_type']
		para_data = list_to_str(api_dic['para_list'])
		api_type = api_dic['type']

		if api_type == "EVAS" :
			fp_src = gl_source_file
			fp_hdr = gl_header_file
			fp_link_src = gl_link_source_file
			fp_link_hdr = gl_link_header_file
			api_suffix = "th"
		elif api_type == "EVAS GL" :
			fp_src = evgl_source_file
			fp_hdr = evgl_header_file
			fp_link_src = evgl_link_source_file
			fp_link_hdr = evgl_link_header_file
			api_suffix = "evgl_th"
		elif api_type == "EVAS GL API" :
			fp_src = evgl_api_source_file
			fp_hdr = evgl_api_header_file
			api_suffix = "evgl_api_th"

		# check gl type,..
		ifdef_stmt = ""
		endif_stmt = ""
		if 'gl_type' in api_dic and api_dic['gl_type'] == "GL" :
			ifdef_stmt = "\n#ifndef GL_GLES\n"
			endif_stmt = "#endif\n"

		### ifndef,...
		LOG(ifdef_stmt)
		FWRITE(fp_src, ifdef_stmt)

		### API prototype (for debug)
		debug_msg = api_define_print(api_dic)
		LOG(debug_msg)
		FWRITE(fp_src, debug_msg)

		### data structure
		struct_data = gen_structure(api_dic)
		LOG(struct_data)
		if len(struct_data) > 0 :
			FWRITE(fp_src, struct_data)

		# define function pointer
		if 'func_ptr' in api_dic :
			funcPtr_data = gen_function_pointer(api_dic)
			LOG(funcPtr_data)
			FWRITE(fp_src, funcPtr_data)

		# generate call-back function
		api_data = gen_gl_thread_api(api_dic)
		LOG(api_data)
		FWRITE(fp_src, api_data)

		# generate wrapper function
		api_data = gen_thread_cmd_api(api_dic)
		LOG(api_data)
		FWRITE(fp_src, api_data)

		###  endif
		LOG(endif_stmt)
		FWRITE(fp_src, endif_stmt)

		### header
		header_data = ifdef_stmt
		header_data += gen_wrapper_api_header(api_dic)
		header_data += endif_stmt

		LOG(header_data)
		FWRITE(fp_hdr, header_data)

		### _link file
		if api_type == "EVAS" or api_type == "EVAS GL" :

			LOG(ifdef_stmt)
			FWRITE(fp_link_src, ifdef_stmt)

			if 'func_ptr' in api_dic :
				fp_suffix = api_dic['func_ptr']
				set_func = "void (*%s_%s_set)(void *func) = NULL;\n" %(api_name, fp_suffix)
				get_func = "void *(*%s_%s_get)(void) = NULL;\n" %(api_name, fp_suffix)
				LOG(set_func + get_func)
				FWRITE(fp_link_src, set_func + get_func)

				LOG(ifdef_stmt)
				FWRITE(fp_link_hdr, ifdef_stmt)

				set_func2 = "extern void (*%s_%s_set)(void *func);\n" %(api_name, fp_suffix)
				get_func2 = "extern void *(*%s_%s_get)(void);\n" %(api_name, fp_suffix)
				LOG(set_func2 + get_func2)
				FWRITE(fp_link_hdr, set_func2 + get_func2)

			func_ptr_data = "%s (*evas_%s_%s)(%s) = NULL;\n" %(return_type, api_name,
														  api_suffix, para_data)
			LOG(func_ptr_data)
			FWRITE(fp_link_src, func_ptr_data)

			LOG(endif_stmt)
			FWRITE(fp_link_src, endif_stmt)

			# check gl types,...
			if not('func_ptr' in api_dic) and 'gl_type' in api_dic :
				LOG(ifdef_stmt)
				FWRITE(fp_link_hdr, ifdef_stmt)

			extern_func_ptr_data = "extern %s (*evas_%s_%s)(%s);\n" %(return_type, api_name,
																 api_suffix, para_data)
			LOG(extern_func_ptr_data)
			FWRITE(fp_link_hdr, extern_func_ptr_data)

			LOG(endif_stmt)
			FWRITE(fp_link_hdr, endif_stmt)

	#print "END OF LOOP2"


	### init_symbol func. @_link file
	if  g_evas_total > 0 :
		init_func_data = gen_init_func_data("gl")
		gl_link_source_file.write(init_func_data)

		#print "LOOP3-1 START>>>"
		symload_data = ""
		for api_dic in api_dic_list :
			# check gl type,..
			ifdef_stmt = ""
			endif_stmt = ""
			if 'gl_type' in api_dic and api_dic['gl_type'] == "GL" :
				ifdef_stmt = "\n#ifndef GL_GLES\n"
				endif_stmt = "\n#endif\n"

			if api_dic['type'] == "EVAS" :
				symload_data += _NEWLINE
				symload_data += ifdef_stmt
				if 'func_ptr' in api_dic :
					symload_data += gen_symload_set_get(api_dic)

				symload_data += "LINK2GENERIC(evas_%s_th);" %(api_dic['api_name'])
				symload_data += endif_stmt
		LOG(symload_data)
		FWRITE(gl_link_source_file, symload_data)

		#print "END OF LOOP3"
		FWRITE(gl_link_source_file, "\n}")
		FWRITE(gl_link_header_file, "\n\nextern void _gl_thread_link_gl_generated_init();\n")

	if  g_evgl_total > 0 :
		init_func_data = gen_init_func_data("evgl")
		evgl_link_source_file.write(init_func_data)

		#print "LOOP3-2 START>>>"
		symload_data = ""
		ifdef_stmt = ""
		endif_stmt = ""
		for api_dic in api_dic_list :
			# check gl type,..
			ifdef_stmt = ""
			endif_stmt = ""
			if 'gl_type' in api_dic and api_dic['gl_type'] == "GL" :
				ifdef_stmt = "\n#ifndef GL_GLES\n"
				endif_stmt = "\n#endif\n"

			if api_dic['type'] == "EVAS GL" :
				symload_data += ifdef_stmt
				symload_data += _NEWLINE
				if 'func_ptr' in api_dic :
					symload_data += gen_symload_set_get(api_dic)

				symload_data += "LINK2GENERIC(evas_%s_evgl_th);" %(api_dic['api_name'])
				symload_data += endif_stmt
		LOG(symload_data)
		FWRITE(evgl_link_source_file, symload_data)

		#print "END OF LOOP3"
		FWRITE(evgl_link_source_file, "\n}")
		FWRITE(evgl_link_header_file, "\n\nextern void _gl_thread_link_evgl_generated_init();\n")

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
						version='%(prog)s 1.98')

	args = parser.parse_args()
	g_verbose = args.verbose
	g_debug_lvl = args.debug_level
	g_print_err = args.print_err
	g_evas_total = 0
	g_evgl_total = 0
	g_evgl_api_total = 0

	# parsing data file
	result = get_api_def(args.input_data)

	res_evas = result[0]
	res_evgl = result[1]
	res_evgl_api = result[2]

	g_evas_total = res_evas['success'] + res_evas['fail']
	g_evgl_total = res_evgl['success'] + res_evgl['fail']
	g_evgl_api_total = res_evgl_api['success'] + res_evgl_api['fail']

	
	print "[EVAS:%d]" %(g_evas_total)
	if g_evas_total > 0 : print_result(res_evas)

	print "[EVAS GL:%d]" %(g_evgl_total)
	if g_evgl_total > 0 : print_result(res_evgl)

	print "[EVAS GL API:%d]" %(g_evgl_api_total)
	if g_evgl_api_total > 0 : print_result(res_evgl_api)

	print "* Total APIs: %d" %(g_evas_total + g_evgl_total + g_evgl_api_total)

	# generate files
	gen_files = generate_files(args.out_file_name)

	# generate codes
	generate_code(gen_files)

	print "* generated files>>"
	for f in gen_files :
		print f


	#eflindent
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

