/******************************************************************************/
/*                                                                            */
/*    Copyright (c) 2011, DNA Research, the 3Delight Developers.              */
/*    All Rights Reserved.                                                    */
/*                                                                            */
/******************************************************************************/

#include "exrfile.h"
#include "scanlinezipblock.h"

int EXRFile::OpenOutputFile(const char* i_fileName)
{
	m_file = fopen(i_fileName, "wb");
	
	if (!m_file)
	{
		return IMF_ERROR_ERROR;
	}
	
	m_channelList = new ChannelList(*m_header->GetChannelList());
	m_channelList->Sort();
	
	m_header->GetAttribute("dataWindow", "box2i", (char*)&m_dataWindow[0]);
	m_header->GetAttribute("compression", "compression", (char*)&m_compression);
	
	int width = m_dataWindow[2] - m_dataWindow[0] + 1;
	size_t datasize = m_channelList->GetPixelSize() * width;
	if (m_compression == IMF_ZIP_COMPRESSION)
	{
		m_scanline_block =
			new ScanLineZipBlock(
				m_file,
				datasize,
				m_dataWindow[1]);
	}
	else
	{
		m_scanline_block =
			new ScanLineBlock(
				m_file,
				datasize,
				m_dataWindow[1]);
	}

	int height = m_dataWindow[3] - m_dataWindow[1] + 1;

	m_blocks =  height / m_scanline_block->NumLinesInBlock();
	m_blocks += height % m_scanline_block->NumLinesInBlock() > 0;

	m_offset_table = new unsigned long [m_blocks];
	m_offset_table_counter = 0;

	return IMF_ERROR_NOERROR;
}

int EXRFile::CloseFile()
{
	delete m_channelList;
	m_channelList = 0x0;

	delete m_scanline_block;
	m_scanline_block = 0x0;

	WriteOffsets();

	delete [] m_offset_table;
	m_offset_table = 0x0;

	fclose(m_file);
	m_file = 0x0;
	
	return IMF_ERROR_NOERROR;
}

int EXRFile::WriteMagic()const
{
	/*
		According to [1]:
		The magic number, of type int, is always 20000630 (decimal).
		It allows file readers to distinguish OpenEXR files from other files,
		since the first four bytes of an OpenEXR file are always
		0x76, 0x2f, 0x31 and 0x01.
	*/
	char magic[] = {0x76, 0x2f, 0x31, 0x01};
	
	fwrite(magic, 1, 4, m_file);
	
	return IMF_ERROR_NOERROR;
}

int EXRFile::WriteVersion()const
{
	/*
		According [1]:
		The version field, of type int, is treated as two separate bit fields.
		The 8 least significant bits (bits 0 through 7) contain the file format
		version number. The 24 most significant bits (8 through 31) are treated
		as a set of boolean flags.
	*/
	char version[] = {0x02, 0x00, 0x00, 0x00};
	
	fwrite(version, 1, 4, m_file);
	
	return IMF_ERROR_NOERROR;
}

int EXRFile::WriteHeader()const
{
	m_header->WriteToFile(m_file);

	return IMF_ERROR_NOERROR;
}

int EXRFile::WriteZerroOffsets()
{
	/*
		According to [1]:
		The line offset table allows random access to scan line blocks.
		The table is a sequence of scan line offsets, with one offset per
		scan line block. A scan line offset, of type unsigned long, indicates
		the distance, in bytes, between the start of the file and the start
		of the scan line block.

		It is not possible to get sizes of compressed blocks right now
		so we fill offset table with 0.
	*/

	/* Save current position of the file.
	 * Current position is start of offset table */
	fgetpos (m_file, &m_offset_position);

	/* Fill offset table by 0s */
	/* The offset is 64 bits */	
	char offset[] = {0,0,0,0,0,0,0,0};
	for ( unsigned int i=0; i<m_blocks; i++)
	{
		fwrite(offset, sizeof(offset), 1, m_file);
	}

	return IMF_ERROR_NOERROR;
}

int EXRFile::WriteOffsets()
{
	/*
		According to [1]:
		The line offset table allows random access to scan line blocks.
		The table is a sequence of scan line offsets, with one offset per
		scan line block. A scan line offset, of type unsigned long, indicates
		the distance, in bytes, between the start of the file and the start
		of the scan line block.
	*/

	/* Move current position to start of offset table */
	fsetpos (m_file, &m_offset_position);

	unsigned long offset;
	for ( unsigned int i=0; i<m_blocks; i++)
	{
		offset = m_offset_table[i];
		fwrite(&offset, sizeof(offset), 1, m_file);
	}

	return IMF_ERROR_NOERROR;
}

int EXRFile::SetFBData(
	const char *i_base,
	size_t i_xStride,
	size_t i_yStride)
{
	m_fb_base = i_base;
	m_fb_xStride = i_xStride;
	m_fb_yStride = i_yStride;
	
	return IMF_ERROR_NOERROR;
}

int EXRFile::WriteFBPixels(int i_numScanLines)
{
	/*
		Sort data in right order and write to file
		
		input data:
		 pixel data source is m_fb_base,
		 Pixel (x, y) is at address
		 base + x * xStride + y * yStride
		 
		 in following order
		 r1 g1 b1 a1 r2 g2 b2 a2 .. rn gn bn an
		
		output data:
		 r1 r2 .. rn g1 g2 .. gn b1 b2 .. bn a1 a2 .. an
	*/
	int width = m_dataWindow[2] - m_dataWindow[0] + 1;
	
	for (int i=0; i<i_numScanLines; ++i)
	{
		int datasize = m_channelList->GetPixelSize() * width;
		char *data = (char*) malloc( datasize );
		
		/* Reorder data */
		for (int j=0; j<width; j++)
		{
			for (int n=0; n<m_channelList->NumChannels(); ++n)
			{
				size_t pixel_size = m_channelList->GetChannelPixelSize(n);
				size_t channel_offset = m_channelList->GetChannelOffset(n);
				size_t channel_alphabet_offset =
					m_channelList->GetChannelAlphabetOffset(n);
				
				char* destanation = 
					data +
					channel_alphabet_offset * width +
					pixel_size * j;
				
				const char* source =
					m_fb_base +
					i * m_fb_yStride +
					j * m_fb_xStride +
					m_dataWindow[0] * m_fb_xStride +
					channel_offset;
				
				memcpy(destanation, source, pixel_size);
			}
		}

		/* Save position of current block to offset table */
		fpos_t pos;
		fgetpos (m_file, &pos);
		unsigned int k =
			m_offset_table_counter++/m_scanline_block->NumLinesInBlock();
		m_offset_table[k] = pos;

		m_scanline_block->StoreNextLine(data);
		m_scanline_block->WriteToFile();
		
		free(data);
	}

	return IMF_ERROR_NOERROR;
}

