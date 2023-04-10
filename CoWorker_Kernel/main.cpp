#include <ntifs.h>
#include <ntddk.h>

#define CDO_DEVICE_NAME L"\\Device\\cwk"
#define CDO_SYMBOL_NAME L"\\??\\cwksymbollic"

// 从应用层给驱动发送一个字符串。
#define  CWK_DVC_SEND_STR \
	(ULONG)CTL_CODE( \
	FILE_DEVICE_UNKNOWN, \
	0x911,METHOD_BUFFERED, \
	FILE_WRITE_DATA)

// 从驱动读取一个字符串
#define  CWK_DVC_RECV_STR \
	(ULONG)CTL_CODE( \
	FILE_DEVICE_UNKNOWN, \
	0x912,METHOD_BUFFERED, \
	FILE_READ_DATA)

extern "C"
{
    VOID cwkUnload(PDRIVER_OBJECT driver);
    NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING regpath);
}

PDEVICE_OBJECT g_cdo = NULL;

void cwkUnload(PDRIVER_OBJECT driver)
{
    UNREFERENCED_PARAMETER(driver);
    UNICODE_STRING ustrSymbolicName = RTL_CONSTANT_STRING(CDO_SYMBOL_NAME);

    ASSERT(g_cdo != NULL);
    IoDeleteSymbolicLink(&ustrSymbolicName);
    IoDeleteDevice(g_cdo);
}

NTSTATUS cwkDispatch(
    IN PDEVICE_OBJECT dev,
    IN PIRP irp)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(irp);
    ULONG ulRetLen = 0;

    while (dev == g_cdo)
    {
        // 如果这个请求不是发给g_cdo的，那就非常奇怪了。
        // 因为这个驱动只生成过这一个设备。所以可以直接
        // 返回失败。
        if (irpsp->MajorFunction == IRP_MJ_CREATE || irpsp->MajorFunction == IRP_MJ_CLOSE)
        {
            // 生成和关闭请求，这个一律简单地返回成功就可以
            // 了。就是无论何时打开和关闭都可以成功。
            break;
        }

        if (irpsp->MajorFunction == IRP_MJ_DEVICE_CONTROL)
        {
            // 处理DeviceIoControl。
            PVOID pBuffer = irp->AssociatedIrp.SystemBuffer;
            ULONG ulInLen = irpsp->Parameters.DeviceIoControl.InputBufferLength;
            ULONG ulOutLen = irpsp->Parameters.DeviceIoControl.OutputBufferLength;
            //ULONG len;
            switch (irpsp->Parameters.DeviceIoControl.IoControlCode)
            {
            case CWK_DVC_SEND_STR:
                ASSERT(pBuffer != NULL);
                ASSERT(ulInLen > 0);
                ASSERT(ulOutLen == 0);
                DbgPrint((char*)pBuffer);
                // 已经打印过了，那么现在就可以认为这个请求已经成功。
                break;
            case CWK_DVC_RECV_STR:
            default:
                // 到这里的请求都是不接受的请求。未知的请求一律返回非法参数错误。
                ntStatus = STATUS_INVALID_PARAMETER;
                break;
            }
        }
        break;
    }
    // 到这里的请求都是不接受的请求。未知的请求一律返回非法参数错误。
    irp->IoStatus.Information = ulRetLen;
    irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return ntStatus;
}


NTSTATUS
DriverEntry(
    PDRIVER_OBJECT driver,
    PUNICODE_STRING regpath
)
{
    UNREFERENCED_PARAMETER(regpath);

    NTSTATUS ntStatus;
    UNICODE_STRING ustrCdoName = RTL_CONSTANT_STRING(CDO_DEVICE_NAME);
    UNICODE_STRING ustrCdoSym = RTL_CONSTANT_STRING(CDO_SYMBOL_NAME);

    //生成一个设备对象
    ntStatus = IoCreateDevice
    (
        driver,
        0,
        &ustrCdoName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &g_cdo
    );

    //生成符号链接
    IoDeleteSymbolicLink(&ustrCdoSym);
    ntStatus = IoCreateSymbolicLink(&ustrCdoSym, &ustrCdoName);
    if (!NT_SUCCESS(ntStatus))
    {
        IoDeleteDevice(g_cdo);
    }

    //设置派遣函数
    for (size_t i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        driver->MajorFunction[i] = cwkDispatch;
    }

    driver->DriverUnload = cwkUnload;

    // 清除控制设备的初始化标记
    g_cdo->Flags &= ~DO_DEVICE_INITIALIZING;
    return STATUS_SUCCESS;
}