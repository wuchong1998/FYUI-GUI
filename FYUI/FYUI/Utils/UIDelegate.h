#pragma once

namespace FYUI {

    class FYUI_API CDelegateBase	 
    {
    public:
        /**
         * @brief 构造 CDelegateBase 对象
         * @details 用于构造 CDelegateBase 对象。具体行为由当前对象状态以及传入参数共同决定。
         * @param pObject [in] Object对象
         * @param pFn [in] 函数指针
         */
        CDelegateBase(void* pObject, void* pFn);
        /**
         * @brief 构造 CDelegateBase 对象
         * @details 用于构造 CDelegateBase 对象。具体行为由当前对象状态以及传入参数共同决定。
         * @param rhs [in] 用于比较或复制的源对象
         */
        CDelegateBase(const CDelegateBase& rhs);
        /**
         * @brief 析构 CDelegateBase 对象
         * @details 用于析构 CDelegateBase 对象。具体行为由当前对象状态以及传入参数共同决定。
         */
        virtual ~CDelegateBase();
        /**
         * @brief 执行 Equals 操作
         * @details 用于执行 Equals 操作。具体行为由当前对象状态以及传入参数共同决定。
         * @param rhs [in] 用于比较或复制的源对象
         * @return bool 操作成功返回 true，否则返回 false
         */
        bool Equals(const CDelegateBase& rhs) const;
        /**
         * @brief 执行 operator 运算
         * @details 用于执行 operator 运算。具体行为由当前对象状态以及传入参数共同决定。
         * @return bool 操作成功返回 true，否则返回 false
         */
        bool operator() (void* param);
        /**
         * @brief 执行 Copy 操作
         * @details 用于执行 Copy 操作。具体行为由当前对象状态以及传入参数共同决定。
         * @return CDelegateBase* 返回结果对象指针，失败时返回 nullptr
         */
        virtual CDelegateBase* Copy() const = 0; // add const for gcc

    protected:
        virtual void* GetFn() const;
        /**
         * @brief 获取Object
         * @details 用于获取Object。具体行为由当前对象状态以及传入参数共同决定。
         * @return void* 返回结果对象指针，失败时返回 nullptr
         */
        void* GetObject();
        /**
         * @brief 执行 Invoke 操作
         * @details 用于执行 Invoke 操作。具体行为由当前对象状态以及传入参数共同决定。
         * @param param [in] 回调参数
         * @return bool 操作成功返回 true，否则返回 false
         */
        virtual bool Invoke(void* param) = 0;

    private:
        void* m_pObject;
        void* m_pFn;
    };

    class CDelegateStatic: public CDelegateBase
    {
        typedef bool (*Fn)(void*);
    public:
        /**
         * @brief 构造 CDelegateStatic 对象
         * @details 用于构造 CDelegateStatic 对象。具体行为由当前对象状态以及传入参数共同决定。
         * @param pFn [in] 函数指针
         */
        CDelegateStatic(Fn pFn) : CDelegateBase(NULL, pFn) { } 
        CDelegateStatic(const CDelegateStatic& rhs) : CDelegateBase(rhs) { } 
        virtual CDelegateBase* Copy() const { return new CDelegateStatic(*this); }

    protected:
        virtual bool Invoke(void* param)
        {
            Fn pFn = (Fn)GetFn();
            return (*pFn)(param); 
        }
    };

    template <class O, class T>
    class CDelegate : public CDelegateBase
    {
        typedef bool (T::* Fn)(void*);
    public:
        /**
         * @brief 构造 CDelegate 对象
         * @details 用于构造 CDelegate 对象。具体行为由当前对象状态以及传入参数共同决定。
         * @param pObj [in] 对象实例
         * @param pFn [in] 函数指针
         */
        CDelegate(O* pObj, Fn pFn) : CDelegateBase(pObj, &pFn), m_pFn(pFn) { }
        CDelegate(const CDelegate& rhs) : CDelegateBase(rhs) { m_pFn = rhs.m_pFn; } 
        virtual CDelegateBase* Copy() const { return new CDelegate(*this); }
        virtual void* GetFn() const { return *(void**)&m_pFn; }

    protected:
        virtual bool Invoke(void* param)
        {
            O* pObject = (O*) GetObject();
            return (pObject->*m_pFn)(param); 
        }  

    private:
        Fn m_pFn;
    };


    template <class O, class T>
    /**
     * @brief 执行 MakeDelegate 操作
     * @details 用于执行 MakeDelegate 操作。具体行为由当前对象状态以及传入参数共同决定。
     * @param pObject [in] Object对象
     * @param param [in] 回调参数
     * @return CDelegate<O, T> 返回 执行 MakeDelegate 操作 的结果
     */
    CDelegate<O, T> MakeDelegate(O* pObject, bool (T::* pFn)(void*))
    {
        return CDelegate<O, T>(pObject, pFn);
    }

    inline CDelegateStatic MakeDelegate(bool (*pFn)(void*))
    {
        return CDelegateStatic(pFn); 
    }

    class FYUI_API CEventSource
    {
        typedef bool (*FnType)(void*);
    public:
        /**
         * @brief 析构 CEventSource 对象
         * @details 用于析构 CEventSource 对象。具体行为由当前对象状态以及传入参数共同决定。
         */
        ~CEventSource();
        /**
         * @brief 执行 operator bool 运算
         * @details 用于执行 operator bool 运算。具体行为由当前对象状态以及传入参数共同决定。
         */
        operator bool();
        /**
         * @brief 执行 operator+= 运算
         * @details 用于执行 operator+= 运算。具体行为由当前对象状态以及传入参数共同决定。
         * @param d [in] d参数
         */
        void operator+= (const CDelegateBase& d); // add const for gcc
        void operator+= (FnType pFn);
        /**
         * @brief 执行 operator-= 运算
         * @details 用于执行 operator-= 运算。具体行为由当前对象状态以及传入参数共同决定。
         * @param d [in] d参数
         */
        void operator-= (const CDelegateBase& d);
        /**
         * @brief 执行 operator-= 运算
         * @details 用于执行 operator-= 运算。具体行为由当前对象状态以及传入参数共同决定。
         * @param pFn [in] 函数指针
         */
        void operator-= (FnType pFn);
        /**
         * @brief 执行 operator 运算
         * @details 用于执行 operator 运算。具体行为由当前对象状态以及传入参数共同决定。
         * @return bool 操作成功返回 true，否则返回 false
         */
        bool operator() (void* param);
        /**
         * @brief 执行 Clear 操作
         * @details 用于执行 Clear 操作。具体行为由当前对象状态以及传入参数共同决定。
         */
        void Clear();

    protected:
        CStdPtrArray m_aDelegates;
    };

} 
