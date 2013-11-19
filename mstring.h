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


int		mstringValid(const mstring* str);

void	mstringNew(mstring* str, size_t len);

void	mstringDelete(mstring* str);

void	mstringDuplicate(mstring* src, mstring* dst);

void	mstringSet(mstring *str, void* src, size_t len);

void	mstringAppend(mstring* str, void* src, size_t len, size_t pos);

int		mstringCompare(const mstring* a, const mstring* b);

size_t	mstringLength(const mstring* str);

int		mstringPrintf(mstring* str, const char* format, ...);

void	mstringClear(mstring* str);

void	mstringGrow(mstring* str, size_t newlen);



// -------- internal ----------

void	mstringDebug(const mstring* str);

void	mstringFatal(const mstring* str, char* message);

void	mstringComplain(mstring* str, char* message);