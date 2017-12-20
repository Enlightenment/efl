header_checks = [
  'alloca.h',
  'asm/hwcap.h',
  'bsd/string.h',
  'dirent.h',
  'execinfo.h',
  'mcheck.h',
  'netinet/in.h',
  'stdlib.h',
  'sys/auxv.h',
  'sys/inotify.h',
  'sys/ioctl.h',
  'sys/mman.h',
  'sys/types.h',
  'sys/socket.h',
  'arpa/inet.h',
  'sys/epoll.h',
  'sys/un.h',
  'dirent.h']

function_checks = [
# function name | headers that are needed | libraries to include | Defines that are needed
  ['alloca', ['alloca.h']],
  ['backtrace', ['execinfo.h']],
  ['backtrace_symbols', ['execinfo.h']],
  ['clock_gettime', ['time.h']],
  ['dirfd', ['dirent.h sys/types.h']],
  ['fchmod', ['sys/stat.h']],
  ['fcntl', ['fcntl.h']],
  ['fork', ['unistd.h']],
  ['fpathconf', ['unistd.h']],
  ['geteuid', ['unistd.h']],
  ['getpagesize', ['unistd.h']],
  ['getpwent', ['sys/types.h', 'pwd.h']],
  ['getuid', ['unistd.h']],
  ['getxattr', ['sys/types.h', 'sys/xattr.h']],
  ['iconv_t', ['iconv.h']],
  ['listxattr', ['sys/types.h', 'sys/xattr.h']],
  ['mallinfo', ['malloc.h']],
  ['malloc_info', ['malloc.h']],
  ['malloc_usable_size', ['malloc.h']],
  ['mkdirat', ['sys/stat.h']],
  ['mmap', ['sys/mman.h']],
  ['mtrace', ['mcheck.h']],
  ['prctl', ['sys/prctl.h']],
  ['realpath', ['stdlib.h']],
  ['setxattr', ['sys/types.h', 'sys/xattr.h']],
  ['siglongjmp', ['setjmp.h']],
  ['strerror_r', ['string.h']],
#FIXME strlcpy is detected by meson but drops at compilation time
#  ['strlcpy', ['string.h']],
  ['siginfo_t', ['signal.h']],
  ['strerror_r', ['string.h']],
#from here on we specify the dependencies
  ['dlopen', ['dlfcn.h'],                               ['dl']],
  ['dlsym', ['dlfcn.h'],                                ['dl']],
  ['lround', ['math.h'],                                ['m']],
  ['shm_open', ['sys/mman.h', 'sys/stat.h', 'fcntl.h'], ['rt']],
#from here on we specify arguments
  ['splice', ['fcntl.h'],                               [],      '-D_GNU_SOURCE=1'],
  ['sched_getcpu', ['sched.h'],                         [],      '-D_GNU_SOURCE=1'],
  ['dladdr', ['dlfcn.h'],                               ['dl'],  '-D_GNU_SOURCE=1']
]

strerror_r_char_p = cc.compiles('''#define _GNU_SOURCE
                  #include <string.h>
                  int func (void)
                    {
                       char error_string[256];
                       char *ptr = strerror_r (-2, error_string, 256);
                       char c = *strerror_r (-2, error_string, 256);
                       return c != 0 && ptr != (void*) 0L;
                    }
                 ''',
                 name : 'strerror_r() returns char *')

if strerror_r_char_p
  config_h.set('STRERROR_R_CHAR_P', '1')
endif

#for later use, a bunch of librarie findings
m = cc.find_library('m')
#just keep this here as required false, if it is not inplace the library rt will just be linked as NOP
dl = cc.find_library('dl', required: false)
rt = cc.find_library('rt', required: false)

thread_dep = dependency('threads')

#check for the headers
foreach header : header_checks
  if cc.has_header(header)
    config_h.set10('HAVE_'+header.underscorify().to_upper(), true)
  endif
endforeach

foreach function : function_checks
  function_name = function[0]
  headers_to_search = function[1]
  dependencies = []
  args = []

  # if there is a library, make sure they exist
  if function.length() > 2
    foreach library : function[2]
      lib = cc.find_library(library, required : false)
      if lib.found() == true
        dependencies += lib
      endif
    endforeach
  endif

  #check if there are args
  if function.length() > 3
    args = function[3]
  endif

  # Only check the header if the dependencies are ready
  foreach header : headers_to_search
    if cc.has_header_symbol(header, function_name,
        dependencies : dependencies,
        args : args)
      config_h.set10('HAVE_'+function_name.to_upper(), true)
    endif
  endforeach
endforeach

# The next checks are manually for now due to the fact that some names are not within the default pattern
if (cc.has_header_symbol('sys/stat.h', 'fstatat'))
  config_h.set10('HAVE_ATFILE_SOURCE', true)
endif

config_h.set('VMAJ', version_major)
config_h.set('VMIN', version_minor)
config_h.set('VMIC', version_micro)
config_h.set('VREV', '0')
config_h.set_quoted('SHARED_LIB_SUFFIX', '.so')
