#include <windows.h>
#include <stdio.h>              
#include <stdlib.h>             
#include <assert.h>             
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include "FMManager.h"
//-----------------------------------------------------------------------------------
#ifndef _NO_FMM_
FMManager MManager;
#endif
//-----------------------------------------------------------------------------------
bool FMManager::isInit = false;
#ifdef FMM_LOCKING_ENABLE
	bool FMManager::isLockingEnable = true;
#endif
#ifdef FMM_BOUNDSCHECKING_ENABLE
	unsigned char FMManager::MemoryMask[(FMM_DEFAULTMEMSIZE+7)>>3];
#endif
//-----------------------------------------------------------------------------------
#define STARFORCE
#define INMUTEX \
	HANDLE HMUT = CreateMutex(0, 0, "FMM_MUTEX");\
	WaitForSingleObject(HMUT, INFINITE);
#define OUTMUTEX ReleaseMutex(HMUT); CloseHandle(HMUT);
//-----------------------------------------------------------------------------------
#ifdef FMM_BOUNDSCHECKING_ENABLE
	LPTOP_LEVEL_EXCEPTION_FILTER FMM_OrigFilter;
	LONG __stdcall FMM_PageFaultExceptionFilter(EXCEPTION_POINTERS* pEx);
#endif
//-----------------------------------------------------------------------------------
bool FMManager::Initialize(unsigned totalSize)
{
	if(isInit) return true;
	#ifdef FMM_BOUNDSCHECKING_ENABLE
		FMM_OrigFilter = SetUnhandledExceptionFilter(FMM_PageFaultExceptionFilter);
		memset(MemoryMask, 0, (FMM_DEFAULTMEMSIZE+7)>>3);
	#endif

	hMutex = CreateMutex(0, 1, "fex_memory_manager");
	int err = GetLastError();
	assert(hMutex != INVALID_HANDLE_VALUE);

	ZeroMemory(&HeapBlock, sizeof(HeapBlock));
	ZeroMemory(&MemoryStatus, sizeof(MemoryStatus));
	ZeroMemory(HashTableA, sizeof(HashTableA));
	ZeroMemory(HashTableS, sizeof(HashTableS));
	InvalidHandlers = NULL;
	Handlers = NULL;
	handlers = 0;
	HeapEnd = NULL;
	PageSize = 0;
	PhysSize = 0;
	isAllocateAllPages = true;

	FMM_LOG_OPEN();

	SYSTEM_INFO sSysInfo; 
	GetSystemInfo(&sSysInfo);
	GlobalMemoryStatus(&MemoryStatus);

	PageSize = sSysInfo.dwPageSize;
	PhysSize = (unsigned)((unsigned __int64)MemoryStatus.dwAvailPhys*FMM_PHYSMEM_PERCENT)/100;

	HeapBlock.BaseAddress = NULL;
	while(totalSize >= FMM_MINMEM_RESERVE)
	{
		HeapBlock.BaseAddress = 
			(FMPTR)VirtualAlloc(NULL, totalSize, MEM_RESERVE, PAGE_NOACCESS);
		if(!HeapBlock.BaseAddress)
			totalSize -= 1024*1024*20;
		else
			break;
	}
	if(!HeapBlock.BaseAddress)
	{
		FMM_ERROR("VirtualAlloc heap reserve failed\nNot enought memory to run program");
		return false;
	}
	HeapBlock.TotalSize = totalSize;
	HeapBlock.TotalPages = (HeapBlock.TotalSize + PageSize-1)/PageSize;
	HeapEnd = HeapBlock.BaseAddress;

	// - распределяем блок хендлов
	unsigned hSize = FMM_MAXHANDLERS*sizeof(Handlers[0]);
	Handlers = (FMMHANDLER*)VirtualAlloc(NULL, hSize, MEM_COMMIT, PAGE_READWRITE);
	
	HANDLERS_OPEN();
	{
		ZeroMemory(Handlers, hSize);
		handlers = 1;
		Handlers[0].BaseAddress = HeapBlock.BaseAddress;
		Handlers[0].TotalSize = HeapBlock.TotalSize;
		Handlers[0].Next = NULL;
		Handlers[0].Prev = NULL;
		Handlers[0].SubNextA = NULL;
		Handlers[0].SubNextS = NULL;
		Handlers[0].SubPrevS = NULL;
		Handlers[0].SubPrevA = NULL;
		Handlers[0].isFree = true;
		FMMADDHASHS(Handlers);
	}
	HANDLERS_CLOSE();

	isInit = true;
	return true;
}
//-----------------------------------------------------------------------------------
bool FMManager::Release(void)
{
	if(!isInit) 
		return false;
	FMM_LOG_CLOSE();
	isInit=false;
	return !!VirtualFree((void*)HeapBlock.BaseAddress, 0, MEM_RELEASE);
}
//-----------------------------------------------------------------------------------
// Logging routines
//-----------------------------------------------------------------------------------
bool FMManager::FMM_LOG_OPEN(void)
{
	#ifdef FMM_LOGGING_ENABLE
	if((logfile = fopen(FMM_LOGFILENAME, "wt")) == NULL)
	{
		MessageBox(NULL, FMM_LOGFILENAME, "Cannot open log file", MB_OK);
		ExitProcess(-1);
	}
	#endif
	return true;
}
//-----------------------------------------------------------------------------------
bool FMManager::FMM_LOG_CLOSE(void)
{
	#ifdef FMM_LOGGING_ENABLE
	if(!logfile)
		return false;
	fclose(logfile);
	#endif
	return true;
}
//-----------------------------------------------------------------------------------
void FMManager::FMM_ERROR(char* str, ...) 
{
	if(!str)	return;
	static char buf[1024];

	va_list args;
	va_start(args, str);
	sprintf(buf, str, args);
	va_end(args);
	MessageBox(NULL, buf, "FMMANAGER ERROR!", MB_OK);
}
//-----------------------------------------------------------------------------------
void FMManager::FMM_LOG(char* str, ...) 
{
	#ifdef FMM_LOGGING_ENABLE
	if(!str)	return;

	va_list args;
	va_start(args, str);
	vfprintf(logfile, str, args);
	vprintf(str, args);
	fflush(logfile);
	va_end(args);
	#endif
}
//-----------------------------------------------------------------------------------
// Allocation routines
//-----------------------------------------------------------------------------------
FMManager::FMMHANDLER* FMManager::GetFreeHandler(void)
{
	FMMHANDLER* h = NULL;
	if(InvalidHandlers)
	{
		h = InvalidHandlers;
		InvalidHandlers = InvalidHandlers->Next;
	}
	else
	{
		if(handlers >= FMM_MAXHANDLERS)
		{
			FMM_ERROR("There are too much handlers: %u!\n", handlers);
			return NULL;
		}
		h = &Handlers[handlers++];
		h->r2 = handlers-1;
	}
	#ifdef FMM_LOCKING_ENABLE
		h->Locked = 0;
	#endif
	return h;
}
//-----------------------------------------------------------------------------------
FMManager::FMMHANDLER* FMManager::GetHandlerByAddress(FMPTR addr)
{
	FMMHANDLER* _h = NULL;
	FMMHANDLER* h = NULL;

	// - ищем по адресу среди занятых блоков
	unsigned hash_idx = FMMHASHA(addr), chain_idx = 0;

	_h = HashTableA[hash_idx];
	while(_h)
	{
		if(_h->BaseAddress == addr)
		{
			h = _h;
			break;
		}
		_h = _h->SubNextA;
		chain_idx++;
	}
	if(!h)
	{
		FMM_LOG("Cannot find block with address 0x%08x!\n", addr);
		return NULL;
	}
	return h;
}
/*-----------------------------------------------------------------------------------
	Выделение хендла происходит путем нахождения хендла подходящего размера в цепочках
таблицы размеров, начиная с индекса, полученного из запрашиваемого размера.
	Нет необходимости выравнивания - до двух хендлов. Первый (левый) хендл становится 
выделенным, и он помещается в таблицу адресов. Второй (правый) хендл становится пустым 
и помещается в таблицу размеров.
/----------------------------------------------------------------------------------*/
FMManager::FMMHANDLER* FMManager::AddHandler(unsigned size)
{
	assert(handlers);

	FMMHANDLER* _h = NULL;
	FMMHANDLER* h = NULL;

	unsigned chain_idx = 0;

	// - ищем хендл блока достаточного размера
	for(unsigned i = FMMHASHS(size); i < FMM_MAXHASHITEMS; i++)
	{
		chain_idx = 0;
		_h = HashTableS[i];
		while(_h)
		{
			// - внутри цепочки, идущей из таблицы HashTableS, все хендлы пустые
			if(_h->TotalSize >= size)
			{
				h = _h;
				break;
			}
			_h = _h->SubNextS;
			chain_idx++;
		}
		if(h) break;
	}
	if(!h)
	{
		FMM_LOG("Cannot find block of %u bytes size!\n", size);
		return NULL;
	}
	// - требуется удалить найденный хендл из таблицы размеров
	DELETE_HS(h);

	if(h->TotalSize > size)
	{
		// - получим еще один хендл, который будет соответствовать остатку блока
		_h = GetFreeHandler();
		if(!_h) return NULL;
	
		_h->BaseAddress = h->BaseAddress + size;
		_h->TotalSize = h->TotalSize - size;
		_h->isFree = true;
		h->TotalSize = size;

		FMMHANDLER* h0 = h;
		FMMHANDLER* h1 = _h;
		FMMHANDLER* h2 = h->Next;
		
		h0->Next = h1;
		h1->Next = h2;
		h1->Prev = h0;
		if(h2)
			h2->Prev = h1;

		// - помещаем пустой хендл в таблицу размеров
		FMMADDHASHS(_h);
	}
	// - помещаем занятый хендл в таблицу адресов
	FMMADDHASHA(h);

	return h;
}
/*-----------------------------------------------------------------------------------
	Выделение хендла происходит путем нахождения хендла подходящего размера в цепочках
таблицы размеров, начиная с индекса, полученного из запрашиваемого размера.
Необходимо выравнивание - до трех хендлов. Первый свободный, второй выделенный, третий 
свободный.
/----------------------------------------------------------------------------------*/
FMManager::FMMHANDLER* FMManager::AddPageAlignedHandler(unsigned size)
{
	assert(handlers);

	size = ((size + PageSize-1)/PageSize)*PageSize;
	FMMHANDLER* _h = NULL;
	FMMHANDLER* h = NULL;

	unsigned chain_idx = 0;

	// - ищем хендл блока достаточного размера
	for(unsigned i = FMMHASHS(size); i < FMM_MAXHASHITEMS; i++)
	{
		chain_idx = 0;
		_h = HashTableS[i];
		while(_h)
		{
			// - внутри цепочки, идущей из таблицы HashTableS, все хендлы пустые
			if(_h->TotalSize >= ((unsigned)_h->BaseAddress)%PageSize + size)
			{
				h = _h;
				break;
			}
			_h = _h->SubNextS;
			chain_idx++;
		}
		if(h) break;
	}
	if(!h)
	{
		FMM_LOG("Cannot find aligned block of %u bytes size!\n", size);
		return NULL;
	}

	// - отделяем начало хендла в пустой хендл, если не выровнено начало на размер страницы PageSize
	unsigned delta_size = (PageSize - ((unsigned)h->BaseAddress)%PageSize)%PageSize;
	if(delta_size)
	{
		// - создаем новый хендл, выровненый по границе PageSize
		_h = GetFreeHandler();
		if(!_h) return NULL;
	
		_h->BaseAddress = h->BaseAddress + delta_size;
		_h->TotalSize = h->TotalSize - delta_size;
		_h->isFree = true;
		h->TotalSize = delta_size;

		FMMHANDLER* h0 = h;
		FMMHANDLER* h1 = _h;
		FMMHANDLER* h2 = h->Next;
		
		h0->Next = h1;
		h1->Next = h2;
		h1->Prev = h0;
		if(h2)
			h2->Prev = h1;

		// - помещаем пустой хендл в таблицу размеров
		FMMADDHASHS(_h);
		h = _h;
	}
	assert(((unsigned)h->BaseAddress)%PageSize == 0);

	// - требуется удалить найденный хендл из таблицы размеров
	DELETE_HS(h);

	// - имеем выровненый по границе PageSize хендл, с размерами >= size
	if(h->TotalSize > size)
	{
		// - получим еще один хендл, который будет соответствовать остатку блока
		_h = GetFreeHandler();
		if(!_h) return NULL;
	
		_h->BaseAddress = h->BaseAddress + size;
		_h->TotalSize = h->TotalSize - size;
		_h->isFree = true;
		h->TotalSize = size;

		FMMHANDLER* h0 = h;
		FMMHANDLER* h1 = _h;
		FMMHANDLER* h2 = h->Next;
		
		h0->Next = h1;
		h1->Next = h2;
		h1->Prev = h0;
		if(h2)
			h2->Prev = h1;

		// - помещаем пустой хендл в таблицу размеров
		FMMADDHASHS(_h);
	}
	// - помещаем занятый хендл в таблицу адресов
	FMMADDHASHA(h);
	return h;
}
//-----------------------------------------------------------------------------------
FMPTR FMManager::Allocate(int _size)
{	
	if(!isInit && !Initialize())
		return NULL;

	INMUTEX;
	if(!_size)
	{
		FMM_LOG("Cannot allocate 0 bytes!\n");
		OUTMUTEX;
		return NULL;
	}
	if(HeapBlock.AllocatedSize + _size > HeapBlock.TotalSize) 
	{
		FMM_ERROR("Cannot allocate %u bytes! No free memory!\n", _size);
		FMM_ERROR("Memory allocated %u\n", HeapBlock.AllocatedSize);
		OUTMUTEX;
		return NULL;
	}
	unsigned size = (_size+0x0f)&~0x0f;
	#ifdef FMM_BOUNDSCHECKING_ENABLE
		if(size == _size)	size += 16;
	#endif

	FMMHANDLER* h;
	#ifdef FMM_LOCKING_ENABLE
		if(isLockingEnable)
			h = AddPageAlignedHandler(size);
		else
			h = AddHandler(size);
	#else
		h = AddHandler(size);
	#endif

	if(!h)
	{
		OUTMUTEX;
		return NULL;
	}	
	FMPTR addr1 = h->BaseAddress;
	FMPTR addr2 = addr1 + h->TotalSize;

	if(HeapEnd < addr2) HeapEnd = addr2;
	HeapBlock.AllocatedSize += size;

	SetAccess(addr1, addr2-addr1, PAGE_EXECUTE_READWRITE);

	#ifdef FMM_LOCKING_ENABLE
		if(isLockingEnable)
			Lock(addr1);
	#endif

	#ifdef FMM_BOUNDSCHECKING_ENABLE
		// - установка маски выделенных байтов
		int Len = _size;
		int BitsOffset = addr1 - HeapBlock.BaseAddress;
		assert((BitsOffset&7) == 0);
		_asm {
			mov	edi,BitsOffset
			shr	edi,3
			add	edi,offset MemoryMask
			mov	ecx,Len
			shr	ecx,3
			xor	eax,eax
			dec	eax
			cld
			rep	stosb
			mov	ecx,Len
			and	ecx,7
			jcxz	_1
			dec	ecx
			mov	al,0x80
			sar	al,cl
			stosb
_1:
		};
		SetAccess(h->BaseAddress + _size, size-_size, PAGE_NOACCESS);
	#endif
	FMM_LOG("%u bytes allocated\n", size);
	OUTMUTEX;
	return h->BaseAddress;
}
/*-----------------------------------------------------------------------------------
	Нахождение удаляемого хендла происходит путем поиска хендла с заданным адресом
в цепочке таблицы адресов, адресуемой хеш-значением заданного адреса. При нахождении 
хендла, память освобождается, и производится удаление свободных соседей данного хендла.
Удаляемые хендлы становятся инвалидными и помещаются в список инвалидных хендлов.
/----------------------------------------------------------------------------------*/
bool FMManager::Deallocate(FMPTR addr)
{
	if(!isInit) return false;
	INMUTEX;
	FMMHANDLER* h = GetHandlerByAddress(addr);
	if(!h)
	{
		FMM_LOG("Cannot find block with address 0x%08x!\n", addr);
		OUTMUTEX;
		return false;
	}
	FMM_LOG("Deallocated %u bytes from address 0x%08x\n", h->TotalSize, addr);

	#ifdef FMM_BOUNDSCHECKING_ENABLE
		// - сброс маски выделенных байтов
		int Len = h->TotalSize;
		int BitsOffset = h->BaseAddress - HeapBlock.BaseAddress;
		assert((BitsOffset&7) == 0);
		_asm {
			mov	edi,BitsOffset
			shr	edi,3
			add	edi,offset MemoryMask
			mov	ecx,Len
			add	ecx,0xF
			shr	ecx,3
			xor	eax,eax
			cld
			rep	stosb
		};
	#endif

	// - требуется удалить найденный хендл из таблицы адресов
	DELETE_HA(h);

	// - освободим память и установим атрибуты страниц
	FMPTR addr1 = h->BaseAddress;
	FMPTR addr2 = addr1 + h->TotalSize;

	if(HeapEnd == addr2) HeapEnd = addr1;
	HeapBlock.AllocatedSize -= h->TotalSize;

	FMMHANDLER* h0 = h->Prev;
	FMMHANDLER* h1 = h;
	FMMHANDLER* h2 = h->Next;

	// - номер первой занимаемой страницы
	int page1 = (int)((addr1 - HeapBlock.BaseAddress)/PageSize);
	// - номер последней занимаемой страницы
	// - отнимаем 1, (блок > 0) для того, чтобы получить корректный номер страницы
	int page2 = (int)((addr2 - HeapBlock.BaseAddress - 1)/PageSize);

	FMMHANDLER* hPrev = h0;
	FMMHANDLER* hNext = h2;
	bool isLUse = false;
	bool isRUse = false;
	while(hPrev)
	{
		// - получим номер страницы, последней для левого блока
		int page = (int)(hPrev->BaseAddress + hPrev->TotalSize - 
				  			  HeapBlock.BaseAddress - 1)/PageSize;
		// - если вышли за пределы левой	страницы, выходим
		if(page < page1) break;
		// - если на этой странице есть непустой блок, то страница занята
		if(!hPrev->isFree) 
		{
			isLUse = true;
			break;
		}
		hPrev = hPrev->Prev;
	}
	while(hNext)
	{
		// - получим номер страницы, первой для правого блока
		int page = (int)(hNext->BaseAddress - HeapBlock.BaseAddress)/PageSize;
		// - если вышли за пределы правой страницы, выходим
		if(page > page2) break;
		// - если на этой странице есть непустой блок, то страница занята
		if(!hNext->isFree) 
		{
			isRUse = true;
			break;
		}
		hNext = hNext->Next;
	}

	if(isLUse)
		page1++;
	if(isRUse)
		page2--;
/*
	if(page1 >= 0 && page2 >= page1)
	{
		// - сюда мы попадаем только в случае, если надо освободить хоть одну страницу!
		if(!VirtualFree((void*)(page1*PageSize + HeapBlock.BaseAddress), 
														(page2-page1)*PageSize+1, MEM_DECOMMIT))
		{
			FMM_MESSAGE("Cannot decommit pages from %u to %u!\n", page1, page2);
			return false;
		}
	}
*/
	if(h0 && h0->isFree)
	{
		// - удаляем предыдущий хендл (Prev)
		h1->TotalSize += h0->TotalSize;
		h1->BaseAddress = h0->BaseAddress;

		DELETE_HS(h0);
		DELETE_H(h0);
	}
	if(h2 && h2->isFree)
	{
		// - удаляем следующий хендл (Next)
		h1->TotalSize += h2->TotalSize;

		DELETE_HS(h2);
		DELETE_H(h2);
	}
	// - добавляем хендл в таблицу размеров
	FMMADDHASHS(h1);
	OUTMUTEX;
	return true;
}
//-----------------------------------------------------------------------------------
// Access routines
//-----------------------------------------------------------------------------------
bool FMManager::SetAccess(FMPTR base, unsigned size, unsigned mode)
{
	if(!VirtualAlloc(base, size, MEM_COMMIT, mode))
	{
		FMM_ERROR("Cannot commit memory at 0x%08x size 0x%08x!\n", base, size);
		return false;
	}
	return true;
}
//-----------------------------------------------------------------------------------
#ifdef FMM_LOCKING_ENABLE
int FMManager::Lock(FMPTR addr)
{
	if(!isInit || !isLockingEnable) return false;
	INMUTEX;
	FMMHANDLER* h = GetHandlerByAddress(addr);
	if(!h)
	{
		FMM_LOG("Cannot find block with address 0x%08x!\n", addr);
		OUTMUTEX;
		return -1;
	}
	assert(h->BaseAddress == addr);
	if(((unsigned)h->BaseAddress)%PageSize || h->TotalSize%PageSize)
	{
		FMM_LOG("Cannot lock block whith address 0x%08x and size %u!\n", addr, h->TotalSize);
		OUTMUTEX;
		return -1;
	}
	FMM_LOG("Locking block of %u bytes at address 0x%08x\n", h->TotalSize, h->BaseAddress);
	h->Locked++;
	if(!SetAccess(h->BaseAddress, h->TotalSize, PAGE_NOACCESS))
	{
		OUTMUTEX;
		return -1;
	}
	OUTMUTEX;
	return h->Locked;
}
#endif
//-----------------------------------------------------------------------------------
#ifdef FMM_LOCKING_ENABLE
int FMManager::Unlock(FMPTR addr)
{
	if(!isInit || !isLockingEnable) return false;
	INMUTEX;
	FMMHANDLER* h = GetHandlerByAddress(addr);
	if(!h)
	{
		FMM_LOG("Cannot find block with address 0x%08x!\n", addr);
		OUTMUTEX;
		return -1;
	}
	assert(h->BaseAddress == addr);
	if(((unsigned)h->BaseAddress)%PageSize || h->TotalSize%PageSize)
	{
		FMM_LOG("Cannot unlock block whith address 0x%08x and size %u!\n", addr, h->TotalSize);
		OUTMUTEX;
		return -1;
	}
	FMM_LOG("Unlocking block of %u bytes at address 0x%08x\n", h->TotalSize, h->BaseAddress);

	h->Locked--;
	if(h->Locked < 0)
	{
		FMM_LOG("!Warning!: Unlocking block of %u bytes at address 0x%08x\n", h->TotalSize, h->BaseAddress);
		FMM_LOG("           Locking count is %d\n", h->Locked);
	}

	if(!SetAccess(h->BaseAddress, h->TotalSize, PAGE_EXECUTE_READWRITE))
	{
		OUTMUTEX;
		return -1;
	}
	OUTMUTEX;
	return h->Locked;
}
#endif
//-----------------------------------------------------------------------------------
// Additional routines
//-----------------------------------------------------------------------------------
bool FMManager::GetInfo(FMMINFO* m)
{
	m->AllocatedSize = HeapBlock.AllocatedSize;
	m->AllHandlers = handlers;
	m->AllocatedHandlers = 0;
	m->FreeHandlers = 0;

	unsigned i;
	for(i = 0; i < handlers; i++)
		Handlers[i].r2 = 0;

	for(i = 0; i < FMM_MAXHASHITEMS; i++)
	{
		FMMHANDLER* h = HashTableA[i];
		unsigned chain_idx = 0;
		while(h)
		{
			if(h->r2)
			{
				FMM_LOG("Cycling detected at HashTableA[%u] in the element number %u!\n", i, chain_idx);
				return false;
			}
			h->r2 = 1;
			m->AllocatedHandlers++;
			if(m->AllocatedHandlers > handlers)
			{
				FMM_LOG("Cycling detected at HashTableA[%u]!\n", i);
				return false;
			}
			chain_idx++;
			h = h->SubNextA;
		}
	}
	
	for(i = 0; i < FMM_MAXHASHITEMS; i++)
	{
		FMMHANDLER* h = HashTableS[i];
		unsigned chain_idx = 0;
		while(h)
		{
			if(h->r2)
			{
				FMM_LOG("Cycling detected at HashTableS[%u] in the element number %u!\n", i, chain_idx);
				return false;
			}
			h->r2 = 1;
			m->FreeHandlers++;
			if(m->FreeHandlers > handlers)
			{
				FMM_LOG("Cycling detected at HashTableS[%u]!\n", i);
				return false;
			}
			chain_idx++;
			h = h->SubNextS;
		}
	}

	m->TotalPages = HeapBlock.TotalPages;
	m->NonePages = 0;
	m->NewPages = 0;
	m->PresentPages = 0;
	m->CashedPages = 0;

	MEMORY_BASIC_INFORMATION Buffer;

	unsigned TotalPages = 0;
	unsigned NonePages = 0;
	unsigned PresentPages = 0;

	unsigned addr1 = (unsigned)HeapBlock.BaseAddress;
	unsigned addr2 = addr1 + HeapBlock.TotalSize;

	while(addr1 < addr2)
	{
		VirtualQuery((void*)addr1, &Buffer, sizeof(MEMORY_BASIC_INFORMATION));
		
		if(Buffer.RegionSize > HeapBlock.TotalSize)
			Buffer.RegionSize = HeapBlock.TotalSize;

		addr1 += Buffer.RegionSize;
		unsigned pages = (Buffer.RegionSize + PageSize-1)/PageSize;
		TotalPages += pages;

		if(Buffer.State == MEM_FREE || Buffer.State == MEM_RESERVE)
			NonePages += pages;
		else
		if(Buffer.State == MEM_COMMIT)
			PresentPages += pages;
	}
	return true;
}
//-----------------------------------------------------------------------------------
#ifdef FMM_BOUNDSCHECKING_ENABLE
PVOID AccessAddress = NULL;
#endif
//-----------------------------------------------------------------------------------
#ifdef FMM_BOUNDSCHECKING_ENABLE
LONG __stdcall FMM_PageFaultExceptionFilter(EXCEPTION_POINTERS* pEx)
{
	DWORD dwCode = pEx->ExceptionRecord->ExceptionCode;
	if(dwCode == EXCEPTION_ACCESS_VIOLATION)
	{
		AccessAddress = (PVOID)pEx->ExceptionRecord->ExceptionInformation[1];
		unsigned BitsOffset = (unsigned)AccessAddress - (unsigned)MManager.HeapBlock.BaseAddress;
		char Flag = 0;
		_asm {
			mov	edi,BitsOffset
			shr	edi,3
			add	edi,offset FMManager::MemoryMask
			mov	ecx,AccessAddress
			and	ecx,7
			mov	al,0x80
			shr	al,cl
			test	[edi],al
			jz		_2
			mov	Flag,1
_2:
		};
		if(!Flag)
		{
			char msg[128];
			sprintf(msg, "Memory Access Violation at address: 0x%p", AccessAddress);
			MessageBox(0, msg, "FMManager", MB_OK);
			return EXCEPTION_EXECUTE_HANDLER;
		}
		void* res = VirtualAlloc(AccessAddress, 1, MEM_COMMIT, PAGE_READWRITE);
		pEx->ContextRecord->EFlags |= 0x100;		
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	if(dwCode == EXCEPTION_SINGLE_STEP)
	{
		void* res = VirtualAlloc(AccessAddress, 1, MEM_COMMIT, PAGE_NOACCESS);
		pEx->ContextRecord->EFlags &= ~0x100;		
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif
//-----------------------------------------------------------------------------------
