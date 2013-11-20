#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mstring.h"

//void testmstring() {
int main()
{

    mstring M;

    // expected output: none
    mstringNew(&M, 126);

    // expected output: dirty
    mstringNew(&M, 17);

    // expected output: invalid 
    char invalid[sizeof(mstring)];
    memset(invalid, 24, sizeof(mstring));
    //mstringDuplicate((mstring*)&invalid, &M); // fatal
    //mstringSet((mstring*)&invalid, "abcd",4); // fatal
    mstringDelete((mstring *) & invalid);

    mstring Z;

    // expected output: none
    mstringDelete(&M);

    mstringNew(&M, 12);
    mstringSet(&M, "abcd", 3);
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

    mstringSet(&M, "abcd", 0);
    printf("-+-+-+-+-+ %d\n", (int) strlen(M.buf));
    mstringAppend(&M, "efghij", strlen("efgh"), strlen(M.buf));
    //mstringAppend(&M, "ijklmnop", 8, strlen(M.buf)); // fatal
    // expected output: abcdefgh
    printf("Contents of M: %s\n", M.buf);

    mstringDebug(&M);

    mstringDelete(&M);
    //printf("Allocating will fail...");
    //mstringNew(&M,0xFFFFFFFFFFFFFFFF-32); // fatal

    mstringNew(&M, 16);
    mstringNew(&Z, 16);
    mstringSet(&M, "abcdefg", 7);
    mstringSet(&Z, "hijklmn", 7);

    if (mstringCompare(&M, &Z) < 0)
        printf("~~ mstringCompare 1 of 3: working\n");
    mstringSet(&M, "opwrstu", 7);
    if (mstringCompare(&M, &Z) > 0)
        printf("~~ mstringCompare 2 of 3: working\n");
    mstringSet(&M, "hijklmn", 7);
    if (mstringCompare(&M, &Z) == 0)
        printf("~~ mstringCompare 3 of 3: working\n");

    if (mstringLength(&M) == M.len)
        printf("~~ mstringLength: working\n");

    mstringNew(&M, 32);

    mstringPrintf(&M, "%s %x", "Here we come a wassailing", 0xbadc0d35);
    printf("Printf test: %s\n", M.buf);
    mstringPrintf(&M, "%s", "short string");
    printf("Printf test: %s\n", M.buf);
    mstringPrintf(&M, "%s", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    printf("Printf test: %s\n", M.buf);

    mstringClear(&M);
    if (M.buf[0] == 0 && M.buf[31] == 0)
        printf("~~ mstringClear: probably working\n");

    mstringNew(&M, 16);
    mstringSet(&M, "0123456789ABCDE", 0);
    printf("Auto-strlen: %s\n", M.buf);
    //mstringSet(&M, "0123456789ABCDEF", 0); // fatal
    //printf("Auto-strlen: %s\n", M.buf);

    mstringGrow(&M, 32);
    mstringSet(&M, "0123456789ABCDEFGHIJKLMNOP", 0);
    printf("Growing: %s - %d\n", M.buf, (int) M.len);

    //mstringGrow(&M, 8); // fatal

    printf("Sizeof ulong: %ld\n", sizeof(unsigned long));

    mstringDebug(&M);
    mstringHexdump(&M);

    printf("Awkward size:\n");
    mstringDelete(&M);
    mstringNew(&M, 17);
    mstringSet(&M, "abcdefghijklmnop", 0);
    mstringDebug(&M);
    mstringHexdump(&M);

}
