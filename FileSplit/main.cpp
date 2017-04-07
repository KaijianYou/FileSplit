// �˴������Ŀ˵��

#include <iostream>
#include <string>
#include <ctime>
#include <locale>

#include <windows.h>
#include <direct.h>
#include <tchar.h>

#define PER_RW_BYTES		0x00200000		// ÿ�ζ�д�ֽ�����2 MB

using std::string;
using std::cout;
using std::endl;

static int outfileSeqNum = 0;     // �ļ����

struct time {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
};

// ��ȡϵͳ��ǰʱ��
void getNowTime(struct time *t)
{
	time_t now_time;
	struct tm *time_info;
	time(&now_time);
	time_info = localtime(&now_time);

	t->year = time_info->tm_year + 1900;
	t->month = time_info->tm_mon + 1;
	t->day = time_info->tm_mday;
	t->hour = time_info->tm_hour;
	t->minute = time_info->tm_min;
	t->second = time_info->tm_sec;
}

// �������ļ�
HANDLE createFile(const string &filePrefix)
{
	outfileSeqNum += 1;

	string outfileName = filePrefix;
	char fileSuffix[10];
	sprintf(fileSuffix, "_%02d", outfileSeqNum);
	outfileName.append(fileSuffix, strlen(fileSuffix));

	int outfileNameSize = static_cast<int>(outfileName.length() + 1);
	wchar_t *wOutfileName = new wchar_t[outfileNameSize];
	MultiByteToWideChar(CP_ACP, 0, outfileName.c_str(), -1, wOutfileName, outfileNameSize);

	HANDLE hFile = CreateFile(wOutfileName, GENERIC_WRITE, 0, NULL, 
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		delete[] wOutfileName;
		cout << "�����ļ�ʧ�ܣ�������룺" << GetLastError() << endl;
		return INVALID_HANDLE_VALUE;
	}

	delete[] wOutfileName;
	return hFile;
}

// �ļ�ָ�붨λ
__int64 fileSeek(HANDLE hf, __int64 distance, DWORD MoveMethod)
{
	LARGE_INTEGER li;
	li.QuadPart = distance;
	li.LowPart = SetFilePointer(hf, li.LowPart, &li.HighPart, MoveMethod);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		li.QuadPart = -1;
	}

	return li.QuadPart;
}

// �����ļ�·��
void handleFilePath(const string &infilePath, string &infileDir, 
		            string &infileFullName, string &infileName)
{
	string::size_type infilePathLength = infilePath.size();
	string::size_type backslashLastIndex;
	if (string::npos == (backslashLastIndex = infilePath.rfind('\\', infilePathLength - 1))) {
		infileFullName = infilePath;
		infileDir = ".\\";
	} else {
		infileFullName = infilePath.substr(backslashLastIndex + 1);
		infileDir = infilePath.substr(0, backslashLastIndex);
	}

	string::size_type infileFullNameLength = infileFullName.size();
	string::size_type dotLastIndex;
	if (string::npos == (dotLastIndex = infileFullName.rfind('.',
			infileFullNameLength - 1))) {
		infileName = infileFullName;
	} else {
		infileName = infileFullName.substr(0, dotLastIndex);
	}
}

// �����ļ��в��л�����Ŀ¼�����ļ�����
bool createDir(const string& infileDir, const string& infileName, char dirName[])
{
	struct time t;
	getNowTime(&t);

	_chdir(infileDir.c_str());

	// �����ļ���
	sprintf(dirName, ".//%s_%04d-%02d-%02d_%02d-%02d-%02d", infileName.c_str(),
			t.year, t.month, t.day, t.hour, t.minute, t.second);
	if (-1 == _mkdir(dirName)) {
		cout << "�����ļ��� " << dirName << " ʧ�ܣ�" << endl;
		return false;
	}

	return true;
}

// ��ȡ������
void getAppName(TCHAR appName[])
{
	const int FULL_PATH_MAX_SIZE = 512;
	TCHAR appPath[FULL_PATH_MAX_SIZE] = { 0 };
	char g_strAppPath[FULL_PATH_MAX_SIZE] = { 0 };
	GetModuleFileName(NULL, appPath, FULL_PATH_MAX_SIZE);
	_tcsncpy(appName, _tcsrchr(appPath, '\\') + 1, _tcschr(appPath, '\0') - (_tcsrchr(appPath, '\\') + 1));
}

int main(int argc, char *argv[])
{
	const int FILE_NAME_SIZE = 256;
	TCHAR appName[FILE_NAME_SIZE] = { '/0' };
	getAppName(appName);
	
	if (argc != 4) {
		cout << "Usage: ";
		std::wcout << appName << " [-bkmg] Bytes" << endl;
		cout << "    -b Byte" << endl;
		cout << "    -k Kilobyte" << endl;
		cout << "    -m Megabyte" << endl;
		cout << "    -g Gigabyte" << endl;
		return EXIT_FAILURE;
	}

	DWORD numBytes = atol(argv[3]);
	DWORD nbytes = 1;
	switch (argv[2][1]) {
		case 'b': nbytes = numBytes; break;
		case 'k': nbytes = numBytes * 1024; break;
		case 'm': nbytes = numBytes * 1024 * 1024; break;
		case 'g': nbytes = numBytes * 1024 * 1024 * 1024; break;
		default: nbytes = 0; break;
	}
	
	if (nbytes == 0) {
		cout << "��������" << endl;
		return EXIT_FAILURE;
	}

	string infilePath(argv[1]);

	int infilePathSize = static_cast<int>(infilePath.length() + 1);
	wchar_t *wInfilePath = new wchar_t[infilePathSize];
	MultiByteToWideChar(CP_ACP, 0, infilePath.c_str(), -1, wInfilePath, infilePathSize);

	// �����ļ����󣨴��ļ���
	HANDLE hInfile = CreateFile(wInfilePath, GENERIC_READ, 0, 
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hInfile == INVALID_HANDLE_VALUE) {
		delete[] wInfilePath;
		cout << "���ļ�ʧ�ܣ�������룺" << GetLastError() << endl;
		return EXIT_FAILURE;
	}
	delete[] wInfilePath;

	// ��ȡ�ļ���С
	DWORD dwFileSizeHigh;
	__int64 qwFileSize = GetFileSize(hInfile, &dwFileSizeHigh);
	qwFileSize |= (((__int64)dwFileSizeHigh) << 32);
	cout << "�ļ���С��" << qwFileSize << " �ֽ�" << endl;
	cout << "���ļ���С��" << nbytes << " �ֽ�" << endl;

	if ((__int64)nbytes > qwFileSize) {
		cout << "���ļ���С�����ļ���С,�޷��з�!" << endl;
		return EXIT_FAILURE;
	}

	// �����ļ��з�����
	__int64 fileNum = (qwFileSize % nbytes == 0) ? qwFileSize / nbytes : qwFileSize / nbytes + 1;
	cout << "���зֳ� " << fileNum << " ���ļ�" << endl;

	string infileFullName;
	string infileDir;
	string infileName;

	// �����ļ�·��
	handleFilePath(infilePath, infileDir, infileFullName, infileName);

	// �����ļ��к��л�����Ŀ¼
	char dirName[256];
	if (!createDir(infileDir, infileName, dirName)) {
		return EXIT_FAILURE;
	}
	_chdir(dirName);

	cout << "�����з��ļ������Ժ�......" << endl;

	// ��λ�ļ���дָ�뵽�ļ�ͷ
	if (-1 == fileSeek(hInfile, 0, FILE_BEGIN)) {
		cout << "�ļ�ָ�붨λ����" << endl;
		return EXIT_FAILURE;
	}

	__int64 toBeReadBytes = qwFileSize;
	DWORD readBytes = 0;
	DWORD splitFileReadBytes = 0;

	while (toBeReadBytes > 0) {
		// �����ļ�
		HANDLE hOutfile;
		if (INVALID_HANDLE_VALUE == (hOutfile = createFile(infileName))) {
			cout << "�����ļ�ʧ�ܣ�������룺" << GetLastError() << endl;
			return EXIT_FAILURE;
		}
		printf("���ڴ���� %d ���ļ�...\r", outfileSeqNum);
		splitFileReadBytes = (DWORD)(toBeReadBytes < nbytes ? toBeReadBytes : nbytes);

		DWORD oneFileReadBytes = 0;
		DWORD eachTimeReadBytes = 0;
		for (DWORD oneFileToBeReadBytes = splitFileReadBytes; oneFileToBeReadBytes > 0; ) {
			eachTimeReadBytes = (DWORD)(oneFileToBeReadBytes < PER_RW_BYTES ?
					oneFileToBeReadBytes : PER_RW_BYTES);

			BYTE *pb = new BYTE[eachTimeReadBytes];
			// ���ļ�
			DWORD dwReadBytes = 0;
			if (!ReadFile(hInfile, pb, eachTimeReadBytes, &dwReadBytes, NULL)) {
				cout << "���ļ�����" << endl;
				return EXIT_FAILURE;
			}

			// д�ļ�
			DWORD dwWriteBytes = 0;
			if (!WriteFile(hOutfile, pb, eachTimeReadBytes, &dwWriteBytes, NULL)) {
				cout << "д�ļ�����" << endl;
				return EXIT_FAILURE;
			}

			delete[] pb;

			oneFileReadBytes = eachTimeReadBytes;
			oneFileToBeReadBytes -= oneFileReadBytes;
		}

		readBytes = splitFileReadBytes;
		toBeReadBytes -= readBytes;

		CloseHandle(hOutfile);
	}

	CloseHandle(hInfile);

	return EXIT_SUCCESS;
}
