/*
	mstring : melissa string
	
	by melissa / 0xabad1dea feb 2013
	
	something I thought of and threw together while snowed in
	on a Saturday night, so take with a grain of salt
	
	a relentlessly paranoid string/buffer api that is convinced
	you are going to screw something up. Inspired by stack canaries,
	it embeds calculated values which will reveal most forms of
	accidental or malicious corruption before performing any write
	operations. It lives by the motto "death before corruption" so
	expect it to flip out and call abort().
	
	Guarantees null termination, but may be used with null-embedded
	data. (It mallocs slightly more than you ask for. It also stores
	a magic number past the end of your buffer.)
	
	You may use read-only standard functions such as strlen() on
	mstring.buf and mstring.len directly after they are initialized.
	
	still needs implementing: mstringCompare, mstringCompareSecure,
	mstringResize, lemme know if you think of anything else
	
	*** this is currently NOT LICENSED for reuse ***
	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mstring.h"



/* determines if an alleged mstring is internally consistent. */
int mstringValid(mstring* str) {
	if(str == NULL || str->buf == NULL) return 0;
	
	int* bufterminator;
	bufterminator = (int*)(((long)str->buf + str->len + 1 + 3)&~3);
	
	// the last clause can segfault if buf points to lala land,
	// but short circuit evaluation will usually prevent this.
	// I consider segfaulting to be a reasonable reaction however,
	// as it's a good indication something is *very* wrong.
	if(((long)str->canarybuf ^ (long)str->buf) == (long)0xabad1dea
	&& ((long)str->canarylen ^ (long)str->len) == (long)0xbad1dea5
	&& (*bufterminator == 0xabad1dea)) return 1;
	
	return 0; }



/* initialize or re-initialize an mstring structure with a blank buffer. */
void mstringNew(mstring* str, size_t len) {
	
	// this should be alignment safe now. Stop telling me about your weird arch.
	if((len + 2 + (sizeof(int)<<1))  < len) 
		mstringFatal(NULL, "length wraparound in mstringNew()");
	
	// reused valid mstring - clean it up for you
	if(mstringValid(str)) {
		//mstringComplain(str, "re-initializing dirty mstring");
		free(str->buf); }
	
	str->len = len;
	str->buf = malloc(len+2+(sizeof(int)<<1)); 
	if(!str->buf) mstringFatal(str, "malloc failed in mstringNew()");
	// you should be able to comment this out if it offends you,
	// but I like guaranteeing a known state
	memset(str->buf, 0, len+1);
	
	str->canarybuf = (long)0xabad1dea ^ (long)str->buf; // I'm so vain
	str->canarylen = (long)0xbad1dea5 ^ (long)str->len;
	
	int* bufterminator;
	bufterminator = (int*)(((long)str->buf + len + 1 + 3)&~3);
	*bufterminator = 0xabad1dea;
	
	return; }



/* free the buffer if valid and blank out the mstring structure */
void mstringDelete(mstring* str) {
	if(mstringValid(str)) free(str->buf); 
	else mstringComplain(str,"deleting invalid mstring");
	
	memset(str, 0, sizeof(mstring)); }



/* deep copy from src to dst - new buffer allocated */
void mstringDuplicate(mstring* src, mstring* dst) {
	if(mstringValid(src)) {
			mstringNew(dst, src->len);
			memcpy(dst->buf, src->buf, src->len);
			return; }
	
	mstringFatal(src, "bad source in mstringDuplicate()"); }
	
	

/* copy arbitrary bytes to buffer - guaranteed null terminated */
void mstringSet(mstring* str, void* src, size_t len) {
	if(len > str->len) mstringFatal(str, "mstringSet() excessive length");
	
	if(mstringValid(str)) {
		memcpy(str->buf, src, len);
		// sssh it's okay, we allocated for this
		str->buf[len] = 0; } 
	
	else mstringFatal(str, "invalid mstring in mstringSet()"); }



/* append moar data to partially filled buffer - pos is zero based */
void mstringAppend(mstring* str, void* src, size_t len, size_t pos) {
	if(!mstringValid(str)) mstringFatal(str, "invalid source in mstringAppend()");
	if((len > str->len) || (pos > str->len) || ((len+pos) > str->len)
	|| (len+pos) < len  || (len+pos) < pos)
		mstringFatal(str, "excessive length passed in mstringAppend()");
	memcpy((str->buf)+pos,src,len);
	str->buf[pos+len] = 0; }



/* prettyprint the structure */
void mstringDebug(mstring* str) {
	if(!str) { fprintf(stderr, "--------\nNULL!!!!\n--------\n"); return; }
	int* bufterminator;
	bufterminator = (int*)(((long)str->buf + str->len + 1 + 3)&~3);
	fprintf(stderr, "--------\n");
	fprintf(stderr, "address: %p\n", &str);
	fprintf(stderr, "canarybuf: %ld / 0x%lx\n", str->canarybuf, 
	(long)str->canarybuf ^ (long)str->buf);
	fprintf(stderr, "buf: %p\n", str->buf);
	fprintf(stderr, "len: %u\n",(unsigned int)str->len);
	fprintf(stderr, "canarylen: %ld / 0x%lx\n", str->canarylen, 
	(long)str->canarylen ^ (long)str->len);
	if(mstringValid(str)) fprintf(stderr, "bufterminator: 0x%x\n", *bufterminator);
	else fprintf(stderr, "bufterminator: not printed to avoid bad deref\n");
	fprintf(stderr,  "--------\n"); }



/* an honorable death in the face of memory corruption! */
void mstringFatal(mstring* str, char* message) {
	fprintf(stderr, "FATAL: %s\n", message);
	mstringDebug(str);
	abort(); }



/* an honorable whine in the face of slightly incorrect usage! */
void mstringComplain(mstring* str, char* message) {
	fprintf(stderr, "COMPLAIN: %s\n", message);
	mstringDebug(str); }