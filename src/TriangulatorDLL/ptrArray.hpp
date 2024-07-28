#ifndef _TMPPTRARRAY_H
#define _TMPPTRARRAY_H

// - динамический массив с хранением указателей на объекты

#include "FMM\\FMM.h"
#include <memory.h>
#include <stdio.h>
#include <assert.h>

template <class T>
class ptrArray {
//-----------------------------------------------------------------------------------
   int curDelta;
public:
   int sizeUsed;
   int sizeFull;
   T* last;

   typedef T* pT;
   T** pArray;

   //-----------------------------------------------------------------------
   ptrArray()
   {
		last = NULL;
      pArray = NULL;
      Reset(4096, 4096);
   }
   //-----------------------------------------------------------------------
   virtual ~ptrArray()
   {
      RemoveAll();
   }
   //-----------------------------------------------------------------------
   T* operator[](int pos) const
   {
		if(pos < 0)
			return NULL;
      assert(pos < sizeUsed);
      return pArray[pos];
   }
   //-----------------------------------------------------------------------
   bool Reset(int size, int delta = 0)   // - очистка массива с инициализацией
   {
   	RemoveAll();

	   if(size != 0)
   	{
   		pArray = new pT[size];
   		if(!pArray) return false;
   		memset(pArray, 0, sizeof(T*)*size);
   	}
   	curDelta = delta;
   	sizeUsed = 0; 
   	sizeFull = size;
   	return true;
   }
   //-----------------------------------------------------------------------
   bool Expand(void)
   {
	   if(sizeFull == sizeUsed)
   	{
   		int newSize = sizeFull + curDelta + 1;
   		T** p = new pT[newSize];
   		if(!p) return false;
	   	memset(p, 0, sizeof(T*)*newSize);
   		memcpy(p, pArray, sizeof(T*)*sizeUsed);
   		delete pArray;
   		pArray = p;
   		sizeFull = newSize;
   	}
      return true;
   }
   //-----------------------------------------------------------------------
   T* AddNULL(void)
   {
      if(!Expand()) return NULL;

      pArray[sizeUsed] = NULL;
		last = pArray[sizeUsed++];
   	return last;
   }
   //-----------------------------------------------------------------------
   T* Add(void)
   {
      if(!Expand()) return NULL;

      pArray[sizeUsed] = new T;
		last = pArray[sizeUsed++];
   	return last;
   }
   //-----------------------------------------------------------------------
   T* Add(T* pointer)
   {
      if(!Add()) return NULL;
      memcpy(last, pointer, sizeof(T));
   	return last;
   }
   //-----------------------------------------------------------------------
   T* GetAt(int pos)
   {
      if(pos < 0 || sizeUsed <= pos) return 0;
      return pArray[pos];
   }
   //-----------------------------------------------------------------------
   bool InsertAt(int pos, T *pointer)
   {
      int i;
      if(pos < sizeUsed)
      {
         if(!Expand()) return false;

         for(i = sizeUsed; i > pos; i--)
   		  memcpy((pArray+i), (pArray+i-1), sizeof(T*));

         memcpy(pArray[pos], pointer, sizeof(T));
         sizeUsed++;
      }
      else
      if(pos == sizeUsed)
      {
         return Add(pointer);
      }
      else return false;
      return true;
   }
   //-----------------------------------------------------------------------
   bool Remove(int pos)
   {
      if(pos < 0 || sizeUsed <= pos) return false;

      delete pArray[pos];
      pArray[pos] = NULL;

      for(int i = pos; i < (sizeUsed-1); i++)
      memmove((pArray+i), (pArray+i+1), sizeof(T*));

      sizeUsed--;
      return true;
   }
   //-----------------------------------------------------------------------
   bool Remove(T *pointer)
   {
      int pos = Find(pointer, 0);
      if(pos == -1) return false;

      return Remove(pos);
   }
   //-----------------------------------------------------------------------
   void RemoveAll()
   {
      if(!pArray) return;
		for(int i = 0; i < sizeUsed; i++)
			if(pArray[i]) { delete pArray[i]; pArray[i] = NULL; }
		delete[] pArray;
      pArray = NULL;
      sizeFull = 0;
      sizeUsed = 0;
   }
   //-----------------------------------------------------------------------
   int Find(T *pointer, int first = 0)
   {
	   for(int i = first; i < sizeUsed; i++)
   	{
   		if(memcmp(pArray[i], pointer, sizeof(T)) == 0) return i;
   	}
   	return -1;
   }
   //-----------------------------------------------------------------------
};
#endif
