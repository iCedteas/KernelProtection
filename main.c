#include "h.h"

OB_PREOP_CALLBACK_STATUS Callback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation)
{
	PUCHAR curProcessName = PsGetProcessImageFileName((PEPROCESS)OperationInformation->Object);
	if (strcmp((PCHAR)RegistrationContext, curProcessName) == 0)
	{
		if (OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE)
			// ���̵ľ���ķ���Ȩ��
			OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
		if (OperationInformation->Operation == OB_OPERATION_HANDLE_DUPLICATE)
			// �̵߳ľ���ķ���Ȩ��
			OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess = 0;
	}
	return OB_PREOP_SUCCESS;
}

VOID Protection1(PCHAR procName, PDRIVER_OBJECT pDriverObject)
{
	/*
	* ����win10
	* ObRegisterCallbacks �ڲ��� MmVerifyCallbackFunctionCheckFlags ����У��ͨ���ſ���ע��ɹ�
	* E8 A5 52 BE FF 85 C0 0F 84 BF 72 09 00
	*
	* MmVerifyCallbackFunctionCheckFlags:
	* ...
	* ldrData = MiLookupDataTableEntry(PINT64, 0);
	* v2 = 32
	* �����ڲ�ͨ�� if ( ldrData && (!v2 || *(_DWORD *)(ldrData + 104) & v2) ) ����У��
	* *(_DWORD *)(ldrData + 104) ��ʵ���� pDriverObject->DriverSection->Flags
	* ��������ֱ�� Flags|=0x20
	*/
	((PLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection)->Flags |= 0x20;


	OB_OPERATION_REGISTRATION or ;
	or .ObjectType = PsProcessType;
	or .Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	or .PreOperation = (POB_PRE_OPERATION_CALLBACK)&Callback;
	or .PostOperation = NULL;

	OB_CALLBACK_REGISTRATION callbackRegistration = { 0 };
	callbackRegistration.Version = OB_FLT_REGISTRATION_VERSION;
	callbackRegistration.OperationRegistrationCount = 1;
	UNICODE_STRING  altitude = RTL_CONSTANT_STRING(L"20202");
	callbackRegistration.Altitude = altitude;
	callbackRegistration.RegistrationContext = procName;
	callbackRegistration.OperationRegistration = &or ;

	PVOID h;
	ObRegisterCallbacks(&callbackRegistration, &h);
	// DriverUnload ʱ���� ObUnRegisterCallbacks ȡ���ص�
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegPath)
{
	// ����1:ͨ���ٷ��ṩע��ص� ObRegisterCallbacks ʵ��
	// Protection1("notepad.exe", pDriverObject);

	// ����2:ͨ��hook KiFastSystemCall
	Protection2("notepad.exe");

	return STATUS_SUCCESS;
}

