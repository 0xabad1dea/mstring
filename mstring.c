/*
	mstring : melissa's string
	by melissa / 0xabad1dea
	initial commit:	feb 2013
	last updated:	nov 2013
	
	A relentlessly paranoid string buffer API for the pure joy of
	implementing one. It's basically just stack canaries except
	built into your malloced strings regardless of compiler. If you 
	think that you have no use for such a thing, well, you're probably 
	right; this is mostly practice for me *writing* C rather than just 
	criticizing other people's C all day.
	
	You may use read-only standard functions such as strlen() on
	mstring.buf and mstring.len directly after they are initialized, but
	my goal is to wrap all of the good ones :)
	
	Haters { gonna hate } my compact brace style

	--TODO--
	mstringCompareSecure (constant time)
	mstringShrink
	mstringMemCompare and mstringMemCompareSecure
	more as I find a need...
		
	@mdowd totally spent at least two minutes looking at this so
	it is the safest code in the world. Or maybe not. ~peril~ 

	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include "mstring.h"

#define ulong unsigned long
#define uint unsigned int


// TODO: make these random to better resist malicious attack 
// (it's okay-ish as is because the pointers are factored in)
// yes, these numbers are specifically chosen out of vanity
ulong bufferkey = 0xabad1dea;
ulong lengthkey = 0xbad1dea5;

/* determines if an alleged mstring is internally consistent. */
int mstringValid(const mstring* str) {
	if(str == NULL || str->buf == NULL) return 0;
	
	ulong* bufterminator;
	// my court wizards tell me this will maintain correct alignment
	// this formula has caused me a lot of grief - should be completely
	// 32/64 bit safe now, I think?!
	bufterminator = (ulong*)(((uintptr_t)str->buf + str->len + 1 + 8)&~7);
	
	// the last clause can segfault if buf points to lala land,
	// but short circuit evaluation will usually prevent this.
	// there isn't really anything I can do about this.
	if((str->canarybuf ^ (uintptr_t)str->buf) == bufferkey
	&& (str->canarylen ^ (uintptr_t)str->len) == lengthkey
	&& (*bufterminator == (bufferkey ^ (uintptr_t)str))) return 1;
	
	return 0; }



/* initialize or re-initialize an mstring structure  */
void mstringNew(mstring* str, size_t len) {
	
	if(str == NULL) mstringFatal(str, "null pointer passed to mstringNew()");
	
	// this should be alignment safe now.
	if((len + 2 + (8<<1))  < len) 
		mstringFatal(str, "length wraparound in mstringNew()");
	
	// reused valid mstring - clean it up for you
	if(mstringValid(str)) {
		//mstringComplain(str, "re-initializing dirty mstring");
		mstringDelete(str); }
	
	str->len = len;
	// (extra space for extra null terminator + buffer terminator)
	str->buf = (char*)malloc(len+2+(8<<1)); // I know, I have bare literals!
	if(!str->buf) mstringFatal(str, "malloc failed in mstringNew()");
	str->buf[len] = 0; // personal preference to make sure any buf
	// can always be read out as a c string
	
	str->canarybuf = bufferkey ^ (uintptr_t)str->buf;
	str->canarylen = lengthkey ^ (uintptr_t)str->len;
	
	ulong* bufterminator;
	bufterminator = (ulong*)(((uintptr_t)str->buf + len + 1 + 8)&~7);
	*bufterminator = bufferkey ^ (uintptr_t)str;
	
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
	
	

/* copy arbitrary bytes to buffer - len 0 to take strlen of src + null terminator */
void mstringSet(mstring* str, void* src, size_t len) {
	if(len == 0) // I considered checking for wraparound here, 
		len = strlen((char*)src) + 1; // but it's actually pretty pointless :)
	if(len > str->len) mstringFatal(str, "excessive length in mstringSet()");
	
	
	if(mstringValid(str)) {  memcpy(str->buf, src, len); 
	str->buf[len] = 0; } // safe: may go into the reserved spot: won't truncate
	
	else mstringFatal(str, "invalid mstring in mstringSet()"); }



/* append more data to partially filled buffer - pos is zero based */
void mstringAppend(mstring* str, void* src, size_t len, size_t pos) {
	if(!mstringValid(str)) mstringFatal(str, "invalid source in mstringAppend()");
	if((len > str->len) || (pos > str->len) || ((len+pos) > str->len)
	|| (len+pos) < len  || (len+pos) < pos) // o-o-o-overkilllll
		mstringFatal(str, "excessive length in mstringAppend()");
	memcpy((str->buf)+pos,src,len);
	str->buf[pos+len] = 0; }


/* compare two mstrings - strcmp wrapper */
int mstringCompare(const mstring* a, const mstring* b) {
	if(!mstringValid(a)) 
		mstringFatal(a, "invalid 'a' to mstringCompare()");
	if(!mstringValid(b)) 
		mstringFatal(b, "invalid 'b' to mstringCompare()");
	
	return strcmp(a->buf, b->buf); }



/* trivial wrapper of the length property, if you prefer */
size_t mstringLength(const mstring* str) {
	if(!mstringValid(str)) 
		mstringFatal(str, "invalid mstring in mstringLength()");
	return str->len; }



/* snprintf into buffer: will not overflow: null terminates within bounds */
int mstringPrintf(mstring* str, const char* format, ...) {
	va_list args;
	int result;
	if(!mstringValid(str)) 
		mstringFatal(str, "invalid mstring in mstringPrintf()");
	if(!format)
		mstringFatal(str, "null format in mstringPrintf()");
	
	va_start(args,format);
	result = vsnprintf(str->buf, str->len, format, args);
	// !@#$ MICROSOFT with its !@#$ DIFFERENT SPEC
	str->buf[(str->len)-1] = 0;
	va_end(args);
	return result; }



/* erase the contents of an mstring */
void mstringClear(mstring* str) {
	if(!mstringValid(str))
		mstringFatal(str, "invalid mstring in mstringClear()");
	memset(str->buf, 0, str->len); }



/* increase buffer size */
void mstringGrow(mstring* str, size_t newlen){
	char* tmp;
	size_t tmplen;
	if(!mstringValid(str)) mstringFatal(str, "invalid source in mstringGrow()");
	if(newlen == str->len) return; // nothing to see here
	if(newlen < str->len) mstringFatal(str, "trying to shrink in mstringGrow()");
	tmplen = str->len;
	tmp = (char*)malloc(tmplen);
	if(!tmp) mstringFatal(str, "malloc failure in mstringGrow()");
	memcpy(tmp,str->buf,tmplen);
	mstringDelete(str);
	mstringNew(str, newlen);
	memcpy(str->buf,tmp,tmplen);
	free(tmp); }



//------------------------------- snip ---------------------------------------


/* prettyprint the structure */
void mstringDebug(const mstring* str) {
	if(!str) { fprintf(stderr, "--------\nNULL!!!!\n--------\n"); return; }
	ulong* bufterminator;
	bufterminator = (ulong*)(((uintptr_t)str->buf + str->len + 1 + 8)&~7);
	fprintf(stderr, "--------\n");
	fprintf(stderr, "address:\t%p\n", &str);
	fprintf(stderr, "canarybuf:\t0x%lx / 0x%lx\n", str->canarybuf, 
	(uintptr_t)str->canarybuf ^ (uintptr_t)str->buf);
	fprintf(stderr, "buf:\t\t%p\n", str->buf);
	fprintf(stderr, "len:\t\t%lu\n",(ulong)str->len);
	fprintf(stderr, "canarylen:\t0x%lx / 0x%lx\n", str->canarylen, 
	(ulong)str->canarylen ^ (ulong)str->len);
	if(mstringValid(str)) fprintf(stderr, "bufterminator:\t\t0x%lx\n", (ulong)*bufterminator);
	else fprintf(stderr, "bufterminator: not printed: bad deref\n");
	fprintf(stderr, "expected bufterminator:\t0x%lx\n", (ulong)(bufferkey ^ (uintptr_t)str));
	fprintf(stderr,  "--------\n"); }
	
	
/* hex prints the buffer including terminator etc. */
void mstringHexdump(const mstring* str) {
	size_t total = str->len+2+(8<<1);
	uint i;
	for(i = 0; i < total; i++) {
		// forget to specify two significant digits for a good time
		printf("%.2hhx ", (unsigned char)str->buf[i]); 
		if(i == (str->len) - 1) printf("==== "); }
	printf("\n"); }



/* an honorable death in the face of memory corruption! */
void mstringFatal(const mstring* str, char* message) {
	fprintf(stderr, "FATAL: %s\n", message);
	mstringDebug(str);
	abort(); }



/* an honorable whine in the face of slightly incorrect usage! */
void mstringComplain(mstring* str, char* message) {
	fprintf(stderr, "COMPLAIN: %s\n", message);
	mstringDebug(str); }