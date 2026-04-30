#pragma once

#include <shlobj.h>
#include <vector>
namespace FYUI {
	////////////////////////////////////////////////////////////////////////////////
	///
	typedef std::vector<FORMATETC> FormatEtcArray;
	typedef std::vector<STGMEDIUM> StgMediumArray;

	////////////////////////////////////////////////////////////////////////////////
	///
	class FYUI_API CEnumFormatEtc : public IEnumFORMATETC
	{
	private:
		ULONG           m_cRefCount;
		FormatEtcArray  m_pFmtEtc;
		size_t           m_iCur;

	public:
		/**
		 * @brief 构造 CEnumFormatEtc 对象
		 * @details 用于构造 CEnumFormatEtc 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param ArrFE [in] ArrFE参数
		 */
		CEnumFormatEtc(const FormatEtcArray& ArrFE);
		//IUnknown members
		/**
		 * @brief 执行 STDMETHOD 操作
		 * @details 用于执行 STDMETHOD 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param QueryInterface [in] Query接口参数
		 */
		STDMETHOD(QueryInterface)(REFIID, void FAR* FAR*);
		/**
		 * @brief 执行 STDMETHOD_ 操作
		 * @details 用于执行 STDMETHOD_ 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param ULONG [in] ULONG参数
		 * @param AddRef [in] 添加Ref参数
		 */
		STDMETHOD_(ULONG, AddRef)(void);
		/**
		 * @brief 执行 STDMETHOD_ 操作
		 * @details 用于执行 STDMETHOD_ 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param ULONG [in] ULONG参数
		 * @param Release [in] Release参数
		 */
		STDMETHOD_(ULONG, Release)(void);

		//IEnumFORMATETC members
		/**
		 * @brief 执行 STDMETHOD 操作
		 * @details 用于执行 STDMETHOD 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param Next [in] Next参数
		 */
		STDMETHOD(Next)(ULONG, LPFORMATETC, ULONG FAR *);
		/**
		 * @brief 执行 STDMETHOD 操作
		 * @details 用于执行 STDMETHOD 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param Skip [in] Skip参数
		 */
		STDMETHOD(Skip)(ULONG);
		/**
		 * @brief 执行 STDMETHOD 操作
		 * @details 用于执行 STDMETHOD 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param Reset [in] 重置参数
		 */
		STDMETHOD(Reset)(void);
		/**
		 * @brief 执行 STDMETHOD 操作
		 * @details 用于执行 STDMETHOD 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param Clone [in] 克隆参数
		 */
		STDMETHOD(Clone)(IEnumFORMATETC FAR * FAR*);
	};

	////////////////////////////////////////////////////////////////////////////////
	///
	class FYUI_API CIDropSource : public IDropSource
	{
		long m_cRefCount;
	public:
		bool m_bDropped;
		/**
		 * @brief 构造 CIDropSource 对象
		 * @details 用于构造 CIDropSource 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CIDropSource():m_cRefCount(0),m_bDropped(false) {}
		//IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);        
		/**
		 * @brief 添加Ref
		 * @details 用于添加Ref。具体行为由当前对象状态以及传入参数共同决定。
		 * @return ULONG STDMETHODCALLTYPE 返回 添加Ref 的结果
		 */
		virtual ULONG STDMETHODCALLTYPE AddRef( void);
		/**
		 * @brief 执行 Release 操作
		 * @details 用于执行 Release 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return ULONG STDMETHODCALLTYPE 返回 执行 Release 操作 的结果
		 */
		virtual ULONG STDMETHODCALLTYPE Release( void);
		//IDropSource
		/**
		 * @brief 执行 QueryContinueDrag 操作
		 * @details 用于执行 QueryContinueDrag 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param fEscapePressed [in] fEscapePressed参数
		 * @param grfKeyState [in] grfKey状态参数
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 QueryContinueDrag 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag( 
			/* [in] */ BOOL fEscapePressed,
			/* [in] */ DWORD grfKeyState);

		/**
		 * @brief 执行 GiveFeedback 操作
		 * @details 用于执行 GiveFeedback 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwEffect [in] Effect数值
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 GiveFeedback 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE GiveFeedback( 
			/* [in] */ DWORD dwEffect);
	};

	////////////////////////////////////////////////////////////////////////////////
	///
	class FYUI_API CIDataObject : public IDataObject//,public IAsyncOperation
	{
		CIDropSource* m_pDropSource;
		long m_cRefCount;
		FormatEtcArray m_ArrFormatEtc;
		StgMediumArray m_StgMedium;

	public:
		/**
		 * @brief 构造 CIDataObject 对象
		 * @details 用于构造 CIDataObject 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pDropSource [in] 拖放来源对象
		 */
		CIDataObject(CIDropSource* pDropSource);
		/**
		 * @brief 析构 CIDataObject 对象
		 * @details 用于析构 CIDataObject 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CIDataObject();
		/**
		 * @brief 复制Medium
		 * @details 用于复制Medium。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pMedDest [in] MedDest对象
		 * @param pMedSrc [in] MedSrc对象
		 * @param pFmtSrc [in] FmtSrc对象
		 */
		void CopyMedium(STGMEDIUM* pMedDest, const STGMEDIUM* pMedSrc, const FORMATETC* pFmtSrc);
		//IUnknown
		/**
		 * @brief 执行 QueryInterface 操作
		 * @details 用于执行 QueryInterface 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param riid [in] riid参数
		 * @param ppvObject [in] ppvObject参数
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 QueryInterface 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);        
		/**
		 * @brief 添加Ref
		 * @details 用于添加Ref。具体行为由当前对象状态以及传入参数共同决定。
		 * @return ULONG STDMETHODCALLTYPE 返回 添加Ref 的结果
		 */
		virtual ULONG STDMETHODCALLTYPE AddRef( void);
		/**
		 * @brief 执行 Release 操作
		 * @details 用于执行 Release 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return ULONG STDMETHODCALLTYPE 返回 执行 Release 操作 的结果
		 */
		virtual ULONG STDMETHODCALLTYPE Release( void);

		//IDataObject
		/**
		 * @brief 获取数据
		 * @details 按指定的数据格式读取当前数据对象中的内容，并填充输出介质。
		 * @param pformatetcIn [in] 请求的数据格式描述
		 * @param pmedium [out] 接收数据的存储介质
		 * @return HRESULT 成功返回 S_OK，不支持或失败时返回对应错误码
		 */
		virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetData( 
			/* [unique][in] */ FORMATETC __RPC_FAR *pformatetcIn,
			/* [out] */ STGMEDIUM __RPC_FAR *pmedium);

		/**
		 * @brief 获取数据Here
		 * @details 在调用方提供的存储介质中直接写入指定格式的数据内容。
		 * @param pformatetc [in] 请求的数据格式描述
		 * @param pmedium [in] 由调用方提供并接收结果的存储介质
		 * @return HRESULT 成功返回 S_OK，不支持或失败时返回对应错误码
		 */
		virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetDataHere( 
			/* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
			/* [out][in] */ STGMEDIUM __RPC_FAR *pmedium);

		/**
		 * @brief 执行 QueryGetData 操作
		 * @details 用于执行 QueryGetData 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pformatetc [in] pformatetc参数
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 QueryGetData 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE QueryGetData( 
			/* [unique][in] */ FORMATETC __RPC_FAR *pformatetc);

		/**
		 * @brief 获取CanonicalFormatEtc
		 * @details 用于获取CanonicalFormatEtc。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pformatectIn [in] pformatectIn参数
		 * @param pformatetcOut [in] pformatetcOut参数
		 * @return HRESULT STDMETHODCALLTYPE 返回 获取CanonicalFormatEtc 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc( 
			/* [unique][in] */ FORMATETC __RPC_FAR *pformatectIn,
			/* [out] */ FORMATETC __RPC_FAR *pformatetcOut);

		/**
		 * @brief 设置数据
		 * @details 向数据对象注册一份指定格式的数据，并按需要接管存储介质的释放责任。
		 * @param pformatetc [in] 数据格式描述
		 * @param pmedium [in] 待保存的数据存储介质
		 * @param fRelease [in] 是否由数据对象接管存储介质释放
		 * @return HRESULT 成功返回 S_OK，失败时返回对应错误码
		 */
		virtual /* [local] */ HRESULT STDMETHODCALLTYPE SetData( 
			/* [unique][in] */ FORMATETC __RPC_FAR *pformatetc,
			/* [unique][in] */ STGMEDIUM __RPC_FAR *pmedium,
			/* [in] */ BOOL fRelease);

		/**
		 * @brief 执行 EnumFormatEtc 操作
		 * @details 用于执行 EnumFormatEtc 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwDirection [in] Direction数值
		 * @param ppenumFormatEtc [in] ppenumFormatEtc参数
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 EnumFormatEtc 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE EnumFormatEtc( 
			/* [in] */ DWORD dwDirection,
			/* [out] */ IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenumFormatEtc);

		/**
		 * @brief 执行 DAdvise 操作
		 * @details 用于执行 DAdvise 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pformatetc [in] pformatetc参数
		 * @param advf [in] advf参数
		 * @param pAdvSink [in] AdvSink对象
		 * @param pdwConnection [in] pdwConnection参数
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 DAdvise 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE DAdvise( 
			/* [in] */ FORMATETC __RPC_FAR *pformatetc,
			/* [in] */ DWORD advf,
			/* [unique][in] */ IAdviseSink __RPC_FAR *pAdvSink,
			/* [out] */ DWORD __RPC_FAR *pdwConnection);

		/**
		 * @brief 执行 DUnadvise 操作
		 * @details 用于执行 DUnadvise 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwConnection [in] Connection数值
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 DUnadvise 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE DUnadvise( 
			/* [in] */ DWORD dwConnection);

		/**
		 * @brief 执行 EnumDAdvise 操作
		 * @details 用于执行 EnumDAdvise 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param ppenumAdvise [in] ppenumAdvise参数
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 EnumDAdvise 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE EnumDAdvise( 
			/* [out] */ IEnumSTATDATA __RPC_FAR *__RPC_FAR *ppenumAdvise);

		//IAsyncOperation
		//virtual HRESULT STDMETHODCALLTYPE SetAsyncMode( 
		//    /* [in] */ BOOL fDoOpAsync)
		//{
		//	return E_NOTIMPL;
		//}
		//
		//virtual HRESULT STDMETHODCALLTYPE GetAsyncMode( 
		//    /* [out] */ BOOL __RPC_FAR *pfIsOpAsync)
		//{
		//	return E_NOTIMPL;
		//}
		//
		//virtual HRESULT STDMETHODCALLTYPE StartOperation( 
		//    /* [optional][unique][in] */ IBindCtx __RPC_FAR *pbcReserved)
		//{
		//	return E_NOTIMPL;
		//}
		//
		//virtual HRESULT STDMETHODCALLTYPE InOperation( 
		//    /* [out] */ BOOL __RPC_FAR *pfInAsyncOp)
		//{
		//	return E_NOTIMPL;
		//}
		//
		//virtual HRESULT STDMETHODCALLTYPE EndOperation( 
		//    /* [in] */ HRESULT hResult,
		//    /* [unique][in] */ IBindCtx __RPC_FAR *pbcReserved,
		//    /* [in] */ DWORD dwEffects)
		//{
		//	return E_NOTIMPL;
		//}
	};

	////////////////////////////////////////////////////////////////////////////////
	///
	class FYUI_API CIDropTarget : public IDropTarget
	{
	public:
		/**
		 * @brief 构造 CIDropTarget 对象
		 * @details 用于构造 CIDropTarget 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CIDropTarget();
		/**
		 * @brief 析构 CIDropTarget 对象
		 * @details 用于析构 CIDropTarget 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~CIDropTarget();

	public:
		/**
		 * @brief 设置目标Wnd
		 * @details 用于设置目标Wnd。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hWnd [in] 窗口句柄
		 */
		void SetTargetWnd(HWND hWnd) { m_hTargetWnd = hWnd; }
		void AddSuportedFormat(FORMATETC& ftetc) { m_formatetc.push_back(ftetc); }

	public:
		//return values: true - release the medium. false - don't release the medium 
		virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium,DWORD *pdwEffect) = 0;

		/**
		 * @brief 执行 QueryInterface 操作
		 * @details 用于执行 QueryInterface 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param riid [in] riid参数
		 * @param ppvObject [in] ppvObject参数
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 QueryInterface 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
		/**
		 * @brief 添加Ref
		 * @details 用于添加Ref。具体行为由当前对象状态以及传入参数共同决定。
		 * @return ULONG STDMETHODCALLTYPE 返回 添加Ref 的结果
		 */
		virtual ULONG STDMETHODCALLTYPE AddRef( void) { return ++m_cRefCount; }
		virtual ULONG STDMETHODCALLTYPE Release( void);

		/**
		 * @brief 执行 QueryDrop 操作
		 * @details 用于执行 QueryDrop 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param grfKeyState [in] grfKey状态参数
		 * @param pdwEffect [in] pdwEffect参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool QueryDrop(DWORD grfKeyState, LPDWORD pdwEffect);
		/**
		 * @brief 执行 DragEnter 操作
		 * @details 用于执行 DragEnter 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pDataObj [in] 数据Obj对象
		 * @param grfKeyState [in] grfKey状态参数
		 * @param pt [in] 坐标点
		 * @param pdwEffect [in] pdwEffect参数
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 DragEnter 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE DragEnter(
			/* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
			/* [in] */ DWORD grfKeyState,
			/* [in] */ POINTL pt,
			/* [out][in] */ DWORD __RPC_FAR *pdwEffect);
		/**
		 * @brief 执行 DragOver 操作
		 * @details 用于执行 DragOver 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param grfKeyState [in] grfKey状态参数
		 * @param pt [in] 坐标点
		 * @param pdwEffect [in] pdwEffect参数
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 DragOver 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE DragOver( 
			/* [in] */ DWORD grfKeyState,
			/* [in] */ POINTL pt,
			/* [out][in] */ DWORD __RPC_FAR *pdwEffect);
		/**
		 * @brief 执行 DragLeave 操作
		 * @details 用于执行 DragLeave 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 DragLeave 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE DragLeave( void);    
		/**
		 * @brief 执行 Drop 操作
		 * @details 用于执行 Drop 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pDataObj [in] 数据Obj对象
		 * @param grfKeyState [in] grfKey状态参数
		 * @param pt [in] 坐标点
		 * @param pdwEffect [in] pdwEffect参数
		 * @return HRESULT STDMETHODCALLTYPE 返回 执行 Drop 操作 的结果
		 */
		virtual HRESULT STDMETHODCALLTYPE Drop(
			/* [unique][in] */ IDataObject __RPC_FAR *pDataObj,
			/* [in] */ DWORD grfKeyState,
			/* [in] */ POINTL pt,
			/* [out][in] */ DWORD __RPC_FAR *pdwEffect);

	protected:
		HWND m_hTargetWnd;

	private:
		DWORD m_cRefCount;
		bool m_bAllowDrop;
		struct IDropTargetHelper *m_pDropTargetHelper;
		FormatEtcArray m_formatetc;
		FORMATETC* m_pSupportedFrmt;
	};

	////////////////////////////////////////////////////////////////////////////////
	///
	class FYUI_API CDragSourceHelper
	{
	public:
		/**
		 * @brief 构造 CDragSourceHelper 对象
		 * @details 用于构造 CDragSourceHelper 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CDragSourceHelper()
		{
			m_pDragSourceHelper = NULL;
			CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDragSourceHelper, (void**)&m_pDragSourceHelper);
		}

		/**
		 * @brief 析构 CDragSourceHelper 对象
		 * @details 用于析构 CDragSourceHelper 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~CDragSourceHelper()
		{
			if( m_pDragSourceHelper!= NULL ) {
				m_pDragSourceHelper->Release();
				m_pDragSourceHelper=NULL;
			}
		}

	public:
		// IDragSourceHelper
		/**
		 * @brief 执行 InitializeFromBitmap 操作
		 * @details 用于执行 InitializeFromBitmap 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hBitmap [in] h位图参数
		 * @param pt [in,out] 坐标点
		 * @param rc [in,out] 矩形区域
		 * @param pDataObject [in] 数据Object对象
		 * @param crColorKey [in] cr颜色Key参数
		 * @return HRESULT 返回 执行 InitializeFromBitmap 操作 的结果
		 */
		HRESULT InitializeFromBitmap(HBITMAP hBitmap,  POINT& pt, RECT& rc,	IDataObject* pDataObject, COLORREF crColorKey = GetSysColor(COLOR_WINDOW))
		{
			if(m_pDragSourceHelper == NULL) {
				return E_FAIL;
			}

			SHDRAGIMAGE di;
			BITMAP bm;
			GetObject(hBitmap, sizeof(bm), &bm);
			di.sizeDragImage.cx = bm.bmWidth;
			di.sizeDragImage.cy = bm.bmHeight;
			di.hbmpDragImage = hBitmap;
			di.crColorKey = crColorKey; 
			di.ptOffset.x = pt.x - rc.left;
			di.ptOffset.y = pt.y - rc.top;
			return m_pDragSourceHelper->InitializeFromBitmap(&di, pDataObject);
		}

		/**
		 * @brief 执行 InitializeFromWindow 操作
		 * @details 用于执行 InitializeFromWindow 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hwnd [in] hwnd参数
		 * @param pt [in,out] 坐标点
		 * @param pDataObject [in] 数据Object对象
		 * @return HRESULT 返回 执行 InitializeFromWindow 操作 的结果
		 */
		HRESULT InitializeFromWindow(HWND hwnd, POINT& pt,IDataObject* pDataObject)
		{		
			if(m_pDragSourceHelper == NULL) {
				return E_FAIL;
			}
			return m_pDragSourceHelper->InitializeFromWindow(hwnd, &pt, pDataObject);
		}

	private:
		IDragSourceHelper* m_pDragSourceHelper;
	};
}
