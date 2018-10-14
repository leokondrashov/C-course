/*! \file Shakespeare.c
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

#define INPUT_FILE "ShakespeareSonnets.txt"
#define OUTPUT_FILE_1 "ShakespeareSonnetsSorted.txt"
#define OUTPUT_FILE_2 "ShakespeareSonnetsSortedFromEnd.txt"
#define OUTPUT_FILE_3 "ShakespeareSonnetsOriginal.txt"

void processTextFromFile(const char *ifile, const char *ofile1, 
		const char *ofile2, const char *ofile3);
char **readTextFromFile(const char *file);
int sizeofFile(const char *file);
int countLines(char **lines);
void writeToFile(const char *file, const char **lines);
int countCharsInStr(const char c, const char *str);
int getLine(char buff[], int maxSize);
void sort(char *lines[], int left, int right, int (*comp)(const char *, const char *));
int strComp(const char *first, const char *second);
int strCompFromEnd(const char *first, const char *second);
void swap(char *lines[], int i, int j);

int main() {
	processTextFromFile(INPUT_FILE, OUTPUT_FILE_1, OUTPUT_FILE_2, OUTPUT_FILE_3);
}

/*!
* process text (sort text from ifile forward and write it to ofile1, sort backward and write it to ofile2, original text is written to ofile3)
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

/*!
*file reader
*
*\param [in] file name of input file
*
*\return pointer to the array of lines with NULL on the end
*
*\note lines[0] points to full buffer containing whole file
*\note memory free required ('lines[0]' first and 'lines' afterwards)
*/

char **readTextFromFile(const char *file) {
	assert(file != NULL);
	
	int buffSize = sizeofFile(file);
	if (buffSize < 0)
		return NULL;
	char *buff = (char *)calloc(buffSize + 1, sizeof(char));
	
	FILE *input = fopen(file, "rb");
	fread(buff, sizeof(char), buffSize, input);
	fclose(input);
	
	int nLines = countCharsInStr('\n', buff) + 2; //because of ending 0 and possibly absent \n after last line
	char **lines = (char **)calloc(nLines, sizeof(*lines));
	int i = 0;
	lines[0] = buff;
	for (char *it = strchr(buff, '\n'); it; it = strchr(it + 1, '\n')) {
		lines[++i] = it + 1;
		*it = '\0';
	}
	lines[i] = NULL;
	
	return lines;
}

/*
*routine returning size of file
*/

int sizeofFile(const char *file) {
	assert(file != NULL);
	
	struct stat st = {};
	if (stat(file, &st) == -1)
		return -1;
	return st.st_size;
}

/*
*counts lines in array of strings
*/

int countLines(char **lines) {
	assert(lines != NULL);
	
	int i = 0;
	while(lines[i])
		i++;
	return i;
}

/*
*writes lines to file separated with \n
*/

void writeToFile(const char *file, const char **lines) {
	assert(lines != NULL);
	assert(file != NULL);
	
	FILE *out = fopen(file, "wb");
	do {
		fwrite(*lines, sizeof(char), strlen(*lines), out);
		fwrite("\n", sizeof(char), 1, out);
	} while (*(++lines));
}

/*
*counting char c in str
*/

int countCharsInStr(const char c, const char *str) {
	int i = 0;
	for (char *it = strchr(str, c); it; i++, it = strchr(it + 1, c));
	return i;
}

/*!
*Line input
*
*\param [out] buff string which will contain input line
*\param [in] maxSize size of given buffer
*
*\return length of input line
*/

int getLine(char buff[], int maxSize) {
	assert(buff != NULL);
	
	int i = 0, c = 0;
	
	while (i < maxSize - 1 && (c = getchar()) != EOF && c != '\n') {
		if (c == ' ' && i == 0)
			continue;
		buff[i++] = c;
	}
	
	buff[i] = '\0';
	return i;
}

/*!
*Sort of strings
*
*\param [in] lines array of strings to sort
*\param [in] left left boarder of sorting part of array
*\param [in] right right boarder of sorting part of array
*\param [in] comp pointer to comparator for strings
*
*\note uses quick sort
*/

void sort(char *lines[], int left, int right, int (*comp)(const char *, const char *)) {
	assert(lines != NULL);	
	
	int last;
	
	if (left >= right)
		return;
	swap(lines, left, (left + right) / 2);
	last = left;
	for (int i = left + 1; i <= right; i++)
		if ((*comp)(lines[i], lines[left]) < 0)
			swap(lines, i, ++last);
	swap(lines, left, last);
	sort(lines, left, last - 1, comp);
	sort(lines, last + 1, right, comp);
}

/*!
*Swapper
*
*\param [in] lines array of strings two of them needed to swap
*\param [in] i index of first element to swap
*\param [in] j index of second element to swap
*/

void swap(char *lines[], int i, int j) {
	assert(lines != NULL);
	
	char *t;
	
	t = lines[i];
	lines[i] = lines[j];
	lines[j] = t;
}

/*!
*Comparator for strings
*
*\param [in] first first string to compare
*\param [in] second second string to compare
*
*\return <0 if first < second; >0 if first > second and 0 if first == second
*
*\note compare string from the beginning ignoring punctuation
*/

int strComp(const char *first, const char *second) {
	assert(first != NULL);
	assert(second != NULL);
	
	int i = 0, j = 0;
	
	while (!isalpha(first[i])) //skipping non-alphabetical symbols
		i++;
	while (!isalpha(second[j]))
		j++;
	
	while (tolower(first[i]) == tolower(second[j])) {
		i++;
		j++;
		if (ispunct(first[i]))
			while (ispunct(first[i]))
				i++;
		if (ispunct(second[j]))
			while (ispunct(second[j]))
				j++;
		if (first[i] == '\0' && second[j] == '\0')
			return 0;
	}
	
	return tolower(first[i]) - tolower(second[j]);
}

/*!
*Comparator for strings from end
*
*\param [in] first first string to compare
*\param [in] second second string to compare
*
*\return <0 if first < second; >0 if first > second and 0 if first == second
*
*\note compare string from the ending ignoring punctuation
*/

int strCompFromEnd(const char *first, const char *second) {
	assert(first != NULL);
	assert(second != NULL);
	
	int i = strlen(first), j = strlen(second);
	
	while (!isalpha(first[i])) //skipping non-alphabetical symbols
		i--;
	while (!isalpha(second[j]))
		j--;
	
	while (tolower(first[i]) == tolower(second[j])) {
		i--;
		j--;
		if (ispunct(first[i]))
			while (ispunct(first[i]))
				i--;
		if (ispunct(second[j]))
			while (ispunct(second[j]))
				j--;
		if (first[i] == '\0' && second[j] == '\0')
			return 0;
	}
	
	return tolower(first[i]) - tolower(second[j]);
}
