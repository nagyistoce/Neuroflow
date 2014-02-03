#include "stdafx.h"
#include "version.h"

#define MAJOR_VER 1
#define MINOR_VER 0
#define BUILD_VER 394
#define REVISION_VER 38867

std::wstring nf::version()
{
    return std::to_wstring(MAJOR_VER) + L"." + std::to_wstring(MINOR_VER) + L"." + std::to_wstring(BUILD_VER) + L"." + std::to_wstring(REVISION_VER);
}