#include <system/pathutils.h>

namespace Shipyard
{;

void NormalizePath(StringT& path)
{
    char* pBuffer = path.GetWriteBuffer();

    while (*pBuffer != '\0')
    {
        if (*pBuffer == '\\')
        {
            *pBuffer = '/';
        }

        pBuffer++;
    }
}

void NormalizePath(const StringT& pathToNormalize, StringT* normalizedPath)
{
    normalizedPath->Resize(pathToNormalize.Size());

    const char* pBuffer = pathToNormalize.GetBuffer();
    char* pOutBuffer = normalizedPath->GetWriteBuffer();

    while (*pBuffer != '\0')
    {
        if (*pBuffer == '\\')
        {
            *pOutBuffer = '/';
        }
        else
        {
            *pOutBuffer = *pBuffer;
        }

        pBuffer++;
        pOutBuffer++;
    }
}

}