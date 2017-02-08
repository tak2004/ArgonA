#include "ArgonA/VirtualArchive.hpp"
#include <RadonFramework/System/IO/FileSystem.hpp>

namespace PTS { namespace ArgonA {

RF_Type::UInt32 fnv_32_str(const RF_Type::String& Text, RF_Type::UInt32 HashValue)
{
    const char *s = Text.c_str();
    /*
    * FNV-1 hash each octet in the buffer
    */
    while(*s)
    {
        /* multiply by the 32 bit FNV magic prime mod 2^32 */
        HashValue += (HashValue << 1) + (HashValue << 4) + (HashValue << 7) + 
            (HashValue << 8) + (HashValue << 24);
        /* xor the bottom with the current octet */
        HashValue ^= (RF_Type::UInt32)*s++;
    }
    /* return our new hash value */
    return HashValue;
}

VirtualArchiveWriter::VirtualArchiveWriter()
:m_PackageSize(307200000)
,m_Nodes(0)
{

}

VirtualArchiveWriter::~VirtualArchiveWriter()
{

}

RF_Type::Bool VirtualArchiveWriter::Open(const RF_Type::String& path)
{
    RF_Type::String fullPath;
    RF_SysFile::RealPath(path, fullPath);
    return Open(RF_IO::Uri("file:///"_rfs + fullPath));
}

RF_Type::Bool VirtualArchiveWriter::Open(const RF_IO::Uri& Path)
{
    RF_IO::Directory dir;
    dir.SetLocation(Path);
    RF_Type::Bool result = false;
    if(dir.Exists() && dir.IsDirectory())
    {
        m_Path = dir.Location().OriginalString();
        m_NodeFile.Open(dir.SubFile("0.var"_rfs)->Location(),
                        RF_SysFile::FileAccessMode::Write, 
                        RF_SysFile::FileAccessPriority::DelayReadWrite);
        m_Identifier.fourcc = 'VIAR';
        m_Identifier.version = Version::Value;
        m_NodeFile.Write(reinterpret_cast<RF_Type::UInt8*>(&m_Identifier), 0,
                         sizeof(ArchiveIdentifier));
        m_Header.filenodes = 0;
        m_Header.mainfileCRC = 0;
        m_Header.packageSize = m_PackageSize;
        m_NodeFile.Write(reinterpret_cast<RF_Type::UInt8*>(&m_Header), 0,
                         sizeof(ArchiveHeader));
    }
    return result;
}

bool VirtualArchiveWriter::Append(const RF_Type::String& Filename,
    RF_Mem::AutoPointerArray<RF_Type::UInt8>& Data)
{    
    bool result = false;
    if(Data.Count() < m_PackageSize && Data.Count() > 0 && Data)
    {
        RF_Type::Size lastPackageSize = 0;
        FileNode node;
        node.crc = 0;
        node.contentSize = Data.Count() + Filename.Size() + sizeof(RF_Type::UInt16);
        node.namehash = fnv_32_str(Filename, 0);
        if(m_DataFiles.Count() > 0)
        {
            lastPackageSize = m_DataFiles[m_DataFiles.Count() - 1].Position();
        }

        if((lastPackageSize + node.contentSize > m_PackageSize) || (m_DataFiles.IsEmpty()))
        {
            auto name = RF_Type::String::Format("%s/%d.var"_rfs, m_Path.c_str(),
                m_DataFiles.Count() + 1);            
            m_DataFiles.CreateElementAtEnd().Open(name, 
                RF_SysFile::FileAccessMode::Write, 
                RF_SysFile::FileAccessPriority::DelayReadWrite);
        }
        RF_IO::FileStream* stream = m_DataFiles.Last();
        node.dataoffset = stream->Position() + m_DataFiles.Count()*m_PackageSize;
        m_NodeFile.Write(reinterpret_cast<RF_Type::UInt8*>(&node), 0, sizeof(node));
        
        stream->WriteType<RF_Type::UInt16>(Filename.Size());
        stream->Write(reinterpret_cast<const RF_Type::UInt8*>(Filename.c_str()),
            0, Filename.Size());
        stream->Write(Data.Get(), 0, Data.Count());
        ++m_Nodes;
        
        result = true;
    }
    return result;
}

void VirtualArchiveWriter::Close()
{
    m_Header.filenodes = m_Nodes;
    m_NodeFile.Seek(sizeof(ArchiveIdentifier), RF_IO::SeekOrigin::Begin);
    m_NodeFile.Write(reinterpret_cast<RF_Type::UInt8*>(&m_Header), 0, sizeof(ArchiveHeader));
    m_NodeFile.Close();

    for(RF_Type::Size i = 0; i < m_DataFiles.Count(); ++i)
    {
        m_DataFiles[i].Close();
    }
    m_DataFiles.Clear();
}

VirtualArchiveReader::VirtualArchiveReader()
:m_Identifier(0)
,m_Header(0)
,m_Nodes(0)
,m_AutoConvert(true)
{

}

VirtualArchiveReader::~VirtualArchiveReader()
{
    Close();
}

RF_Type::Bool VirtualArchiveReader::Open(const RF_Type::String& Path)
{
    RF_Type::Bool result = false;
    RF_IO::Directory dir;
    RF_Type::String fullPath;
    RF_SysFile::RealPath(Path, fullPath);
    dir.SetLocation("file:///"_rfs + fullPath);
    if(dir.Exists() && dir.IsDirectory())
    {
        m_Path = "file:///"_rfs + fullPath;
        RF_IO::File file;
        file.SetLocation(RF_IO::Uri(m_Path + "/0.var"_rfs));
        m_Data = file.Read();
        m_Identifier = reinterpret_cast<ArchiveIdentifier*>(m_Data.Get());
        if(m_Identifier->fourcc == 'VIAR')
        {
            if(m_AutoConvert && m_Identifier->version != Version::Value)
            {
                m_Data = Convert(m_Data);
                m_Identifier = reinterpret_cast<ArchiveIdentifier*>(m_Data.Get());
                file.Write(m_Data);
            }
            m_Header = reinterpret_cast<ArchiveHeader*>(m_Identifier + 1);
            m_Nodes = reinterpret_cast<FileNode*>(m_Header + 1);
            result = true;
        }
    }
    return result;
}

void VirtualArchiveReader::Close()
{
    m_Identifier = 0;
    m_Header = 0;
    m_Nodes = 0;
    m_Data.Reset();
}

const FileNode* VirtualArchiveReader::Find(const RF_Type::String& Filename)const
{
    RF_Type::UInt32 hash = fnv_32_str(Filename, 0);
    FileNode* result = 0;
    for(RF_Type::Size i = 0; i < m_Header->filenodes; ++i)
    {
        if(m_Nodes[i].namehash == hash)
        {
            result = &m_Nodes[i];
            break;
        }
    }
    return result;
}

RF_Mem::AutoPointerArray<RF_Type::UInt8> VirtualArchiveReader::GetData(const FileNode& File)
{
    RF_Mem::AutoPointerArray<RF_Type::UInt8> result;
    
    RF_Type::Size package = File.dataoffset / m_Header->packageSize;
    auto name = RF_Type::String::Format("%s/%d.var"_rfs, m_Path.c_str(), package);
    RF_IO::File file;
    file.SetLocation(name);
    if(file.Exists())
    {
        RF_IO::FileStream stream;
        stream.Open(name, RF_SysFile::FileAccessMode::Read, RF_SysFile::FileAccessPriority::ReadThroughput);
        RF_Type::Size offset = File.dataoffset % m_Header->packageSize;
        stream.Seek(offset, RF_IO::SeekOrigin::Current);
        RF_Type::UInt16 filenameSize;
        stream.ReadType<RF_Type::UInt16>(filenameSize);
        stream.Seek(filenameSize, RF_IO::SeekOrigin::Current);
        result = RF_Mem::AutoPointerArray<RF_Type::UInt8>(File.contentSize - filenameSize - sizeof(RF_Type::UInt16));
        stream.Read(result.Get(), 0, result.Count());
        stream.Close();
    }
    return result;
}

RF_Type::String VirtualArchiveReader::GetName(const FileNode& File)
{
    RF_Type::String result;

    RF_Type::Size package = File.dataoffset / m_Header->packageSize;
    auto name = RF_Type::String::Format("%s/%d.var"_rfs, m_Path.c_str(), package);
    RF_IO::File file;
    file.SetLocation(name);
    if(file.Exists())
    {
        RF_IO::FileStream stream;
        stream.Open(name, RF_SysFile::FileAccessMode::Read, RF_SysFile::FileAccessPriority::ReadThroughput);
        RF_Type::Size offset = File.dataoffset % m_Header->packageSize;
        stream.Seek(offset, RF_IO::SeekOrigin::Begin);
        RF_Type::UInt16 filenameSize;
        stream.ReadType(filenameSize);
        RF_Mem::AutoPointerArray<RF_Type::UInt8> filename = stream.ReadBytes(filenameSize);
        result = RF_Type::String(reinterpret_cast<char*>(filename.Get()), filename.Count());
        stream.Close();
    }
    return result;
}

RF_Collect::Array<FileNode*> VirtualArchiveReader::ListFiles()
{
    RF_Collect::Array<FileNode*> result(m_Header->filenodes);
    for(RF_Type::Size i = 0; i < m_Header->filenodes; ++i)
    {
        result(i) = &m_Nodes[i];
    }
    return result;
}

RF_Mem::AutoPointerArray<RF_Type::UInt8> VirtualArchiveReader::Convert(
    RF_Mem::AutoPointerArray<RF_Type::UInt8>& In)
{
    // currently there's only one version
    return In;
}

} }