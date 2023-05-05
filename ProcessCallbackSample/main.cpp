#include <ntifs.h>
#include <ntddk.h>
#ifdef __cplusplus
EXTERN_C_START
#endif

BOOLEAN g_bSuccessRegister = false;

void
DriverUnload(
    PDRIVER_OBJECT DriverObject
);
NTSTATUS
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
);

VOID
CreateProcessNotifyEx(
    _Inout_ PEPROCESS Process,
    _In_ HANDLE ProcessId,
    _Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
);
#ifdef __cplusplus
EXTERN_C_END
#endif

void
DriverUnload(
    PDRIVER_OBJECT DriverObject
)
{
    UNREFERENCED_PARAMETER(DriverObject);
    if (g_bSuccessRegister)
    {
        PsSetCreateProcessNotifyRoutineEx(CreateProcessNotifyEx, true);
        g_bSuccessRegister = false;
    }

    return;
}

NTSTATUS
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    DriverObject->DriverUnload = DriverUnload;

    ntStatus = PsSetCreateProcessNotifyRoutineEx(CreateProcessNotifyEx, FALSE);
    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }

    g_bSuccessRegister = true;

    return ntStatus;
}

VOID
CreateProcessNotifyEx(
    _Inout_ PEPROCESS Process,
    _In_ HANDLE ProcessId,
    _Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
    HANDLE hParentProcessID = nullptr;
    HANDLE hPareentThreadID = nullptr;
    HANDLE hCurrentThreadID = nullptr;

    hCurrentThreadID = PsGetCurrentThreadId();

    if (CreateInfo == NULL)
    {
        DbgPrint("CreateProcessNotifyEx [Destroy][CurrentThreadId: 0x%llx][ProcessId = 0x%llx]\n", hCurrentThreadID, ProcessId);
        return;
    }

    hParentProcessID = CreateInfo->CreatingThreadId.UniqueProcess;
    hPareentThreadID = CreateInfo->CreatingThreadId.UniqueThread;

    DbgPrint("CreateProcessNotifyEx [Create][CurrentThreadId: 0x%llx][ParentID 0x%llx:0x%llx][EProcess = 0x%llx,ProcessId = 0x%lxx,ProcessName=%wZ]\n",
        hCurrentThreadID, hParentProcessID, hPareentThreadID, Process, ProcessId, CreateInfo->ImageFileName);
    return;
}