/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

/*
	compressor

	Performs zlib-style compression
*/
#include "compressor.h"

#include <stdlib.h>
#include <zlib.h>
#include <string.h>
#include <stdio.h>

/*
	CompressData

	This tries to re-order pixel data so that it compresses better.
	Unfortunately, this is a total failure for floating point data.
*/
int CompressData( char *inPtr, int inSize, char *outPtr)
{
	// Don't try to compress 0 sized arrays
	if( inSize == 0 ) return 0;

	char *_tmpBuffer = (char*) malloc( inSize );

	char *t1 = _tmpBuffer;
	char *t2 = _tmpBuffer + (inSize + 1) / 2;
	const char *stop = inPtr + inSize;

	char *inIterate = inPtr;

	while (true)
	{
		if (inIterate < stop)
			*(t1++) = *(inIterate++);
		else
			break;

		if (inIterate < stop)
			*(t2++) = *(inIterate++);
		else
			break;
	}

	{
		unsigned char *t = (unsigned char *) _tmpBuffer + 1;
		unsigned char *stop = (unsigned char *) _tmpBuffer + inSize;
		int p = t[-1];

		while (t < stop)
		{
			int d = int (t[0]) - p + (128 + 256);
			p = t[0];
			t[0] = d;
			++t;
		}
	}

	uLongf outSize = inSize;
	int cstatus = ::compress(
			(Bytef *)outPtr, &outSize, (const Bytef *) _tmpBuffer, inSize);

	free( _tmpBuffer );

	if( cstatus == Z_OK )
	{
		return outSize;
	}

	// A buffer error means that the data cannot be compressed ( the compressed version is larger than the original )
	// In this case, OpenEXR dictates that we write the original data
	// Any other error is unexpected.
	if( cstatus != Z_BUF_ERROR )  printf( "UNEXPECTED ERROR WRITING DEEP EXR\n" );

	memcpy( outPtr, inPtr, inSize );

	return inSize;
}

