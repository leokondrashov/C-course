/*! \file Shakespeare.c
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <textProcessor.h>

#define INPUT_FILE "ShakespeareSonnets.txt"
#define OUTPUT_FILE_1 "ShakespeareSonnetsSorted.txt"
#define OUTPUT_FILE_2 "ShakespeareSonnetsSortedFromEnd.txt"
#define OUTPUT_FILE_3 "ShakespeareSonnetsOriginal.txt"

void processTextFromFile(const char *ifile, const char *ofile1, 
		const char *ofile2, const char *ofile3);

int main() {
	processTextFromFile(INPUT_FILE, OUTPUT_FILE_1, OUTPUT_FILE_2, OUTPUT_FILE_3);
}

/*!
* \brief process text (sort text from ifile forward and write it to ofile1, sort backward and write it to ofile2, original text is written to ofile3)
*/

void processTextFromFile(const char *ifile, const char *ofile1, 
			const char *ofile2, const char *ofile3) {
	assert(ifile != NULL);
	assert(ofile1 != NULL);
	assert(ofile2 != NULL);
	assert(ofile3 != NULL);
	
	char **origLines = readTextFromFile(ifile);
	if (origLines == NULL) {
		printf("Some troubles with reading file %s", ifile);
		exit(1);
	}
	int nLines = countLines(origLines);	
	
	char **sortedLines = (char **)calloc(nLines + 1, sizeof(*sortedLines));
	for (int i = 0; origLines[i]; i++)
		sortedLines[i] = origLines[i];
	sortedLines[nLines] = NULL;
	
	sort(sortedLines, 0, nLines - 1, strComp);
	writeToFile(ofile1, sortedLines);
	
	sort(sortedLines, 0, nLines - 1, strCompFromEnd);
	writeToFile(ofile2, sortedLines);
	
	writeToFile(ofile3, origLines);
	
	free(origLines[0]);
	free(origLines);
	free(sortedLines);
//	int fileSize = sizeofFile(ifile);
}
