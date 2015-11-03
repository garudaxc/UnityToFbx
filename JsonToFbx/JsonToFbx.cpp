// JsonToFbx.cpp : 定义控制台应用程序的入口点。
//


#include "fbxsdk.h"
#include "rapidjson/document.h"
#include <cstdio>
#include "stdio.h"
#include <string>
#include <tchar.h>


using namespace rapidjson;
using namespace std;


char* srcBuff = 0;
int sizeSrc = 0;

int EnsureBuffer(char** buf, int buffSize, int curSize)
{
	if (curSize <= buffSize) {
		return buffSize;
	}

	free(*buf);
	*buf = (char*)malloc(curSize);
	return curSize;
}


bool Export(const char* data, const char* outFile);


int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		printf("need input file (.json)\n");
		return 0;
	}

	printf("input file: %s\n", argv[1]);

	char outFile[256];
	strcpy(outFile, argv[1]);
	char* ext = strstr(outFile, ".json");
	if (ext == NULL)
	{
		printf("input file should be json file\n");
		return 0;
	}

	strcpy(ext, ".fbx");
	printf("output file: %s\n", outFile);
	

	FILE* pf = fopen(argv[1], "rb");
	if (pf == NULL)
	{
		printf("can not open file %s\n", argv[1]);
		return 0;
	}

	fseek(pf, 0, SEEK_END);
	uint32_t size = ftell(pf);
	fseek(pf, 0, SEEK_SET);

	sizeSrc = EnsureBuffer(&srcBuff, sizeSrc, size + 1);

	fread((void*)srcBuff, size, 1, pf);
	fclose(pf);

	srcBuff[size] = '\0';

	bool result = Export(srcBuff, outFile);	

	free(srcBuff);

	return 0;
}
