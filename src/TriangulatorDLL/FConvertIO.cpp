//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include <time.h>
#include <dir.h>
#include "FConvert.h"
#include "FCompressor.h"
//---------------------------------------------------------------------------
char* FConvert::GetTextureName(int t, const char* Suffix)
{
   static char buf[_MAX_PATH+1];
   sprintf(buf, "Texture%s-%03u.tga", Suffix, t);
   return buf;
}
//---------------------------------------------------------------------------
bool FConvert::Export(FTriangulator* tr)
{
   for(int f = 0; f < tr->Frames.sizeUsed; f++)
   {
      char FileName[_MAX_PATH+1];
      sprintf(FileName, "Frame%s-%03u.wrl", tr->Suffix, f);

      FILE* out = fopen(FileName, "wt");
      if(!out) return false;

      fprintf(out, "#VRML V2.0 utf8\n\n");
      fprintf(out, "# Produced by Triangulator");

      time_t timer;
      struct tm *tblock;
      timer = time(NULL);
      tblock = localtime(&timer);

      char str[80];
      strcpy(str, asctime(tblock));
      fprintf(out, " %s\n", str);

      FFrame* F = tr->Frames[f];
      for(int i = 0; i < F->Meshes.sizeUsed; i++)
      {
         fprintf(out, "DEF Mesh%03u Transform {\n", i);
         fprintf(out, "   translation 0 0 0\n");
         fprintf(out, "   rotation 0 0 0 0\n");
         fprintf(out, "   children [\n");
         fprintf(out, "   Shape {\n");
         fprintf(out, "      appearance Appearance {\n");
         fprintf(out, "         texture ImageTexture {\n");
         fprintf(out, "            url \"%s\"\n", GetTextureName(F->Meshes[i]->TextureIdx, tr->Suffix));
         fprintf(out, "         }\n");
         fprintf(out, "      }\n");
         fprintf(out, "      geometry DEF Mesh%03u-FACES IndexedFaceSet {\n", i);
         fprintf(out, "         ccw TRUE\n");
         fprintf(out, "         solid TRUE\n");
         fprintf(out, "         coord DEF Mesh%03u-COORD Coordinate { point [\n", i);
         fprintf(out, "            ");

         for(int k = 0; k < F->Meshes[i]->Vertices.sizeUsed; k++)
         {
            int x = F->Meshes[i]->Vertices[k].x;
            int y = F->Meshes[i]->Vertices[k].y;
            int z = F->Meshes[i]->Vertices[k].z;
            fprintf(out, "%d %d %d ", x, -y, z);
         }

         fprintf(out, "]\n         }\n");
         fprintf(out, "         texCoord DEF Mesh%03u-TEXCOORD TextureCoordinate { point [\n", i);
         fprintf(out, "            ");

         float tSide = tr->Textures[F->Meshes[i]->TextureIdx]->Width;
         for(int k = 0; k < F->Meshes[i]->Vertices.sizeUsed; k++)
         {
            FVertex* V = &F->Meshes[i]->Vertices[k];
            fprintf(out, "%.4f %.4f ", ((float)V->tx+0.5)/(float)tSide,
                                      -((float)V->ty+0.5)/(float)tSide);
         }

         fprintf(out, "]\n         }\n");
         fprintf(out, "         coordIndex [\n");
         fprintf(out, "            ");

         for(int k = 0; k < F->Meshes[i]->Triangles.sizeUsed; k++)
            fprintf(out, "%u %u %u -1 ", F->Meshes[i]->Triangles[k].v0,
                                         F->Meshes[i]->Triangles[k].v1,
                                         F->Meshes[i]->Triangles[k].v2);
         fprintf(out, "]\n");
         fprintf(out, "         texCoordIndex [\n");
         fprintf(out, "            ");

         for(int k = 0; k < F->Meshes[i]->Triangles.sizeUsed; k++)
            fprintf(out, "%u %u %u -1 ", F->Meshes[i]->Triangles[k].v0,
                                         F->Meshes[i]->Triangles[k].v1,
                                         F->Meshes[i]->Triangles[k].v2);
         fprintf(out, "]\n");
         fprintf(out, "         }\n");
         fprintf(out, "      }\n");
         fprintf(out, "   ]\n");
         fprintf(out, "}\n");
      }
      fclose(out);
   }
   return true;
}
void log(char* sz,...)
{
	FILE* F=fopen("log.txt","a");
	if(F){
		va_list va;
		va_start( va, sz );
		vfprintf ( F, sz, va );
		va_end( va );
                fclose(F);
	}
}
extern bool SkipFrameMode;
extern int g16_NFrames;
extern int g16_NSectors;
void log(char* sz,...);
int LastFrameEfficiency=0;
//---------------------------------------------------------------------------
bool FConvert::SaveToFile(const char* FileName, FTriangulator* tr)
{
   //log("saving %s\n",FileName);
   FStream FIStream;
   FStream VStream;
   FStream IStream;
   FStream MStream;
   FStream TIStream;
   FStream TStream;

   LastFrameEfficiency = 0;

   int sizeofV = 8;//10;
   if(isCorrectLevels) sizeofV += 4;
   const int sizeofF = 12;
   const int sizeofI = 2;
   const int sizeofM = 18;
   const int sizeofT = 8*4;

   int MaxW = 0, MaxH = 0;
   int TexturesNumber = tr->Textures.sizeUsed-1;
   int RealNFrames=tr->Frames.sizeUsed;
   if(g16_NSectors>1){
      RealNFrames=g16_NFrames;
   }
   //int mod=SkipFrameMode?2:1;

   FIStream.reload(RealNFrames*sizeofF);
   FIStream.Size = FIStream.Pos = RealNFrames*sizeofF;
   //log("nframes=%d s=%d f=%d\n",tr->Frames.sizeUsed,g16_NSectors,g16_NFrames);

   int cf=0;
   // - запись кадров
   for(int f = 0; f < tr->Frames.sizeUsed; f++)
   {
      FFrame* F = tr->Frames[f];

      // - сохраним позицию начала мешей для кадра
      int MeshesOffset = MStream.Pos/sizeofM;
      int minX=10000;
      int minY=10000;
      int maxX=-10000;
      int maxY=-10000;

      // - запись мешей кадра
      for(int m = 0; m < F->Meshes.sizeUsed; m++)
      {
         FMesh* M = F->Meshes[m];
         my_assert(M->TextureIdx != -1);

         // - индекс текстуры
         MStream.putshort(M->TextureIdx);
         // - смещение внутри _глобального_ вершинного буфера
         MStream.putint(VStream.Pos/sizeofV);
         // - число вершин в меше
         MStream.putint(M->Vertices.sizeUsed);
         // - смещение внутри _глобального_ индексного буфера
         MStream.putint(IStream.Pos/sizeofI);
         // - число треугольников в меше
         MStream.putint(M->Triangles.sizeUsed);

         // - запись вершин в общий поток
         for(int v = 0; v < M->Vertices.sizeUsed; v++)
         {
            FVertex* V = &M->Vertices[v];

            if(V->x>maxX)maxX=V->x;
            if(V->y>maxY)maxY=V->y;
            if(V->x<minX)minX=V->x;
            if(V->x<minY)minY=V->y;

            VStream.putshort((short)V->x);
            VStream.putshort((short)V->y);
            //VStream.putshort((short)V->z);
            VStream.putshort((short)V->tx);
            VStream.putshort((short)V->ty);
            //VStream.putc(V->tx);
            //VStream.putc(V->ty);
            if(isCorrectLevels)
            {
               VStream.putc((unsigned char)V->r);
               VStream.putc((unsigned char)V->g);
               VStream.putc((unsigned char)V->b);
               VStream.putc(255);
            }
         }

         // - запись индексов вершин (треугольников)
         for(int t = 0; t < M->Triangles.sizeUsed; t++)
         {
            FTriangle* T = &M->Triangles[t];
            IStream.putshort((short)T->v0);
            IStream.putshort((short)T->v1);
            IStream.putshort((short)T->v2);
         }
      }
      if(maxX<minX){
         minX=0;
         minY=0;
         maxX=0;
         maxY=0;
      }
      if(SkipFrameMode && g16_NSectors>1){
         if(!((g16_NFrames/g16_NSectors)&1)){
            FIStream.putshort(f*sizeofF*2+0, MeshesOffset);
            FIStream.putshort(f*sizeofF*2+2, F->Meshes.sizeUsed);
            FIStream.putshort(f*sizeofF*2+4, minX);
            FIStream.putshort(f*sizeofF*2+6, minY);
            FIStream.putshort(f*sizeofF*2+8, F->Width);
            FIStream.putshort(f*sizeofF*2+10, F->Height);

            FIStream.putshort(f*sizeofF*2+12, MeshesOffset);
            FIStream.putshort(f*sizeofF*2+14, F->Meshes.sizeUsed);
            FIStream.putshort(f*sizeofF*2+16, minX);
            FIStream.putshort(f*sizeofF*2+18, minY);
            FIStream.putshort(f*sizeofF*2+20, F->Width);
            FIStream.putshort(f*sizeofF*2+22, F->Height);
         }else{
            FIStream.putshort(cf*sizeofF+0, MeshesOffset);
            FIStream.putshort(cf*sizeofF+2, F->Meshes.sizeUsed);
            FIStream.putshort(cf*sizeofF+4, minX);
            FIStream.putshort(cf*sizeofF+6, minY);
            FIStream.putshort(cf*sizeofF+8, F->Width);
            FIStream.putshort(cf*sizeofF+10,F->Height);
            cf++;
            int nf=tr->Frames.sizeUsed/g16_NSectors;
            int rf=f%nf;
            if(rf<nf-1){
               FIStream.putshort(cf*sizeofF+0, MeshesOffset);
               FIStream.putshort(cf*sizeofF+2, F->Meshes.sizeUsed);
               FIStream.putshort(cf*sizeofF+4, minX);
               FIStream.putshort(cf*sizeofF+6, minY);
               FIStream.putshort(cf*sizeofF+8, F->Width);
               FIStream.putshort(cf*sizeofF+10,F->Height);
               cf++;
            }
            //log("rf=%d nf=%d f=%d curcf=%d\n x0=%d y0=%d W=%d H=%d FW=%d FH=%d",rf,nf,f,cf,minX,minY,maxX+1,maxY+1,F->Width,F->Height);
         }
      }else{
         FIStream.putshort(f*sizeofF+0, MeshesOffset);
         FIStream.putshort(f*sizeofF+2, F->Meshes.sizeUsed);
         FIStream.putshort(f*sizeofF+4, minX);
         FIStream.putshort(f*sizeofF+6, minY);
         FIStream.putshort(f*sizeofF+8, F->Width);
         FIStream.putshort(f*sizeofF+10,F->Height);
      }
      if(F->Width > MaxW) MaxW = F->Width;
      if(F->Height > MaxH) MaxH = F->Height;
   }
   //log("cf=%d\n",cf);
   // - запись текстур
   for(int k = 0; k < TexturesNumber; k++)
   {
      FBitmap* bmp = tr->Textures[k];
      FStream pTStream;

      int PackMode = 0;
      ColorsNumber=-1;
      if(ColorsNumber != -1)
      {
         // - calculate colors number
         while(ColorsNumber > 0 && *(int*)&PalRGB[(ColorsNumber-1)*4] == 0) ColorsNumber--;
         if(ColorsNumber <= 0) ColorsNumber = 1;

         // - store the palette
         pTStream.putc(ColorsNumber-1);
         for(int i = 0; i < ColorsNumber; i++)
         {
            pTStream.putc(PalRGB[i*4+0]);
            pTStream.putc(PalRGB[i*4+1]);
            pTStream.putc(PalRGB[i*4+2]);
         }

         if(ColorsNumber > 32)
         {
            // - store the indexed colors
            for(int j = 0; j < bmp->Height; j++)
            {
               for(int i = 0; i < bmp->Width; i++)
               {
                  RGBAX& col = bmp->GetPixel(i, j);
                  int r = col.b;
                  int g = col.g;
                  int b = col.r;

                  int Idx = FindPalRGBElement(r, g, b);
                  pTStream.putc(Idx);
               }
            }

            // - store the alphas
            for(int j = 0; j < bmp->Height; j++)
            {
               for(int i = 0; i < bmp->Width; i += 2)
               {
                  RGBAX& col1 = bmp->GetPixel(i, j);
                  RGBAX& col2 = bmp->GetPixel(i+1, j);
                  int a1 = col1.a;
                  int a2 = col2.a;
                  pTStream.putc(a1|(a2>>4));
               }
            }
            PackMode = 1;
         }
         else
         {
            // - store the indexed colors
            int NLen = 0;
            int NPos = -1;
            for(int j = 0; j < bmp->Height; j++)
            {
               for(int i = 0; i < bmp->Width; i++)
               {
                  RGBAX& col = bmp->GetPixel(i, j);
                  int r = col.b;
                  int g = col.g;
                  int b = col.r;
                  int a = col.a;
                  int n = col.n;
                  if(n)
                  {
                     if(NLen < 31 && NPos >= 0)
                     {
                        my_assert((pTStream.Data[NPos]>>3) == NLen);
                        my_assert((pTStream.Data[NPos]&3) == 0);
                        pTStream.Data[NPos] += 1<<3;
                        NLen++;
                     }
                     else
                     {
                        NLen = 1;
                        NPos = pTStream.Pos;
                        pTStream.putc(NLen<<3);
                     }
                  }
                  else
                  {
                     NLen = 0;
                     NPos = -1;
                  }
                  int Idx = FindPalRGBElement(r, g, b);
                  if(a == 0) Idx = 0;
                  Idx = (Idx<<3)|(a>>5);
                  pTStream.putc(Idx);
               }
            }
            PackMode = 2;
         }
      }
      else
      {
         for(int j = 0; j < bmp->Height; j++)
         {
            for(int i = 0; i < bmp->Width; i++)
            {
               RGBAX& col = bmp->GetPixel(i, j);
               int r = col.b;
               int g = col.g;
               int b = col.r;
               int a = col.a;
               int n = col.n;
               if(k == TexturesNumber-1){
                  if(a>0)LastFrameEfficiency++;
               }
               //a = (a&0xE0)|(!!n<<4);
               a=(a>>1)+((!!n)<<7);
               pTStream.putc(b);
               pTStream.putc(g);
               pTStream.putc(r);
               pTStream.putc(a);
            }
         }
      }

      // - pack the texture
      {
         //FCompressor* Compressor = new FCompressor(FCOMPRESSOR_TYPE_FLZ);
         FCompressor* Compressor = new FCompressor(FCOMPRESSOR_TYPE_DXT3);

         unsigned char* outbuf = NULL;
         unsigned outlen = 0;
         Compressor->CompressBlock(&outbuf, &outlen, pTStream.Data, pTStream.Size);
         my_assert(outbuf && outlen);

         pTStream.release();
         pTStream.attach(outbuf, outlen);

         delete Compressor;
      }

      TIStream.putshort(bmp->Width);
      TIStream.putshort(bmp->Height);
      TIStream.putc(FCOMPRESSOR_TYPE_FLZ);
      TIStream.putc(PackMode);
      TIStream.putshort(0);
      TIStream.putint(TStream.Pos);
      TIStream.putint(pTStream.Size);
      TIStream.putint(0x7FFFFFFF);
      TIStream.putint(0x7FFFFFFF);
      TIStream.putint(0x7FFFFFFF);
      TIStream.putint(0x7FFFFFFF);

      TStream.putblock(pTStream.Data, pTStream.Size);
   }

   FStream OutStream;

   OutStream.putint('D2PG');
   OutStream.putint(0);             // - FileSize
   OutStream.putint(RealNFrames);
   OutStream.putint(0);             // - VerticesOffset
   OutStream.putint(VStream.Pos/sizeofV);
   OutStream.putint(0);             // - IndicesOffset
   OutStream.putint(IStream.Pos/sizeofI);
   OutStream.putint(0);             // - MeshesOffset
   OutStream.putint(MStream.Pos/sizeofM);
   OutStream.putint(0);             // - TexturesInfoOffset
   OutStream.putint(0);             // - TexturesOffset
   OutStream.putint(TIStream.Pos/sizeofT);
   OutStream.putint(2+2+1+1);       // - InfoLen
   OutStream.putshort(MaxW);        // - Width
   OutStream.putshort(MaxH);        // - Height
   OutStream.putc(Directions);      // - Directions
   OutStream.putc(isCorrectLevels); // - Vertex format

   OutStream.putblock(FIStream.Data, FIStream.Size);
   OutStream.putint(9*4, OutStream.Pos);
   OutStream.putblock(TIStream.Data, TIStream.Size);
   OutStream.putint(3*4, OutStream.Pos);
   OutStream.putblock(VStream.Data,  VStream.Size);
   OutStream.putint(5*4, OutStream.Pos);
   OutStream.putblock(IStream.Data,  IStream.Size);
   OutStream.putint(7*4, OutStream.Pos);
   OutStream.putblock(MStream.Data,  MStream.Size);
   OutStream.putint(10*4, OutStream.Pos);
   OutStream.putblock(TStream.Data,  TStream.Size);
   OutStream.putint(1*4, OutStream.Pos);

   OutStream.save((char*)FileName);

   if(TexturesNumber<3)LastFrameEfficiency=65536;

   return true;
}
//---------------------------------------------------------------------------
int G2DUnpackTexture(unsigned char* InData, unsigned short* OutData, int W, int H, int Flags)
{
   int CompressMode = Flags&0xFF;
   int PackMode = Flags>>8;

   // - декомпрессированная текстура (индекс или rgb)
   unsigned char* TextData = NULL;
   unsigned int TextLen = W*H*2;
   bool isAttached = false;

   // - декомпрессия текстуры
   if(CompressMode != FCOMPRESSOR_TYPE_STORE)
   {
      static FCompressor Compressor;
      Compressor.DecompressBlock(&TextData, &TextLen, InData);
   }
   else
   {
      TextData = InData;
      isAttached = true;
   }
   //log("G2DUnpackTexture: size=%d PackMode=%d W=%d H=%d\n",TextLen,PackMode,W,H);

   int ColorsNumber = -1;

   // - распаковка текстуры
   if(PackMode == 0)
   {
      //log("ARGB unpack\n");
      // - ARGB mode
      //memcpy(OutData, TextData, W*H*2);
      int sz=W*H;
      for(int i=0;i<sz;i++){
         int b=TextData[i*4  ];
         int g=TextData[i*4+1];
         int r=TextData[i*4+2];
         int a=TextData[i*4+3]*2;
         if(a>255){
            a-=255;
            r=r*a/255;
            g=g*a/255;
            b=b*a/255;
            r+=a;
            if(r>255)r=255;
            a=255;
         }
         a&=255-0x10;
         OutData[i]=((a>>4)<<12)+((r>>4)<<8)+((g>>4)<<4)+(b>>4);
      }
   }
   else
   {
      // - indexed mode
      int TextPos = 0;

      // - грузим палитру
      static unsigned char PalRGB[1024];
      ColorsNumber = (int)TextData[TextPos++]+1;
      for(int i = 0; i < ColorsNumber; i++)
      {
         PalRGB[i*4+0] = TextData[TextPos++];
         PalRGB[i*4+1] = TextData[TextPos++];
         PalRGB[i*4+2] = TextData[TextPos++];
         PalRGB[i*4+3] = 0;
      }

      unsigned char* ColorData = TextData + TextPos;
      int PixelsPos = 0, OutPos = 0;

      if(PackMode == 1)
      {
         // - грузим пикселы, альфа отдельно
         unsigned char* AlphaData = ColorData + W*H;
         for(int PixelsPos = 0; PixelsPos < W*H/2; PixelsPos++)
         {
            int Idx1 = (int)ColorData[(PixelsPos<<1)+0]<<2;
            int Idx2 = (int)ColorData[(PixelsPos<<1)+1]<<2;

            int aa = AlphaData[PixelsPos];
            int a1 = aa&0xF0;
            int a2 = (aa<<4)&0xF0;

            int r1 = PalRGB[Idx1+0];
            int g1 = PalRGB[Idx1+1];
            int b1 = PalRGB[Idx1+2];

            int r2 = PalRGB[Idx2+0];
            int g2 = PalRGB[Idx2+1];
            int b2 = PalRGB[Idx2+2];

            OutData[OutPos++] = ((a1<<8)&0xF000)|((r1<<4)&0xF00)|(g1&0xF0)|((b1>>4)&0xF);
            OutData[OutPos++] = ((a2<<8)&0xF000)|((r2<<4)&0xF00)|(g2&0xF0)|((b2>>4)&0xF);
         }
      }
      else
      if(PackMode == 2)
      {
         // - грузим пикселы, альфа упакована вместе с индексами
         int PixelsPos = 0, OutPixels = 0, NLen = 0;
         while(OutPixels < W*H)
         {
            int Idx = (int)ColorData[PixelsPos++];
            int a = (Idx<<5)&0xFF;

            Idx >>= 3;
            if(a == 0 && Idx != 0)
            {
               NLen = Idx;
               continue;
            }
            Idx <<= 2;

            int r = PalRGB[Idx+0];
            int g = PalRGB[Idx+1];
            int b = PalRGB[Idx+2];

            if(NLen)
            {
               a |= 0x10;
               NLen--;
            }

            OutData[OutPos++] = ((a<<8)&0xF000)|((r<<4)&0xF00)|(g&0xF0)|((b>>4)&0xF);
            OutPixels++;
         }
      }
   }

   if(!isAttached)
      delete[] TextData;

   return ColorsNumber;
}
//---------------------------------------------------------------------------
bool FConvert::LoadFromFile(const char* FileName, FTriangulator* tr)
{
   tr->Initialize("");

   int sizeofV = 8;//10;
   const int sizeofI = 2;
   const int sizeofF = 12;//8
   const int sizeofM = 18;

   FStream InStream;
   if(!InStream.reload((char*)FileName)) return false;

   int Magic = InStream.getint();
   if(Magic != 'D2PG')
      return false;

   int FileSize = InStream.getint();
   if((int)InStream.Size != FileSize)
      return false;

   int FramesNumber        = InStream.getint();
   int gVerticesOffset     = InStream.getint();
   int gVerticesNumber     = InStream.getint();
   int gIndicesOffset      = InStream.getint();
   int gIndicesNumber      = InStream.getint();
   int gMeshesOffset       = InStream.getint();
   int gMeshesNumber       = InStream.getint();
   int gTexturesInfoOffset = InStream.getint();
   int gTexturesOffset     = InStream.getint();
   int TexturesNumber      = InStream.getint();

   // - Info loading
   int InfoLen = InStream.getint();
   int MapW = 0;
   int MapH = 0;
   int Directions = 1;
   int VertexFormat = 0;
   if(InfoLen >= 2)
   {
      MapW = InStream.getshort();
      InfoLen -= 2;
   }
   if(InfoLen >= 2)
   {
      MapH = InStream.getshort();
      InfoLen -= 2;
   }
   if(InfoLen >= 1)
   {
      Directions = InStream.getc();
      InfoLen -= 1;
   }
   if(InfoLen >= 1)
   {
      VertexFormat = InStream.getc();
      InfoLen -= 1;
   }

   if(VertexFormat == 1)
   {
      sizeofV += 4;
      isCorrectLevels = true;
   }
   else
      isCorrectLevels = false;

   // - streams attaching
   FStream FIStream, VStream, IStream, MStream, TIStream, TStream;
   FIStream.attach(InStream.Data + InStream.Pos, FramesNumber*sizeofF);
   VStream.attach(InStream.Data + gVerticesOffset, InStream.Size - gVerticesOffset);
   IStream.attach(InStream.Data + gIndicesOffset, InStream.Size - gIndicesOffset);
   MStream.attach(InStream.Data + gMeshesOffset, InStream.Size - gMeshesOffset);
   TIStream.attach(InStream.Data + gTexturesInfoOffset, InStream.Size - gTexturesInfoOffset);
   TStream.attach(InStream.Data + gTexturesOffset, InStream.Size - gTexturesOffset);

   // - загрузка текстур
   for(int k = 0; k < TexturesNumber; k++)
   {
      // - информация о текстуре
      int W = TIStream.getshort();
      int H = TIStream.getshort();
      int Flags = TIStream.getshort();
      int Reserved = TIStream.getshort();
      int TextureOffset = TIStream.getint();
      int TextureSize = TIStream.getint();
      int VerticesOffset = TIStream.getint();
      int VerticesNumber = TIStream.getint();
      int IndicesOffset = TIStream.getint();
      int IndicesNumber = TIStream.getint();

      // - распаковка текстуры
      FStream tStream;
      tStream.reload(W*H*2);
      tStream.Size = W*H*2;
      ColorsNumber = G2DUnpackTexture(TStream.Data + TextureOffset, tStream.usData, W, H, Flags);

      // - создаем текстуру
      FBitmap* bmp = new FBitmap(W, H);
      tr->Textures.AddNULL();
      tr->Textures.pArray[tr->Textures.sizeUsed-1] = tr->Textures.last = bmp;

      // - грузим текстуру в битмап
      for(int j = 0; j < H; j++)
      {
         for(int i = 0; i < W; i++)
         {
            int col = tStream.getshort();
            int r = (col<<4)&0xF0;
            int g = col&0xF0;
            int b = (col>>4)&0xF0;
            int a = (col>>8)&0xF0;
            int n = !!(a&0x10);
            a &= ~0x10;
            bmp->PutPixel(i, j, RGBAX(r, g, b, a, 0, n));
         }
      }
      tr->SaveTexture(k);
   }
   tr->Textures.Add();

   // - загрузка кадров
   for(int f = 0; f < FramesNumber; f++)
   {
      int FMeshesOffset = FIStream.getshort();
     	int FMeshesNumber = FIStream.getshort();
      FIStream.getshort();
      FIStream.getshort();


      FFrame* F = tr->Frames.Add();
      F->Width = FIStream.getshort();
      F->Height = FIStream.getshort();

      F->Bitmap = new FBitmap(MapW+200, MapH+200);
      F->Bitmap->Clear();

      // - аттачим поток мешей
      FStream mStream;
      mStream.attach(MStream.Data + FMeshesOffset*sizeofM, MStream.Size - FMeshesOffset*sizeofM);

      // - грузим меши
      for(int m = 0; m < FMeshesNumber; m++)
      {
         // - информация о меше
         int TextureIdx       = mStream.getshort();
         int mVerticesOffset  = mStream.getint();
         int mVerticesNumber  = mStream.getint();
         int mTrianglesOffset = mStream.getint();
         int mTrianglesNumber = mStream.getint();

         // - аттачим потоки, связанные с данным мешем
         FStream vStream, iStream;
         vStream.attach(VStream.Data + mVerticesOffset*sizeofV, mVerticesNumber*sizeofV);
         iStream.attach(IStream.Data + mTrianglesOffset*sizeofI, mTrianglesNumber*3*sizeofI);

         // - создадим меш
         FMesh* M = F->Meshes.Add();

         // - установим индекс текстуры кадра
         M->TextureIdx = TextureIdx;

         // - грузим меш
         for(int t = 0; t < mTrianglesNumber; t++)
         {
            int v0 = iStream.getshort();
            int v1 = iStream.getshort();
            int v2 = iStream.getshort();

            int x0 = 0, y0 = 0, z0 = 0;
            int x1 = 0, y1 = 0, z1 = 0;
            int x2 = 0, y2 = 0, z2 = 0;

            int tx0 = 0, ty0 = 0;
            int tx1 = 0, ty1 = 0;
            int tx2 = 0, ty2 = 0;

            int a0 = 0, r0 = 0, g0 = 0, b0 = 0;
            int a1 = 0, r1 = 0, g1 = 0, b1 = 0;
            int a2 = 0, r2 = 0, g2 = 0, b2 = 0;

            if(VertexFormat == 0)
            {

               x0  = vStream.sData[v0*4+0];
               y0  = vStream.sData[v0*4+1];
               z0  = 0;//vStream.sData[v0*5+2];
               tx0 = vStream.sData[v0*4+2];
               ty0 = vStream.sData[v0*4+3];

               x1  = vStream.sData[v1*4+0];
               y1  = vStream.sData[v1*4+1];
               z1  = 0;//vStream.sData[v1*5+2];
               tx1 = vStream.sData[v1*4+2];
               ty1 = vStream.sData[v1*4+3];

               x2  = vStream.sData[v2*4+0];
               y2  = vStream.sData[v2*4+1];
               z2  = 0;//vStream.sData[v2*5+2];
               tx2 = vStream.sData[v2*4+2];
               ty2 = vStream.sData[v2*4+3];
               /*
               x0  = vStream.sData[v0*3+0];
               y0  = vStream.sData[v0*3+1];
               z0  = 0;//vStream.sData[v0*5+2];
               tx0 = vStream.cData[v0*6+4];
               ty0 = vStream.cData[v0*6+5];

               x1  = vStream.sData[v1*3+0];
               y1  = vStream.sData[v1*3+1];
               z1  = 0;//vStream.sData[v1*5+2];
               tx1 = vStream.cData[v1*6+4];
               ty1 = vStream.cData[v1*6+5];

               x2  = vStream.sData[v2*3+0];
               y2  = vStream.sData[v2*3+1];
               z2  = 0;//vStream.sData[v2*5+2];
               tx2 = vStream.cData[v2*6+4];
               ty2 = vStream.cData[v2*6+5];
               */
            }
            else
            {
               FStream _vStream;
               _vStream.attach((unsigned char*)&vStream.sData[v0*7], sizeofV);
               x0  = _vStream._getshort();
               y0  = _vStream._getshort();
               z0  = _vStream._getshort();
               tx0 = _vStream._getshort();
               ty0 = _vStream._getshort();
               r0  = _vStream._getc();
               g0  = _vStream._getc();
               b0  = _vStream._getc();
               a0  = _vStream._getc();

               _vStream.attach((unsigned char*)&vStream.sData[v1*7], sizeofV);
               x1  = _vStream._getshort();
               y1  = _vStream._getshort();
               z1  = _vStream._getshort();
               tx1 = _vStream._getshort();
               ty1 = _vStream._getshort();
               r1  = _vStream._getc();
               g1  = _vStream._getc();
               b1  = _vStream._getc();
               a1  = _vStream._getc();

               _vStream.attach((unsigned char*)&vStream.sData[v2*7], sizeofV);
               x2  = _vStream._getshort();
               y2  = _vStream._getshort();
               z2  = _vStream._getshort();
               tx2 = _vStream._getshort();
               ty2 = _vStream._getshort();
               r2  = _vStream._getc();
               g2  = _vStream._getc();
               b2  = _vStream._getc();
               a2  = _vStream._getc();
            }

            v0 = M->StoreVertex(x0, y0, z0);
            v1 = M->StoreVertex(x1, y1, z1);
            v2 = M->StoreVertex(x2, y2, z2);

            M->Vertices[v0].tx = tx0;
            M->Vertices[v0].ty = ty0;
            M->Vertices[v0].a  = a0;
            M->Vertices[v0].r  = r0;
            M->Vertices[v0].g  = g0;
            M->Vertices[v0].b  = b0;
            M->Vertices[v1].tx = tx1;
            M->Vertices[v1].ty = ty1;
            M->Vertices[v1].a  = a1;
            M->Vertices[v1].r  = r1;
            M->Vertices[v1].g  = g1;
            M->Vertices[v1].b  = b1;
            M->Vertices[v2].tx = tx2;
            M->Vertices[v2].ty = ty2;
            M->Vertices[v2].a  = a2;
            M->Vertices[v2].r  = r2;
            M->Vertices[v2].g  = g2;
            M->Vertices[v2].b  = b2;

            M->StoreEdge(x0, y0, x1, y1);
            M->StoreEdge(x1, y1, x2, y2);
            M->StoreEdge(x2, y2, x0, y0);
            M->StoreTriangle(v0, v1, v2);

            if(isCorrectLevels)
            {
               F->PutCorrectedTriangle(&M->Vertices[v0],
                                       &M->Vertices[v1],
                                       &M->Vertices[v2],
                                       tx0-x0, ty0-y0,
                                       tr->Textures[M->TextureIdx]);
            }
            else
            {
               F->PutTexturedTriangle(&M->Vertices[v0], &M->Vertices[v1], &M->Vertices[v2],
                                      tr->Textures[M->TextureIdx], F->Bitmap);
//               F->PutTriangle(x0, y0, x1, y1, x2, y2,
//                  tx0-x0, ty0-y0, tr->Textures[F->TextureIdx]);
            }
         }
      }
   }

   return true;
}
//---------------------------------------------------------------------------

