#ifndef LIBARGONA_H
#define LIBARGONA_H

#if defined(_WIN32)
#ifndef APIENTRY
#if defined(__MINGW32__) || defined(__CYGWIN__)
#define APIENTRY __stdcall
#elif (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) || defined(__BORLANDC__)
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

#ifndef ARGONAAPIENTRY
#define ARGONAAPIENTRY APIENTRY
#endif

#ifdef ARGONA_BUILD
#define ARGONAAPI extern __declspec(dllexport)
#else
#define ARGONAAPI extern __declspec(dllimport)
#endif

#else

#if defined(__GNUC__) && __GNUC__>=4
#define ARGONAAPI extern __attribute__ ((visibility("default")))
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#define ARGONAAPI extern __global
#else
#define ARGONAAPI extern
#endif

#ifndef ARGONAAPIENTRY
#define ARGONAAPIENTRY
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
typedef void* ARGONA_FILE;

typedef void* ARGONA_HANDLE;

static const ARGONA_HANDLE ARGONA_INVALID_HANDLE = 0;

static const ARGONA_FILE ARGONA_INVALID_FILE = 0;

ARGONAAPI void ARGONAAPIENTRY Initialize();

ARGONAAPI void ARGONAAPIENTRY Shutdown();

ARGONAAPI ARGONA_HANDLE ARGONAAPIENTRY Open(const char* Path, size_t PathLength);

ARGONAAPI ARGONA_HANDLE ARGONAAPIENTRY Create(const char* Path, size_t PathLength);

ARGONAAPI void ARGONAAPIENTRY Close(ARGONA_HANDLE ArchiveHandle);

ARGONAAPI bool ARGONAAPIENTRY Append(ARGONA_HANDLE ArchiveHandle, const char* Filename,
    size_t FilenameLength, const unsigned char* Data, size_t DataSize);

ARGONAAPI ARGONA_FILE ARGONAAPIENTRY Find(ARGONA_HANDLE ArchiveHandle, const char* Filename,
    size_t FilenameLength);

ARGONAAPI size_t ARGONAAPIENTRY GetData(ARGONA_HANDLE ArchiveHandle, ARGONA_FILE FileHandle,
    char** Buffer);

ARGONAAPI size_t ARGONAAPIENTRY GetName(ARGONA_HANDLE ArchiveHandle, ARGONA_FILE FileHandle, 
    char** Buffer);

ARGONAAPI void ARGONAAPIENTRY FreeBuffer(char** Buffer);

ARGONAAPI size_t ARGONAAPIENTRY GetContentSize(ARGONA_FILE FileHandle);

ARGONAAPI size_t ARGONAAPIENTRY Files(ARGONA_HANDLE ArchiveHandle);

ARGONAAPI ARGONA_FILE ARGONAAPIENTRY GetFile(ARGONA_HANDLE ArchiveHandle, size_t Index);
#ifdef __cplusplus
}
#endif

#endif // LIBARGONA_H