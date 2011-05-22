#ifndef _RC4_INC
#define _RC4_INC

typedef struct {
	char lookup[256];
	int x, y;
} mir_arc4_ctx;

struct ARC4_INTERFACE {
	int cbSize;
	void (*arc4_init)(mir_arc4_ctx *ctx, char *key, int keylen);
	void (*arc4_crypt)(mir_arc4_ctx *ctx, char *dataIn, char *dataOut, int datalen);
};

#define MS_SYSTEM_GET_ARC4I  "Miranda/System/GetARC4I"

static __inline int mir_getARC4I( struct ARC4_INTERFACE* dest )
{
	dest->cbSize = sizeof(*dest);
	return CallService( MS_SYSTEM_GET_ARC4I, 0, (LPARAM)dest );
}


void InitARC4Module();
void DeinitARC4Module();

#endif
