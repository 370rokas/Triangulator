//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "my_assert.h"
#include "FCompressor.h"
#include "FStream.hpp"
#include "bzlib.h"
#include "flz/flz.h"
#include "lzo/lzo1x.h"
//---------------------------------------------------------------------------
int lzo1x_decompress_asm_fast(unsigned char* src, unsigned int src_len,
                              unsigned char* dst, unsigned int* dst_len,
                              unsigned char* wrkmem);
//---------------------------------------------------------------------------
bool ucl_decompress(const unsigned char* src, unsigned int src_len,
                    unsigned char *dst, unsigned int *dst_len);
//---------------------------------------------------------------------------
int lzo1x_decompress_asm_fast(unsigned char* src, unsigned int src_len,
                              unsigned char* dst, unsigned int* dst_len,
                              unsigned char* wrkmem);
//---------------------------------------------------------------------------
bool FCompressor::Initialize(void)
{
   if(isInitialized) return true;
   isInitialized = true;
   return true;
}
//---------------------------------------------------------------------------
bool FCompressor::Release(void)
{
   if(!isInitialized) return true;
   isInitialized = false;
   return true;
}
//---------------------------------------------------------------------------
void log(char* sz,...);
bool FCompressor::CompressBlock(unsigned char** pOutData, unsigned* pOutLen,
                                unsigned char* InData, unsigned InLen)
{
   my_assert(pOutData);
   my_assert(pOutLen);

   FStream InStream;
   FStream OutStream;

   InStream.attach(InData, InLen);
   OutStream.reload(InLen*2 + 9);
   OutStream.Pos = OutStream.Size = 9;

   unsigned char* OutData = OutStream.Data + OutStream.Pos;
   unsigned OutLen = InLen;

   switch(type) {
      case FCOMPRESSOR_TYPE_DXT3:
      {
         typedef int (*t_dxt3_compress_nomip)(void* InBuffer,void* OutBuffer,int Lx,int Ly);
         HMODULE hDll;
         if((hDll = LoadLibrary("dxtpack.dll")) == NULL)
      	 {
            MessageBox(0, "Load library error!", "flz.dll", 0);
            abort();
      	 }

         t_dxt3_compress_nomip dxt3_compress_nomip =
            (t_dxt3_compress_nomip)GetProcAddress(hDll, "dxt3_compress_nomip");

         if(dxt3_compress_nomip == NULL)
      	 {
            MessageBox(0, "Cannot find 'FLZCompressMemory'", "flz.dll", 3);
            abort();
      	 }
         int Lx=256;
         while(Lx*Lx*4>InLen)Lx>>=1;
         unsigned char* outD=new unsigned char[Lx*Lx+1024];
         //log("before compress: size=%d\n",InLen);
         int sz=dxt3_compress_nomip(InData,outD,Lx,Lx);
         //log("after compress: size=%d\n",sz);
         FreeLibrary(hDll);

         static long wrkmem[LZO1X_999_MEM_COMPRESS];
      	unsigned int outlen = 0;

	      if(lzo_init() != LZO_E_OK)
            throw("lzo_init() failed !!!");
         unsigned char* outD2=new unsigned char[Lx*Lx+8192];
         unsigned OutLen2;
      	int r = lzo1x_999_compress(outD, sz, outD2, &OutLen2, wrkmem);
         if(r != LZO_E_OK)
	      	throw("internal error - compression failed: %d\n", r);

         OutStream.putblock(outD2, OutLen2);
         OutLen=OutLen2;
         log("lzo: %d->%d\n",sz,OutLen2);
         delete[]outD;
         delete[]outD2;

         break;
      }
      break;
      case FCOMPRESSOR_TYPE_BZ2:
      {
         bz_stream strm_C;
         //---------------------- BZ2 initialization ---------------------------
         {
            int blockSize100k = 9;
            int verbosity = 0;
            int workFactor = 30;
            strm_C.bzalloc = NULL;
            strm_C.bzfree = NULL;
            strm_C.opaque = NULL;
            int ret = BZ2_bzCompressInit(&strm_C, blockSize100k, verbosity, workFactor);
            if(ret != BZ_OK) return false;
         }
         //------------------------- Compression -------------------------------
         {
            strm_C.next_in = InData;
            strm_C.next_out = OutData;
            strm_C.avail_in = InLen;
            strm_C.avail_out = OutLen;
            int ret = BZ2_bzCompress(&strm_C, BZ_FINISH);
            my_assert(ret != BZ_FINISH_OK);
            my_assert(ret == BZ_STREAM_END);

            // normal termination
            OutLen -= strm_C.avail_out;
         }
         //---------------------- BZ2 deinitialization -------------------------
         {
            BZ2_bzCompressEnd(&strm_C);
         }
         //---------------------------------------------------------------------
         break;
      }
      case FCOMPRESSOR_TYPE_PPMd:
         my_assert(!"PPMd is not supported!");
      case FCOMPRESSOR_TYPE_LZO:
         my_assert(!"LZO is not supported!");
      case FCOMPRESSOR_TYPE_FLZ:
      {
         typedef bool (*t_FLZCompressMemory)(unsigned char** pOutData,
                       							   unsigned* pOutLen,
                     							   unsigned char* InData,
							                        unsigned InLen,
                     							   bool isQuietMode,
                     							   int CompressionLevel);

         HMODULE hDll;
         if((hDll = LoadLibrary("flz.dll")) == NULL)
      	{
		      MessageBox(0, "Load library error!", "flz.dll", 0);
      		abort();
      	}

         t_FLZCompressMemory FLZCompressMemory =
            (t_FLZCompressMemory)GetProcAddress(hDll, "FLZCompressMemory");

         if(FLZCompressMemory == NULL)
      	{
		      MessageBox(0, "Cannot find 'FLZCompressMemory'", "flz.dll", 3);
      		abort();
      	}

	   	unsigned char* OutD;
   		FLZCompressMemory(&OutD, &OutLen, InStream.Data, InStream.Size, true, 0);
         OutStream.putblock(OutD, OutLen);
         GlobalFree(OutD);

         FreeLibrary(hDll);
         break;
      }
   };
   OutStream.putc(0, type);
   OutStream.putint(1, OutLen);
   OutStream.putint(5, InLen);

   *pOutData = OutStream.Data;
   *pOutLen = OutLen + 9;
   OutStream.drop();

   return true;
}
//---------------------------------------------------------------------------
bool FCompressor::DecompressBlock(unsigned char** pOutData, unsigned* pOutLen, unsigned char* InData)
{
   my_assert(pOutData);
   my_assert(pOutLen);

   FStream InStream;
   FStream OutStream;
   InStream.attach(InData, 9);

   unsigned char Type = InStream.getc();
   unsigned InLen = InStream.getint();
   unsigned OutLen = InStream.getint();

   InStream.attach(InData+9, InLen);
   OutStream.reload(OutLen+3);

   switch(Type) {
      case FCOMPRESSOR_TYPE_DXT3:
      {
         typedef int (*t_dxt3_decompress_nomip)(void* InBuffer,void* OutBuffer,int Lx,int Ly);
         HMODULE hDll;
         if((hDll = LoadLibrary("dxtpack.dll")) == NULL)
      	 {
            MessageBox(0, "Load library error!", "flz.dll", 0);
            abort();
      	 }

         t_dxt3_decompress_nomip dxt3_decompress_nomip =
            (t_dxt3_decompress_nomip)GetProcAddress(hDll, "dxt3_decompress_nomip");

         if(dxt3_decompress_nomip == NULL)
      	 {
            MessageBox(0, "Cannot find 'dxtpack.dll'", "dxtpack.dll", 3);
            abort();
      	 }
         unsigned char* outD2=new unsigned char[256*256+1024];
         unsigned InLen2;
         lzo1x_decompress_asm_fast(InData+9, InLen,outD2,&InLen2,NULL);
         int Lx=256;
         while(Lx*Lx>InLen2)Lx>>=1;
         unsigned char* outD=new unsigned char[Lx*Lx*4+1024];
         //log("before decompress: size=%d Lx=%d\n",InLen,Lx);
         int sz=dxt3_decompress_nomip(outD2,outD,Lx,Lx);
         delete[]outD2;
         //log("after decompress: size=%d\n",sz);
         OutStream.putblock(outD, sz);
         delete[]outD;
         FreeLibrary(hDll);
         OutLen=sz;
         break;
      }
      break;
      case FCOMPRESSOR_TYPE_BZ2:
      {
         bz_stream strm_D;
         //---------------------- BZ2 initialization ---------------------------
         {
            int verbosity = 0;

            strm_D.bzalloc = NULL;
            strm_D.bzfree = NULL;
            strm_D.opaque = NULL;
            int ret = BZ2_bzDecompressInit(&strm_D, verbosity, 0);
            if(ret != BZ_OK) return false;
         }
         //------------------------- Decompression -----------------------------
         {
            strm_D.next_in = InStream.Data;
            strm_D.next_out = OutStream.Data;
            strm_D.avail_in = InLen;
            strm_D.avail_out = OutLen;

            int ret = BZ2_bzDecompress(&strm_D);
            my_assert(ret != BZ_OK);
            my_assert(ret == BZ_STREAM_END);

            // normal termination
            OutLen -= strm_D.avail_out;
         }
         //---------------------- BZ2 deinitialization -------------------------
         {
            BZ2_bzDecompressEnd(&strm_D);
         }
         //---------------------------------------------------------------------
         break;
      }
      case FCOMPRESSOR_TYPE_PPMd:
         my_assert(!"PPMd is not supported!");
      case FCOMPRESSOR_TYPE_LZO:
      {
   		if(lzo1x_decompress_asm_fast(InStream.Data, InLen,
            OutStream.Data, &OutStream.Pos, NULL) < 0) throw("LZO decompression failed!");
         break;
      }
      case FCOMPRESSOR_TYPE_FLZ:
      {
         if(!FLZDecompressMemory(InStream.Data, OutStream.Data, InLen, OutLen))
            throw("FLZ decompression failed!");
         break;
      }
   };
   *pOutData = OutStream.Data;
   *pOutLen = OutLen;
   OutStream.drop();
   //log("decompress done: size=%d\n",OutLen);

   return true;
}
//---------------------------------------------------------------------------

