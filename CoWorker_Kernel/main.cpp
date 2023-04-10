#include <ntifs.h>
#include <ntddk.h>

#define CDO_DEVICE_NAME L"\\Device\\cwk"
#define CDO_SYMBOL_NAME L"\\??\\cwksymbollic"

// ��Ӧ�ò����������һ���ַ�����
#define  CWK_DVC_SEND_STR \
	(ULONG)CTL_CODE( \
	FILE_DEVICE_UNKNOWN, \
	0x911,METHOD_BUFFERED, \
	FILE_WRITE_DATA)

// ��������ȡһ���ַ���
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
        // �����������Ƿ���g_cdo�ģ��Ǿͷǳ�����ˡ�
        // ��Ϊ�������ֻ���ɹ���һ���豸�����Կ���ֱ��
        // ����ʧ�ܡ�
        if (irpsp->MajorFunction == IRP_MJ_CREATE || irpsp->MajorFunction == IRP_MJ_CLOSE)
        {
            // ���ɺ͹ر��������һ�ɼ򵥵ط��سɹ��Ϳ���
            // �ˡ��������ۺ�ʱ�򿪺͹رն����Գɹ���
            break;
        }

        if (irpsp->MajorFunction == IRP_MJ_DEVICE_CONTROL)
        {
            // ����DeviceIoControl��
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
                // �Ѿ���ӡ���ˣ���ô���ھͿ�����Ϊ��������Ѿ��ɹ���
                break;
            case CWK_DVC_RECV_STR:
            default:
                // ������������ǲ����ܵ�����δ֪������һ�ɷ��طǷ���������
                ntStatus = STATUS_INVALID_PARAMETER;
                break;
            }
        }
        break;
    }
    // ������������ǲ����ܵ�����δ֪������һ�ɷ��طǷ���������
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

    //����һ���豸����
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

    //���ɷ�������
    IoDeleteSymbolicLink(&ustrCdoSym);
    ntStatus = IoCreateSymbolicLink(&ustrCdoSym, &ustrCdoName);
    if (!NT_SUCCESS(ntStatus))
    {
        IoDeleteDevice(g_cdo);
    }

    //������ǲ����
    for (size_t i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        driver->MajorFunction[i] = cwkDispatch;
    }

    driver->DriverUnload = cwkUnload;

    // ��������豸�ĳ�ʼ�����
    g_cdo->Flags &= ~DO_DEVICE_INITIALIZING;
    return STATUS_SUCCESS;
}