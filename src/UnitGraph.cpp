//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "FBitmap.h"
#include "FScreen.h"
#include "FTriangulator.h"
//---------------------------------------------------------------------------
extern FScreen* myScreen;
//---------------------------------------------------------------------------
extern FTriangulator Triangulator;
extern FTriangulator TriangulatorS;
extern int CurrentFrameIdx;
extern int CurrentTextureIdx;
//---------------------------------------------------------------------------
void DrawFrame(HDC Handle, FFrame* F, FFrame* FS, FBitmap* Texture, bool isMeshDraw, bool isZDraw)
{
   FBitmap* bmp = F->Bitmap;

   int W = F->Width - bmp->OriginX*2;
   int H = F->Height - bmp->OriginY*2;

   if(!W || !H) return;

   // - создаем экран
   if(myScreen)
   {
      if(myScreen->Width != W || myScreen->Height != H)
      {
         delete myScreen;
         myScreen = NULL;
      }
   }
   if(myScreen == NULL)
      myScreen = new FScreen(W, H, Handle);

   myScreen->Clear(RGB(128, 128, 128));

   for(int j = 0; j < bmp->Height; j++)
   {
      for(int i = 0; i < bmp->Width; i++)
      {
         int x = i;
         int y = j;

         if(x >= 0 && x < myScreen->Width && y >= 0 && y < myScreen->Height)
         {
            RGBAX& col = bmp->GetPixel(i, j);

            int r, g, b, a, n;
            if(isZDraw)
            {
               r = g = b = col.z;
               a = col.a;
               n = 0;
            }
            else
            {
               r = col.r;
               g = col.g;
               b = col.b;
               a = col.a;
               n = col.n;
            }

            if(n)
            {
               r = 0;
               g = 0xE0*a/0xE0;
               b = 0;
               a = 255;
            }
            if(a != 255)
            {
               int foneB = 128;
               int foneG = 128;
               int foneR = 128;

               r = foneR-(a*(foneR-r)>>8);
               g = foneG-(a*(foneG-g)>>8);
               b = foneB-(a*(foneB-b)>>8);
               if(r > 255) r = 255;
               if(g > 255) g = 255;
               if(b > 255) b = 255;
            }
            myScreen->PutPixel(x, y, RGB(r, g, b));
         }
      }
   }
   if(isMeshDraw)
   {
      for(int m = 0; m < F->Meshes.sizeUsed; m++)
      {
         FMesh* M = F->Meshes[m];
         for(int i = 0; i < M->Triangles.sizeUsed; i++)
         {
            int v0 = M->Triangles[i].v0;
            int v1 = M->Triangles[i].v1;
            int v2 = M->Triangles[i].v2;

            int x0 = M->Vertices[v0].x - bmp->OriginX;
            int y0 = M->Vertices[v0].y - bmp->OriginY;
            int x1 = M->Vertices[v1].x - bmp->OriginX;
            int y1 = M->Vertices[v1].y - bmp->OriginY;
            int x2 = M->Vertices[v2].x - bmp->OriginX;
            int y2 = M->Vertices[v2].y - bmp->OriginY;

//            myScreen->AddTriangle(x0, y0, x1, y1, x2, y2, RGB(0, 50, 50));
            myScreen->AddLine(x0, y0, x1, y1, RGB(0, 255, 255));
            myScreen->AddLine(x1, y1, x2, y2, RGB(0, 255, 255));
            myScreen->AddLine(x2, y2, x0, y0, RGB(0, 255, 255));
         }
         for(int i = 0; i < M->Vertices.sizeUsed; i++)
         {
            int x = M->Vertices[i].x - bmp->OriginX;
            int y = M->Vertices[i].y - bmp->OriginY;

            unsigned col = RGB(0, 0, 255);
            myScreen->PutPixel(x, y, col);
            if(x > 0)
               myScreen->PutPixel(x-1, y, col);
            if(x < myScreen->Width)
               myScreen->PutPixel(x+1, y, col);
            if(y > 0)
               myScreen->PutPixel(x, y-1, col);
            if(y < myScreen->Height)
               myScreen->PutPixel(x, y+1, col);
         }
      }
   }
}
//---------------------------------------------------------------------------
int DrawTexture(HDC Handle, FBitmap* bmp)
{
   if(bmp == NULL) return 0;

   int W = bmp->Width*2;
   int H = bmp->Height;
   int AlphaNulls = 0;

   // - создаем экран
   if(myScreen)
   {
      if(myScreen->Width != W || myScreen->Height != H)
      {
         delete myScreen;
         myScreen = NULL;
      }
   }
   if(myScreen == NULL)
      myScreen = new FScreen(W, H, Handle);

   for(int j = 0; j < H; j++)
   {
      for(int i = 0; i < bmp->Width; i++)
      {
         RGBAX& col = bmp->GetPixel(i, j);
         int r = col.r;
         int g = col.g;
         int b = col.b;
         int a = col.a;
         int x = !!col.x*255;

         if(a != 255)
         {
            int foneB = 128;
            int foneG = 128;
            int foneR = 128;

            r = foneR-(a*(foneR-r)>>8);
            g = foneG-(a*(foneG-g)>>8);
            b = foneB-(a*(foneB-b)>>8);

            if(r > 255) r = 255;
            if(g > 255) g = 255;
            if(b > 255) b = 255;
         }
         if(a == 0)
            AlphaNulls++;

         myScreen->PutPixel(i, j, RGB(r, g, b));
         if(x == 0)
            myScreen->PutPixel(i + bmp->Width, j, RGB(0, 0, 188));
         else
            myScreen->PutPixel(i + bmp->Width, j, RGB(a, a, a));
      }
   }
   return AlphaNulls;
}
//---------------------------------------------------------------------------

