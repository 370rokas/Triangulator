//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "FConvert.h"
//---------------------------------------------------------------------------
FBitmap* FConvert::ConvertA24(const char* FileName)
{
   FILE *in;
   if((in = fopen(FileName, "rb")) == NULL)
   {
      _ERROR("ERROR loading A24 format: File open error %s!", FileName);
      return NULL;
   }

	int Lx, Ly;
	unsigned char KeyR, KeyG, KeyB, zz;

	fread(&Lx, 4, 1, in);
	if(Lx != 'AR24')
   {
      _ERROR("ERROR loading A24 format: Not an A24 file format: %s!", FileName);
      return NULL;
	}

	fread(&Lx, 4, 1, in);
	fread(&Ly, 4, 1, in);
  	fread(&KeyR, 1, 1, in);
   fread(&KeyG, 1, 1, in);
 	fread(&KeyB, 1, 1, in);
	fread(&zz, 1, 1, in);

   fseek(in, 0, SEEK_END);
	int DSize = ftell(in)-16;
   fseek(in, 16, SEEK_SET);

	unsigned char* _NData = new unsigned char[DSize];
	fread(_NData, DSize, 1, in);

	int DstLen = Lx*Ly*3;
	unsigned char* Data = new unsigned char[Lx*Ly*3];

	int APos = 0;
	int NPos = 0;
	int Cmd, Len;

	while(APos < DSize && NPos < DstLen)
   {
		Cmd = _NData[APos];
		if(!(Cmd&0x80))
      {
			//pix. data
			if(Cmd&0x40)
         {
				//Long data
				Cmd=((Cmd&0x3F)<<8) + _NData[APos+1];
				APos += 2;
				Len = Cmd*3;
				memcpy(Data+NPos, _NData+APos, Len);
				APos += Len;
				NPos += Len;
			}
         else
         {
				APos++;
				Len = (Cmd&0x7F)*3;
				memcpy(Data+NPos, _NData+APos, Len);
				APos += Len;
				NPos += Len;
			}
		}
      else
      {
			//key color
			if(Cmd&0x40)
         {
				//Long data
				Cmd = ((Cmd&0x3F)<<8)+_NData[APos+1];
				APos += 2;
				Len = Cmd;
				for(int i=0; i < Len; i++)
            {
					Data[NPos]=KeyR;
					Data[NPos+1]=KeyG;
					Data[NPos+2]=KeyB;
					NPos+=3;
				}
			}
         else
         {
				APos++;
				Len=Cmd&0x3F;
				for(int i=0;i<Len;i++)
            {
					Data[NPos]=KeyR;
					Data[NPos+1]=KeyG;
					Data[NPos+2]=KeyB;
					NPos+=3;
				}
			}
		}
	}
   delete[] _NData;
	fclose(in);

   //---------------------------------------------------------------------------
   FBitmap* bmp = new FBitmap(Lx, Ly);

   // - set the data
   for(int j = 0; j < Ly; j++)
   {
      for(int i = 0; i < Lx; i++)
      {
         unsigned pos = j*Lx*3 + i*3;
         bmp->PutPixel(i, j, RGBAX(Data[pos+0], Data[pos+1], Data[pos+2], 255, 0));
      }
   }
   delete[] Data;
   return bmp;
}
//---------------------------------------------------------------------------

