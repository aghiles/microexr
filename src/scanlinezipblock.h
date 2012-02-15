/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

/*
	class ScanLineBlock

	IMF_ZIP_COMPRESSION scan line block in exr file
*/

#ifndef __scanlinezipblock_h_
#define __scanlinezipblock_h_

#include "scanlineblock.h"

class ScanLineZipBlock : public ScanLineBlock
{
public:
	ScanLineZipBlock(
		FILE *i_file,
		size_t i_linesize,
		int i_firstline) :
		
		ScanLineBlock(i_file, i_linesize, i_firstline, NumLinesInBlock())
	{
	}
	
	~ScanLineZipBlock()
	{
		if (m_currentLine%NumLinesInBlock() != 0)
		{
			WriteCurrentBlockToFile();
		}
	}
	
	/*
		NumLinesInBlock
		
		One or more scan lines are stored together as a scan-line block.
		The number of scan lines per block depends on how the pixel
		data are compressed.
	*/
	int NumLinesInBlock()const
	{
		return 16;
	}
	
	/*
		StoreNextLine
		
		Copy data to ScanLineBlock's buffer
	*/
	int StoreNextLine(char* i_data);
	/*
		WriteToFile
		
		If buffer full, write stored data to exr file
	*/
	int WriteToFile()const;
	
private:
	/*
		WriteCurrentBlockToFile
		
		Write stored data to exr file
	*/
	int WriteCurrentBlockToFile()const;
};

#endif

