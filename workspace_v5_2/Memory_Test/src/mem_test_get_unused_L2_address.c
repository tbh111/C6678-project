/*
parser map file to determine the start address of unused L2 for test
Author: Brighton Feng
Created on 2010-11-18
*/

#define L2_TEST_DEFAULT_START_ARRD 		(0x840000)

#include <stdio.h>
#include <string.h>

unsigned int get_unused_L2_address()
{
	int i;
	unsigned int origin, length, used, unused;
	FILE *fpMapFile;
	char string[160];

	if((fpMapFile=fopen("Memory_Test.map","rt"))==NULL)
	{
		puts("!!!Can not open MAP File: Mem_Test.map!!!");
		return L2_TEST_DEFAULT_START_ARRD;
	}

	/*search for the information about memory usage*/
	for(i= 0; i< 25; i++)
	{
		fgets(string, 160, fpMapFile);
		if(strlen(string)<50)
			continue; 
		sscanf(string, "%*s %x %x %x %x", &origin, &length, &used, &unused);

		/*valid memory usage information*/
		if((0x800000<= origin)&&(origin<= 0xa00000) && (length==(used+unused))&& unused>64*1024)
		{
			/*round up to be multiple of 64 bytes*/
			used= (used+63)/64*64;
			fclose(fpMapFile);
			return (origin+used);
		}
	}		
	fclose(fpMapFile);
  	return L2_TEST_DEFAULT_START_ARRD;
}


