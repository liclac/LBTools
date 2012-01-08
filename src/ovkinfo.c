//
//  ovkinfo.c
//  LBTools
//
//  Created by Johannes Ekberg on 2012-01-08.
//  Copyright (c) 2012 MacaroniCode Software. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#pragma pack(push, 1)
typedef struct
{
	uint32_t length;
	uint32_t location;
	uint32_t fileID;
} ovk_header_field_t;
#pragma pack(pop)

void printheader(ovk_header_field_t header, int number);

int main (int argc, const char * argv[])
{
	printf("ovkinfo - Little Busters! OVK Archive Examination Tool\n");
	printf("by uppfinnarn (@uppfinnarn on Twitter)\n");
	printf("Version 0.1, 8 Jan 2012\n");
	printf("\n");
	
	if(argc < 1)
	{
		printf("Usage: ovkinfo <archive>\n");
		return 0;
	}
	
	int i;
	for(i = 1; i < argc; i++)
	{
		printf("Scanning file %s... ", argv[i]);
		
		FILE *file = fopen(argv[i], "rb");
		if(file == NULL)
		{
			perror("Couldn't Open");
			return 0;
		}
		
		uint32_t count;
		fread(&count, sizeof(uint32_t), 1, file);
		printf("%d Files\n", count);
		
		ovk_header_field_t *headers = malloc(sizeof(ovk_header_field_t)*count);
		fread(headers, sizeof(ovk_header_field_t), count, file);
		
		int j;
		for(j = 0; j < count; j++)
		{
			printheader(headers[j], j+1);
		}
		
		free(headers);
		fclose(file);
	}
}

void printheader(ovk_header_field_t header, int number)
{
	printf("File %4d: %8d\n", number, header.fileID);
	
}
