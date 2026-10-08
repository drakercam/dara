#include "include/io.h"

#include <stdlib.h>
#include <stdio.h>

char* fileGetContents(const char* filePath) {
	char* buffer = 0;
	long length;
	
	FILE* f = fopen(filePath, "rb");
	
	if (f) {
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		
		buffer = calloc(length + 1, sizeof(char));
		
		if (buffer) {
			fread(buffer, 1, length, f);
		}
		
		buffer[length] = '\0';
		
		fclose(f);
		return buffer;
	}
	
	printf("Error reading file: %s\n", filePath);
	exit(2);
}
