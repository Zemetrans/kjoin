#include <stdio.h>
#include <stdlib.h>
#include <keapi/keapi.h>
#include <time.h>

int main()
{
	KEApiLibInitialize();
	KEAPI_BOARD_INFO BoardInfo;
	KEApiGetBoardInfo(&BoardInfo);
	printf("Board Name: %s\n", BoardInfo.boardName);
	printf("Manufacturing Date: %s\n", ctime(&BoardInfo.manufacturingDate));
	KEApiLibUnInitialize();
	return 0;
}