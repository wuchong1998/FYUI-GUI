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
   

    /**
     * @brief 获取Instance
     * @details 用于获取Instance。具体行为由当前对象状态以及传入参数共同决定。
     * @return SDK* 返回结果对象指针，失败时返回 nullptr
     */
    static SDK* GetInstance();


    /**
     * @brief 析构 SDK 对象
     * @details 用于析构 SDK 对象。具体行为由当前对象状态以及传入参数共同决定。
     */
    virtual ~SDK();

    /** 加载SDK库 */
    /**
     * @brief 加载Sdk动态库
     * @details 用于加载Sdk动态库。具体行为由当前对象状态以及传入参数共同决定。
     * @param cur_module_dir [in] 当前moduledir参数
     * @param sdk_dll_file_name [in] sdk动态库文件名称参数
     * @return bool 操作成功返回 true，否则返回 false
     */
    bool LoadSdkDll(const char* cur_module_dir, const char* sdk_dll_file_name);

    /** 卸载SDK库 */
    /**
     * @brief 执行 UnLoadSdkDll 操作
     * @details 用于执行 UnLoadSdkDll 操作。具体行为由当前对象状态以及传入参数共同决定。
     */
    void UnLoadSdkDll();

    /** SDK 初始化完成 */
    /**
     * @brief 执行 OnSDKInited 操作
     * @details 用于执行 OnSDKInited 操作。具体行为由当前对象状态以及传入参数共同决定。
     */
    void OnSDKInited();

    /** 获得指定接口 */
    /**
     * @brief 获取Function
     * @details 用于获取Function。具体行为由当前对象状态以及传入参数共同决定。
     * @param function_name [in] function名称参数
     * @return void* 返回结果对象指针，失败时返回 nullptr
     */
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

    /**
     * @brief 构造 SDK 对象
     * @details 用于构造 SDK 对象。具体行为由当前对象状态以及传入参数共同决定。
     * @param i [in] i参数
     */
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
