#pragma once

/*
	Function:	
		bool FLZCompressMemory(unsigned char** pOutData, unsigned* pOutLen, unsigned char* InData, unsigned InLen)
	pOutData:			Pointer to the packed data (allocated by the compressor!)
	pOutLen:				Pointer to packed data size (fills by the compressor)
	InData:				Input data
	InLen:				Input data size
	isQuietMode:		Show debug info?
	CompressionLevel:	Compression level [0-4]
*/

bool FLZCompressMemory(unsigned char** pOutData, 
							  unsigned* pOutLen, 
							  unsigned char* InData, 
							  unsigned InLen,
							  bool isQuietMode,
							  int CompressionLevel = 3
							 );

/*
	Function:	
		bool FLZDecompressMemory(unsigned char* InData, unsigned char* OutData, int InLen, int OutLen)
	InData:		Packed data
	OutData:		Unpacked data
	InLen:		Packed data size
	OutLen:		Unpacked data size
*/

bool FLZDecompressMemory(unsigned char* InData, unsigned char* OutData, int InLen, int OutLen);
