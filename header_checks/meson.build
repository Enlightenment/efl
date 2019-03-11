if get_option('native-arch-optimization')
  if host_machine.cpu_family() == 'x86' or host_machine.cpu_family() == 'x86_64'
    native_header = 'immintrin.h'
  elif host_machine.cpu_family() == 'arm'
    native_header = 'arm_neon.h'
  elif host_machine.cpu_family() == 'aarch64'
    native_header = 'arm_neon.h'
  elif host_machine.cpu_family() == 'ppc' or host_machine.cpu_family() == 'ppc64'
    native_header = 'altivec.h'
  endif

  if cc.has_header(native_header) == false
    error('Error, header '+native_header+' is required')
  endif

  config_h.set10('HAVE_'+native_header.underscorify().to_upper(), true)
endif

header_checks = [
  'alloca.h',
  'asm/hwcap.h',
  'bsd/string.h',
  'dirent.h',
  'execinfo.h',
  'mcheck.h',
  'netinet/in.h',
  'netinet/ssl.h',
  'netinet/tcp.h',
  'netinet/udp.h',
  'net/if.h',
  'stdlib.h',
  'mman.h',
  'sys/auxv.h',
  'sys/inotify.h',
  'sys/ioctl.h',
  'sys/mman.h',
  'sys/types.h',
  'sys/socket.h',
  'sys/filio.h',
  'arpa/inet.h',
  'sys/epoll.h',
  'sys/un.h',
  'sys/wait.h',
  'sys/resource.h',
  'sys/times.h',
  'dirent.h',
  'longinfo.h',
  'exotic.h',
  'ieeefp.h',
  'node/uv.h',
  'sys/timerfd.h',
  'features.h',
  'langinfo.h',
  'locale.h',
  'uv.h',
  'ws2tcpip.h',
  'crt_externs.h'
]

#### The below is logically broken
#### the declaration of symbol + headers when you look the symbols up
#### in man pages you'll find that, for example, kevent needs you to
#### include ALL of the headers listed below. same for setxattr,
#### listxattr, pthread_getcpuclockid ... i stopped looking at this
#### point because it seems this is the pattern, but the foreach below
#### does not do this. it includes one header at a time from the list
#### then checks to see if the symbol exists. this leads to failures
#### in the checks (specifically i noticed kevent on bsd). so the whole
#### construct for this is wrong. it needs a rethink. i'm putting this
#### comment here as a note that this is the case for now as i'm just
#### trying to fix the meson build on freebsd for now

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
  ['iconv', ['iconv.h']],
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
  ['gettimeofday', ['sys/time.h']],
  ['execvp', ['unistd.h']],
  ['pause', ['unistd.h']],
  ['isfinite', ['math.h']],
#FIXME strlcpy is detected by meson but drops at compilation time
#  ['strlcpy', ['string.h']],
  ['siginfo_t', ['signal.h']],
  ['strerror_r', ['string.h']],
  ['pthread_getcpuclockid', ['pthread.h', 'time.h']],
  ['timerfd_create', ['sys/timerfd.h']],
  ['kevent', ['sys/types.h', 'sys/event.h', 'sys/time.h']],
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

if (cc.has_header('sys/mman.h'))
  config_h.set10('HAVE_MMAN_H', true)
endif

config_h.set('VMAJ', version_major)
config_h.set('VMIN', version_minor)
config_h.set('VMIC', version_micro)
config_h.set('VREV', '0')

#jpeg detection ... life is a bit more complex there 

jpeg = dependency('libjpeg', required: false)
if jpeg.found() == false
  jpeg = cc.find_library('jpeg')
endif

if sys_bsd == true
  config_h.set('HAVE_NOTIFY_KEVENT', '1')
endif

if sys_osx == true
  config_h.set('HAVE_NOTIFY_COCOA', '1')
endif
config_h.set('SIZEOF_INT', cc.sizeof('int'))
config_h.set('SIZEOF_LONG', cc.sizeof('long'))

if sys_windows == true
  config_h.set('HAVE_DLSYM', '1')
  config_h.set('HAVE_NOTIFY_WIN32', '1')
endif
