/* an implementation of fork for Windows */

#include <windows.h>
#include <setjmp.h>

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PVOID ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef enum _MEMORY_INFORMATION_ {
    MemoryBasicInformation,
    MemoryWorkingSetList,
    MemorySectionName,
    MemoryBasicVlmInformation
} MEMORY_INFORMATION_CLASS;

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _USER_STACK {
    PVOID FixedStackBase;
    PVOID FixedStackLimit;
    PVOID ExpandableStackBase;
    PVOID ExpandableStackLimit;
    PVOID ExpandableStackBottom;
} USER_STACK, *PUSER_STACK;

typedef LONG KPRIORITY;
typedef ULONG_PTR KAFFINITY;

typedef struct _THREAD_BASIC_INFORMATION {
    LONG                ExitStatus;
    PVOID                   TebBaseAddress;
    CLIENT_ID               ClientId;
    KAFFINITY               AffinityMask;
    KPRIORITY               Priority;
    KPRIORITY               BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemHandleInformation = 0x10
} SYSTEM_INFORMATION_CLASS;

typedef LONG (NTAPI *ZwWriteVirtualMemory_t)(IN HANDLE,
    IN PVOID, IN PVOID, IN ULONG, OUT PULONG OPTIONAL);

typedef LONG (NTAPI *ZwCreateProcess_t)(OUT PHANDLE,
    IN ACCESS_MASK, IN POBJECT_ATTRIBUTES, IN HANDLE, IN BOOLEAN,
    IN HANDLE OPTIONAL, IN HANDLE OPTIONAL, IN HANDLE OPTIONAL);

typedef LONG (WINAPI *ZwQuerySystemInformation_t)(SYSTEM_INFORMATION_CLASS,
    PVOID, ULONG, PULONG);

typedef LONG (NTAPI *ZwQueryVirtualMemory_t)(IN HANDLE,
    IN PVOID, IN MEMORY_INFORMATION_CLASS, OUT PVOID, IN ULONG,
    OUT PULONG OPTIONAL);

typedef LONG (NTAPI *ZwGetContextThread_t)(IN HANDLE,
    OUT PCONTEXT);

typedef LONG (NTAPI *ZwCreateThread_t)(OUT PHANDLE,
    IN ACCESS_MASK, IN POBJECT_ATTRIBUTES, IN HANDLE, OUT PCLIENT_ID,
    IN PCONTEXT, IN PUSER_STACK, IN BOOLEAN);

typedef LONG (NTAPI *ZwResumeThread_t)(IN HANDLE, OUT PULONG OPTIONAL);
typedef LONG (NTAPI *ZwClose_t)(IN HANDLE);
typedef LONG (NTAPI *ZwQueryInformationThread_t)(IN HANDLE,
    IN THREAD_INFORMATION_CLASS, OUT PVOID, IN ULONG, OUT PULONG OPTIONAL);

static ZwCreateProcess_t          ZwCreateProcess;
static ZwQuerySystemInformation_t ZwQuerySystemInformation;
static ZwQueryVirtualMemory_t     ZwQueryVirtualMemory;
static ZwCreateThread_t           ZwCreateThread;
static ZwGetContextThread_t       ZwGetContextThread;
static ZwResumeThread_t           ZwResumeThread;
static ZwClose_t                  ZwClose;
static ZwQueryInformationThread_t ZwQueryInformationThread;
static ZwWriteVirtualMemory_t     ZwWriteVirtualMemory;

static jmp_buf jenv;
static int child_entry(void) {
    longjmp(jenv, 1);
    return 0;
}

static int init_ntdll(void) {
    HMODULE ntdll = GetModuleHandle("ntdll");
    if (!ntdll) return 0;

#define GETPROC(name) name = (name##_t)GetProcAddress(ntdll, #name)

    GETPROC(ZwCreateProcess);
    GETPROC(ZwQuerySystemInformation);
    GETPROC(ZwQueryVirtualMemory);
    GETPROC(ZwCreateThread);
    GETPROC(ZwGetContextThread);
    GETPROC(ZwResumeThread);
    GETPROC(ZwQueryInformationThread);
    GETPROC(ZwWriteVirtualMemory);
    GETPROC(ZwClose);

    return !!ZwCreateProcess;
}

int fork_win(void) {
    if (setjmp(jenv)) return 0;
    if (!ZwCreateProcess && !init_ntdll()) return -1;

    HANDLE hproc = 0, hthread = 0;
    OBJECT_ATTRIBUTES oa = { sizeof(oa) };
    ZwCreateProcess(&hproc, PROCESS_ALL_ACCESS, &oa, (HANDLE)-1, TRUE, 0, 0, 0);

    CONTEXT context = { CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS
        | CONTEXT_FLOATING_POINT };

    ZwGetContextThread((HANDLE)-2, &context);

    MEMORY_BASIC_INFORMATION mbi;

#if _WIN64
    context.Rip = (ULONG)child_entry;
    ZwQueryVirtualMemory((HANDLE)-1, (PVOID)context.Rsp, MemoryBasicInformation,
        &mbi, sizeof(mbi), 0);
#else
    context.Eip = (ULONG)child_entry;
    ZwQueryVirtualMemory((HANDLE)-1, (PVOID)context.Esp, MemoryBasicInformation,
        &mbi, sizeof(mbi), 0);
#endif

    USER_STACK stack = { 0, 0, (PCHAR)mbi.BaseAddress + mbi.RegionSize,
        mbi.BaseAddress, mbi.AllocationBase };
    CLIENT_ID cid;

    ZwCreateThread(&hthread, THREAD_ALL_ACCESS, &oa, hproc, &cid, &context,
        &stack, TRUE);

    THREAD_BASIC_INFORMATION tbi;
    ZwQueryInformationThread((HANDLE)-2, ThreadMemoryPriority, &tbi,
        sizeof(tbi), 0);
    PNT_TIB tib = (PNT_TIB)tbi.TebBaseAddress;
    ZwQueryInformationThread(hthread, ThreadMemoryPriority, &tbi,
        sizeof(tbi), 0);
    ZwWriteVirtualMemory(hproc, tbi.TebBaseAddress, &tib->ExceptionList,
        sizeof(tib->ExceptionList), 0);

    ZwResumeThread(hthread, 0);

    ZwClose(hthread);
    ZwClose(hproc);

    return (int)cid.UniqueProcess;
}