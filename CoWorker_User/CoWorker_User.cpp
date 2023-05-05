// CoWorker_User.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

#define CWK_DEV_SYM L"\\\\.\\cwksymbollic"

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

int main()
{
    HANDLE hDevice = NULL;
	DWORD dwRetLen = 0;

	char pszMsg[] = "Hello driver, this is a message from app.\r\n";

    hDevice = CreateFile(CWK_DEV_SYM, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("coworker demo: Open device failed.\r\n");
		return -1;
	}
	else
	{
		printf("coworker demo: Open device successfully.\r\n");
	}

	if (!DeviceIoControl(hDevice, CWK_DVC_SEND_STR, pszMsg, strlen(pszMsg) + 1, NULL, 0, &dwRetLen, 0))
	{
		printf("coworker demo: Send message failed.\r\n");
		return -2;
	}
	else
	{
		printf("coworker demo: Send message successfully.\r\n");
	}
	CloseHandle(hDevice);
	return 0;
}
