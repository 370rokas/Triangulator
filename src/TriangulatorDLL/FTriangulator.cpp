//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "FTriangulator.h"
//---------------------------------------------------------------------------
int MAX_TEXTURE_SIZE = 0;
//---------------------------------------------------------------------------
void FTriangulator::Initialize(char* suffix)
{
   strcpy(Suffix, suffix);
   Frames.RemoveAll();
   Textures.RemoveAll();
   OpaquePixels = 0, AllPixels = 0;
   if(g_isAnimation)
      MAX_TEXTURE_SIZE = 256; //128
   else
      MAX_TEXTURE_SIZE = 256;
}
//---------------------------------------------------------------------------
void FTriangulator::AddFrame(const FBitmap* bmp, const FBitmap* bmpZ, double Scale)
{
   Frames.Add();
   Frames.last->isCorrectLevels = isCorrectLevels;
   Frames.last->Triangulate(bmp, bmpZ, Scale);
}
//---------------------------------------------------------------------------
FBitmap* FTriangulator::CreateTexture(int Mul)
{
   int tSide = 0;
/*   if(g_isAnimation)
   {
      int Pixels = 0;
      for(int f = 0; f < Frames.sizeUsed; f++)
      {
         FFrame* F = Frames[f];
         for(int m = 0; m < F->Meshes.sizeUsed; m++)
         {
            if(F->Meshes[m]->TextureIdx == -1)
               Pixels += F->Meshes[m]->OpaquePixels;
         }
      }

      // - выбор размера квадрата, целиком содержащего данные
      tSide = MAX_TEXTURE_SIZE;
      while(tSide > 32 && Pixels <= tSide*tSide/4)
         tSide >>= 1;

      // - оптимальное разбиение выбранного квадрата при малом количестве данных
      while(tSide*tSide - Pixels >= 32*32)
         tSide >>= 1;

      if(Mul)
         tSide <<= Mul;
   }
   else
*/   
      tSide = MAX_TEXTURE_SIZE;

   if(tSide > MAX_TEXTURE_SIZE) tSide = MAX_TEXTURE_SIZE;

   FBitmap* bmp = new FBitmap(tSide, tSide);
   bmp->Clear();
   return bmp;
}
//---------------------------------------------------------------------------
void FTriangulator::SaveTexture(int t)
{
   FBitmap* bmp = Textures[t];
   for(int j = 0; j < bmp->Height; j++)
   {
      for(int i = 0; i < bmp->Width; i++)
      {
         RGBAX col = bmp->GetPixel(i, j);
         OpaquePixels += !!col.a;
         AllPixels++;
      }
   }
}
//---------------------------------------------------------------------------
bool isClearTexture = true;
bool FTriangulator::PackMeshes(FFrame* F)
{
   struct _wrap {
      static int meshes_sort_function(const void *a, const void *b)
      {
         FMesh* M1 = *(FMesh**)a;
         FMesh* M2 = *(FMesh**)b;
         if(M1->Area > M2->Area) return -1;
         if(M1->Area < M2->Area) return 1;
         return 0;
      }
   };
   qsort(F->Meshes.pArray, F->Meshes.sizeUsed, 4, _wrap::meshes_sort_function);

   for(int m = 0; m < F->Meshes.sizeUsed; m++)
   {
      FMesh* M = F->Meshes[m];
      M->DeltaTextX = 0x7FFFFFFF;
      M->DeltaTextY = 0x7FFFFFFF;
   }

   FBitmap* bmp = Textures.last;

   srand(0x1234567);

   int Mul = 0;
   while(1)
   {
      int NonPackedNumber = 0;
      for(int m = 0; m < F->Meshes.sizeUsed; m++)
      {
         FMesh* M = F->Meshes[m];
         if(M->DeltaTextX != 0x7FFFFFFF) continue;
         int x = 0, y = 0;
         int K = F->TryToPut(bmp, x, y, M);
         if(K >= 0)
         {
            F->PutMeshMap(bmp, x, y, M);
            M->DeltaTextX = x;
            M->DeltaTextY = y;
            M->TextureIdx = Textures.sizeUsed-1;
            isClearTexture = false;
         }
         else
            NonPackedNumber++;
      }
      if(F->Meshes.sizeUsed == 0) isClearTexture = false;

      if(isClearTexture)
      {
         if(bmp->Width >= MAX_TEXTURE_SIZE)
         {
            if(isClearTexture)
               assert(!isClearTexture);
         }
         else
         {
            bmp = CreateTexture(++Mul);
            Textures.AddNULL();
            Textures.pArray[Textures.sizeUsed-1] = Textures.last = bmp;
            isClearTexture = true;
         }
      }
      else
      {
         if(NonPackedNumber == 0) break;
         bmp = CreateTexture();
         Textures.AddNULL();
         Textures.pArray[Textures.sizeUsed-1] = Textures.last = bmp;
         isClearTexture = true;
         Mul = 0;
      }
   }

   // - окончательный просчет всех координат
   for(int m = 0; m < F->Meshes.sizeUsed; m++)
   {
      FMesh* M = F->Meshes[m];

      for(int i = 0; i < M->Vertices.sizeUsed; i++)
      {
         int x = M->Vertices[i].x;
         int y = M->Vertices[i].y;

         int tx = x + M->DeltaTextX - F->Bitmap->OriginX;
         int ty = y + M->DeltaTextY - F->Bitmap->OriginY;

         M->Vertices[i].tx = tx;
         M->Vertices[i].ty = ty;

         M->Vertices[i].x = x = (int)((double)x*F->Scale);
         M->Vertices[i].y = y = (int)((double)y*F->Scale);
         M->Vertices[i].z = F->GetZ(x, y);

         if(F->Width < x) F->Width = x;
         if(F->Height < y) F->Height = y;
      }
   }

   return true;
}
//---------------------------------------------------------------------------
void FTriangulator::PackFrames(void(*ProgressHookFn)(int))
{
   if(Frames.sizeUsed == 0) return;
   AllPixels = OpaquePixels = 0;

   Textures.RemoveAll();
   for(int f = 0; f < Frames.sizeUsed; f++)
   {
      if(isCorrectLevels)
         Frames[f]->CorrectLevels();
   }

   FBitmap* bmp = CreateTexture();
   Textures.AddNULL();
   Textures.pArray[Textures.sizeUsed-1] = Textures.last = bmp;
   isClearTexture = true;

   if(!g_isAnimation)
   {
      // - идем по всем кадрам
      for(int f = 0; f < Frames.sizeUsed; f++)
      {
         // - пытаемся упаковать кадр
         FFrame* F = Frames[f];
         PackMeshes(F);
         if(ProgressHookFn) ProgressHookFn(f);
      }
      SaveTexture(Textures.sizeUsed-1);
      bmp = CreateTexture();
      Textures.AddNULL();
      Textures.pArray[Textures.sizeUsed-1] = Textures.last = bmp;
   }
   else
   {
      // - идем по всем кадрам
      int PackedNumber = 0;
      bool isClearTexture = true;
      for(int f = 0; f < Frames.sizeUsed; f++)
      {
         // - пытаемся упаковать кадр
         FFrame* F = Frames[f];
         bool res = F->PackMeshes(bmp);

         if(!res)
         {
            // - пытаемся паковать разбитый кадр
            FFrame* F1 = F->CloneSplitted();
            res = F1->PackMeshes(bmp);
            if(res)
            {
               delete F;
               F = Frames.pArray[f] = F1;
               Frames.last = Frames.pArray[Frames.sizeUsed-1];
               isClearTexture = false;
            }
            else
               delete F1;
         }
         else
            isClearTexture = false;

         if(!res)
         {
            if(isClearTexture)
            {
               _ERROR("Cannot pack: texture size is too small");
               return;
            }
            SaveTexture(Textures.sizeUsed-1);
            bmp = CreateTexture();
            Textures.AddNULL();
            Textures.pArray[Textures.sizeUsed-1] = Textures.last = bmp;
            isClearTexture = true;
            f--;
         }
         else
         {
            for(int m = 0; m < F->Meshes.sizeUsed; m++)
               F->Meshes[m]->TextureIdx = Textures.sizeUsed-1;
            if(ProgressHookFn) ProgressHookFn(f);
            PackedNumber++;
            if(PackedNumber >= Frames.sizeUsed) break;
         }
      }
      bmp = CreateTexture();
      Textures.AddNULL();
      Textures.pArray[Textures.sizeUsed-1] = Textures.last = bmp;
   }
}
//---------------------------------------------------------------------------

