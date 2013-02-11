#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mstring.h"

//void testmstring() {
int main() {	
	
	mstring M;
	
	// expected output: none
	mstringNew(&M,126);
	
	// expected output: dirty
	mstringNew(&M,17);
	
	// expected output: invalid 
	char invalid[sizeof(mstring)];
	memset(invalid, 24, sizeof(mstring));
	//mstringDuplicate((mstring*)&invalid, &M); // fatal
	//mstringSet((mstring*)&invalid, "abcd",4); // fatal
	mstringDelete((mstring*)&invalid);
	
	
	mstring Z;

	// expected output: none
	mstringDelete(&M);
	
	mstringNew(&M,12);
	mstringSet(&M,"abcd",3);
	// expected output: abc
	printf("Contents of M: %s\n", M.buf);
	
	//mstringSet(&M,"abcdefghijklmnop",14); // fatal
	
	// expected output: none
	mstringDuplicate(&M, &Z);
	// expected output: abc
	printf("Contents of Z: %s\n", Z.buf);
	
	// expected output: none
	mstringDelete(&M);
	mstringNew(&M, 0);
	//mstringSet(&M, "1",1); // fatal
	
	mstringDelete(&M);
	
	mstringNew(&M, 12);
	
	mstringSet(&M, "abcd", strlen("abcd"));
	mstringAppend(&M, "efghij", strlen("efgh"), strlen(M.buf));
	//mstringAppend(&M, "ijklmnop", 8, strlen(M.buf)); // fatal
	// expected output: abcdefgh
	printf("Contents of M: %s\n", M.buf);
	
	mstringDebug(&M);
	
	
	mstringDelete(&M);
	printf("Allocating will fail...");
	mstringNew(&M,0xFFFFFFFFFFFFFFFF-32); // fatal
	
	
	}