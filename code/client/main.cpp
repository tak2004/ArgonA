#include <RadonFramework/Radon.hpp>
#include <RadonFramework/IO/Log.hpp>
#include <ArgonA/VirtualArchive.hpp>

namespace ApplicationOptions {
enum Type
{
    ApplicationDirectory,
    Archive,
    Extract,
    Append,
    MAX
};
}

void main(int argc, const char* argv[])
{
    RadonFramework::Radon framework;

    RF_Mem::AutoPointer<RF_Diag::Appender> console(new RF_IO::LogConsole);
    RF_IO::Log::AddAppender(console);

    RF_Type::String errorOut;
    RF_IO::Parameter consoleParameter;
    RF_Mem::AutoPointerArray<RF_IO::OptionRule> rules(new RF_IO::OptionRule[ApplicationOptions::MAX], ApplicationOptions::MAX);
    rules[ApplicationOptions::ApplicationDirectory].Init(0, 0, RF_IO::StandardRuleChecker::Text, 0, RF_IO::OptionRule::Required);
    rules[ApplicationOptions::Archive].Init("f", "archive", RF_IO::StandardRuleChecker::Text, "Specify an archive.", RF_IO::OptionRule::Required);
    rules[ApplicationOptions::Extract].Init("x", "extract", RF_IO::StandardRuleChecker::Text, "Extract all files.", RF_IO::OptionRule::Optional);
    rules[ApplicationOptions::Append].Init("a", "append", RF_IO::StandardRuleChecker::Text, "Append the specified directory to the archive.", RF_IO::OptionRule::Optional);
    
    if(consoleParameter.ParsingWithErrorOutput(argv, argc, rules, errorOut))
    {
        RF_Type::String archivePath;
        RF_IO::Uri archiveUri;

        // resolve symbolic links and relative paths into a absolute path
        RF_SysFile::RealPath(consoleParameter.Values()[ApplicationOptions::Archive].Value(),
            archivePath);
        // convert a system path into a URI
        RF_SysFile::SystemPathToUri(archivePath, archiveUri);

        if(consoleParameter.Values()[ApplicationOptions::Extract].IsSet())
        {
            RF_Type::String extractPath;
            RF_IO::Uri extractUri;
            // resolve symbolic links and relative paths into a absolute path
            RF_SysFile::RealPath(consoleParameter.Values()[ApplicationOptions::Extract].Value(),
                extractPath);
            // convert a system path into a URI
            RF_SysFile::SystemPathToUri(extractPath, extractUri);

            RF_IO::Directory destination;
            destination.SetLocation(extractUri);
            if(!destination.Exists())
            {
                destination.CreateNewDirectory();
            }

            PTS::ArgonA::VirtualArchiveReader reader;
            reader.Open(archivePath);

            auto files = reader.ListFiles();
            for (RF_Type::Size i = 0; i < files.Count(); ++i)
            {
                auto filename = reader.GetName(*files(i));
                auto filedata = reader.GetData(*files(i));
                auto file = destination.SubFile(filename);
                file->Write(filedata);
            }
        }

        if(consoleParameter.Values()[ApplicationOptions::Append].IsSet())
        {
            RF_Type::String appendPath;
            RF_IO::Uri appendUri;
            // resolve symbolic links and relative paths into a absolute path
            RF_SysFile::RealPath(consoleParameter.Values()[ApplicationOptions::Append].Value(),
                appendPath);
            // convert a system path into a URI
            RF_SysFile::SystemPathToUri(appendPath, appendUri);

            RF_IO::Directory content;
            content.SetLocation(appendUri);
            if (content.Exists())
            {
                RF_IO::Uri fileUri;
                PTS::ArgonA::VirtualArchiveWriter writer;
                writer.Open(archiveUri);

                auto files = content.FilesIncludingSubdirectories();
                for (RF_Type::Size i = 0; i < files.Count(); ++i)
                {
                    auto file = content.SubFile(files[i]);
                    if(file)
                    {
                        auto data = file->Read();
                        writer.Append(files[i], data);
                    }
                }

                writer.Close();
            }
        }
    }
    else
    {
        RF_IO::LogError(errorOut.c_str());
    }

    RF_Pattern::Singleton<RF_Thread::ThreadPool>::GetInstance().DisableAndWaitTillDone();
}