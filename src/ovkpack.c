//
//  main.c
//  ovkpack
//
//  Created by Johannes Ekberg on 2012-01-04.
//  Copyright (c) 2012 MacaroniCode Software. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __WIN32__
	#define kBR "\r\n"
#else
	#define kBR "\n"
#endif

static char oggFragment[] = {
	0x4F, 0x67, 0x67, 0x53,
	0x00, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x29, 0x98,
	0x98, 0xF7, 0x01, 0x1E,
	
	0x01, 0x76, 0x6F, 0x72,
	0x62, 0x69, 0x73, 0x00,
	0x00, 0x00, 0x00, 0x01,
	0x44, 0xAC, 0x00, 0x00,
	
	0x00, 0x00, 0x00, 0x00,
	0xB0, 0xAD, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xB8, 0x01
};

void repack(const char *inpath, const char *outpath, bool debug);

int main (int argc, const char * argv[])
{
	printf("ovkpack by uppfinnarn (@uppfinnarn on Twitter)" kBR);
	printf("Version 0.3, 8 January 2012" kBR);
	printf(kBR);
	
	if(argc < 3)
	{
		printf("Usage:" kBR);
		printf("	ovkpack <input spec file> <output file>" kBR);
		printf("		Pack the files specified in the given .ovk file" kBR);
		return 0;
	}
	
	const char *inpath = argv[1];
	const char *outpath = argv[2];
	bool debug = false;
	if(argc > 3)
	{
		if(strcmp(argv[1], "--debug") == 0) { debug = true; inpath = argv[2]; outpath = argv[3]; }
	}
	
	if(debug) printf("Debug Information Printing Activated" kBR);
	
	repack(inpath, outpath, debug);
	
    return 0;
}

void repack(const char *inpath, const char *outpath, bool debug)
{
	printf("Packing OVK %s according to %s" kBR, outpath, inpath);
	
	FILE *infile = fopen(inpath, "r");
	if(infile == NULL) { perror("Couldn't open input file:"); return;}
	FILE *outfile = fopen(outpath, "wb");
	if(outfile == NULL) { perror("Couldn't open output file:"); fclose(infile); return; }
	
	char buffer[1024];
	
	//Count the number of files in the archive
	uint32_t count = 0;
	while(fgets(buffer, sizeof(buffer), infile) != NULL) count++;
	rewind(infile);
	printf("Total Number of Files: %d" kBR, count);
	
	//Write Header
	fwrite(&count, sizeof(uint32_t), 1, outfile);
	
	uint32_t *lengths = malloc(sizeof(uint32_t)*count);
	uint32_t *locations = malloc(sizeof(uint32_t)*count);
	uint32_t *uids = malloc(sizeof(uint32_t)*count);
	void **data = malloc(sizeof(void*)*count);
	
	uint32_t i = 0;
	uint32_t pos = sizeof(uint32_t) + (sizeof(uint32_t)*3*count) + sizeof(oggFragment);
	printf("Header Size: %lu + (%lu*3*%d) + %lu = %d\n", sizeof(uint32_t), sizeof(uint32_t), count, sizeof(oggFragment), pos);
	while(fgets(buffer, sizeof(buffer), infile) != NULL)
	{
		//--> Get the Filename
		buffer[1023] = '\0';					//Prevent Buffer Overflows by truncating the path if necessary
		buffer[strlen(buffer)-1] = '\0';		//Remove the trailing \n
		if(buffer[strlen(buffer)-1] == '\r')
			buffer[strlen(buffer)-1] = '\0';	//Remove trailing \r if there is one
		
		if(debug) printf("Scanning File %04d: %s...", i+1, buffer);
		else printf("\rScanning File %04d/%04d", i+1, count);
		
		//Read Data
		FILE *file = fopen(buffer, "rb");
		if(file == NULL)
		{
			perror("Couldn't open file");
			printf(kBR);
			break;
		}
		
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);
		void *fdata = malloc(size);
		rewind(file);
		fread(fdata, size, 1, file);
		
		data[i] = malloc(size);
		memcpy(data[i], fdata, size);
		uint32_t usize = (uint32_t)size;
		lengths[i] = usize;
		locations[i] = pos;
		uids[i] = i+1;
		
		fclose(file);
		free(fdata);
		
		if(debug)
		{
			printf(" Length: %08d", lengths[i]);
			printf(" Location: %08d", locations[i]);
			printf(kBR);
		}
		
		i++;
		pos += size;
	}
	
	printf(kBR);
	printf("Writing File Header...");
	
	rewind(outfile);
	fwrite(&count, sizeof(uint32_t), 1, outfile);
	printf(" File Count: %08d (%08x)" kBR, count, count);
	
	int j = 0;
	for(j = 0; j < i; j++)
	{
		uint32_t length = lengths[j];
		uint32_t location = locations[j];
		uint32_t uid = uids[j];
		
		fwrite(&length, sizeof(uint32_t), 1, outfile);
		fwrite(&location, sizeof(uint32_t), 1, outfile);
		fwrite(&uid, sizeof(uint32_t), 1, outfile);
		
		if(debug) printf("Header Field %04d Written:		%08d %08d %08d		[%08x %08x %08x]" kBR,
						 j, length, location, uid, length, location, uid);
		else printf("\rWritten Header Field %4d/%d", j+1, count);
	}
	if(!debug) printf(kBR);
	
	fwrite(oggFragment, sizeof(oggFragment), 1, outfile);
	if(debug) printf(kBR);
	printf("OggS Fragment Written" kBR);
	if(debug) printf(kBR);
	
	int t = 0;
	for(t = 0; t < i; t++)
	{
		void *fdata = data[t];
		fwrite(fdata, lengths[t], 1, outfile);
		if(debug) printf("Written file %4d, length=%04d, pos=%04d, ftell=%04ld (should be %04u)" kBR,
						 t+1, lengths[t], locations[t], ftell(outfile), locations[t]+lengths[t]);
		else printf("\rWritten file %4d/%d", t+1, count);
		free(fdata);
	}
	if(!debug) printf(kBR);
	
	free(lengths);
	free(locations);
	free(uids);
	free(data);
	
	fclose(infile);
	fclose(outfile);
}

