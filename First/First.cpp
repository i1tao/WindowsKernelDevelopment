#include <ntifs.h>

extern "C"
{
    VOID DriverUnload(PDRIVER_OBJECT DriverObject);
    NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
    if (DriverObject != nullptr)
    {
        DbgPrint("[%ws]Driver Upload,Driver Object Address:%p", __FUNCTIONW__, DriverObject);
    }
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    DbgPrint("[%ws]Hello Kernel World\n", __FUNCTIONW__);
    if (RegistryPath != NULL)
    {
        DbgPrint("[%ws]Driver RegistryPath:%wZ\n", __FUNCTIONW__, RegistryPath);
    }
    if (DriverObject != NULL)
    {
        DbgPrint("[%ws]Driver Object Address:%p\n", __FUNCTIONW__, DriverObject);
        DriverObject->DriverUnload = DriverUnload;
    }
    return STATUS_SUCCESS;
}

