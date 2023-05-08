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
LoadImageNotifyRoutine(
    _In_opt_ PUNICODE_STRING FullImageName,
    _In_ HANDLE ProcessId,                // pid into which image is being mapped
    _In_ PIMAGE_INFO ImageInfo
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
        PsRemoveLoadImageNotifyRoutine(LoadImageNotifyRoutine);
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

    ntStatus = PsSetLoadImageNotifyRoutine(LoadImageNotifyRoutine);

    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }

    g_bSuccessRegister = true;
    return ntStatus;

}

VOID
LoadImageNotifyRoutine(
    _In_opt_ PUNICODE_STRING FullImageName,
    _In_ HANDLE ProcessId,                // pid into which image is being mapped
    _In_ PIMAGE_INFO ImageInfo
)
{
    PIMAGE_INFO_EX pInfo = NULL;

    if (!FullImageName || !ImageInfo)
    {
        return;
    }
    if (ImageInfo->ExtendedInfoPresent)
    {
        pInfo = CONTAINING_RECORD(ImageInfo, IMAGE_INFO_EX, ImageInfo);
        DbgPrint("ModLoad Name = %wZ,ProcessID = 0x%x,FileObj = 0x%x,ImageBase = 0x%x,Size = 0x%x\n",
            FullImageName, ProcessId, pInfo->FileObject, pInfo->ImageInfo.ImageBase, pInfo->ImageInfo.ImageSize);
    }
    return;
}