// Setup.cpp : Este archivo contiene la funci√≥n "main". La ejecuci√≥n del programa comienza y termina ah√≠.
//

#include <iostream>
#include <fstream>
#include <windows.h>
#include <tlhelp32.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dsound.h>
#include <mmsystem.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "winmm.lib")


// Variable global para contar los dispositivos
int deviceCount = 0;
BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
    deviceCount++;
    std::cout << "Sound Device: " << lpcstrDescription << std::endl;
    return TRUE; // Continue enumeration
}

int ListSoundDevices()
{   
    deviceCount = 0; // Reset the device count
    DirectSoundEnumerateA(DSEnumCallback, nullptr);
    return deviceCount;
}

int ListSoundDevicesWinmm()
{
    UINT numDevices = waveOutGetNumDevs();
    std::cout << "Number of sound devices: " << numDevices << std::endl;
	int count = 0;

    for (UINT i = 0; i < numDevices; ++i)
    {
        WAVEOUTCAPS caps;
        if (waveOutGetDevCaps(i, &caps, sizeof(WAVEOUTCAPS)) == MMSYSERR_NOERROR)
        {
            std::wcout << L"Device " << i << L": " << caps.szPname << std::endl;
            std::cout << "  Manufacturer ID: " << caps.wMid << std::endl;
            std::cout << "  Product ID: " << caps.wPid << std::endl;
            std::cout << "  Driver Version: " << HIWORD(caps.vDriverVersion) << "." << LOWORD(caps.vDriverVersion) << std::endl;
            std::cout << "  Channels: " << caps.wChannels << std::endl;
			count++;
		}
    }
	return count;
}


int ListDisplayAdapters()
{

    IDXGIFactory* pFactory = nullptr;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);

    if (FAILED(hr))
    {
        std::cerr << "Failed to create DXGIFactory." << std::endl;
        return 1;
    }

    UINT i = 0;
    IDXGIAdapter* pAdapter = nullptr;
    while (pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        pAdapter->GetDesc(&desc);

        std::wcout << L"Adapter " << i << L": " << desc.Description << std::endl;
        //std::wcout << L"  Dedicated Video Memory: " << desc.DedicatedVideoMemory / 1024 / 1024 << L" MB" << std::endl;
        //std::wcout << L"  Dedicated System Memory: " << desc.DedicatedSystemMemory / 1024 / 1024 << L" MB" << std::endl;
        //std::wcout << L"  Shared System Memory: " << desc.SharedSystemMemory / 1024 / 1024 << L" MB" << std::endl;

        pAdapter->Release();
        i++;
    }

    pFactory->Release();
    return 0;
}

int CheckDirectXSupport()
{
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    D3D_FEATURE_LEVEL featureLevel;
    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // Adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,                    // Software
        0,                          // Flags
        featureLevels,              // Feature levels
        _countof(featureLevels),    // Number of feature levels
        D3D11_SDK_VERSION,
        &pDevice,
        &featureLevel,
        &pContext
    );

    if (SUCCEEDED(hr)) {
        std::cout << "DirectX Feature Level: " << std::hex << featureLevel << std::endl;
        pDevice->Release();
        pContext->Release();
		return 0;
    }  else {
        std::cerr << "Failed to create Direct3D device." << std::endl;
		return 1;
    }
}


int GetDirectXVersion() {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\DirectX"), 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        return 1;
    }

    DWORD dataType;
    char version[256];
    DWORD dataSize = sizeof(version);
    if (RegQueryValueEx(hKey, TEXT("Version"), nullptr, &dataType, (LPBYTE)version, &dataSize) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return 1;
    }

    RegCloseKey(hKey);
	std::cout << "DirectX Version: " << version << std::endl;
    return 0;
}



bool CheckRegistryKeyExists(HKEY hKeyRoot, LPCSTR subkey) {
    HKEY hKey;
    LONG result = RegOpenKeyExA(hKeyRoot, subkey, 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool CheckRegistryValueExists(HKEY hKeyRoot, LPCSTR subkey, LPCSTR valueName) {
    HKEY hKey;
    LONG result = RegOpenKeyExA(hKeyRoot, subkey, 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        DWORD dataType;
        result = RegQueryValueExA(hKey, valueName, NULL, &dataType, NULL, NULL);
        RegCloseKey(hKey);
        return (result == ERROR_SUCCESS);
    }
    return false;
}

bool CreateRegistryKey(HKEY hKeyRoot, LPCSTR subkey) {
    HKEY hKey;
    LONG result = RegCreateKeyExA(
        hKeyRoot, 
        subkey, 
        0, 
        NULL, 
        REG_OPTION_NON_VOLATILE, 
        KEY_ALL_ACCESS, 
        NULL, 
        &hKey, 
        NULL
    );

    if (result != ERROR_SUCCESS) {
        std::cerr << "Error al crear la clave " << subkey << ": " << result << std::endl;
        return false;
    }
    RegCloseKey(hKey);
    return true;
}

// FunciÛn para aÒadir un valor a una clave del registro
bool SetRegistryValue(HKEY hKeyRoot, const std::string& subkey, const std::string& valueName, 
        DWORD dataType, const BYTE* data, DWORD dataSize) {
    HKEY hKey;
    LONG result = RegOpenKeyExA(hKeyRoot, subkey.c_str(), 0, KEY_SET_VALUE, &hKey);
    if (result != ERROR_SUCCESS) {
        std::cerr << "Error al abrir la clave " << subkey << ": " << result << std::endl;
        return false;
    }

    result = RegSetValueExA(hKey, valueName.c_str(), 0, dataType, data, dataSize);
    if (result != ERROR_SUCCESS) {
        std::cerr << "Error al establecer el valor " << valueName << " en la clave " << subkey << ": " << result << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}


int create_reg_hard() {

    if (!CreateRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok")) return 1;
    if (!CreateRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\BookMark")) return 1;
    if (!CreateRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\BookMark\\Black Sky RO")) return 1;
    if (!CreateRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\NameBaloon")) return 1;
    if (!CreateRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\NameBaloon\\Black Sky RO")) return 1;
    if (!CreateRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\NeoCash")) return 1;
    if (!CreateRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option")) return 1;
    if (!CreateRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortcutItem")) return 1;
    if (!CreateRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortcutItem\\Black Sky RO")) return 1;
    if (!CreateRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortCutList")) return 1;
    if (!CreateRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\SkillUseLevelInfo")) return 1;
    if (!CreateRegistryKey(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\RenewSetup")) return 1;


    DWORD value = 0x10;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "BITPERPIXEL", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "BITWISE", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "CHECKSAVE", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x2;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "CURSLOT", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "DEVICECNT", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "DIGITALBITSTYPE", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "DIGITALRATETYPE", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "FOG", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    BYTE guidDevice[] = { 0xe0, 0x3d, 0xe6, 0x84, 0xaa, 0x46, 0xcf, 0x11, 0x81, 0x6f, 0x00, 0x00, 0xc0, 0x20, 0x15, 0x6e };
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "GUIDDEVICE", REG_BINARY, guidDevice, sizeof(guidDevice))) return 1;

    BYTE guidDriver[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "GUIDDRIVER", REG_BINARY, guidDriver, sizeof(guidDriver))) return 1;

    value = 0x3e8;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "HEIGHT", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "ISFULLSCREENMODE", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "ISLIGHTMAP", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "ISVOODOO", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0xa;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "MODECNT", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "NUMACCOUNT", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x2;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "NUMSAMPLETYPE", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "NUMSERVER", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "SHOWTIPSATSTARTUP", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "SOUNDMODE", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "SPEAKERTYPE", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x2;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "SPRITEMODE", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x2;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "TEXTUREMODE", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "TRILINEARFILTER", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x780;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "WIDTH", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok", "ID", REG_SZ, (const BYTE*)"narradord", strlen("narradord") + 1)) return 1;
    //if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\BookMark\\Black Sky RO\\Belzebub", "Level 1 Cookbook", REG_DWORD, (const BYTE*)&(DWORD) { 0x1 }, sizeof(DWORD))) return 1;

    //if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\BookMark\\Black Sky RO\\George", "Adventure Story Vol.1", REG_DWORD, (const BYTE*)&(DWORD) { 0x1 }, sizeof(DWORD))) return 1;
    //if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\BookMark\\Black Sky RO\\George", "Level 4 Cookbook", REG_DWORD, (const BYTE*)&(DWORD) { 0x1 }, sizeof(DWORD))) return 1;
    //if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\BookMark\\Black Sky RO\\George", "Rune Royal Family Book", REG_DWORD, (const BYTE*)&(DWORD) { 0x3 }, sizeof(DWORD))) return 1;

    // AÒadir valores a las subclaves
    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\NameBaloon\\Black Sky RO", "POS", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\NeoCash", "CASH_CATEGORY", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "bgmIsPaused", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x01317c0b;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "EXAID0", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x01315219;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "EXAID1", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x43660000;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "g_indoorViewDistance", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0xc2200000;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "g_indoorViewLatitude", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "g_isFixedCamera", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x44070ccb;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "g_outdoorViewDistance", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0xc2200000;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "g_outdoorViewLatitude", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "isEffectOn", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "isSoundOn", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "m_bAutoOpenDetailWindowIfLowMemory", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "m_bMakeMissEffect", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "m_isItemSnap", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "m_isNoCtrl", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "m_isOpaque", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "m_isPlayWhisperOpenSound", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "m_isShowWhisperWnd", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "m_isShowWhisperWnd_Friend", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "m_monsterSnapOn_NoSkill", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "m_monsterSnapOn_Skill", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "NUMEXAID", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "NUMEXNAME", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "onHoUserAI", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "onMerUserAI", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x64;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "soundVolume", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    value = 0x45;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "streamVolume", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;

    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "SKINNAME", REG_SZ, (const BYTE*)"<Basic Skin>", strlen("<Basic Skin>") + 1)) return 1;

    //if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "EXNAME0", REG_SZ, (const BYTE*)"Eric Corvinus", strlen("Eric Corvinus") + 1)) return 1;

    //if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option", "EXNAME1", REG_SZ, (const BYTE*)"Jovencita", strlen("Jovencita") + 1)) return 1;

    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortCutList", "0", REG_SZ, (const BYTE*)"/who", strlen("/who") + 1)) return 1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortCutList", "1", REG_SZ, (const BYTE*)"/who", strlen("/who") + 1)) return 1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortCutList", "2", REG_SZ, (const BYTE*)"/who", strlen("/who") + 1)) return 1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortCutList", "3", REG_SZ, (const BYTE*)"/who", strlen("/who") + 1)) return 1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortCutList", "4", REG_SZ, (const BYTE*)"/who", strlen("/who") + 1)) return 1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortCutList", "5", REG_SZ, (const BYTE*)"/who", strlen("/who") + 1)) return 1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortCutList", "6", REG_SZ, (const BYTE*)"/who", strlen("/who") + 1)) return 1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortCutList", "7", REG_SZ, (const BYTE*)"/who", strlen("/who") + 1)) return 1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortCutList", "8", REG_SZ, (const BYTE*)"/who", strlen("/who") + 1)) return 1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortCutList", "9", REG_SZ, (const BYTE*)"/who", strlen("/who") + 1)) return 1;

    value = 0x0;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\SkillUseLevelInfo", "0", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\SkillUseLevelInfo", "2", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;
    if (!SetRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\SkillUseLevelInfo", "4", REG_DWORD, (const BYTE*)&value, sizeof(DWORD))) return 1;


    std::cout << "Todas las claves se crearon exitosamente." << std::endl;
    return 0;
}

int create_reg() {

    const char* fileName = "tools\\blasky.reg";
    char buffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buffer);

	// Comprueba si existe el archivo .reg
	std::ifstream file(fileName);
	if (!file) {
		std::cerr << "No se encontrÛ el archivo: " << fileName << std::endl;
		// Print work path
		
		std::cout << "Current path: " << buffer << std::endl;
		return 1;
	} else {
		file.close();
	}

    std::string command = "regedit ";
    command += "\"";
    command += buffer;
    command += "\\";
    command += fileName;
    command += "\"";
    int result = system(command.c_str());

    if (result != 0) {
        std::cerr << "Error ejecutando el archivo .reg" << std::endl;
        return 1;
    }

    std::cout << "Archivo .reg ejecutado con Èxito." << std::endl;
	return 0;

}

// Mata el proceso BlackSkyRO.dll o BlackSkyROc.exe
int kill_process(){

	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		std::cerr << "Error al crear snapshot" << std::endl;
		return 1;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32)) {
		std::cerr << "Error al obtener el primer proceso" << std::endl;
		CloseHandle(hProcessSnap);
		return 1;
	}

	do {
		//  e32.szExeFile to string

		if (wcscmp(pe32.szExeFile, L"BlackSkyRO.dll") == 0 || wcscmp(pe32.szExeFile, L"BlackSkyROc.exe") == 0) {
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
			if (hProcess != NULL) {
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
            } else {
                std::cout << "No se ha podido finalizar el proceso" << std::endl;
            }
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return 0;
}

/*

[HKEY_CURRENT_USER\SOFTWARE\Gravity Soft\Ragnarok]
[HKEY_CURRENT_USER\SOFTWARE\Gravity Soft\Ragnarok\BookMark]
[HKEY_CURRENT_USER\SOFTWARE\Gravity Soft\Ragnarok\BookMark\Black Sky RO]
[HKEY_CURRENT_USER\SOFTWARE\Gravity Soft\Ragnarok\NameBaloon]
[HKEY_CURRENT_USER\SOFTWARE\Gravity Soft\Ragnarok\NameBaloon\Black Sky RO]
[HKEY_CURRENT_USER\SOFTWARE\Gravity Soft\Ragnarok\NeoCash]
[HKEY_CURRENT_USER\SOFTWARE\Gravity Soft\Ragnarok\Option]
[HKEY_CURRENT_USER\SOFTWARE\Gravity Soft\Ragnarok\ShortcutItem]
[HKEY_CURRENT_USER\SOFTWARE\Gravity Soft\Ragnarok\ShortcutItem\Black Sky RO]
[HKEY_CURRENT_USER\SOFTWARE\Gravity Soft\Ragnarok\ShortCutList]
[HKEY_CURRENT_USER\SOFTWARE\Gravity Soft\Ragnarok\SkillUseLevelInfo]
[HKEY_CURRENT_USER\SOFTWARE\Gravity Soft\RenewSetup]

*/

int check_reg() {
    return CheckRegistryKeyExists(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok") &&
        CheckRegistryKeyExists(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\BookMark") &&
        CheckRegistryKeyExists(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\BookMark\\Black Sky RO") &&
        CheckRegistryKeyExists(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\NameBaloon") &&
        CheckRegistryKeyExists(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\NameBaloon\\Black Sky RO") &&
        CheckRegistryKeyExists(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\NeoCash") &&
        CheckRegistryKeyExists(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\Option") &&
        CheckRegistryKeyExists(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortcutItem") &&
        CheckRegistryKeyExists(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortcutItem\\Black Sky RO") &&
        CheckRegistryKeyExists(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\ShortCutList") &&
        CheckRegistryKeyExists(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\Ragnarok\\SkillUseLevelInfo") &&
        CheckRegistryKeyExists(HKEY_CURRENT_USER, "SOFTWARE\\Gravity Soft\\RenewSetup") ? 1 : 0;
}


int main()
{
    int error = 0;
    std::cout << "Configurando Black Sky RO...\n";

    // Paso 1: Crear registros en el sistema
    std::cout << "Comprobando registro...\n";
    if (check_reg() == 0) {
        std::cout << "Faltan registros... se van a crear\n";
        if (create_reg_hard()) {
            error = 1;
        }
    }

   
    // Paso 2: Comprueba si existe la carpeta savedata. Si no existe, abre "BlaskySetup.exe"
    const char* dir = "savedata\\OptionInfo.lua";
    struct stat sb;
	if (stat(dir, &sb)) {
		std::cout << "No se ha encontrado la carpeta savedata\n";
		std::cout << "Ejecutando BlaskySetup.exe\n";
		// Ejecutar BlaskySetup.exe y espera a que se cierre
        system("BlaskySetup.exe");
		//ShellExecuteA(NULL, "open", "BlaskySetup.exe", NULL, NULL, SW_SHOWNORMAL);
	}

    //Paso 3: Comprueba directx y los adaptadores
    if (ListDisplayAdapters() || CheckDirectXSupport() || GetDirectXVersion()) {
        // Muestra un mensaje de error msgbox popup
		std::cerr << "Error al listar los adaptadores de pantalla" << std::endl;
        std::cerr << "Comprueba los drivers de la tarjeta gr·fica y la instalaciÛn de DirectX" << std::endl;
        error = 1;
    }

    // Paso 4: Comprueba el sonido
    if (ListSoundDevices() == 0 || ListSoundDevicesWinmm() == 0) {
        std::cerr << "Error al listar las tarjetas de sonido" << std::endl;
        std::cerr << "Prueba a instalar Media Feature Pack de Windows" << std::endl;
        error = 1;
    }


    if (error) {
        std::cout << "Ha ocurrido un error con la configuraciÛn" << std::endl;
        kill_process();
        //Pause
        std::cin.get();
        return 1;
    }


    return 0;
}

// Ejecutar programa: Ctrl + F5 o men√∫ Depurar > Iniciar sin depurar
// Depurar programa: F5 o men√∫ Depurar > Iniciar depuraci√≥n

// Sugerencias para primeros pasos: 1. Use la ventana del Explorador de soluciones para agregar y administrar archivos
//   2. Use la ventana de Team Explorer para conectar con el control de c√≥digo fuente
//   3. Use la ventana de salida para ver la salida de compilaci√≥n y otros mensajes
//   4. Use la ventana Lista de errores para ver los errores
//   5. Vaya a Proyecto > Agregar nuevo elemento para crear nuevos archivos de c√≥digo, o a Proyecto > Agregar elemento existente para agregar archivos de c√≥digo existentes al proyecto
//   6. En el futuro, para volver a abrir este proyecto, vaya a Archivo > Abrir > Proyecto y seleccione el archivo .sln
