#include "FMM\\FMM.h"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <math.h>
#include <direct.h>
#include <string.h>
#include <stdio.h>
#include <mem.h>
#include "FBitmap.h"
//----------------------------------------------------------------------------
bool FBitmap::LoadFromFile(const char* FileName)
{
   char drive[MAXDRIVE];
   char dir[MAXDIR];
   char file[MAXFILE];
   char ext[MAXEXT];

   fnsplit(FileName, drive, dir, file, ext);

   bool res = true;
   if(stricmp(ext, ".bmp") == 0)
      res = LoadBMP(FileName);
   else
   if(stricmp(ext, ".tga") == 0)
      res = LoadTGA(FileName);
   else
      res = false;

   return res;
}
//----------------------------------------------------------------------------
bool FBitmap::SaveToFile(const char* FileName)
{
   char drive[MAXDRIVE];
   char dir[MAXDIR];
   char file[MAXFILE];
   char ext[MAXEXT];

   fnsplit(FileName, drive, dir, file, ext);

   bool res = true;
   if(stricmp(ext, ".bmp") == 0)
      res = SaveBMP(FileName);
   else
   if(stricmp(ext, ".tga") == 0)
      res = SaveTGA(FileName);
   else
      res = false;

   return res;
}
//----------------------------------------------------------------------------
// BMP section
//----------------------------------------------------------------------------
struct BMPHeader
{
 	short	Type;			// type of file, must be 'BM'
  	int	Size;			// size of file in bytes
  	short	Reserved1, Reserved2;
  	int	OffBits;		// offset from this header to actual data
};

struct BMPInfoHeader
{
  	long	Size;
  	long	Width;			// width of bitmap in pixels
  	long	Height;			// height of bitmap in pixels
	short	Planes;			// # of planes
	short	BitCount;		// bits per pixel
	long	Compression;	// type of compression, BI_RGB - no compression
	long	SizeImage;		// size of image in bytes
	long	XPelsPerMeter;	// hor. resolution of the target device
	long	YPelsPerMeter;	// vert. resolution
	long	ClrUsed;
	long	ClrImportant;
};
//----------------------------------------------------------------------------
bool FBitmap::LoadBMP(const char* fname)
{
	BMPHeader Header;
	BMPInfoHeader InfoHeader;

	memset(&Header, 0, sizeof(Header));
	memset(&InfoHeader, 0, sizeof(InfoHeader));

	int file = open(fname, O_RDONLY | O_BINARY);
	if(file == -1)
	{
      printf("File %s open error!", fname);
		return false;
	}
	// read header data
	read(file, &Header.Type, 2);
	read(file, &Header.Size, 4);
	read(file, &Header.Reserved1, 2);
	read(file, &Header.Reserved2, 2);
	read(file, &Header.OffBits, 4);

	read(file, &InfoHeader.Size, 4);
	read(file, &InfoHeader.Width, 4);
	read(file, &InfoHeader.Height, 4);
	read(file, &InfoHeader.Planes, 2);
	read(file, &InfoHeader.BitCount, 2);
	read(file, &InfoHeader.Compression, 4);
	read(file, &InfoHeader.SizeImage, 4);
	read(file, &InfoHeader.XPelsPerMeter, 4);
	read(file, &InfoHeader.YPelsPerMeter, 4);
	read(file, &InfoHeader.ClrUsed, 4);
	read(file, &InfoHeader.ClrImportant, 4);

	if(InfoHeader.BitCount != 24)
	{
      printf("Bitmap %s must be 24 bpp!", fname);
		return false;
	}

   Width = InfoHeader.Width;
   Height = InfoHeader.Height;

   int size = Width*Height;
   int linelen = (Width*3+3)&~3;

	if(Data) delete Data;
   Data = new RGBAX[size];

	lseek(file, Header.OffBits, SEEK_SET);
   unsigned char* t_buf = new unsigned char[linelen];

   for(int y = Height-1; y >= 0; y--)
   {
      read(file, t_buf, linelen);
      for(int x = 0; x < Width; x++)
		{
         unsigned offs = y*Width + x;
      	Data[offs].r = t_buf[x*3+0];
      	Data[offs].g = t_buf[x*3+1];
      	Data[offs].b = t_buf[x*3+2];
		}
   }
   delete[] t_buf;
	close(file);
	return true;
}
//----------------------------------------------------------------------------
bool FBitmap::SaveBMP(const char* FileName)
{
   FILE *out;
	BMPHeader Hdr;
	BMPInfoHeader InfoHdr;

   if((out = fopen(FileName, "wb")) == NULL)
   {
      printf("Cannot create bitmap file %s!", FileName);
      return false;
   }

	Hdr.Type = 'BM';
	Hdr.Size = 14+40+((Width*3+3)&~3)*Height;
	Hdr.Reserved1 = 0;
	Hdr.Reserved2 = 0;
	Hdr.OffBits = 14+40;

	InfoHdr.Size = 0x28;
	InfoHdr.Width = Width;
	InfoHdr.Height = Height;
	InfoHdr.Planes = 1;
	InfoHdr.BitCount = 24;
	InfoHdr.Compression = 0;
	InfoHdr.SizeImage = ((Width*3+3)&~3)*Height;
	InfoHdr.XPelsPerMeter = 0;
	InfoHdr.YPelsPerMeter = 0;
	InfoHdr.ClrUsed = 0;
	InfoHdr.ClrImportant = 0;

	// write header data
	fwrite(&Hdr.Type, 2, 1, out);
	fwrite(&Hdr.Size, 4, 1, out);
	fwrite(&Hdr.Reserved1, 2, 1, out);
	fwrite(&Hdr.Reserved2, 2, 1, out);
	fwrite(&Hdr.OffBits, 4, 1, out);

	fwrite(&InfoHdr.Size, 4, 1, out);
	fwrite(&InfoHdr.Width, 4, 1, out);
	fwrite(&InfoHdr.Height, 4, 1, out);
	fwrite(&InfoHdr.Planes, 2, 1, out);
	fwrite(&InfoHdr.BitCount, 2, 1, out);
	fwrite(&InfoHdr.Compression, 4, 1, out);
	fwrite(&InfoHdr.SizeImage, 4, 1, out);
	fwrite(&InfoHdr.XPelsPerMeter, 4, 1, out);
	fwrite(&InfoHdr.YPelsPerMeter, 4, 1, out);
	fwrite(&InfoHdr.ClrUsed, 4, 1, out);
	fwrite(&InfoHdr.ClrImportant, 4, 1, out);

   int linelen = (Width*3+3)&~3;
   unsigned char* buf = new unsigned char[linelen];

   for(int y = Height-1; y >= 0; y--)
   {
      for(int x = 0; x < Width; x++)
      {
         unsigned offs = y*Width + x;
      	buf[x*3+0] = Data[offs].r;
      	buf[x*3+1] = Data[offs].g;
   		buf[x*3+2] = Data[offs].b;
      }
      fwrite(buf, linelen, 1, out);
   }
   delete[] buf;
	fclose(out);
   return true;
}
//----------------------------------------------------------------------------
// TGA section
//----------------------------------------------------------------------------
/*
	byte ImageIDLength;
	byte ColorMapType;
	byte ImageType;
	//begin ColorMapSpecification field
		unsigned short int FirstEntryIndex;
		unsigned short int ColorMapEntryCount;
		byte ColorMapEntrySize;
	//end ColorMapSpecification field
	//begin ImageSpecification field
		unsigned short int XOrigin;
		unsigned short int YOrigin;
		unsigned short int Width;
		unsigned short int Height;
		byte PixelDepth;
		//begin ImageDescriptor bit-field
			byte AlphaChannelBits : 4;
			byte LeftToRight : 1;
			byte BottomToTop : 1;
			byte UnusedImageDescriptor : 2;
		//end ImageDescriptor bit-field
	//end ImageSpecification field
*/
//----------------------------------------------------------------------------
#pragma pack(1)
struct TGAHeader {
	BYTE		lenImageID;	// длина поля ImageID
	BYTE		colMapType;	// 00h
	BYTE		compress;	// Image type compressed
								// TRUE COLOR= 02h,	RLE comp = 0Ah
	BYTE		mapSpec[5];	// Map specification =00h (?)
   WORD     XOrigin;
	WORD     YOrigin;
	WORD		width;		// [0Ch]
	WORD		height;		// [0Eh]
	BYTE     bpp;
   union {
      struct {
      	BYTE  AlphaChannelBits : 4;
   		BYTE  isRight : 1;
   		BYTE  isTop : 1;
   		BYTE  UnusedImageDescriptor : 2;
      };
      BYTE Bits;
   };
};
struct TGAFooter {
   DWORD ExtAreaOffset;
   DWORD DevDirOffset;
   BYTE  Signature[18];
};
#pragma pack()
//------------------------------------------------------------------------------
bool FBitmap::LoadTGA(const char* FileName)
{
	int file = open(FileName, O_RDONLY | O_BINARY);
	TGAHeader Hdr;
   TGAFooter Ftr;
   bool isPreMultAlpha = false;

	if(file == -1)
   {
//      FGA_ERROR("File open error %s!", FileName);
      return false;
   }

   lseek(file, -26, SEEK_END);
	read(file, &Ftr, 26);

   if(memcmp(Ftr.Signature, "TRUEVISION-XFILE.", 18) == 0)
   {
      if(Ftr.ExtAreaOffset)
      {
         lseek(file, Ftr.ExtAreaOffset + 494, SEEK_SET);
         BYTE b = 0;
      	read(file, &b, 1);
//         if(b == 4)
//            isPreMultAlpha = true;
      }
   }
   lseek(file, 0, SEEK_SET);

	read(file, &Hdr, 18);

	if(Hdr.compress != 0x2)
   {
//      FGA_ERROR("Targa must be true color!");
      return false;
   }
	if(Hdr.bpp != 32)
   {
//      FGA_ERROR("Targa must be 32 bpp!");
      return false;
   }
   /////////////////////////// Loading image /////////////////////////////////
   Width = Hdr.width;
   Height = Hdr.height;

	int linelen = Width*4;
   int size = Width*Height;

	if(Data) delete Data;
   Data = new RGBAX[size];

	lseek(file, Hdr.lenImageID, SEEK_CUR);
   unsigned char* buf = new unsigned char[linelen];

   #define STOREPIXEL(pp) {\
      unsigned r = buf[pp*4+0];\
      unsigned g = buf[pp*4+1];\
      unsigned b = buf[pp*4+2];\
      unsigned a = buf[pp*4+3];\
      if(isPreMultAlpha && a)\
      {\
         r = r*255/a;\
         g = g*255/a;\
         b = b*255/a;\
         if(r > 255) r = 255;\
         if(g > 255) g = 255;\
         if(b > 255) b = 255;\
      }\
    	pos[pp].r = r;\
    	pos[pp].g = g;\
    	pos[pp].b = b;\
    	pos[pp].a = a;\
    	pos[pp].x = 0;\
   }

   if(Hdr.isTop)
   {
      if(Hdr.isRight)
      {
         for(int y = 0; y < Height; y++)
         {
            read(file, buf, linelen);
            RGBAX* pos = Data + y*Width;

            for(int x = Width-1; x >= 0; x--)
               STOREPIXEL(x);
         }
      }
      else
      {
         for(int y = 0; y < Height; y++)
         {
            read(file, buf, linelen);
            RGBAX* pos = Data + y*Width;

            for(int x = 0; x < Width; x++)
               STOREPIXEL(x);
         }
      }
   }
   else
   {
      if(Hdr.isRight)
      {
         for(int y = Height-1; y >= 0; y--)
         {
            read(file, buf, linelen);
            RGBAX* pos = Data + y*Width;

            for(int x = Width-1; x >= 0; x--)
               STOREPIXEL(x);
         }
      }
      else
      {
         for(int y = Height-1; y >= 0; y--)
         {
            read(file, buf, linelen);
            RGBAX* pos = Data + y*Width;

            for(int x = 0; x < Width; x++)
               STOREPIXEL(x);
         }
      }
   }
   delete[] buf;
	close(file);

   return true;
}
//--------------------------------------------------------------------------
bool FBitmap::SaveTGA(const char* FileName)
{
   FILE *out;
   if((out = fopen(FileName, "wb")) == NULL)
   {
      printf("Cannot create bitmap file %s!", FileName);
      return false;
   }

   TGAHeader Hdr;
   memset(&Hdr, 0, sizeof(Hdr));
   Hdr.compress = 2;
   Hdr.width = Width;
   Hdr.height = Height;
   Hdr.bpp = 32;
   Hdr.Bits = 8;

	// write header data
	fwrite(&Hdr, sizeof(Hdr), 1, out);

   int linelen = (Width*4+3)&~3;
   unsigned char* buf = new unsigned char[linelen];

   for(int y = Height-1; y >= 0; y--)
   {
      for(int x = 0; x < Width; x++)
      {
         unsigned offs = y*Width + x;
      	buf[x*4+0] = Data[offs].r;
      	buf[x*4+1] = Data[offs].g;
   		buf[x*4+2] = Data[offs].b;
   		buf[x*4+3] = Data[offs].a;
      }
      fwrite(buf, linelen, 1, out);
   }
   delete[] buf;
	fclose(out);
   return true;
}
//--------------------------------------------------------------------------

