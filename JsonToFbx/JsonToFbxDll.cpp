// JsonToFbxDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include <stdio.h>
#include <tchar.h>


#define EXPORT_API _declspec(dllexport)

bool Export(const char* data, const char* outFile);


extern "C"
{
	
	EXPORT_API int ExportToFbx(const char* data, const char* outFile)
	{

		bool bRes = Export(data, outFile);

		return bRes ? 0 : -1;

	}

}