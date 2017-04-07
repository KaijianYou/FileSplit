// 此处添加项目说明

#include <iostream>
#include <string>
#include <ctime>
#include <locale>

#include <windows.h>
#include <direct.h>
#include <tchar.h>

#define PER_RW_BYTES		0x00200000		// 每次读写字节数：2 MB

using std::string;
using std::cout;
using std::endl;

static int outfileSeqNum = 0;     // 文件序号

struct time {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
};

// 获取系统当前时间
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

// 创建新文件
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
		cout << "创建文件失败，错误代码：" << GetLastError() << endl;
		return INVALID_HANDLE_VALUE;
	}

	delete[] wOutfileName;
	return hFile;
}

// 文件指针定位
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

// 处理文件路径
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

// 创建文件夹并切换工作目录到该文件夹下
bool createDir(const string& infileDir, const string& infileName, char dirName[])
{
	struct time t;
	getNowTime(&t);

	_chdir(infileDir.c_str());

	// 创建文件夹
	sprintf(dirName, ".//%s_%04d-%02d-%02d_%02d-%02d-%02d", infileName.c_str(),
			t.year, t.month, t.day, t.hour, t.minute, t.second);
	if (-1 == _mkdir(dirName)) {
		cout << "创建文件夹 " << dirName << " 失败！" << endl;
		return false;
	}

	return true;
}

// 获取程序名
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
		cout << "参数错误！" << endl;
		return EXIT_FAILURE;
	}

	string infilePath(argv[1]);

	int infilePathSize = static_cast<int>(infilePath.length() + 1);
	wchar_t *wInfilePath = new wchar_t[infilePathSize];
	MultiByteToWideChar(CP_ACP, 0, infilePath.c_str(), -1, wInfilePath, infilePathSize);

	// 创建文件对象（打开文件）
	HANDLE hInfile = CreateFile(wInfilePath, GENERIC_READ, 0, 
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hInfile == INVALID_HANDLE_VALUE) {
		delete[] wInfilePath;
		cout << "打开文件失败，错误代码：" << GetLastError() << endl;
		return EXIT_FAILURE;
	}
	delete[] wInfilePath;

	// 获取文件大小
	DWORD dwFileSizeHigh;
	__int64 qwFileSize = GetFileSize(hInfile, &dwFileSizeHigh);
	qwFileSize |= (((__int64)dwFileSizeHigh) << 32);
	cout << "文件大小：" << qwFileSize << " 字节" << endl;
	cout << "分文件大小：" << nbytes << " 字节" << endl;

	if ((__int64)nbytes > qwFileSize) {
		cout << "分文件大小大于文件大小,无法切分!" << endl;
		return EXIT_FAILURE;
	}

	// 计算文件切分数量
	__int64 fileNum = (qwFileSize % nbytes == 0) ? qwFileSize / nbytes : qwFileSize / nbytes + 1;
	cout << "将切分成 " << fileNum << " 个文件" << endl;

	string infileFullName;
	string infileDir;
	string infileName;

	// 处理文件路径
	handleFilePath(infilePath, infileDir, infileFullName, infileName);

	// 创建文件夹和切换工作目录
	char dirName[256];
	if (!createDir(infileDir, infileName, dirName)) {
		return EXIT_FAILURE;
	}
	_chdir(dirName);

	cout << "正在切分文件，请稍候......" << endl;

	// 定位文件读写指针到文件头
	if (-1 == fileSeek(hInfile, 0, FILE_BEGIN)) {
		cout << "文件指针定位出错！" << endl;
		return EXIT_FAILURE;
	}

	__int64 toBeReadBytes = qwFileSize;
	DWORD readBytes = 0;
	DWORD splitFileReadBytes = 0;

	while (toBeReadBytes > 0) {
		// 创建文件
		HANDLE hOutfile;
		if (INVALID_HANDLE_VALUE == (hOutfile = createFile(infileName))) {
			cout << "创建文件失败，错误代码：" << GetLastError() << endl;
			return EXIT_FAILURE;
		}
		printf("正在处理第 %d 个文件...\r", outfileSeqNum);
		splitFileReadBytes = (DWORD)(toBeReadBytes < nbytes ? toBeReadBytes : nbytes);

		DWORD oneFileReadBytes = 0;
		DWORD eachTimeReadBytes = 0;
		for (DWORD oneFileToBeReadBytes = splitFileReadBytes; oneFileToBeReadBytes > 0; ) {
			eachTimeReadBytes = (DWORD)(oneFileToBeReadBytes < PER_RW_BYTES ?
					oneFileToBeReadBytes : PER_RW_BYTES);

			BYTE *pb = new BYTE[eachTimeReadBytes];
			// 读文件
			DWORD dwReadBytes = 0;
			if (!ReadFile(hInfile, pb, eachTimeReadBytes, &dwReadBytes, NULL)) {
				cout << "读文件出错！" << endl;
				return EXIT_FAILURE;
			}

			// 写文件
			DWORD dwWriteBytes = 0;
			if (!WriteFile(hOutfile, pb, eachTimeReadBytes, &dwWriteBytes, NULL)) {
				cout << "写文件出错！" << endl;
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
