#ifndef FCompressorH
#define FCompressorH

enum {
   FCOMPRESSOR_TYPE_STORE,
   FCOMPRESSOR_TYPE_BZ2,
   FCOMPRESSOR_TYPE_PPMd,
   FCOMPRESSOR_TYPE_LZO,
   FCOMPRESSOR_TYPE_FLZ,
   FCOMPRESSOR_TYPE_DXT3
};

class FCompressor {
public:
   bool isInitialized;
   int type;
   bool CompressBlock(unsigned char** pOutData, unsigned* pOutLen, unsigned char* InData, unsigned InLen);
   bool DecompressBlock(unsigned char** pOutData, unsigned* pOutLen, unsigned char* InData);

   bool Initialize(void);
   bool Release(void);

   FCompressor(int _type = FCOMPRESSOR_TYPE_BZ2)
   {
      type = _type;
      isInitialized = false;
   }
   ~FCompressor()
   {
      Release();
   }
};

#endif
