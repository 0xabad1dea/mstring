/* 

	mstring: melissa string
	
	this is currently **NOT LICENSED** for reuse

*/


typedef struct {
	unsigned long canarybuf;
	size_t len;
	char* buf;
	unsigned long canarylen;
	
} mstring;


int mstringValid(mstring* str);

void mstringNew(mstring* str, size_t len);

void mstringDelete(mstring* str);

void mstringDuplicate(mstring* src, mstring* dst);

void mstringSet(mstring *str, void* src, size_t len);

void mstringAppend(mstring* str, void* src, size_t len, size_t pos);

void mstringDebug(mstring* str);

void mstringFatal(mstring* str, char* message);

void mstringComplain(mstring* str, char* message);