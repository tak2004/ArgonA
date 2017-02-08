#ifndef PTS_ARGONA_VIRTUALARCHIVE_HPP
#define PTS_ARGONA_VIRTUALARCHIVE_HPP

#include <RadonFramework/RadonInline.hpp>

namespace PTS { namespace ArgonA {

#pragma pack(push, 1)
struct ArchiveIdentifierV1
{
    RF_Type::UInt32 fourcc;
    RF_Type::UInt16 version;
};
static_assert(sizeof(ArchiveIdentifierV1) == 6, "This struct have to be 6byte big!");

struct ArchiveHeaderV1
{
    RF_Type::UInt32 packageSize; // max bytes per archive part
    RF_Type::UInt32 mainfileCRC;
    RF_Type::UInt32 filenodes;
};
static_assert(sizeof(ArchiveHeaderV1) == 12, "This struct have to be 12byte big!");

struct FileNodeV1
{
    RF_Type::UInt64 dataoffset; // offset in bytes from start of the archive
    RF_Type::UInt32 namehash; // id
    RF_Type::UInt32 contentSize; // starts with the Filename and then the file content
    RF_Type::UInt32 crc;
}; 
static_assert(sizeof(FileNodeV1) == 20, "This struct have to be 20byte big!");
#pragma pack(pop)

typedef ArchiveIdentifierV1 ArchiveIdentifier;
typedef ArchiveHeaderV1 ArchiveHeader;
typedef FileNodeV1 FileNode;

enum Version
{
    Value = 1
};

class VirtualArchiveWriter
{
public:
    VirtualArchiveWriter();

    ~VirtualArchiveWriter();

    RF_Type::Bool Open(const RF_Type::String& Path);
    RF_Type::Bool Open(const RF_IO::Uri& Path);

    RF_Type::Bool Append(const RF_Type::String& Filename, 
        RF_Mem::AutoPointerArray<RF_Type::UInt8>& Data);

    void Close();
protected:
    RF_Type::UInt32 m_PackageSize;
    RF_Type::String m_Path;
    RF_IO::FileStream m_NodeFile;
    RF_Collect::List<RF_IO::FileStream> m_DataFiles;
    RF_Type::UInt32 m_Nodes;
    ArchiveIdentifier m_Identifier;
    ArchiveHeader m_Header;
};

class VirtualArchiveReader
{
public:
    VirtualArchiveReader();

    ~VirtualArchiveReader();

    RF_Type::Bool Open(const RF_Type::String& Path);

    void Close();

    const FileNode* Find(const RF_Type::String& Filename)const;

    RF_Mem::AutoPointerArray<RF_Type::UInt8> GetData(const FileNode& File);

    RF_Type::String GetName(const FileNode& File);

    RF_Collect::Array<FileNode*> ListFiles();

    static RF_Mem::AutoPointerArray<RF_Type::UInt8> Convert(RF_Mem::AutoPointerArray<RF_Type::UInt8>& In);
protected:
    RF_Type::String m_Path;
    RF_Mem::AutoPointerArray<RF_Type::UInt8> m_Data;
    ArchiveIdentifier* m_Identifier;
    ArchiveHeader* m_Header;
    FileNode* m_Nodes;
    RF_Type::Bool m_AutoConvert;
};

} }

#ifndef PTS_SHORTHAND_NAMESPACE_ARGONA
#define PTS_SHORTHAND_NAMESPACE_ARGONA
namespace ARGONA = PTS::ArgonA;
#endif

#endif // PTS_ARGONA_VIRTUALARCHIVE_HPP