// USN File search.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

vector<USN_FileRecord> USN_Record_List;

/*
GetVolumeInformationA 函数定义
WINBASEAPI
BOOL
WINAPI
GetVolumeInformationA(
_In_opt_ LPCSTR lpRootPathName,
_Out_writes_opt_(nVolumeNameSize) LPSTR lpVolumeNameBuffer,
_In_ DWORD nVolumeNameSize,
_Out_opt_ LPDWORD lpVolumeSerialNumber,
_Out_opt_ LPDWORD lpMaximumComponentLength,
_Out_opt_ LPDWORD lpFileSystemFlags,
_Out_writes_opt_(nFileSystemNameSize) LPSTR lpFileSystemNameBuffer,
_In_ DWORD nFileSystemNameSize
);

*/

BOOL IsNTFS()
{
	// Get Information, Here just use ntfs;
	const char *RootPathName = "f:\\";
	char VolumeNameBuffer[MAX_PATH];
	LPDWORD VolumenSerialNumber = new DWORD;
	LPDWORD MaximumComponentLength = new DWORD;
	LPDWORD FileSystemFlags = new DWORD;
	char FileSystemName[MAX_PATH];
	int status = GetVolumeInformationA(RootPathName, 
		VolumeNameBuffer, MAX_PATH, VolumenSerialNumber, MaximumComponentLength,
		FileSystemFlags, FileSystemName, MAX_PATH);

	if (status != 0) 
	{
		if (strcmp(FileSystemName, "NTFS") == 0) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	return FALSE;
}

/*
MSDN上说第一个参数如果是Drive的话，需要形如这样的格式："\\.\C:"
实际上应该是："\\\\.\\C:"，因为 '\' 是个转义字符。

详见：https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-createfilew
搜索关键词Drive

CreateFile is define CreateFileW

CreateFileW(
_In_ LPCWSTR lpFileName,
_In_ DWORD dwDesiredAccess,
_In_ DWORD dwShareMode,
_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
_In_ DWORD dwCreationDisposition,
_In_ DWORD dwFlagsAndAttributes,
_In_opt_ HANDLE hTemplateFile
);
*/

BOOL GetDriveHandle(LPCWSTR DriveName, HANDLE &retHandle)
{
	HANDLE hVol = CreateFile(DriveName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL);

	retHandle = hVol;

	if (hVol != INVALID_HANDLE_VALUE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*
向设备发送ControlCode，让设备执行特定的工作

DeviceIoControl(
_In_ HANDLE hDevice,
_In_ DWORD dwIoControlCode,
_In_reads_bytes_opt_(nInBufferSize) LPVOID lpInBuffer,
_In_ DWORD nInBufferSize,
_Out_writes_bytes_to_opt_(nOutBufferSize,*lpBytesReturned) LPVOID lpOutBuffer,
_In_ DWORD nOutBufferSize,
_Out_opt_ LPDWORD lpBytesReturned,
_Inout_opt_ LPOVERLAPPED lpOverlapped
);
*/
BOOL CreateUSN(HANDLE handle, CREATE_USN_JOURNAL_DATA &cujd)
{
	cujd.AllocationDelta = 0;
	cujd.MaximumSize = 0;

	DWORD br;

	BOOL status = DeviceIoControl(handle, FSCTL_CREATE_USN_JOURNAL, &cujd, sizeof(cujd), NULL, 0, &br, NULL);
	if (status) {
		return TRUE;
	}
	return FALSE;
}

/*
typedef struct {

DWORDLONG UsnJournalID;
USN FirstUsn;
USN NextUsn;
USN LowestValidUsn;
USN MaxUsn;
DWORDLONG MaximumSize;
DWORDLONG AllocationDelta;
WORD   MinSupportedMajorVersion;
WORD   MaxSupportedMajorVersion;

} USN_JOURNAL_DATA_V1, *PUSN_JOURNAL_DATA_V1;
*/
BOOL QueryUSN(HANDLE handle, USN_JOURNAL_DATA &ujd)
{
	DWORD br;
	BOOL status = DeviceIoControl(handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &ujd, sizeof(ujd), &br, NULL);
	if (status) {
		return TRUE;
	}
	return FALSE;
}
/*
USN Record data wiil be stored in a std::vector<USN_RECORD>
*/
void GetUSNRecords(HANDLE handle, USN_JOURNAL_DATA ujd)
{
	MFT_ENUM_DATA_V0 med;
	med.StartFileReferenceNumber = 0;
	med.LowUsn = 0; //  ujd.FirstUsn;
	med.HighUsn = ujd.NextUsn;

#define BUF_LEN 0x10000
	CHAR Buffer[BUF_LEN];
	DWORD usnDataSize;
	USN_RECORD *usn_record;

	while ((0 != DeviceIoControl(handle, FSCTL_ENUM_USN_DATA, &med, sizeof(med), Buffer, BUF_LEN, &usnDataSize, NULL)))
	{
		USN_FileRecord usn_FileRecord;
		DWORD recordByte = usnDataSize - sizeof(USN);
		usn_record = (USN_RECORD *)(((CHAR *)Buffer) + sizeof(USN));

		while (recordByte > 0) 
		{
			//memset(&usn_FileRecord.FileName, _T('\0'), 100);
			//memcpy(&usn_FileRecord.FileName, &usn_record->FileName, usn_record->FileNameLength);
			usn_FileRecord.FileName = new WCHAR[usn_record->FileNameLength/2 + 1];
			usn_FileRecord.FileName[usn_record->FileNameLength/2] = _T('\0');
			for (int i = 0; i < usn_record->FileNameLength/2; i++) {
				usn_FileRecord.FileName[i] = usn_record->FileName[i];
			}
			//usn_FileRecord.FileName[usn_record->FileNameLength + 1] = _T('\0');
			usn_FileRecord.FileReferenceNumber = usn_record->FileReferenceNumber;
			usn_FileRecord.ParentFileReferenceNumber = usn_record->ParentFileReferenceNumber;
			
			USN_Record_List.push_back(usn_FileRecord);

			//DWORD record_length = usn_record->RecordLength;
			recordByte -= usn_record->RecordLength;
			usn_record = (USN_RECORD *)(((CHAR *)usn_record) + usn_record->RecordLength);
		}
		med.StartFileReferenceNumber = *(USN *)&Buffer;
	}
}

struct var_struct
{
	WCHAR *str;
	int length;
};

int main()
{
	BOOL isNTFS = IsNTFS();

	HANDLE handle;
	CREATE_USN_JOURNAL_DATA cujd;
	USN_JOURNAL_DATA ujd;

	BOOL x = GetDriveHandle(_T("\\\\.\\C:"), handle);
	BOOL y = CreateUSN(handle, cujd);
	BOOL z = QueryUSN(handle, ujd);

	GetUSNRecords(handle, ujd);

	cout << x <<" " << y << " " << z << " " <<  handle << endl;
	
	locale loc("chs");
	wcout.imbue(loc);
	for (int i = 100000; i < USN_Record_List.size(); i++) {
		wcout << i << " " << USN_Record_List[i].FileName <<  endl;
	}

	//x = GetDriveHandle(_T("\\\\.\\D:"), handle);
	//y = CreateUSN(handle, cujd);
	//z = QueryUSN(handle, ujd);

	//GetUSNRecords(handle, ujd);
	//for (int j = 0; j < USN_Record_List.size(); j++) {
	//	wcout << i+j << " " << USN_Record_List[j].FileName << endl;
	//}
	cout << USN_Record_List.size() << endl;
	system("pause");
    return 0;
}

