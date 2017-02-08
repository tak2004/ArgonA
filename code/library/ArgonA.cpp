#define ARGONA_BUILD
#include "ArgonA/ArgonA.h"
#include "ArgonA/VirtualArchive.hpp"

class HandleEntry
{
public:
    HandleEntry()
    :reader(0)
    ,writer(0)
    ,prev(0)
    ,next(0)
    {
    }

    ~HandleEntry()
    {
        if(reader != 0)
        {
            reader->Close();
            delete reader;
            reader = 0;
        }
        if(writer != 0)
        {
            writer->Close();
            delete writer;
            writer = 0;
        }
        if(prev != 0)
        {
            prev->next = next;
            prev = 0;
        }
        if(next != 0)
        {
            next->prev = prev;
            next = 0;
        }
    }

    ARGONA::VirtualArchiveReader* reader;
    ARGONA::VirtualArchiveWriter* writer;
    HandleEntry* prev;
    HandleEntry* next;
};

HandleEntry* root = 0;

void ARGONAAPIENTRY Initialize()
{
    root = 0;
}

void ARGONAAPIENTRY Shutdown()
{
    HandleEntry* entry = 0;
    while(root != 0)
    {
        entry = root;
        root = root->next;
        delete entry;
    }
}

ARGONA_HANDLE ARGONAAPIENTRY Open(const char* Path, size_t PathLength)
{
    ARGONA_HANDLE result = ARGONA_INVALID_HANDLE;
    RF_Type::String path(Path, PathLength);

    HandleEntry * newEntry = new HandleEntry();
    newEntry->reader = new ARGONA::VirtualArchiveReader();
    if(newEntry->reader->Open(path))
    {
        newEntry->next = root;
        root->prev = newEntry;
        root = newEntry;
        result = newEntry;
    }
    else
        delete newEntry;
    return result;
}

ARGONA_HANDLE ARGONAAPIENTRY Create(const char* Path, size_t PathLength)
{
    ARGONA_HANDLE result = ARGONA_INVALID_HANDLE;
    RF_Type::String path(Path, PathLength);

    HandleEntry * newEntry = new HandleEntry();
    newEntry->writer = new ARGONA::VirtualArchiveWriter();
    if(newEntry->writer->Open(path))
    {
        newEntry->next = root;
        root->prev = newEntry;
        root = newEntry;
        result = newEntry;
    }
    else
        delete newEntry;
    return result;
}

void ARGONAAPIENTRY Close(ARGONA_HANDLE ArchiveHandle)
{
    if(ArchiveHandle != ARGONA_INVALID_HANDLE)
    {
        delete (HandleEntry*)ArchiveHandle;
    }
}

bool ARGONAAPIENTRY Append(ARGONA_HANDLE ArchiveHandle, const char* Filename, 
    size_t FilenameLength, const unsigned char* Data, size_t DataSize)
{
    bool result = false;
    if(ArchiveHandle != ARGONA_INVALID_HANDLE)
    {
        HandleEntry* entry = (HandleEntry*)ArchiveHandle;
        if(entry->writer != 0)
        {
            RF_Type::String filename(Filename, FilenameLength);
            RF_Mem::AutoPointerArray<RF_Type::UInt8> buffer(DataSize);
            RF_SysMem::Copy(buffer.Get(), Data, DataSize);
            result = entry->writer->Append(filename, buffer);
        }
    }
    return result;
}

ARGONA_FILE ARGONAAPIENTRY Find(ARGONA_HANDLE ArchiveHandle, 
    const char* Filename, size_t FilenameLength)
{
    ARGONA_FILE result = ARGONA_INVALID_FILE;
    if(ArchiveHandle != ARGONA_INVALID_HANDLE)
    {
        HandleEntry* entry = (HandleEntry*)ArchiveHandle;
        if(entry->reader != 0)
        {
            RF_Type::String filename(Filename, FilenameLength);
            const ARGONA::FileNode* file = entry->reader->Find(filename);
            if(file != 0)
            {
                result = (void*)file;
            }
        }
    }
    return result;
}

size_t ARGONAAPIENTRY GetData(ARGONA_HANDLE ArchiveHandle, ARGONA_FILE FileHandle,
    char** Buffer)
{
    size_t result = 0;
    if(FileHandle != ARGONA_INVALID_FILE &&
       ArchiveHandle != ARGONA_INVALID_HANDLE)
    {
        HandleEntry* entry = (HandleEntry*)ArchiveHandle;
        const ARGONA::FileNode* file = (const ARGONA::FileNode*)FileHandle;
        if(entry->reader != 0)
        {
            auto data = entry->reader->GetData(*file);
            result = data.Count();
            *Buffer = reinterpret_cast<char*>(data.Release().Ptr);
        }
    }
    return result;
}

size_t ARGONAAPIENTRY GetContentSize(ARGONA_FILE FileHandle)
{
    size_t result = 0;
    if(FileHandle != ARGONA_INVALID_FILE)
    {
        const ARGONA::FileNode* file = (const ARGONA::FileNode*)FileHandle;
        result = file->contentSize;
    }
    return result;
}

size_t ARGONAAPIENTRY GetName(ARGONA_HANDLE ArchiveHandle, ARGONA_FILE FileHandle, 
    char** Buffer)
{
    size_t result = 0;
    if(FileHandle != ARGONA_INVALID_FILE &&
        ArchiveHandle != ARGONA_INVALID_HANDLE)
    {
        HandleEntry* entry = (HandleEntry*)ArchiveHandle;
        const ARGONA::FileNode* file = (const ARGONA::FileNode*)FileHandle;
        if(entry->reader != 0)
        {
            auto data = entry->reader->GetName(*file);
            RF_Mem::AutoPointerArray<RF_Type::UInt8> resultBuffer(data.Size());
            RF_SysMem::Copy(resultBuffer.Get(), data.c_str(), data.Size());
            result = resultBuffer.Count();
            *Buffer = reinterpret_cast<char*>(resultBuffer.Release().Ptr);
        }
    }
    return result;
}

void ARGONAAPIENTRY FreeBuffer(char** Buffer)
{
    if(*Buffer)
    {
        delete[] *Buffer;
    }
}

size_t ARGONAAPIENTRY Files(ARGONA_HANDLE ArchiveHandle)
{
    size_t result = 0;
    if(ArchiveHandle != ARGONA_INVALID_HANDLE)
    {
        HandleEntry* entry = (HandleEntry*)ArchiveHandle;
        if(entry->reader != 0)
        {
            result = entry->reader->ListFiles().Count();
        }
    }
    return result;
}

ARGONA_FILE ARGONAAPIENTRY GetFile(ARGONA_HANDLE ArchiveHandle, size_t Index)
{
    ARGONA_FILE result = ARGONA_INVALID_FILE;
    if(ArchiveHandle != ARGONA_INVALID_HANDLE)
    {
        HandleEntry* entry = (HandleEntry*)ArchiveHandle;
        if(entry->reader != 0)
        {
            auto files = entry->reader->ListFiles();
            if(files.Count() > Index)
            {
                result = (ARGONA_FILE)files(Index);
            }
        }
    }
    return result;
}
