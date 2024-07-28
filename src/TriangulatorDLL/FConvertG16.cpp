//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include <dir.h>
#include "FConvert.h"
#include "FCompressor.h"
//---------------------------------------------------------------------------
int lzo1x_decompress_asm_fast(unsigned char* src, unsigned int src_len,
                              unsigned char* dst, unsigned int* dst_len,
                              unsigned char* wrkmem);
//---------------------------------------------------------------------------
bool ucl_decompress(const unsigned char* src, unsigned int src_len,
                    unsigned char *dst, unsigned int *dst_len);
//---------------------------------------------------------------------------
bool G16UnpackSegment(unsigned char* InData,       // - упакованные данные
                      unsigned int   InLen,        // - размер входного буфера
                      unsigned char* OutData,      // - распакованный сегмент
                      unsigned char* WorkData,     // - рабочий буфер
                      unsigned int*  FOffsData,    // - массив смещений кадров
                      unsigned int   FramesNumber, // - количество кадров в сегменте
                      unsigned int   Flags);       // - флаги (метод упаковки)
//---------------------------------------------------------------------------
#define G16_COMPRESS_METHOD_MASK 0x3   // 0011
#define G16_NONCOMPRESSED        0     // 0000
#define G16_COMPRESSED_BY_UCL    1     // 0001
#define G16_COMPRESSED_BY_LZO    2     // 0010

#define G16_PACK_METHOD_MASK     0xC   // 1100
#define G16_IDXSTORE             0     // 0000
#define G16_PACKED_BY_2DWAVELET  4     // 0100
#define G16_PACKED_BY_3DWAVELET  8     // 1000
#define G16_444STORE             12    // 1100
//---------------------------------------------------------------------------
unsigned char G16PalRGB[1024];  // - палитра RGB канала
//---------------------------------------------------------------------------
/*
   Функция распаковывает сегмент с кадрами, сегмент в виде:
      unsigned int outlen  // - размер разархивированного блока
      <compressed data>    // - заархивированные данные
   После разархивации получается набор фреймов, содержащих данные о
   квадратах, зависимо от метода упаковки.
*/
//---------------------------------------------------------------------------
bool G16UnpackSegment(unsigned char* InData,       // - упакованные данные
                      unsigned int   InLen,        // - размер входного буфера
                      unsigned char* OutData,      // - распакованный сегмент
                      unsigned char* WorkData,     // - рабочий буфер
                      unsigned int*  FOffsData,    // - массив смещений кадров
                      unsigned int   FramesNumber, // - количество кадров в сегменте
                      unsigned int   Flags)        // - флаги (метод упаковки)
{
   unsigned int OutLen = *(unsigned int*)InData;
   InData += 4;

   //////////////////////////////////////////////////////////////////////////
   // Разархивируем raw-данные из inbuf в workbuf
   //////////////////////////////////////////////////////////////////////////
   if((Flags & G16_COMPRESS_METHOD_MASK) == G16_COMPRESSED_BY_UCL)
   {
   	if(!ucl_decompress(InData, InLen, WorkData, &OutLen))
         return false;
   }
   else
   if((Flags & G16_COMPRESS_METHOD_MASK) == G16_COMPRESSED_BY_LZO)
   {
   	if(lzo1x_decompress_asm_fast(InData, InLen, WorkData, &OutLen, WorkData) < 0)
         return false;
   }
   else
   if((Flags & G16_COMPRESS_METHOD_MASK) == G16_NONCOMPRESSED)
   {
      memcpy(WorkData, InData, InLen);
      OutLen = InLen;
   }
   else
      return false;
   //////////////////////////////////////////////////////////////////////////

   unsigned allsize = 0;
 	int workbuf_pos = 0;
   #define GETC() WorkData[workbuf_pos++]
   #define GETSH(a) {a = *(unsigned short*)&(WorkData[workbuf_pos]); workbuf_pos += 2;}

   if((Flags & G16_PACK_METHOD_MASK) == G16_IDXSTORE)
   {
      /*
         Структура сегмента для IdxStore:
         SEGMENT {
            DWORD ColorOffset;            // - относительное смещение в байтах до данных цвета
            DWORD AlphaOffset;            // - относительное смещение в байтах до альфы

            FRAME {
               WORD SquaresNumber;        // - количество квадратов в кадре
               SQUARE {
                  // - header 4 bytes
                  DWORD Pow	   :4; 	   //  side = 2^Pow
      	         DWORD Config	:4; 	   //  configuration of the polygon
	               DWORD X			:12;	   //  x-position of the chunk in sprite
                  DWORD Y  		:12;	   //  y-position of the chunk in sprite
               } Squares[SquaresNumber];
            } Frames[FramesNumber];

            ColorData[];                  // - массив цветовой информации для всего сегмента
            AlphaData[];                  // - массив информации о альфе для всего сегмента
         }
      */
      unsigned ch = 0, c_pos = 0, a_pos = 0, src_pos = 0, dst_pos = 0;
      #define GETINT(ch) ch = *((unsigned*)&(WorkData[src_pos])); src_pos += 4;
      #define GETSH(ch)  ch = *((unsigned short*)&(WorkData[src_pos])); src_pos += 2;
      #define PUTINT(ch) *((unsigned*)&(OutData[dst_pos])) = ch; dst_pos += 4;
      #define PUTSH(ch)  *((unsigned short*)&(OutData[dst_pos])) = ch; dst_pos += 2;
      #define PUTC(ch)   *((unsigned char*)&(OutData[dst_pos])) = ch; dst_pos += 1;

      GETINT(ch);
      unsigned char* ColorOffset = WorkData + src_pos + ch;
      GETINT(ch);
      unsigned char* AlphaOffset = WorkData + src_pos + ch;

      for(unsigned f = 0; f < FramesNumber; f++)
      {
         WORD SquaresNumber;
         GETSH(SquaresNumber);

         FOffsData[f] = dst_pos;   // - указатель на текущий кадр
         for(unsigned s = 0; s < SquaresNumber; s++)
         {
            GETINT(ch);
            PUTINT(ch);       // - заголовок квадрата
            PUTINT(0);        // - reserved
            int side = 1<<(ch>>28);

            #define MAKECOLOR(_a) {\
               unsigned color = *(unsigned*)&G16PalRGB[(*ColorOffset++)<<2];\
               b = (color>>16)&0xFF;\
               g = (color>>8)&0xFF;\
               r = color&0xFF;\
               ch = ((_a&0xF0)<<8)|((r&0xF0)<<4)|(g&0xF0)|(b>>4);\
            }

				unsigned short* CurrSqrStart = (unsigned short*)&(OutData[dst_pos]);
            for(int j = 0; j < side; j++)
            {
               for(int i = 0; i < side; i+=2)
               {
                  int aa = *AlphaOffset++;
                  int a1 = aa&0xF0;
                  int a2 = (aa<<4)&0xF0;
                  int r, g, b;
                  if(a1)
                  {
                     MAKECOLOR(a1);
                  }
                  else ch = 0;
                  PUTSH(ch);

                  if(a2)
                  {
                     MAKECOLOR(a2);
                  }
                  else ch = 0;
                  PUTSH(ch);
               }
            }
         }
      }
   }
   else
   if((Flags & G16_PACK_METHOD_MASK) == G16_444STORE)
   {
      /*
         Структура сегмента для 444Store:
         SEGMENT {
            DWORD ColorOffset;            // - относительное смещение в байтах до данных цвета
            DWORD AlphaOffset;            // - относительное смещение в байтах до альфы

            FRAME {
               WORD SquaresNumber;        // - количество квадратов в кадре
               SQUARE {
                  // - header 4 bytes
                  DWORD Pow	   :4; 	   //  side = 2^Pow
      	         DWORD Config	:4; 	   //  configuration of the polygon
	               DWORD X			:12;	   //  x-position of the chunk in sprite
                  DWORD Y  		:12;	   //  y-position of the chunk in sprite
               } Squares[SquaresNumber];
            } Frames[FramesNumber];

            ColorData[];                  // - массив цветовой информации для всего сегмента
            AlphaData[];                  // - массив информации о альфе для всего сегмента
         }
      */
      unsigned ch = 0, c_pos = 0, a_pos = 0, src_pos = 0, dst_pos = 0;
      #define GETINT(ch) ch = *((unsigned*)&(WorkData[src_pos])); src_pos += 4;
      #define GETSH(ch)  ch = *((unsigned short*)&(WorkData[src_pos])); src_pos += 2;
      #define PUTINT(ch) *((unsigned*)&(OutData[dst_pos])) = ch; dst_pos += 4;
      #define PUTSH(ch)  *((unsigned short*)&(OutData[dst_pos])) = ch; dst_pos += 2;
      #define PUTC(ch)   *((unsigned char*)&(OutData[dst_pos])) = ch; dst_pos += 1;

      GETINT(ch);
      unsigned char* ColorOffset = WorkData + src_pos + ch;
      GETINT(ch);
      unsigned char* AlphaOffset = WorkData + src_pos + ch;

      for(unsigned f = 0; f < FramesNumber; f++)
      {
         WORD SquaresNumber;
         GETSH(SquaresNumber);

         FOffsData[f] = dst_pos;   // - указатель на текущий кадр
         for(unsigned s = 0; s < SquaresNumber; s++)
         {
            GETINT(ch);
            PUTINT(ch);       // - заголовок квадрата
            PUTINT(0);        // - reserved
            int side = 1<<(ch>>28);

            #undef MAKECOLOR
            #define MAKECOLOR(_a) {\
               unsigned color = *((unsigned short*)ColorOffset);\
               ColorOffset += 2;\
               ch = color;\
               ch |= ((_a&0xF0)<<8);\
            }

            for(int j = 0; j < side; j++)
            {
               for(int i = 0; i < side; i+=2)
               {
                  int aa = *AlphaOffset++;
                  int a1 = aa&0xF0;
                  int a2 = (aa<<4)&0xF0;
                  int r, g, b;
                  if(a1)
                  {
                     MAKECOLOR(a1);
                  }
                  else ch = 0;
                  PUTSH(ch);

                  if(a2)
                  {
                     MAKECOLOR(a2);
                  }
                  else ch = 0;
                  PUTSH(ch);
               }
            }
         }
      }
   }
   else
      return false;
   // -----------------------------------------------------------------------
   return true;
}
//---------------------------------------------------------------------------
bool SkipFrameMode=true;
int g16_NFrames=0;
int g16_NSectors=0;
//---------------------------------------------------------------------------
bool FConvert::LoadFromG16(const char* FileName, const char* Parameters)
{
   //---------------------------------------------------------------------------
   struct G16Header
   {
   	unsigned int   Magic;				// "GU16" "GN16"
   	unsigned int   FileSize;			// total size of the file
   	unsigned char	FramesPerSegment;	// number of sprites in segment
   	unsigned short	Frames;  			// number of sprites in package
   	unsigned short	Width;				// width of the sprite, in pixels
   	unsigned short Height;				// height of the sprite, in pixels
   	unsigned int   MaxWorkbuf;			// maximum workbuf size for unpack
   	unsigned short Segments;      	// number of packing segments
   };
   //---------------------------------------------------------------------------
   struct GPALHeader
   {
   	unsigned int	Magic;				// "GPAL"
   	unsigned int	FileSize;			// total size of the file
   	unsigned int	UnpackedSize;		// size for unpack
   	unsigned short	Flags;				// flags (pack method)
   	unsigned short	Reserved;			// reserved (zero)
   };
   //---------------------------------------------------------------------------
   struct G16SegmentInfo
   {
   	unsigned int Offset;	         //  offset of the segment
   	unsigned int PackMethod;	   //  packing method
   };
   //---------------------------------------------------------------------------
   struct G16Frame
   {
   	unsigned short	Width;	     	//  width of the sprite, in pixels
   	unsigned short	Height;   	  	//  height of the sprite, in pixels
   	unsigned short	Squares; 	  	//  number of chunks in sprite
   	unsigned short	SegmentNumber;	//  number of the global packed segment of data
   };
   //---------------------------------------------------------------------------

   int PackMethod = -1;
   bool isPaintNationalColor = !false;
   bool isGU = false;
   bool isIdx = false;

   FILE *in;
   G16Header hdr;

   if((in = fopen(FileName, "rb")) == NULL)
   {
      _ERROR("Cannot open file %s!", FileName);
      return false;
   }

   fread(&hdr.Magic, 4, 1, in);
   fread(&hdr.FileSize, 4, 1, in);

   // - загрузка дополнительных блоков
   long addr = ftell(in);
   fseek(in, hdr.FileSize, SEEK_SET);
   if(!feof(in))
   {
      // - load GPAL
      {
         GPALHeader hdr;
         fread(&hdr.Magic, 4, 1, in);
         if(feof(in) || hdr.Magic != 'LAPG')
         {
            _ERROR("Undefined file block type! Not is GPAL");
            fseek(in, hdr.FileSize, SEEK_SET);
         }
         else
         {
            fread(&hdr.FileSize, 4, 1, in);
            fread(&hdr.UnpackedSize, 4, 1, in);
            fread(&hdr.Flags, 2, 1, in);
            fread(&hdr.Reserved, 2, 1, in);

            unsigned PackedSize = hdr.FileSize - 4 - 4 - 4 - 2 - 2;
            unsigned char* PackedData = new unsigned char[PackedSize];
            unsigned UnpackedSize = hdr.UnpackedSize+3;
            unsigned char* UnpackedData = new unsigned char[UnpackedSize];

            fread(PackedData, PackedSize, 1, in);

            // ... unpack here
            memcpy(UnpackedData, PackedData, PackedSize);
            delete[] PackedData;

            // - fill the data
            unsigned inpos = 0;
            unsigned short PalettesNumber =
               *(unsigned short*)&(UnpackedData[inpos]);          // - PalettesNumber
            inpos += 2;

            // - palette`s begin
            unsigned int SizeOf =
               *(unsigned int*)&(UnpackedData[inpos]);            // - SizeOf
            inpos += 4;
            unsigned short Items =
               *(unsigned short*)&(UnpackedData[inpos]);          // - Items
            inpos += 2;
            unsigned short Flags =
               *(unsigned short*)&(UnpackedData[inpos]);          // - Flags
            inpos += 2;
//            memcpy(G16Pal235, &(UnpackedData[inpos]), SizeOf-4-2-2);
            inpos += SizeOf-4-2-2;

            // - palette`s begin
            SizeOf =
               *(unsigned int*)&(UnpackedData[inpos]);            // - SizeOf
            inpos += 4;
            Items =
               *(unsigned short*)&(UnpackedData[inpos]);          // - Items
            inpos += 2;
            Flags =
               *(unsigned short*)&(UnpackedData[inpos]);          // - Flags
            inpos += 2;
            memcpy(G16PalRGB, &(UnpackedData[inpos]), SizeOf-4-2-2);
            inpos += SizeOf-4-2-2;

            delete[] UnpackedData;
         }
      }

      // - load GINF
      {
         GPALHeader hdr;
         fread(&hdr.Magic, 4, 1, in);
         if(feof(in) || hdr.Magic != 'FNIG')
         {
            _ERROR("Undefined file block type! Not is GINF");
         }
         else
         {
            fread(&hdr.FileSize, 4, 1, in);
            fread(&hdr.UnpackedSize, 4, 1, in);
            fread(&hdr.Flags, 2, 1, in);
            fread(&hdr.Reserved, 2, 1, in);

            unsigned char* inbuf = new unsigned char[hdr.UnpackedSize];

            unsigned PackedSize = 32;
            unsigned char* PackedData = inbuf;
            fread(PackedData, PackedSize, 1, in);
            // ... decompress here

            // - load the data
            unsigned inpos = 0;
            // - ItemsNumber
            inpos += 2;
            // Begin info block
            inpos += 4;
            inpos += 2;
            Directions = *(unsigned short*)&(inbuf[inpos]);       // - Directions
            inpos += 2;
            inpos += 2;
            inpos += 4;
            inpos += 4;
            inpos += 4;
            inpos += 1;

            g16_NSectors=Directions;

            delete[] inbuf;
         }
      }
   }
   fseek(in, addr, SEEK_SET);

   if(hdr.Magic == '61UG')
   {
      isGU = true;
      fread(&hdr.FramesPerSegment, 1, 1, in);
      fread(&hdr.Frames, 2, 1, in);
      fread(&hdr.Width, 2, 1, in);
      fread(&hdr.Height, 2, 1, in);
      fread(&hdr.MaxWorkbuf, 4, 1, in);
      fread(&hdr.Segments, 2, 1, in);

      g16_NFrames=hdr.Frames;
      int curf=0;


      if(ProgressMax) ProgressMax(hdr.Frames);

      int Width = hdr.Width;
      int Height = hdr.Height;

      unsigned MaxWorkbuf = hdr.MaxWorkbuf;
      if(!MaxWorkbuf) MaxWorkbuf = 1024*1024;
      unsigned char* workbuf = new unsigned char[MaxWorkbuf];

      // - массив ссылок на сегменты (+ 1 для последнего сегмента)
      unsigned int* SegmentsOffsets = new unsigned int[hdr.Segments+1];
      unsigned char* SegmentsFlags  = new unsigned char[hdr.Segments];
      // - грузим ссылки на сегменты
      for(int s = 0; s < hdr.Segments; s++)
      {
      	fread(&SegmentsOffsets[s], 4, 1, in);
         SegmentsFlags[s] = SegmentsOffsets[s]&0xF;
         SegmentsOffsets[s] >>= 4;
      }
      SegmentsOffsets[hdr.Segments] = hdr.FileSize;

      // - массив информации о кадрах (количества квадратов на кадр)
      unsigned short* FramesInfos = new unsigned short[hdr.Frames];
      // - грузим данные о кадрах
      for(int f = 0; f < hdr.Frames; f++)
      	fread(&FramesInfos[f], 2, 1, in);

      unsigned int curr_f = 0;
      // - грузим сегменты
      for(int segm = 0; segm < hdr.Segments; segm++)
      {
         unsigned int FramesInSegment = hdr.FramesPerSegment;
         if((segm+1)*hdr.FramesPerSegment > hdr.Frames)
            FramesInSegment = hdr.Frames%hdr.FramesPerSegment;

         // - размер сегмента (на 4 больше из-за outlen)
         int inlen = SegmentsOffsets[segm+1]-SegmentsOffsets[segm];
         unsigned char* inbuf = new unsigned char[inlen];

         // - читаем сегмент
         fseek(in, SegmentsOffsets[segm], SEEK_SET);
         fread(inbuf, inlen, 1, in);

         unsigned int outlen = *(unsigned int*)(inbuf);

         unsigned char* outbuf = new unsigned char[outlen];
         unsigned int*  fbuf   = new unsigned int[FramesInSegment+1];

         ///////////////////////////////////////////////////////////////////////
         if((SegmentsFlags[segm] & G16_PACK_METHOD_MASK) == G16_IDXSTORE)
            isIdx = true;
         if(!G16UnpackSegment(inbuf, inlen-4, outbuf, workbuf, fbuf, FramesInSegment,
                              SegmentsFlags[segm]))
         {
            return false;
         }
         ///////////////////////////////////////////////////////////////////////
         if((SegmentsFlags[segm] & G16_PACK_METHOD_MASK) == G16_IDXSTORE)
            PackMethod = G16_IDXSTORE;
         else
            PackMethod = G16_444STORE;
         ///////////////////////////////////////////////////////////////////////

         // - имеем распакованные квадраты в установленном формате
         fbuf[FramesInSegment] = outlen;
         // - грузим один сегмент
         for(int f = 0; f < FramesInSegment; f++)
         {
            // - координаты левого вернего угла ББокса
            unsigned x2 = 0, x1 = 0xFFFFFFFF;
            unsigned y2 = 0, y1 = 0xFFFFFFFF;

            // - ищем ББокс
            {
               unsigned spos = fbuf[f];   // - указатель на начало квадратов кадра
               unsigned squares = FramesInfos[curr_f + f];
               for(unsigned s = 0; s < squares; s++)
               {
                  // - грузим заголовок квадрата
                  unsigned dd = *(unsigned*)&(outbuf[spos]);
                  unsigned r = 1<<(dd>>28);
                  int x = (dd>>12)&0xFFF;
                  int y = dd&0xFFF;

                  x <<= 20;
                  x >>= 20;
                  y <<= 20;
                  y >>= 20;

                  if(x < x1) x1 = x;
                  if(y < y1) y1 = y;
                  if(x + r > x2) x2 = x + r;
                  if(y + r > y2) y2 = y + r;

                  spos += 8 + r*r*2;
               }
               if(squares == 0)
               {
                  x1 = x2 = 0;
                  y1 = y2 = 0;
               }
            }
            my_assert(x1 <= x2 && y1 <= y2);

            x1 = 0;
            y1 = 0;

            unsigned W = x2 - x1;
            unsigned H = y2 - y1;

            FBitmap* bmp = new FBitmap(W, H);

            unsigned squares = FramesInfos[curr_f + f];

            bmp->OriginX = x1;
            bmp->OriginY = y1;

            unsigned spos = fbuf[f];   // - указатель на начало квадратов кадра
            for(unsigned s = 0; s < squares; s++)
            {
               // - грузим заголовок квадрата
               unsigned dd = *(unsigned*)&(outbuf[spos]);
               spos += 8;
               unsigned r = 1<<(dd>>28);
               unsigned c = (dd>>24)&0xF;
               int x = (dd>>12)&0xFFF;
               int y = dd&0xFFF;

               x <<= 20;
               x >>= 20;
               y <<= 20;
               y >>= 20;

               // - грузим ARGB квадрата
               for(unsigned j = y-y1; j < y-y1+r; j++)
             	{
        	         for(unsigned i = x-x1; i < x-x1+r; i++)
        		      {
            	   	unsigned short col = *(unsigned short*)&(outbuf[spos]);
                     spos += 2;

   	         		int b = (col>>4)&0xF0;
            			int g = col&0xF0;
            			int r = (col<<4)&0xF0;
            			int a = (col>>8)&0xF0;
            			int n = !!((col>>8)&0x10);
                     bmp->PutPixel(i, j, RGBAX(r, g, b, a, 0, n));
                  }
               }
            }
            if(SkipFrameMode){
               if(g16_NSectors>1){
                  int nf=g16_NFrames/g16_NSectors;
                  int cf=curf%nf;
                  if((cf&1)==0)AddFrame(bmp);
                  void log(char* sz,...);
                  //log("loadg16: g16_NFrames=%d g16_NSectors=%d nf=%d cf=%d\n",g16_NFrames,g16_NSectors,nf,cf);
               }else AddFrame(bmp);
            }else AddFrame(bmp);
            curf++;
            delete bmp;
            if(ProgressCur) ProgressCur(curr_f + f);
         }
         curr_f += FramesInSegment;

         delete[] inbuf;
         delete[] outbuf;
         delete[] fbuf;
      }
      delete[] workbuf;
      delete[] SegmentsOffsets;
      delete[] SegmentsFlags;
      delete[] FramesInfos;
   }
   else
   if(hdr.Magic == '61NG')
   {
      fread(&hdr.Frames, 2, 1, in);
      fread(&hdr.MaxWorkbuf, 4, 1, in);
   	fread(&hdr.Segments, 2, 1, in);

      if(ProgressMax) ProgressMax(hdr.Frames);

      unsigned MaxWorkbuf = hdr.MaxWorkbuf;
      if(!MaxWorkbuf) MaxWorkbuf = 1024*1024;
      unsigned char* workbuf = new unsigned char[MaxWorkbuf];

      // - массив ссылок на сегменты (+ 1 для последнего сегмента)
      unsigned int*  SegmentsOffsets   = new unsigned int[hdr.Segments+1];
      unsigned int*  FramesInSegments  = new unsigned int[hdr.Segments];
      unsigned char* SegmentsFlags     = new unsigned char[hdr.Segments];
      // - грузим ссылки на сегменты
      for(int s = 0; s < hdr.Segments; s++)
      {
      	fread(&SegmentsOffsets[s], 4, 1, in);
         SegmentsFlags[s] = SegmentsOffsets[s]&0xF;
         SegmentsOffsets[s] >>= 4;

      	FramesInSegments[s] = 0;
      	fread(&FramesInSegments[s], 2, 1, in);
      }
      SegmentsOffsets[hdr.Segments] = hdr.FileSize;

      // - массив информации о кадрах
      struct FINFO {
      	unsigned short Squares;
      	unsigned short	Width;
      	unsigned short	Height;
      	unsigned short SegmentNumber;
      } *FramesInfos = new FINFO[hdr.Frames];

      // - грузим данные о кадрах
      for(int f = 0; f < hdr.Frames; f++)
      {
      	FramesInfos[f].Squares = 0;
         FramesInfos[f].Width = 0;
      	FramesInfos[f].Height = 0;;
      	FramesInfos[f].SegmentNumber = 0;

      	fread(&FramesInfos[f].Squares, 2, 1, in);
      	fread(&FramesInfos[f].Width, 2, 1, in);
      	fread(&FramesInfos[f].Height, 2, 1, in);
      	fread(&FramesInfos[f].SegmentNumber, 2, 1, in);
      }

      int curr_f = 0;
      // - грузим сегменты
      for(int segm = 0; segm < hdr.Segments; segm++)
      {
         // - размер сегмента
         unsigned inlen = SegmentsOffsets[segm+1]-SegmentsOffsets[segm];
         unsigned char* inbuf = new unsigned char[inlen];

         // - читаем сегмент
         fseek(in, SegmentsOffsets[segm], SEEK_SET);
         fread(inbuf, inlen, 1, in);

         unsigned outlen = *(unsigned*)(inbuf);

         unsigned FramesInSegment = FramesInSegments[segm];

         unsigned char* outbuf = new unsigned char[outlen];
         unsigned* fbuf = new unsigned[FramesInSegment+1];

         ///////////////////////////////////////////////////////////////////////
         if(!G16UnpackSegment(inbuf, inlen-4, outbuf, workbuf, fbuf, FramesInSegment,
                              SegmentsFlags[segm]))
         {
            return false;
         }
         ///////////////////////////////////////////////////////////////////////
         if((SegmentsFlags[segm] & G16_PACK_METHOD_MASK) == G16_IDXSTORE)
            PackMethod = G16_IDXSTORE;
         else
            PackMethod = G16_444STORE;
         ///////////////////////////////////////////////////////////////////////

         fbuf[FramesInSegment] = outlen;

         for(int f = 0; f < FramesInSegment; f++)
         {
            FBitmap* bmp = new FBitmap(FramesInfos[curr_f+f].Width, FramesInfos[curr_f+f].Height);
            int squares = FramesInfos[curr_f+f].Squares;
            unsigned spos = fbuf[f];
            for(int s = 0; s < squares; s++)
            {
               unsigned dd = *(unsigned*)&(outbuf[spos]);
               spos += 8;
               int r = 1<<(dd>>28);
               int c = (dd>>24)&0xF;
               int x = (dd>>12)&0xFFF;
               int y = dd&0xFFF;

               x <<= 20;
               x >>= 20;
               y <<= 20;
               y >>= 20;

               for(int j = y; j < y+r; j++)
             	{
        	         for(int i = x; i < x+r; i++, spos += 2)
        		      {
                     if(j < 0 || i < 0 || j >= bmp->Height || i >= bmp->Width) continue;
            	   	unsigned short col = *(unsigned short*)&(outbuf[spos]);
   	         		int b = (col>>4)&0xF0;
            			int g = col&0xF0;
            			int r = (col<<4)&0xF0;
            			int a = (col>>8)&0xF0;
            			int n = !!((col>>8)&0x10);
                     bmp->PutPixel(i, j, RGBAX(r, g, b, a, 0, n));
                  }
               }
            }
            AddFrame(bmp);
            delete bmp;
            if(ProgressCur) ProgressCur(curr_f + f);
         }
         curr_f += FramesInSegment;

         delete[] inbuf;
         delete[] outbuf;
         delete[] fbuf;
      }
      delete[] workbuf;
      delete[] SegmentsOffsets;
      delete[] FramesInSegments;
      delete[] SegmentsFlags;
      delete[] FramesInfos;
   }
   else
   {
      _ERROR("Wrong G16 file type!");
      fclose(in);
      return false;
   }
   fclose(in);

   if(PackMethod == G16_IDXSTORE)
   {
      ColorsNumber = 256;
      memcpy(PalRGB, G16PalRGB, 1024);
   }
   else
      ColorsNumber = -1;

   return true;
}
//---------------------------------------------------------------------------
bool FConvert::LoadFromG17(const char* FileName, const char* Parameters)
{
   FStream InStream;
   InStream.reload((char*)FileName);

   FStream OutStream;

   FCompressor* Compressor = new FCompressor;
   Compressor->Initialize();
   Compressor->DecompressBlock(&OutStream.Data, &OutStream.Size, InStream.Data);
   delete Compressor;
   OutStream.AllocatedBytes = OutStream.Size;

   char drive[MAXDRIVE];
   char dir[MAXDIR];
   char file[MAXFILE];
   char ext[MAXEXT];
   char fname[MAXPATH];

   fnsplit(FileName, drive, dir, file, ext);

   if(OutStream.Data[3] == '6')
   {
      fnmerge(fname, drive, dir, file, ".G16");

      OutStream.save(fname);
      OutStream.release();

      if(!LoadFromG16(fname, Parameters)) return false;
      unlink(fname);
   }
   else
   {
      _ERROR("Unknown file format!");
      return false;
   }

   return true;
}
//---------------------------------------------------------------------------

