// USN File search.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

class Volumn
{
public:
	WCHAR vol;
	HANDLE VolHandle;
	CREATE_USN_JOURNAL_DATA VolCujd;
	USN_JOURNAL_DATA VolUjd;
	int findFileCount = 0;
	vector<USN_FileRecord> FindResult;
	vector<USN_FileRecord> USN_Record_List;

	Volumn(WCHAR x) : vol(x)
	{

	}

	BOOL IsNTFS()
	{
		WCHAR RootPathName[] = _T("C:\\");
		RootPathName[0] = vol;
		WCHAR VolumeNameBuffer[MAX_PATH];
		LPDWORD VolumenSerialNumber = new DWORD;
		LPDWORD MaximumComponentLength = new DWORD;
		LPDWORD FileSystemFlags = new DWORD;
		WCHAR FileSystemName[MAX_PATH];
		int status = GetVolumeInformationW(RootPathName,
			VolumeNameBuffer, MAX_PATH, VolumenSerialNumber, MaximumComponentLength,
			FileSystemFlags, FileSystemName, MAX_PATH);

		if (status != 0)
		{
			if (wcscmp(FileSystemName, _T("NTFS")) == 0) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
		return FALSE;
	}

	BOOL InitVolumn()
	{
		if (!IsNTFS())
		{
			return FALSE;
		}
		WCHAR DriveName[] = _T("\\\\.\\C:");
		DriveName[4] = vol;
		BOOL x = GetDriveHandle(DriveName, VolHandle);
		BOOL y = CreateUSN(VolHandle, VolCujd);
		BOOL z = QueryUSN(VolHandle, VolUjd);
		if (!(x && y && z)) {
			return FALSE;
		}
		GetUSNRecords(VolHandle, VolUjd);
		return TRUE;
	}

	VOID InitSearch(CONST WCHAR *Pattern)
	{
		findFileCount = 0;
		FindResult = FindFile(Pattern);
	}

	BOOL GetFindFile(ResultFindFile & resultFindFile)
	{
		if (findFileCount >= FindResult.size()) {
			return FALSE;
		}
		TCHAR filePath[MAX_PATH];
		HANDLE hh = OpenFileById(VolHandle, &(getFileIdDescriptor(FindResult[findFileCount].FileReferenceNumber)), 0, 0, 0, 0);
		GetFinalPathNameByHandle(hh, filePath, MAX_PATH, 0);

		int i = -1;
		do {
			i++;
			resultFindFile.FileName[i] = FindResult[findFileCount].FileName[i];
		} while (FindResult[findFileCount].FileName[i] != '\0' && i<MAX_PATH);
		i = -1;
		do {
			i++;
			resultFindFile.Path[i] = filePath[i + 4];
		} while (filePath[i + 4] != '\0' && i<MAX_PATH);
		findFileCount++;
		return TRUE;
	}

private:

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
		usn_FileRecord.FileName[0] = vol;
		usn_FileRecord.FileName[1] = _T(':');
		usn_FileRecord.FileName[2] = _T('\0');
		usn_FileRecord.ParentFileReferenceNumber = 0;

		while ((0 != DeviceIoControl(handle, FSCTL_ENUM_USN_DATA, &med, sizeof(med), Buffer, BUF_LEN, &usnDataSize, NULL)))
		{
			DWORD recordByte = usnDataSize - sizeof(USN);
			usn_record = (USN_RECORD *)(((CHAR *)Buffer) + sizeof(USN));

			while (recordByte > 0)
			{
				usn_FileRecord.FileName = new WCHAR[usn_record->FileNameLength / 2 + 1];
				wmemcpy(usn_FileRecord.FileName, usn_record->FileName, usn_record->FileNameLength / 2);
				usn_FileRecord.FileName[usn_record->FileNameLength / 2] = _T('\0');
				usn_FileRecord.ParentFileReferenceNumber = usn_record->ParentFileReferenceNumber;
				usn_FileRecord.FileReferenceNumber = usn_record->FileReferenceNumber;

				USN_Record_List.push_back(usn_FileRecord);

				recordByte -= usn_record->RecordLength;
				usn_record = (USN_RECORD *)(((CHAR *)usn_record) + usn_record->RecordLength);
			}
			med.StartFileReferenceNumber = *(USN *)&Buffer;
		}
	}

	BOOL ContainString(WCHAR *str1, CONST WCHAR *str2)
	{
		if (wcsstr(str1, str2) != NULL) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}

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

	FILE_ID_DESCRIPTOR getFileIdDescriptor(const DWORDLONG fileId)
	{
		FILE_ID_DESCRIPTOR fileDescriptor;
		fileDescriptor.Type = FileIdType;
		fileDescriptor.FileId.QuadPart = fileId;
		fileDescriptor.dwSize = sizeof(fileDescriptor);

		return fileDescriptor;
	}

};

Volumn* volumns[26];

void InitSearchFile()
{
	DWORD DriverNumber = GetLogicalDrives();
	DWORD mask = 1;
	for (int i = 0; i < 26; i++) {
		if (DriverNumber & mask) {
			volumns[i] = new Volumn(WCHAR(_T('A') + i));
			volumns[i]->InitVolumn();
		}
		else {
			volumns[i] = NULL;
		}
		mask = mask << 1;
	}
}

int FindFileCount = 0;
vector<ResultFindFile> resultFindFiles;
void InitFindFile(CONST WCHAR *pattern)
{
	FindFileCount = 0;
	resultFindFiles.clear();
	for (int i = 0; i < 26; i++) {
		if (volumns[i] ==NULL || !volumns[i]->IsNTFS()) continue;
		volumns[i]->InitSearch(pattern);
		ResultFindFile resultFindFile;
		while (volumns[i]->GetFindFile(resultFindFile))
		{
			resultFindFiles.push_back(resultFindFile);
		}
	}
}

BOOL GetFindFile(ResultFindFile & resultFindFile)
{
	if (FindFileCount >= resultFindFiles.size()) return FALSE;
	resultFindFile = resultFindFiles[FindFileCount++];
	return TRUE;
}

void SaveToFile()
{

}

void CopyDirectory(CONST WCHAR *Source, CONST WCHAR *Destination)
{
	SHFILEOPSTRUCT s = { 0 };
	s.wFunc = FO_COPY;
	s.pTo = Destination;
	s.pFrom = Source;
	s.fFlags = FOF_SILENT;
	SHFileOperation(&s);
}

void DeleteDirectory(CONST WCHAR *Path)
{
	SHFILEOPSTRUCT s = { 0 };
	s.wFunc = FO_DELETE;
	s.pFrom = Path;
	s.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;
	SHFileOperation(&s);
}

BOOL IsDirectory(CONST WCHAR *Path)
{
	return PathIsDirectoryW(Path);
}

int main()
{
	if(DirectoryExists(_T("C:\\Users\\zzk\\source\\repos\\WpfApp1\\WpfApp1\\bin\\Debug")))
		printf("OK");
	//HANDLE hFile;
	//hFile = CreateFile(_T("Search.txt"), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//if (hFile == INVALID_HANDLE_VALUE)
	//{
	//	return 1;
	//}
	//CONST WCHAR *Text = _T("C:\\中文");
	//DWORD TextLen = (DWORD)lstrlenW(Text);
	//WriteFile(hFile, Text, TextLen * sizeof(WCHAR), 0, 0);
	////InitSearchFile();
	////InitFindFile(_T("hearth"));
	////ResultFindFile resultFindFile;
	////while (GetFindFile(resultFindFile))
	////{
	////	wcout << resultFindFile.FileName << ":" << resultFindFile.Path << endl;
	////}
	system("pause");
}
//
//vector<USN_FileRecord> USN_Record_List;
//unordered_map<DWORDLONG, USN_FileRecord> USN_Hash_Table;
//
///*
//GetVolumeInformationA 函数定义
//WINBASEAPI
//BOOL
//WINAPI
//GetVolumeInformationA(
//_In_opt_ LPCSTR lpRootPathName,
//_Out_writes_opt_(nVolumeNameSize) LPSTR lpVolumeNameBuffer,
//_In_ DWORD nVolumeNameSize,
//_Out_opt_ LPDWORD lpVolumeSerialNumber,
//_Out_opt_ LPDWORD lpMaximumComponentLength,
//_Out_opt_ LPDWORD lpFileSystemFlags,
//_Out_writes_opt_(nFileSystemNameSize) LPSTR lpFileSystemNameBuffer,
//_In_ DWORD nFileSystemNameSize
//);
//
//*/
//
//BOOL IsNTFS()
//{
//	// Get Information, Here just use ntfs;
//	const char *RootPathName = "C:\\";
//	char VolumeNameBuffer[MAX_PATH];
//	LPDWORD VolumenSerialNumber = new DWORD;
//	LPDWORD MaximumComponentLength = new DWORD;
//	LPDWORD FileSystemFlags = new DWORD;
//	char FileSystemName[MAX_PATH];
//	int status = GetVolumeInformationA(RootPathName, 
//		VolumeNameBuffer, MAX_PATH, VolumenSerialNumber, MaximumComponentLength,
//		FileSystemFlags, FileSystemName, MAX_PATH);
//
//	if (status != 0) 
//	{
//		if (strcmp(FileSystemName, "NTFS") == 0) {
//			return TRUE;
//		}
//		else {
//			return FALSE;
//		}
//	}
//	return FALSE;
//}
//
///*
//MSDN上说第一个参数如果是Drive的话，需要形如这样的格式："\\.\C:"
//实际上应该是："\\\\.\\C:"，因为 '\' 是个转义字符。
//
//详见：https://docs.microsoft.com/en-us/windows/desktop/api/fileapi/nf-fileapi-createfilew
//搜索关键词Drive
//
//CreateFile is define CreateFileW
//
//CreateFileW(
//_In_ LPCWSTR lpFileName,
//_In_ DWORD dwDesiredAccess,
//_In_ DWORD dwShareMode,
//_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
//_In_ DWORD dwCreationDisposition,
//_In_ DWORD dwFlagsAndAttributes,
//_In_opt_ HANDLE hTemplateFile
//);
//*/
//
//BOOL GetDriveHandle(LPCWSTR DriveName, HANDLE &retHandle)
//{
//	HANDLE hVol = CreateFile(DriveName,
//		GENERIC_READ | GENERIC_WRITE,
//		FILE_SHARE_READ | FILE_SHARE_WRITE,
//		NULL,
//		OPEN_EXISTING,
//		FILE_ATTRIBUTE_READONLY,
//		NULL);
//
//	retHandle = hVol;
//
//	if (hVol != INVALID_HANDLE_VALUE)
//	{
//		return TRUE;
//	}
//	else
//	{
//		return FALSE;
//	}
//}
//
///*
//向设备发送ControlCode，让设备执行特定的工作
//
//DeviceIoControl(
//_In_ HANDLE hDevice,
//_In_ DWORD dwIoControlCode,
//_In_reads_bytes_opt_(nInBufferSize) LPVOID lpInBuffer,
//_In_ DWORD nInBufferSize,
//_Out_writes_bytes_to_opt_(nOutBufferSize,*lpBytesReturned) LPVOID lpOutBuffer,
//_In_ DWORD nOutBufferSize,
//_Out_opt_ LPDWORD lpBytesReturned,
//_Inout_opt_ LPOVERLAPPED lpOverlapped
//);
//*/
//BOOL CreateUSN(HANDLE handle, CREATE_USN_JOURNAL_DATA &cujd)
//{
//	cujd.AllocationDelta = 0;
//	cujd.MaximumSize = 0;
//
//	DWORD br;
//
//	BOOL status = DeviceIoControl(handle, FSCTL_CREATE_USN_JOURNAL, &cujd, sizeof(cujd), NULL, 0, &br, NULL);
//	if (status) {
//		return TRUE;
//	}
//	return FALSE;
//}
//
///*
//typedef struct {
//
//DWORDLONG UsnJournalID;
//USN FirstUsn;
//USN NextUsn;
//USN LowestValidUsn;
//USN MaxUsn;
//DWORDLONG MaximumSize;
//DWORDLONG AllocationDelta;
//WORD   MinSupportedMajorVersion;
//WORD   MaxSupportedMajorVersion;
//
//} USN_JOURNAL_DATA_V1, *PUSN_JOURNAL_DATA_V1;
//*/
//BOOL QueryUSN(HANDLE handle, USN_JOURNAL_DATA &ujd)
//{
//	DWORD br;
//	BOOL status = DeviceIoControl(handle, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &ujd, sizeof(ujd), &br, NULL);
//	if (status) {
//		return TRUE;
//	}
//	return FALSE;
//} 
//
//void GetUSNRecords(HANDLE handle, USN_JOURNAL_DATA ujd)
//{
//	MFT_ENUM_DATA_V0 med;
//	med.StartFileReferenceNumber = 0;
//	med.LowUsn = 0; //  ujd.FirstUsn;
//	med.HighUsn = ujd.NextUsn;
//
//	const DWORD  BUF_LEN = 0x10000;
//	CHAR Buffer[BUF_LEN];
//	DWORD usnDataSize;
//	USN_RECORD *usn_record;
//	
//	USN_FileRecord usn_FileRecord;
//	usn_FileRecord.FileName = new WCHAR[3];
//	usn_FileRecord.FileName[0] = _T('C');
//	usn_FileRecord.FileName[1] = _T(':');
//	usn_FileRecord.FileName[2] = _T('\0');
//	usn_FileRecord.ParentFileReferenceNumber = 0;
//	USN_Hash_Table[0x5000000000005] = usn_FileRecord;
//
//	while ((0 != DeviceIoControl(handle, FSCTL_ENUM_USN_DATA, &med, sizeof(med), Buffer, BUF_LEN, &usnDataSize, NULL)))
//	{
//		DWORD recordByte = usnDataSize - sizeof(USN);
//		usn_record = (USN_RECORD *)(((CHAR *)Buffer) + sizeof(USN));
//
//		while (recordByte > 0) 
//		{
//			usn_FileRecord.FileName = new WCHAR[usn_record->FileNameLength/2 + 1];
//			wmemcpy(usn_FileRecord.FileName, usn_record->FileName, usn_record->FileNameLength/2);
//			usn_FileRecord.FileName[usn_record->FileNameLength / 2] = _T('\0');
//			usn_FileRecord.ParentFileReferenceNumber = usn_record->ParentFileReferenceNumber;
//			usn_FileRecord.FileReferenceNumber = usn_record->FileReferenceNumber;
//			
//			USN_Record_List.push_back(usn_FileRecord);
//			//USN_Hash_Table[usn_record->FileReferenceNumber] = usn_FileRecord;
//
//			recordByte -= usn_record->RecordLength;
//			usn_record = (USN_RECORD *)(((CHAR *)usn_record) + usn_record->RecordLength);
//		}
//		med.StartFileReferenceNumber = *(USN *)&Buffer;
//	}
//}
//
//WCHAR PATH[MAX_PATH];
//int Path_Count = 0;
//void _GetPath(DWORDLONG frn)
//{
//	USN_FileRecord usn_FileRecord = USN_Hash_Table[frn];
//	if (usn_FileRecord.ParentFileReferenceNumber != 0)
//	{
//		_GetPath(usn_FileRecord.ParentFileReferenceNumber);
//		int i = 0;
//		while (usn_FileRecord.FileName[i] != '\0')
//		{
//			PATH[Path_Count++] = usn_FileRecord.FileName[i];
//			i++;
//		}
//		PATH[Path_Count++] = _T('\\');
//	}
//}
//
///*
//Usage:
//USN_FileRecord usn_FileRecord = USN_Hash_Table[USN_Record_List[45].FileReferenceNumber];
//
//wcout << usn_FileRecord.FileName << endl;
//GetParentPath(_T("D:\\"), usn_FileRecord.FileReferenceNumber);
//wcout << PATH << endl;
//*/
//void GetParentPath(CONST WCHAR *DriverName, DWORDLONG frn)
//{
//	wmemset(PATH, _T('\0'), MAX_PATH);
//	// Length set to 3, Because "X:\"
//	wmemcpy(PATH, DriverName, 3);
//	Path_Count = 3;
//	_GetPath(USN_Hash_Table[frn].ParentFileReferenceNumber);
//	PATH[Path_Count-1] = _T('\0');
//}
//
///*
//Can use wcsstr function to substitue KMP.
//不知道KMP适合多少规模的字符串匹配，会不会使用暴力的方法在规模不大的情况下会更好呢？
//
//使用例子：
//CONST WCHAR *w = wcsstr(_T("asdfdafg"), _T("fdaba"));
//if (w != NULL) {
//cout << "OK" << endl;
//}
//else {
//cout << "NO" << endl;
//}
//*/
//BOOL ContainString(WCHAR *str1, CONST WCHAR *str2)
//{
//	if (wcsstr(str1, str2) != NULL) {
//		return TRUE;
//	}
//	else {
//		return FALSE;
//	}
//}
//
///*
//遍历数组来找到文件
//*/
//vector<USN_FileRecord> FindFile(CONST WCHAR *pattern)
//{
//	vector<USN_FileRecord> usn_list;
//	for (int i = 0; i < USN_Record_List.size(); i++)
//	{
//		if (ContainString(USN_Record_List[i].FileName, pattern))
//		{
//			usn_list.push_back(USN_Record_List[i]);
//		}
//	}
//	return usn_list;
//}
//
//
//BOOL InitSearchFile()
//{
//	// Reverse space to speed up
//	//USN_Hash_Table.reserve(2000000);
//	//USN_Hash_Table.rehash(2000000);
//
//	BOOL isNTFS = IsNTFS();
//	if (!isNTFS) {
//		return FALSE;
//	}
//	HANDLE handle;
//	CREATE_USN_JOURNAL_DATA cujd;
//	USN_JOURNAL_DATA ujd;
//	BOOL x = GetDriveHandle(_T("\\\\.\\C:"), handle);
//	BOOL y = CreateUSN(handle, cujd);
//	BOOL z = QueryUSN(handle, ujd);
//	if (!(x && y && z)) {
//		return FALSE;
//	}
//	GetUSNRecords(handle, ujd);
//	return TRUE;
//}
//
//int findFileCount = 0;
//vector<USN_FileRecord> FindResult;
//
//void InitFindFile(CONST WCHAR *pattern)
//{
//	findFileCount = 0;
//	FindResult = FindFile(pattern);
//}
//
//BOOL GetFindFile(ResultFindFile & resultFindFile)
//{
//	if (findFileCount >= FindResult.size()) {
//		return FALSE;
//	}
//	USN_FileRecord usn_FileRecord = FindResult[findFileCount++];
//	GetParentPath(_T("C:\\"), usn_FileRecord.FileReferenceNumber);
//
//	//wmemcpy(resultFindFile.FileName, usn_FileRecord.FileName, MAX_PATH/sizeof(WCHAR));
//	//wmemcpy(resultFindFile.Path, PATH, MAX_PATH/sizeof(WCHAR));
//	//memcpy(resultFindFile.FileName, usn_FileRecord.FileName, MAX_PATH);
//	//memcpy(resultFindFile.Path, PATH, MAX_PATH);
//	int i = -1;
//	do {
//		i++;
//		resultFindFile.FileName[i] = usn_FileRecord.FileName[i];
//	} while (usn_FileRecord.FileName[i] != '\0' && i<MAX_PATH);
//	i = -1;
//	do {
//		i++;
//		resultFindFile.Path[i] = PATH[i];
//	} while (PATH[i] != '\0' && i<MAX_PATH);
//	return TRUE;
//}
///*
//InitSearchFile();
//InitFindFile(_T("a"));
//ResultFindFile resultFindFile;
//int i = 0;
//while (GetFindFile(resultFindFile))
//{
//i++;
//wcout << i << _T('\t') <<resultFindFile.FileName << _T(":") << resultFindFile.Path << endl;
//}
//*/
//FILE_ID_DESCRIPTOR getFileIdDescriptor(const DWORDLONG fileId)
//{
//	FILE_ID_DESCRIPTOR fileDescriptor;
//	fileDescriptor.Type = FileIdType;
//	fileDescriptor.FileId.QuadPart = fileId;
//	fileDescriptor.dwSize = sizeof(fileDescriptor);
//
//	return fileDescriptor;
//}
//
//int main()
//{
//	BOOL isNTFS = IsNTFS();
//	if (!isNTFS) {
//		return FALSE;
//	}
//	HANDLE handle;
//	CREATE_USN_JOURNAL_DATA cujd;
//	USN_JOURNAL_DATA ujd;
//	BOOL x = GetDriveHandle(_T("\\\\.\\C:"), handle);
//	BOOL y = CreateUSN(handle, cujd);
//	BOOL z = QueryUSN(handle, ujd);
//	if (!(x && y && z)) {
//		return FALSE;
//	}
//	GetUSNRecords(handle, ujd);
//
//	InitFindFile(_T(".mp4"));
//
//	for (int i = 0; i < FindResult.size(); i++) {
//		USN_FileRecord UsnRecord = FindResult[i];
//
//		TCHAR filePath[MAX_PATH];
//		HANDLE hh = OpenFileById(handle, &(getFileIdDescriptor(UsnRecord.FileReferenceNumber)), 0, 0, 0, 0);
//		GetFinalPathNameByHandle(hh, filePath, MAX_PATH, 0);
//
//		int index = -1;
//		do {
//			index++;
//			filePath[index] = filePath[index + 4];
//		} while (filePath[index +4] != '\0');
//		std::wcout << filePath << endl;
//	}
//	system("pause");
//    return 0;
//}
//
