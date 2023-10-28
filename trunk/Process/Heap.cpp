#include "pch.h"
#include "Heap.h"


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI GetProcessHeapsInfo()
/*
Getting Process Heaps

https://msdn.microsoft.com/en-us/library/windows/desktop/ee175820(v=vs.85).aspx
*/
{
    // Retrieve the number of active heaps for the current process so we can calculate the buffer size needed for the heap handles.
    DWORD NumberOfHeaps = GetProcessHeaps(0, NULL);
    if (NumberOfHeaps == 0) {
        _tprintf(TEXT("Failed to retrieve the number of heaps with LastError %u.\n"), GetLastError());
        return 1;
    }

    // Calculate the buffer size.
    PHANDLE aHeaps{};
    SIZE_T BytesToAllocate{};
    HRESULT Result = SIZETMult(NumberOfHeaps, sizeof(*aHeaps), &BytesToAllocate);
    if (Result != S_OK) {
        _tprintf(TEXT("SIZETMult failed with HR %d.\n"), Result);
        return 1;
    }

    // Get a handle to the default process heap.
    HANDLE hDefaultProcessHeap = GetProcessHeap();
    if (hDefaultProcessHeap == NULL) {
        _tprintf(TEXT("Failed to retrieve the default process heap with LastError %u.\n"), GetLastError());
        return 1;
    }

    // Allocate the buffer from the default process heap.
    aHeaps = (PHANDLE)HeapAlloc(hDefaultProcessHeap, 0, BytesToAllocate);
    if (aHeaps == NULL) {
        _tprintf(TEXT("HeapAlloc failed to allocate %Id bytes.\n"), BytesToAllocate);
        return 1;
    }

    // Save the original number of heaps because we are going to compare it to the return value of the next GetProcessHeaps call.
    DWORD HeapsLength = NumberOfHeaps;

    // Retrieve handles to the process heaps and print them to stdout. 
    // Note that heap functions should be called only on the default heap of the process or on private heaps that your component creates by calling HeapCreate.
    NumberOfHeaps = GetProcessHeaps(HeapsLength, aHeaps);
    if (NumberOfHeaps == 0) {
        _tprintf(TEXT("Failed to retrieve heaps with LastError %u.\n"), GetLastError());
        return 1;
    }
    else if (NumberOfHeaps > HeapsLength) {
        // Compare the latest number of heaps with the original number of heaps.
        // If the latest number is larger than the original number, another component has created a new heap and the buffer is too small.
        _tprintf(TEXT("Another component created a heap between calls. ") \
            TEXT("Please try again.\n"));
        return 1;
    }

    _tprintf(TEXT("Process has %u heaps.\n"), HeapsLength);
    for (DWORD HeapsIndex = 0; HeapsIndex < HeapsLength; ++HeapsIndex) {
        _tprintf(TEXT("Heap %u at address: %#p.\n"), HeapsIndex, aHeaps[HeapsIndex]);
    }

    // Release memory allocated from default process heap.
    if (HeapFree(hDefaultProcessHeap, 0, aHeaps) == FALSE) {
        _tprintf(TEXT("Failed to free allocation from default process heap.\n"));
    }

    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI TraversingHeapList(void)
/*
Traversing the Heap List

https://msdn.microsoft.com/en-us/library/windows/desktop/dd299432(v=vs.85).aspx
*/
{
    HANDLE hHeapSnap = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, GetCurrentProcessId());
    if (hHeapSnap == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot failed (%u)\n", GetLastError());
        return 1;
    }

    HEAPLIST32 hl;
    hl.dwSize = sizeof(HEAPLIST32);
    if (Heap32ListFirst(hHeapSnap, &hl)) {
        do {
            HEAPENTRY32 he;
            ZeroMemory(&he, sizeof(HEAPENTRY32));
            he.dwSize = sizeof(HEAPENTRY32);
            if (Heap32First(&he, GetCurrentProcessId(), hl.th32HeapID)) {
                printf("\nHeap ID: %Id\n", hl.th32HeapID);
                do {
                    printf("Block size: %Id\n", he.dwBlockSize);
                    he.dwSize = sizeof(HEAPENTRY32);
                } while (Heap32Next(&he));
            }

            hl.dwSize = sizeof(HEAPLIST32);
        } while (Heap32ListNext(hHeapSnap, &hl));
    }
    else {
        printf("Cannot list first heap (%u)\n", GetLastError());
    }

    CloseHandle(hHeapSnap);
    return 0;
}


EXTERN_C
__declspec(dllexport)
int WINAPI EnumeratingHeap()
/*
Enumerating a Heap

https://msdn.microsoft.com/en-us/library/windows/desktop/ee175819(v=vs.85).aspx
*/
{
    HANDLE hHeap = HeapCreate(0, 0, 0);// Create a new heap with default parameters.
    if (hHeap == NULL) {
        _tprintf(TEXT("Failed to create a new heap with LastError %u.\n"), GetLastError());
        return 1;
    }

    // Lock the heap to prevent other threads from accessing the heap during enumeration.
    if (HeapLock(hHeap) == FALSE) {
        _tprintf(TEXT("Failed to lock heap with LastError %u.\n"), GetLastError());
        return 1;
    }

    _tprintf(TEXT("Walking heap %#p...\n\n"), hHeap);

    PROCESS_HEAP_ENTRY Entry;
    Entry.lpData = NULL;
    while (HeapWalk(hHeap, &Entry) != FALSE) {
        if ((Entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0) {
            _tprintf(TEXT("Allocated block"));

            if ((Entry.wFlags & PROCESS_HEAP_ENTRY_MOVEABLE) != 0) {
                _tprintf(TEXT(", movable with HANDLE %#p"), Entry.Block.hMem);
            }

            if ((Entry.wFlags & PROCESS_HEAP_ENTRY_DDESHARE) != 0) {
                _tprintf(TEXT(", DDESHARE"));
            }
        }
        else if ((Entry.wFlags & PROCESS_HEAP_REGION) != 0) {
            _tprintf(TEXT("Region\n  %u bytes committed\n") \
                TEXT("  %u bytes uncommitted\n  First block address: %#p\n") \
                TEXT("  Last block address: %#p\n"),
                Entry.Region.dwCommittedSize,
                Entry.Region.dwUnCommittedSize,
                Entry.Region.lpFirstBlock,
                Entry.Region.lpLastBlock);
        }
        else if ((Entry.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE) != 0) {
            _tprintf(TEXT("Uncommitted range\n"));
        }
        else {
            _tprintf(TEXT("Block\n"));
        }

        _tprintf(TEXT("  Data portion begins at: %#p\n  Size: %u bytes\n") \
            TEXT("  Overhead: %u bytes\n  Region index: %u\n\n"),
            Entry.lpData,
            Entry.cbData,
            Entry.cbOverhead,
            Entry.iRegionIndex);
    }

    DWORD LastError = GetLastError();
    if (LastError != ERROR_NO_MORE_ITEMS) {
        _tprintf(TEXT("HeapWalk failed with LastError %u.\n"), LastError);
    }

    // Unlock the heap to allow other threads to access the heap after enumeration has completed.
    if (HeapUnlock(hHeap) == FALSE) {
        _tprintf(TEXT("Failed to unlock heap with LastError %u.\n"), GetLastError());
    }

    // When a process terminates, allocated memory is reclaimed by the operating system so it is not really necessary to call HeapDestroy in this example.
    // However, it may be advisable to call HeapDestroy in a longer running application.
    if (HeapDestroy(hHeap) == FALSE) {
        _tprintf(TEXT("Failed to destroy heap with LastError %u.\n"), GetLastError());
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


EXTERN_C
__declspec(dllexport)
int WINAPI TraversingHeapList2(void)
/*
Traversing the Heap List
05/31/2018

The following example obtains a list of heaps for the current process.
It takes a snapshot of the heaps using the CreateToolhelp32Snapshot function,
and then walks through the list using the Heap32ListFirst and Heap32ListNext functions.
For each heap, it uses the Heap32First and Heap32Next functions to walk the heap blocks.

https://docs.microsoft.com/en-us/windows/win32/toolhelp/traversing-the-heap-list
*/
{
    HEAPLIST32 hl{};
    HANDLE hHeapSnap = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, GetCurrentProcessId());
    hl.dwSize = sizeof(HEAPLIST32);
    if (hHeapSnap == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot failed (%u)\n", GetLastError());
        return 1;
    }

    if (Heap32ListFirst(hHeapSnap, &hl)) {
        do {
            HEAPENTRY32 he;
            ZeroMemory(&he, sizeof(HEAPENTRY32));
            he.dwSize = sizeof(HEAPENTRY32);

            if (Heap32First(&he, GetCurrentProcessId(), hl.th32HeapID)) {
                printf("\nHeap ID: %Id\n", hl.th32HeapID);
                do {
                    printf("Block size: %Id\n", he.dwBlockSize);
                    he.dwSize = sizeof(HEAPENTRY32);
                } while (Heap32Next(&he));
            }
            hl.dwSize = sizeof(HEAPLIST32);
        } while (Heap32ListNext(hHeapSnap, &hl));
    }
    else printf("Cannot list first heap (%u)\n", GetLastError());

    CloseHandle(hHeapSnap);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////


void SetHeapInformation()
/*
https://docs.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapsetinformation
*/
{
    // Enable heap terminate-on-corruption. 
    // A correct application can continue to run even if this call fails, 
    // so it is safe to ignore the return value and call the function as follows:
    // (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    // If the application requires heap terminate-on-corruption to be enabled, 
    // check the return value and exit on failure as shown in this example.
    BOOL bResult = HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    if (bResult != FALSE) {
        //DbgPrintA("信息：Heap terminate-on-corruption has been enabled");
    }
    else {
        //DbgPrintA("错误：LastError %d", GetLastError());
    }

    HANDLE hHeap = GetProcessHeap();
    if (hHeap == NULL) {
        //DbgPrintA("错误：LastError %d", GetLastError());
        return;
    }

    // Enable the low-fragmenation heap (LFH). Starting with Windows Vista, 
    // the LFH is enabled by default but this call does not cause an error.
    //
    //#define HEAP_LFH 2
    //SIZE_T HeapInformation = 2;
    //bResult = HeapSetInformation(hHeap,
    //                             HeapCompatibilityInformation,
    //                             &HeapInformation,
    //                             sizeof(SIZE_T));
    //if (bResult != FALSE) {
    //    DbgPrintA("信息：The low-fragmentation heap has been enabled");
    //} else {
    //    DbgPrintA("错误：LastError %d", GetLastError());//ERROR_INVALID_PARAMETER
    //}
}


//////////////////////////////////////////////////////////////////////////////////////////////////
