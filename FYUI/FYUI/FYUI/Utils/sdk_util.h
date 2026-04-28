#pragma once

#ifdef SVG
#include <functional>
#include <list>
#include <map>
#include <string>
#include "assert.h"
#include "wtypes.h"
// 1. 增加导出/导入宏定义
#ifdef FYUI_EXPORTS
#define FYUI_API __declspec(dllexport)
#else
#define FYUI_API __declspec(dllimport)
#endif

class SDK {
public:
   

    static SDK* GetInstance();


    virtual ~SDK();

    /** 加载SDK库 */
    bool LoadSdkDll(const char* cur_module_dir, const char* sdk_dll_file_name);

    /** 卸载SDK库 */
    void UnLoadSdkDll();

    /** SDK 初始化完成 */
    void OnSDKInited();

    /** 获得指定接口 */
    void* GetFunction(const std::string& function_name) {
        void* function_ptr = nullptr;
        auto it = function_map.find(function_name);
        if (it != function_map.end()) {
            function_ptr = it->second;
        } else {
            function_ptr = get_proc_address_wrapper_ == nullptr ? nullptr : get_proc_address_wrapper_(function_name.c_str());
            if (function_ptr)
                function_map[function_name] = function_ptr;
        }
     //   assert(function_ptr);
        return function_ptr;
    }

private:

    SDK(int i)
    {
        instance_nim_ = NULL;
        get_proc_address_wrapper_ = [](const std::string&) -> void* {
            return nullptr;
            };
        function_map.clear();
    }


    HINSTANCE instance_nim_;
    std::map<std::string, void*> function_map;
    std::function<void*(const std::string&)> get_proc_address_wrapper_;
};

FYUI_API extern SDK* g_SDK;
#endif