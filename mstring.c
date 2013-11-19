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

	--TODO--
	mstringCompareSecure (constant time)
	mstringResize
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
	bufterminator = (ulong*)(((ulong)str->buf + str->len + 1 + 3)&~3);
	
	// the last clause can segfault if buf points to lala land,
	// but short circuit evaluation will usually prevent this.
	// there isn't really anything I can do about this.
	if((str->canarybuf ^ (ulong)str->buf) == bufferkey
	&& (str->canarylen ^ (ulong)str->len) == lengthkey
	&& (*bufterminator == bufferkey ^ (ulong)str)) return 1;
	
	return 0; }



/* initialize or re-initialize an mstring structure  */
void mstringNew(mstring* str, size_t len) {
	
	if(str == NULL) mstringFatal(str, "null pointer passed to mstringNew()");
	
	// this should be alignment safe now.
	if((len + 2 + (sizeof(uint)<<1))  < len) 
		mstringFatal(NULL, "length wraparound in mstringNew()");
	
	// reused valid mstring - clean it up for you
	if(mstringValid(str)) {
		//mstringComplain(str, "re-initializing dirty mstring");
		mstringDelete(str); }
	
	str->len = len;
	str->buf = malloc(len+2+(sizeof(uint)<<1)); 
	if(!str->buf) mstringFatal(str, "malloc failed in mstringNew()");
	str->buf[len+1] = 0;
	
	str->canarybuf = bufferkey ^ (ulong)str->buf;
	str->canarylen = lengthkey ^ (ulong)str->len;
	
	ulong* bufterminator;
	bufterminator = (ulong*)(((ulong)str->buf + len + 1 + 3)&~3);
	*bufterminator = bufferkey ^ (ulong)str;
	
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
			dst->buf[(dst->len)+1] = 0;
			return; }
	
	mstringFatal(src, "bad source in mstringDuplicate()"); }
	
	

/* copy arbitrary bytes to buffer - len 0 to take strlen of src */
void mstringSet(mstring* str, void* src, size_t len) {
	if(len == 0) // I considered checking for wraparound here, 
		len = strlen((char*)src) +1; // but it's actually pretty pointless :)
	if(len > str->len) mstringFatal(str, "excessive length in mstringSet()");
	
	
	if(mstringValid(str)) {
		memcpy(str->buf, src, len);
		// I prefer there to always be a null after the buffer so we *can*
		// read it out as a c string - this will NOT protect you from length
		// assumption bugs of your own making when copying out to bare buffers
		str->buf[len] = 0; } 
	
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
	va_end(args);
	return result; }



/* erase the contents of an mstring */
void mstringClear(mstring* str) {
	if(!mstringValid(str))
		mstringFatal(str, "invalid mstring in mstringClear()");
	memset(str->buf, 0, str->len); }



//------------------------------- snip ---------------------------------------


/* prettyprint the structure */
void mstringDebug(const mstring* str) {
	if(!str) { fprintf(stderr, "--------\nNULL!!!!\n--------\n"); return; }
	ulong* bufterminator;
	bufterminator = (ulong*)(((ulong)str->buf + str->len + 1 + 3)&~3);
	fprintf(stderr, "--------\n");
	fprintf(stderr, "address:\t%p\n", &str);
	fprintf(stderr, "canarybuf:\t0x%lx / 0x%lx\n", str->canarybuf, 
	(ulong)str->canarybuf ^ (ulong)str->buf);
	fprintf(stderr, "buf:\t\t%p\n", str->buf);
	fprintf(stderr, "len:\t\t%u\n",(uint)str->len);
	fprintf(stderr, "canarylen:\t0x%lx / 0x%lx\n", str->canarylen, 
	(ulong)str->canarylen ^ (ulong)str->len);
	if(mstringValid(str)) fprintf(stderr, "bufterminator:\t\t0x%lx\n", (ulong)*bufterminator);
	else fprintf(stderr, "bufterminator: not printed: bad deref\n");
	fprintf(stderr, "expected bufterminator:\t0x%lx\n", (ulong)(bufferkey ^ (ulong)str));
	fprintf(stderr,  "--------\n"); }



/* an honorable death in the face of memory corruption! */
void mstringFatal(const mstring* str, char* message) {
	fprintf(stderr, "FATAL: %s\n", message);
	mstringDebug(str);
	abort(); }



/* an honorable whine in the face of slightly incorrect usage! */
void mstringComplain(mstring* str, char* message) {
	fprintf(stderr, "COMPLAIN: %s\n", message);
	mstringDebug(str); }