/*! \file Shakespeare.c
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#define MAXSIZE 100
#define NUMBER_OF_LINES 2156 //number of lines in ShakespeareSonnets.txt

int getLine(char *line, int maxSize);
void sort(char *lines[], int left, int right, int (*comp)(char *, char *));
int strComp(char *first, char *second);
int strCompFromEnd(char *first, char *second);
void swap(char *lines[], int i, int j);

int main() {
	int nlines = 0, len = 0;
	char *lines[NUMBER_OF_LINES] = {};
	char buff[MAXSIZE] = "";
	char *p;
	
	while ((len = getLine(buff, MAXSIZE)) != 0 && nlines < NUMBER_OF_LINES) {
		p = (char *) calloc(len + 1, sizeof(char));
		strcpy(p, buff);
		lines[nlines++] = p;
	}

	sort(lines, 0, nlines - 1, strCompFromEnd);

	for (int i = 0; i < nlines; i++)
		printf("%s\n", lines[i]);
	
	for (int i = 0; i < nlines; i++)
		free(lines[i]);
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

void sort(char *lines[], int left, int right, int (*comp)(char *, char *)) {
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

int strComp(char *first, char *second) {
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

int strCompFromEnd(char *first, char *second) {
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
