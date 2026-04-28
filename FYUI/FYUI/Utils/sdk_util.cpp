#include "pch.h"
#include "sdk_util.h"

#ifdef SVG
SDK* g_SDK = nullptr;

SDK* SDK::GetInstance()
{
    static SDK tp(1);
    return &tp;
}

SDK::~SDK() {
    function_map.clear();
}

bool SDK::LoadSdkDll(const char* cur_module_dir, const char* sdk_dll_file_name) {
    if (sdk_dll_file_name == nullptr)
        return false;

    std::string dir = (sdk_dll_file_name);

    instance_nim_ = ::LoadLibraryA(dir.c_str());
    if (instance_nim_ == NULL) {
        auto error = ::GetLastError();
		//MessageBox(NULL, L"¼ÓÔØSVG¶¯Ì¬¿âÊ§°Ü£¡", L"´íÎó", MB_OK);
        return false;
    }
    get_proc_address_wrapper_ = [&](const std::string& function_name) -> void* {
        return ::GetProcAddress(instance_nim_, function_name.c_str());
    };
    return true;
}

void SDK::UnLoadSdkDll() {
    assert(instance_nim_);
    get_proc_address_wrapper_ = nullptr;
    function_map.clear();
    if (instance_nim_) {
        ::FreeLibrary(instance_nim_);
        instance_nim_ = NULL;
    }
}

void SDK::OnSDKInited() {
}
#endif


