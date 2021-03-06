// SearchFile.cpp: 定义 DLL 应用程序的导出函数。
// 代码注释被删除了，可以读项目USN File Search来理解

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

extern "C" _declspec(dllexport) void InitSearchFile()
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

extern "C" _declspec(dllexport) void InitFindFile(CONST WCHAR *pattern)
{
	for (int i = 0; i < 26; i++) {
		if (volumns[i] == NULL || !volumns[i]->IsNTFS()) continue;
		volumns[i]->InitSearch(pattern);
	}
}

extern "C" _declspec(dllexport) BOOL GetFindFile(ResultFindFile & resultFindFile)
{
	for (int i = 0; i < 26; i++) {
		if (volumns[i] == NULL || !volumns[i]->IsNTFS()) continue;
		if (volumns[i]->GetFindFile(resultFindFile)) {
			return TRUE;
		}
		else {
			continue;
		}
	}
	return FALSE;
}

//vector<USN_FileRecord> USN_Record_List;
//unordered_map<DWORDLONG, USN_FileRecord> USN_Hash_Table;
//
//BOOL IsNTFS(const char *RootPathName)
//{
//	// Get Information, Here just use ntfs;
//	// const char *RootPathName = "f:\\";
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
//			usn_FileRecord.FileName = new WCHAR[usn_record->FileNameLength / 2 + 1];
//			wmemcpy(usn_FileRecord.FileName, usn_record->FileName, usn_record->FileNameLength / 2);
//			usn_FileRecord.FileName[usn_record->FileNameLength / 2] = _T('\0');
//			usn_FileRecord.ParentFileReferenceNumber = usn_record->ParentFileReferenceNumber;
//			usn_FileRecord.FileReferenceNumber = usn_record->FileReferenceNumber;
//
//			USN_Record_List.push_back(usn_FileRecord);
//			USN_Hash_Table[usn_record->FileReferenceNumber] = usn_FileRecord;
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
//		while (usn_FileRecord.FileName[i] != '\0' && Path_Count<MAX_PATH)
//		{
//			PATH[Path_Count++] = usn_FileRecord.FileName[i];
//			i++;
//		}
//		PATH[Path_Count++] = _T('\\');
//	}
//}
//
//void GetParentPath(CONST WCHAR *DriverName, DWORDLONG frn)
//{
//	wmemset(PATH, _T('\0'), MAX_PATH);
//	// Length set to 3, Because "X:\"
//	wmemcpy(PATH, DriverName, 3);
//	Path_Count = 3;
//	_GetPath(USN_Hash_Table[frn].ParentFileReferenceNumber);
//	PATH[Path_Count - 1] = _T('\0');
//}
//
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
//extern "C" _declspec(dllexport) BOOL InitSearchFile()
//{
//	// Reverse space to speed up
//	//USN_Hash_Table.reserve(2000000);
//	//USN_Hash_Table.rehash(2000000);
//
//	BOOL isNTFS = IsNTFS("C:\\");
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
//extern "C" _declspec(dllexport) void InitFindFile(CONST WCHAR *pattern)
//{
//	findFileCount = 0;
//	FindResult = FindFile(pattern);
//}
//
//extern "C" _declspec(dllexport) BOOL GetFindFile(ResultFindFile & resultFindFile)
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