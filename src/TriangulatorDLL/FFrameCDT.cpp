//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "FFrame.h"
#include "cdt\\quadedge.h"
//---------------------------------------------------------------------------
extern int MAX_TEXTURE_SIZE;
//---------------------------------------------------------------------------
void FFrame::CDT(FNode* Contour)
{
   struct _wrap {
      static void edges_callback(void *arg, void *edge, bool isConstrained)
      {
         FFrame* parent = (FFrame*)arg;
         cdt::Edge* E = (cdt::Edge*)edge;

         int x0 = floor(E->Org()->x);
         int y0 = floor(E->Org()->y);
         int x1 = floor(E->Dest()->x);
         int y1 = floor(E->Dest()->y);

         parent->Meshes.last->StoreEdge(x0, y0, x1, y1);
      }
   };

   // - выделение ребер
   cdt::Point2d p0(1 + Bitmap->OriginX, 1 + Bitmap->OriginY),
                p1(Bitmap->Width-2 + Bitmap->OriginX, 1 + Bitmap->OriginY),
                p2(Bitmap->Width-2 + Bitmap->OriginX, Bitmap->Height-2 + Bitmap->OriginY),
                p3(1 + Bitmap->OriginX, Bitmap->Height-2 + Bitmap->OriginY);
   cdt::Mesh* M = new cdt::Mesh(p0, p1, p2, p3);

   for(int i = 0; i < UserSites.sizeUsed; i++)
   {
      p0.x = UserSites[i].x;
      p0.y = UserSites[i].y;
      M->InsertSite(p0);
   }

   for(int j = 0; j < Bitmap->Height; j += MAX_TEXTURE_SIZE/4-1)
   {
      for(int i = 0; i < Bitmap->Width; i += MAX_TEXTURE_SIZE/4-1)
      {
         if(Bitmap->GetPixel(i, j).a)
         {
            p0.x = i + Bitmap->OriginX;
            p0.y = j + Bitmap->OriginY;
            M->InsertSite(p0);
         }
      }
   }

   FNode* n0 = Contour;
   FNode* n1 = Contour->Next;
   while(n0 != n1)
   {
      if(!(n0->x == n1->x && n0->y == n1->y))
      {
         p0.x = n0->x; p0.y = n0->y;
         p1.x = n1->x; p1.y = n1->y;
         // - вставка ребра
         M->InsertEdge(p0, p1);
         Meshes.last->StoreEdge(p0.x, p0.y, p1.x, p1.y, 0);
      }
      n0 = n1; n1 = n1->Next;
      if(n0 == Contour) break;
   }

   // - выгрузить ребра
   M->ApplyEdges(_wrap::edges_callback, this);
   delete M;
}
//---------------------------------------------------------------------------
void FFrame::CDT(void)
{
   // - создаем CDT
   cdt::Point2d p0(1 + Bitmap->OriginX, 1 + Bitmap->OriginY),
                p1(Bitmap->Width-2 + Bitmap->OriginX, 1 + Bitmap->OriginY),
                p2(Bitmap->Width-2 + Bitmap->OriginX, Bitmap->Height-2 + Bitmap->OriginY),
                p3(1 + Bitmap->OriginX, Bitmap->Height-2 + Bitmap->OriginY);

   cdt::Mesh* cdtM = new cdt::Mesh(p0, p1, p2, p3);
   FMesh* M1 = new FMesh;

   // - воссоздаем меш
   FMesh* M = RepairMesh();

   // - загружаем ребра контура в CDT и M1
   for(int e = 0; e < M->Edges.sizeUsed; e++)
   {
      FEdge* E = &M->Edges[e];
      if(E->contourId != -1)
      {
         p0.x = M->Vertices[E->v0].x;
         p0.y = M->Vertices[E->v0].y;
         p1.x = M->Vertices[E->v1].x;
         p1.y = M->Vertices[E->v1].y;
         cdtM->InsertEdge(p0, p1);
         M1->StoreEdge(p0.x, p0.y, p1.x, p1.y, 0);
      }
   }

   // - загрузка внутренних вершин в UserSites
   if(!isInternalSitesLoaded)
   {
      for(int v = 0; v < M->Vertices.sizeUsed; v++)
      {
         int x = M->Vertices[v].x;
         int y = M->Vertices[v].y;
         int vv = M->SearchVertex(x, y);
         if(vv != -1)
         {
            bool isFound = false;
            for(int i = 0; i < UserSites.sizeUsed; i++)
            {
               if(UserSites[i].x == x && UserSites[i].y == y)
               {
                  isFound = true;
                  break;
               }
            }
            if(!isFound)
            {
               UserSites.Add();
               UserSites.last->x = x;
               UserSites.last->y = y;
            }
         }
      }
      isInternalSitesLoaded = true;
   }

   // - загружаем дополнительные вершины
   for(int i = 0; i < UserSites.sizeUsed; i++)
   {
      p0.x = UserSites[i].x;
      p0.y = UserSites[i].y;
      cdtM->InsertSite(p0);
   }

   // - загружаем вершины решетки
   /*
   for(int j = 0; j < Bitmap->Height; j += MaxEdgeLen)
   {
      for(int i = 0; i < Bitmap->Width; i += MaxEdgeLen)
      {
         if(Bitmap->GetPixel(i, j).a)
         {
            p0.x = i;
            p0.y = j;
            cdtM->InsertSite(p0);
         }
      }
   }
   */

   // - удаляем меши
   Meshes.RemoveAll();
   delete M;

   // - вставка нового меша
   Meshes.AddNULL();
   Meshes.pArray[0] = Meshes.last = M1;

   struct _wrap {
      static void edges_callback(void *arg, void *edge, bool isConstrained)
      {
         FFrame* parent = (FFrame*)arg;
         cdt::Edge* E = (cdt::Edge*)edge;

         int x0 = floor(E->Org()->x);
         int y0 = floor(E->Org()->y);
         int x1 = floor(E->Dest()->x);
         int y1 = floor(E->Dest()->y);

         parent->Meshes.last->StoreEdge(x0, y0, x1, y1);
      }
   };
   // - выгрузить ребра
   cdtM->ApplyEdges(_wrap::edges_callback, this);
   delete cdtM;
}
//---------------------------------------------------------------------------

