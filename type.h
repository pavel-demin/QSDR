
/*
 * fundamental type definitions
 */

#ifndef TYPE_H
#define TYPE_H

#define ARG_UNUSED(par)       if (par) do { ; } while (0)

/*
 *	basic definitions
 */
#ifndef __cplusplus
typedef unsigned long	bool;
#define false	0
#define true	1
#endif

#ifndef NULL
#define NULL		(void *) 0
#endif

/* typedef unsigned char	BYTE; */
/* typedef unsigned short	WORD16; */
/* typedef	unsigned int	WORD32; */


typedef signed char	int8;		/* -128 .. +127 */
typedef unsigned char	uint8;  	/*    0 .. +256 */
typedef	unsigned char	uchar;

typedef signed short	int16;  	/* -32768 .. +32767 */
typedef unsigned short	uint16;  	/*      0 .. +65535 */
typedef	unsigned short	ushort;

typedef signed long	int32;  	/* -2147483648 .. +2147483647 */
typedef unsigned long	uint32;  	/*           0 .. +4294967295 */
typedef	unsigned long	ulong;

typedef unsigned long long uint64;


#endif /* TYPE_H */


