//
//  main.c
//  lbunpak
//
//  Created by Johannes Ekberg on 2011-12-29.
//  Copyright (c) 2011 MacaroniCode Software. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define kOutSuffix "_"

void extract(int i, int offset, int lba, char *extension, FILE *source, char *destination);

int main(int argc, const char * argv[])
{
	if(argc < 3)	//We can't work without indata
	{
		printf("Usage: lbunpak <PAK file> <offset map>\n");
		return 0;
	}
	
	//Figure out the Destination Base
	char *destination = malloc(strlen(argv[1]) + strlen(kOutSuffix) + 1);
	strcpy(destination, argv[1]);
	strcat(destination, kOutSuffix);
	
	//Print Diagnostic Data
	printf("Extracting: %s\n", argv[1]);
	printf("Offset Map: %s\n", argv[2]);
	printf("        To: %s*\n", destination);
	
	
	//Load Data
	FILE *source = fopen(argv[1], "rb");		//Open the Source Data File for [r]eading in [b]inary mode
	FILE *offsetsFile = fopen(argv[2], "r");	//Open the Offset Map for [r]eading
	
	if(source == NULL) { perror("Can't open Source File for reading"); goto cleanup; }
	if(offsetsFile == NULL) { perror("Can't open Offset Map for reading"); goto cleanup; }
	
	//Read the Offset Map and extract data from it
	int readings = 1;
	int index = 0;
	while(feof(offsetsFile) == 0)	//Read until we run out of data to run
	{
		int offset, lba;
		char extension[8];
		int fr = fscanf(offsetsFile, "%x %x %7s%*[^\n]", &offset, &lba, extension);
		if(fr == 3)
		{
			printf("Line %04d > ", readings);
			extract(index, offset, lba, extension, source, destination);
			printf("\n");
			index++;
		}
		else
		{
			char buffer[512] = {0};
			fgets(buffer, sizeof(buffer), offsetsFile);
		}
		readings++;
	}
	
cleanup:
	free(destination);
	
	fclose(offsetsFile);
	fclose(source);
	return 0;
}

void extract(int i, int offset, int lba, char *extension, FILE *source, char *destination)
{
	//Each file is labeled as "0001.ext", "0002.ext", etc.
	char *outpath = malloc(strlen(destination)			//Destination Base
						   + 4							//Allow a maximum of 4 digits in the output filename (up to 9999 files)
						   + 1							//The Dot before the extension
						   + strlen(extension)			//The Extension
						   + 1);						//The trailing NULL character
	sprintf(outpath, "%s%04d.%s", destination, i, extension); //Write the data to the outpath buffer
	printf("Writing %s... ", outpath);
	
	//Open the Output File
	FILE *outfile = fopen(outpath, "wb");	//Open for [w]riting in [b]inary mode
	free(outpath);							//Delete the now unneeded Output Path Buffer
	if(outfile == NULL)
	{
		perror("Failed");
		return;
	}
	
	//Get Data Length
	int32_t length;						//Placeholder for the Data Length
	fseek(source, offset, SEEK_SET);	//Seek to the start of the region to extract
	fseek(source, 4, SEEK_CUR);			//Seek past the four padding 00-bytes
	fread(&length, 4, 1, source);		//Scan the following 4 bytes into Length
	length += 8;						//Append 8 to the length to account for the header
	fseek(source, offset, SEEK_SET);	//Seek back to the region start
	printf("(%8d bytes)", length);
	
	//Copy Data
	void *fbuffer = malloc(length);			//Create a Buffer to hold temporary data
	fread(fbuffer, 1, length, source);		//Read the region to be extracted into the buffer
	fwrite(fbuffer, 1, length, outfile);	//Write the buffer to the output file
	free(fbuffer);							//Free the Buffer
	
	fclose(outfile);					//Close the Out File
}

