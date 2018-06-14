/* A stub file that includes the right assembly according to the target
 * architecture and operating system, for gas only.
 *
 * This file is a part of libostd, provided to you under the NCSA license.
 * See the COPYING.md file in the main distribution for further information.
 */

#if defined(__i386__)
#  if defined(__APPLE__)
#    include "i386/ontop_i386_sysv_macho_gas.S"
#  elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#    include "i386/ontop_i386_ms_pe_gas.asm"
#  else
#    include "i386/ontop_i386_sysv_elf_gas.S"
#  endif
#elif defined(__x86_64__)
#  if defined(__APPLE__)
#    include "x86_64/ontop_x86_64_sysv_macho_gas.S"
#  elif defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#    include "x86_64/ontop_x86_64_ms_pe_gas.asm"
#  else
#    include "x86_64/ontop_x86_64_sysv_elf_gas.S"
#  endif
#elif defined(__ppc__)
#  if defined(__APPLE__)
#    include "ppc32/ontop_ppc32_sysv_macho_gas.S"
#  elif defined(_AIX)
#    include "ppc32/ontop_ppc32_sysv_xcoff_gas.S"
#  else
#    include "ppc32/ontop_ppc32_sysv_elf_gas.S"
#  endif
#elif defined(__ppc64__)
#  if defined(__APPLE__)
#    include "ppc64/ontop_ppc64_sysv_macho_gas.S"
#  elif defined(_AIX)
#    include "ppc64/ontop_ppc64_sysv_xcoff_gas.S"
#  else
#    include "ppc64/ontop_ppc64_sysv_elf_gas.S"
#  endif
#elif defined(__aarch64__)
#  if defined(__APPLE__)
#    include "arm64/ontop_arm64_aapcs_macho_gas.S"
#  else
#    include "arm64/ontop_arm64_aapcs_elf_gas.S"
#  endif
#elif defined(__arm__)
#  if defined(__APPLE__)
#    include "arm/ontop_arm_aapcs_macho_gas.S"
#  else
#    include "arm/ontop_arm_aapcs_elf_gas.S"
#  endif
#elif defined(__mips__)
#  include "mips32/ontop_mips32_o32_elf_gas.S"
#else
    #error "Unsupported architecture"
#endif
