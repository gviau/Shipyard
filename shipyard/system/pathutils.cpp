#include <system/pathutils.h>

namespace Shipyard
{;

void NormalizePath(StringT& path)
{
    shipChar* pBuffer = path.GetWriteBuffer();

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

    const shipChar* pBuffer = pathToNormalize.GetBuffer();
    shipChar* pOutBuffer = normalizedPath->GetWriteBuffer();

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