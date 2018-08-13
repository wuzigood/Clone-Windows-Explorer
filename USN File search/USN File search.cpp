// USN File search.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

vector<USN_FileRecord> USN_Record_List;
unordered_map<DWORDLONG, USN_FileRecord> USN_Hash_Table;

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

void GetUSNRecords(HANDLE handle, USN_JOURNAL_DATA ujd)
{
	MFT_ENUM_DATA_V0 med;
	med.StartFileReferenceNumber = 0;
	med.LowUsn = 0; //  ujd.FirstUsn;
	med.HighUsn = ujd.NextUsn;

	const DWORD  BUF_LEN = 0x10000;
	CHAR Buffer[BUF_LEN];
	DWORD usnDataSize;
	USN_RECORD *usn_record;
	
	USN_FileRecord usn_FileRecord;
	usn_FileRecord.FileName = new WCHAR[3];
	usn_FileRecord.FileName[0] = _T('C');
	usn_FileRecord.FileName[1] = _T(':');
	usn_FileRecord.FileName[2] = _T('\0');
	usn_FileRecord.ParentFileReferenceNumber = 0;
	USN_Hash_Table[0x5000000000005] = usn_FileRecord;

	while ((0 != DeviceIoControl(handle, FSCTL_ENUM_USN_DATA, &med, sizeof(med), Buffer, BUF_LEN, &usnDataSize, NULL)))
	{
		DWORD recordByte = usnDataSize - sizeof(USN);
		usn_record = (USN_RECORD *)(((CHAR *)Buffer) + sizeof(USN));

		while (recordByte > 0) 
		{
			usn_FileRecord.FileName = new WCHAR[usn_record->FileNameLength/2 + 1];
			wmemcpy(usn_FileRecord.FileName, usn_record->FileName, usn_record->FileNameLength/2);
			usn_FileRecord.FileName[usn_record->FileNameLength / 2] = _T('\0');
			usn_FileRecord.ParentFileReferenceNumber = usn_record->ParentFileReferenceNumber;
			usn_FileRecord.FileReferenceNumber = usn_record->FileReferenceNumber;
			
			USN_Record_List.push_back(usn_FileRecord);
			USN_Hash_Table[usn_record->FileReferenceNumber] = usn_FileRecord;

			recordByte -= usn_record->RecordLength;
			usn_record = (USN_RECORD *)(((CHAR *)usn_record) + usn_record->RecordLength);
		}
		med.StartFileReferenceNumber = *(USN *)&Buffer;
	}
}

WCHAR PATH[MAX_PATH];
int Path_Count = 0;
void _GetPath(DWORDLONG frn)
{
	USN_FileRecord usn_FileRecord = USN_Hash_Table[frn];
	if (usn_FileRecord.ParentFileReferenceNumber != 0)
	{
		_GetPath(usn_FileRecord.ParentFileReferenceNumber);
		int i = 0;
		while (usn_FileRecord.FileName[i] != '\0')
		{
			PATH[Path_Count++] = usn_FileRecord.FileName[i];
			i++;
		}
		PATH[Path_Count++] = _T('\\');
	}
}

/*
Usage:
USN_FileRecord usn_FileRecord = USN_Hash_Table[USN_Record_List[45].FileReferenceNumber];

wcout << usn_FileRecord.FileName << endl;
GetParentPath(_T("D:\\"), usn_FileRecord.FileReferenceNumber);
wcout << PATH << endl;
*/
void GetParentPath(CONST WCHAR *DriverName, DWORDLONG frn)
{
	wmemset(PATH, _T('\0'), MAX_PATH);
	// Length set to 3, Because "X:\"
	wmemcpy(PATH, DriverName, 3);
	Path_Count = 3;
	_GetPath(USN_Hash_Table[frn].ParentFileReferenceNumber);
	PATH[Path_Count-1] = _T('\0');
}

/*
Can use wcsstr function to substitue KMP.
不知道KMP适合多少规模的字符串匹配，会不会使用暴力的方法在规模不大的情况下会更好呢？

使用例子：
CONST WCHAR *w = wcsstr(_T("asdfdafg"), _T("fdaba"));
if (w != NULL) {
cout << "OK" << endl;
}
else {
cout << "NO" << endl;
}
*/
BOOL ContainString(WCHAR *str1, CONST WCHAR *str2)
{
	if (wcsstr(str1, str2) != NULL) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/*
遍历数组来找到文件
*/
vector<USN_FileRecord> FindFile(CONST WCHAR *pattern)
{
	vector<USN_FileRecord> usn_list;
	for (int i = 0; i < USN_Record_List.size(); i++)
	{
		if (ContainString(USN_Record_List[i].FileName, pattern))
		{
			usn_list.push_back(USN_Record_List[i]);
		}
	}
	return usn_list;
}

int main()
{
	// Reverse space to speed up
	USN_Hash_Table.reserve(2000000);
	USN_Hash_Table.rehash(2000000);

	BOOL isNTFS = IsNTFS();

	HANDLE handle;
	CREATE_USN_JOURNAL_DATA cujd;
	USN_JOURNAL_DATA ujd;

	BOOL x = GetDriveHandle(_T("\\\\.\\C:"), handle);
	BOOL y = CreateUSN(handle, cujd);
	BOOL z = QueryUSN(handle, ujd);

	GetUSNRecords(handle, ujd);

	cout << x <<" " << y << " " << z << " " <<  handle << endl;
	cout << USN_Record_List.size() << endl;
	
	locale loc("chs");
	wcout.imbue(loc);

	vector<USN_FileRecord> list =  FindFile(_T("炉石"));
	cout << list.size() << endl;
	for (int index = 0; index < list.size(); index++) {
		wcout << list[index].FileName << endl;
		GetParentPath(_T("C:\\"), list[index].FileReferenceNumber);
		wcout << PATH << endl;
	}
	system("pause");
    return 0;
}

