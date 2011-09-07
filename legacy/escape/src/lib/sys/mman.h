#ifndef __ESCAPE_SYS_MMAN_H__
#define __ESCAPE_SYS_MMAN_H__

#include <Escape.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @def PROT_NONE
 * Data can not be accessed.
 */

/**
 * @def PROT_READ
 * Data can be read.
 */

/**
 * @def PROT_WRITE
 * Data can be written.
 */

/**
 * @def PROT_EXEC
 * Data can be executed.
 */

#define PROT_NONE  0x00
#define PROT_READ  0x01
#define PROT_WRITE 0x02
#define PROT_EXEC  0x04

/**
 * @def MAP_SHARED
 * Changes are shared.
 */

/**
 * @def MAP_PRIVATE
 * Changes are private.
 */

/**
 * @def MAP_FIXED
 * Interpret the address (addr) exactly.
 */

/**
 * @def MAP_FAILED
 * Error return from mmap().
 */

#define MAP_SHARED  0x0001
#define MAP_PRIVATE 0x0002
#define MAP_FIXED   0x0010

#define MAP_FAILED	((void *)-1)


/**
 * @file mman.h
 * @brief The file that provides the memory map functions
 * @defgroup Mman Functions that manage memory mappping.
 *
 * This header provides the meomry map functions mmap and munmap.
 *
 */

/**
 * Creates or opens a named or unnamed file mapping object for a
 * specified file and maps a view of a file mapping into the
 * address space of a calling process.
 *
 * @param addr Unused
 * @param len Number of bytes to be mapped.
 * @param prot Protections.
 * @param flags Type of the mapped object.
 * @param fd File descriptor that describes the object to map.
 * @param offset Number of bytes from which to start the mapping.
 * @return The starting address of the mapped view on success, -1 otherwise.
 *
 * Create or open an unnamed file mapping object for a specified
 * file described by the file descriptor @p fd. The number of
 * bytes that are mapped is given by @p len and start after
 * @p offset bytes. The parameter @p addr is unused.
 *
 * The only type of the mapped object that is supported is
 * @c MAP_SHARED. If another value if given, -1 is returned.
 *
 * @p prot specifies the protection of the mapped region. If
 * PROT_EXEC is used, it set the execute access. If PROT_READ
 * is used, it sets the read access. If PROT_WRITE is used, it
 * sets the write access.
 *
 * If the map view of file can not be created, -1 is returned.
 * If the mappping can not be done, -1 is returned.
 *
 * If no error occured, the starting address of the mapped view
 * is returned.
 *
 * Conformity: None.
 *
 * @ingroup Mman
 */
EAPI void *mmap(void   *addr,
                size_t len,
                int    prot,
                int    flags,
                int    fd,
                off_t  offset);

/**
 * Unmaps a mapped view of a file from the calling process's
 * address space.
 *
 * @param addr Pointer to the base address.
 * @param len  Unused.
 * @return 0 on success, -1 otherwise.
 *
 * Unmaps a mapped view of a file from the calling process's
 * address space. @p addr is the pointer to the base address.
 * This value must be identical to the value returned by a
 * previous call to mmap(). The parameter @p len is unsed.
 *
 * Conformity: None.
 *
 * @ingroup Mman
 */
EAPI int   munmap(void  *addr,
                  size_t len);


# define MADV_NORMAL      0     /* No further special treatment.  */
# define MADV_RANDOM      1     /* Expect random page references.  */
# define MADV_SEQUENTIAL  2     /* Expect sequential page references.  */
# define MADV_WILLNEED    3     /* Will need these pages.  */
# define MADV_DONTNEED    4     /* Don't need these pages.  */
# define MADV_REMOVE      9     /* Remove these pages and resources.  */
# define MADV_DONTFORK    10    /* Do not inherit across fork.  */
# define MADV_DOFORK      11    /* Do inherit across fork.  */
# define MADV_MERGEABLE   12    /* KSM may merge identical pages.  */
# define MADV_UNMERGEABLE 13    /* KSM may not merge identical pages.  */
# define MADV_HUGEPAGE    14    /* Worth backing with hugepages.  */
# define MADV_NOHUGEPAGE  15    /* Not worth backing with hugepages.  */
# define MADV_HWPOISON    100   /* Poison a page for testing.  */

/**
 * give advice about use of memory
 *
 * @param addr Unused.
 * @param length  Unused.
 * @param advice Unused.
 * @return 0 on success, -1 otherwise.
 *
 * The  madvise()  system  call  advises  the kernel about how to handle
 * paging input/output in the address range beginning at address addr and
 * with size length bytes.  It allows an application to tell the kernel
 * how it expects to use some mapped  or shared  memory  areas,  so  that
 * the  kernel can choose appropriate read-ahead and caching techniques.
 * This call does not influence the semantics of the application (except
 * in the case of MADV_DONTNEED), but may influence its  performance.
 * The kernel is free to ignore the advice.

 * Conformity: None.
 *
 * @ingroup Mman
 */
EAPI int madvise(void *addr,
    size_t length,
    int advice);


#ifdef __cplusplus
}
#endif


#endif /* __ESCAPE_SYS_MMAN_H__ */

