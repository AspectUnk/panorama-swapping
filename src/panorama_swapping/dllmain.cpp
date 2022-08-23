#include <stdio.h>
#include <Windows.h>
#include <map>

#include <MinHook.h>

#pragma comment(lib, "libMinHook.x86.lib")

/// Replacement files

std::map<const char*, const char*> files = {
    { "panorama\\layout\\mainmenu_inventory.xml", "C:\\mainmenu_inventory.xml" }
};

///

template<typename Fn>
Fn GetFunction(PVOID address, int idx)
{
    PVOID* pVTable = *(PVOID**)address;
    return reinterpret_cast<Fn>(pVTable[idx]);
}

class IZip
{
public:
    inline void AddFileToZip(const char* relativename, const char* fullpath)
    {
        return GetFunction<void(__thiscall*)(IZip*, const char*, const char*)>(this, 1)(this, relativename, fullpath);
    }

    inline bool FileExistsInZip(const char* pRelativeName)
    {
        return GetFunction<bool(__thiscall*)(IZip*, const char*)>(this, 2)(this, pRelativeName);
    }

    inline void RemoveFileFromZip(const char* relativename)
    {
        return GetFunction<void(__thiscall*)(IZip*, const char*)>(this, 5)(this, relativename);
    }
    
    inline void ParseFromBuffer(void* buffer, int bufferlength)
    {
        return GetFunction<void (__thiscall*)(IZip*, void*, int)>(this, 14)(this, buffer, bufferlength);
    }
};

typedef void(__fastcall* FnParseFromBuffer)(IZip* ecx, int edx, void* buffer, int bufferlength);
FnParseFromBuffer oParseFromBuffer = nullptr;

void __fastcall ParseFromBuffer_Hooked(IZip* ecx, int edx, void* buffer, int bufferlength)
{
    printf("\t[+] Hook called. ECX: 0x%p\n", ecx);

    oParseFromBuffer(ecx, edx, buffer, bufferlength);

    for (const auto& [relative, path] : files)
    {
        printf("\t[?] File %s\n", relative);
        if (!ecx->FileExistsInZip(relative))
        {
            printf("\t    %c%c Not found in archive\n", 192, 196);
            continue;
        }

        ecx->RemoveFileFromZip(relative);
        ecx->AddFileToZip(relative, path);

        printf("\t    %c%c Swapped to %s\n", 192, 196, path);
    }

    if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK)
    {
        printf("\t[-] Failed to remove hook\n");
        return;
    }

    printf("\t[+] Hook removed\n");
}

PVOID FindPattern(PVOID base, size_t length, const char* pattern, const char* mask)
{
    length -= strlen(mask);

    for (int i = 0; i <= length; i++)
    {
        bool found = true;

        for (int p = 0; mask[p]; p++)
        {
            if (mask[p] == 'x' && pattern[p] != static_cast<char*>(base)[i + p])
            {
                found = false;
                break;
            }
        }

        if (found)
            return static_cast<PBYTE>(base) + i;
    }

    return nullptr;
}

void NewThread(HMODULE hModule)
{
    FILE* file = nullptr;
    HMODULE hPanorama = nullptr;
    PVOID pParseFromBuffer = nullptr;
    
    AllocConsole();
    AttachConsole(ATTACH_PARENT_PROCESS);

    freopen_s(&file, "CONOUT$", "w", stdout);
    if (!file)
        goto exit;
    
    while (hPanorama == nullptr)
    {
        hPanorama = GetModuleHandle(L"panorama.dll");
        Sleep(1);
    }

    pParseFromBuffer = FindPattern(hPanorama, 0x2A86E0, "\x55\x8B\xEC\x56\x8D\x71\x04\x8D\x4E\x0C\xE8", "xxxxxxxxxxx");
    if (!pParseFromBuffer)
    {
        printf("\t[-] Failed to find ParseFromBuffer by pattern\n");
        goto exit;
    }

    printf("\t[?] ParseFromBuffer: 0x%p\n", pParseFromBuffer);

    if (MH_Initialize() != MH_OK)
    {
        printf("\t[-] MinHook not initialized\n");
        goto exit;
    }

    if (MH_CreateHook(pParseFromBuffer, &ParseFromBuffer_Hooked, (LPVOID*)&oParseFromBuffer) != MH_OK)
    {
        printf("\t[-] Failed to set hook\n");
        goto exit;
    }

    if (MH_EnableHook(pParseFromBuffer) != MH_OK)
    {
        printf("\t[-] Failed to enable hook");
        goto exit;
    }

    printf("\t[+] Hook established\n");

    while (true)
    {
        if (GetAsyncKeyState(VK_DELETE) & 0x1)
            break;

        Sleep(1);
    }

exit:

    if (file)
    {
        printf("\t[?] Unloaded\n");
        fclose(file);
    }

    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)NewThread, hModule, NULL, nullptr);
    }
    
    return TRUE;
}

