#pragma once

#include <Richedit.h>

namespace FYUI
{
	class FYUI_API CRichEditUI : public CContainerUI, public IMessageFilterUI
	{
		DECLARE_DUICONTROL(CRichEditUI)

	public:
		/**
		 * @brief 构造 CRichEditUI 对象
		 * @details 用于构造 CRichEditUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CRichEditUI();
		/**
		 * @brief 析构 CRichEditUI 对象
		 * @details 用于析构 CRichEditUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CRichEditUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const override;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName) override;
		/**
		 * @brief 获取控件标志
		 * @details 用于获取控件标志。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetControlFlags() const override;

		/**
		 * @brief 设置启用状态
		 * @details 用于设置启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnabled [in] 是否启用状态
		 */
		void SetEnabled(bool bEnabled) override;
		/**
		 * @brief 判断是否多选行
		 * @details 用于判断是否多选行。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsMultiLine();
		/**
		 * @brief 设置多选行
		 * @details 用于设置多选行。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bMultiLine [in] 是否多选行
		 */
		void SetMultiLine(bool bMultiLine);
		/**
		 * @brief 判断是否Want页签
		 * @details 用于判断是否Want页签。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsWantTab();
		/**
		 * @brief 设置Want页签
		 * @details 用于设置Want页签。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bWantTab [in] 是否Want页签
		 */
		void SetWantTab(bool bWantTab = true);
		/**
		 * @brief 判断是否WantReturn
		 * @details 用于判断是否WantReturn。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsWantReturn();
		/**
		 * @brief 设置WantReturn
		 * @details 用于设置WantReturn。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bWantReturn [in] 是否WantReturn
		 */
		void SetWantReturn(bool bWantReturn = true);
		/**
		 * @brief 判断是否WantCtrlReturn
		 * @details 用于判断是否WantCtrlReturn。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsWantCtrlReturn();
		/**
		 * @brief 设置WantCtrlReturn
		 * @details 用于设置WantCtrlReturn。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bWantCtrlReturn [in] 是否WantCtrlReturn
		 */
		void SetWantCtrlReturn(bool bWantCtrlReturn = true);
		/**
		 * @brief 判断是否Transparent
		 * @details 用于判断是否Transparent。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsTransparent();
		/**
		 * @brief 设置Transparent
		 * @details 用于设置Transparent。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bTransparent [in] 是否Transparent
		 */
		void SetTransparent(bool bTransparent = true);
		/**
		 * @brief 判断是否富文本
		 * @details 用于判断是否富文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsRich();
		/**
		 * @brief 设置富文本
		 * @details 用于设置富文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bRich [in] 是否富文本
		 */
		void SetRich(bool bRich = true);
		/**
		 * @brief 判断是否ReadOnly
		 * @details 用于判断是否ReadOnly。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsReadOnly();
		/**
		 * @brief 设置ReadOnly
		 * @details 用于设置ReadOnly。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bReadOnly [in] 是否ReadOnly
		 */
		void SetReadOnly(bool bReadOnly = true);
		/**
		 * @brief 判断是否密码Mode
		 * @details 用于判断是否密码Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsPasswordMode() const;
		/**
		 * @brief 设置密码Mode
		 * @details 用于设置密码Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bPasswordMode [in] 是否密码Mode
		 */
		void SetPasswordMode(bool bPasswordMode = true);
		/**
		 * @brief 判断是否WordWrap
		 * @details 用于判断是否WordWrap。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsWordWrap();
		/**
		 * @brief 设置WordWrap
		 * @details 用于设置WordWrap。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bWordWrap [in] 是否WordWrap
		 */
		void SetWordWrap(bool bWordWrap = true);
		/**
		 * @brief 获取字体
		 * @details 用于获取字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetFont();
		/**
		 * @brief 设置字体
		 * @details 用于设置字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 */
		void SetFont(int index);
		/**
		 * @brief 设置字体
		 * @details 用于设置字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrFontName [in] Str字体名称对象
		 * @param nSize [in] 尺寸数值
		 * @param bBold [in] 是否Bold
		 * @param bUnderline [in] 是否Underline
		 * @param bItalic [in] 是否Italic
		 */
		void SetFont(std::wstring_view pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic);
		/**
		 * @brief 获取Win样式
		 * @details 用于获取Win样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @return LONG 返回对应的数值结果
		 */
		LONG GetWinStyle();
		/**
		 * @brief 设置Win样式
		 * @details 用于设置Win样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lStyle [in] l样式参数
		 */
		void SetWinStyle(LONG lStyle);
		/**
		 * @brief 获取文本颜色
		 * @details 用于获取文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetTextColor();
		/**
		 * @brief 设置文本颜色
		 * @details 用于设置文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetTextColor(DWORD dwTextColor);
		/**
		 * @brief 获取Limit文本
		 * @details 用于获取Limit文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetLimitText();
		/**
		 * @brief 设置Limit文本
		 * @details 用于设置Limit文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iChars [in] Chars值
		 */
		void SetLimitText(int iChars);

		/**
		 * @brief 设置Limit文本String
		 * @details 用于设置Limit文本String。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrLimitText [in] StrLimit文本对象
		 */
		void SetLimitTextString(std::wstring_view pStrLimitText);
		/**
		 * @brief 获取Limit文本String
		 * @details 用于获取Limit文本String。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetLimitTextString() const;
		/**
		 * @brief 获取文本Length
		 * @details 用于获取文本Length。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwFlags [in] Flags数值
		 * @return long 返回 获取文本Length 的结果
		 */
		long GetTextLength(DWORD dwFlags = GTL_DEFAULT) const;
		/**
		 * @brief 获取文本
		 * @details 用于获取文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetText() const override;
		/**
		 * @brief 设置文本
		 * @details 用于设置文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrText [in] 文本字符串
		 */
		void SetText(std::wstring_view pstrText) override;
		/**
		 * @brief 设置显示文本
		 * @details 用于设置显示文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrText [in] 文本字符串
		 */
		void SetShowText(std::wstring_view pstrText);
		/**
		 * @brief 判断是否Modify
		 * @details 用于判断是否Modify。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsModify() const;
		/**
		 * @brief 设置Modify
		 * @details 用于设置Modify。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bModified [in] 是否Modified
		 */
		void SetModify(bool bModified = true) const;
		/**
		 * @brief 获取选区
		 * @details 用于获取选区。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cr [in,out] cr参数
		 */
		void GetSel(CHARRANGE& cr) const;
		/**
		 * @brief 获取选区
		 * @details 用于获取选区。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nStartChar [in,out] Start字符数值
		 * @param nEndChar [in,out] End字符数值
		 */
		void GetSel(long& nStartChar, long& nEndChar) const;
		/**
		 * @brief 设置选区
		 * @details 用于设置选区。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cr [in,out] cr参数
		 * @return int 返回对应的数值结果
		 */
		int SetSel(CHARRANGE& cr);
		/**
		 * @brief 设置选区
		 * @details 用于设置选区。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nStartChar [in] Start字符数值
		 * @param nEndChar [in] End字符数值
		 * @return int 返回对应的数值结果
		 */
		int SetSel(long nStartChar, long nEndChar);
		/**
		 * @brief 替换选区
		 * @details 用于替换选区。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lpszNewText [in] lpsz新的文本参数
		 * @param bCanUndo [in] 是否检查是否可以撤销
		 */
		void ReplaceSel(std::wstring_view lpszNewText, bool bCanUndo);
		/**
		 * @brief 替换显示选区
		 * @details 用于替换显示选区。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lpszNewText [in] lpsz新的文本参数
		 * @param bCanUndo [in] 是否检查是否可以撤销
		 */
		void ReplaceShowSel(std::wstring_view lpszNewText, bool bCanUndo);
		/**
		 * @brief 替换选区W
		 * @details 用于替换选区W。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lpszNewText [in] lpsz新的文本参数
		 * @param bCanUndo [in] 是否检查是否可以撤销
		 */
		void ReplaceSelW(LPCWSTR lpszNewText, bool bCanUndo = false);
		/**
		 * @brief 获取选区文本
		 * @details 用于获取选区文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetSelText() const;
		/**
		 * @brief 设置选区All
		 * @details 用于设置选区All。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int SetSelAll();
		/**
		 * @brief 设置选区None
		 * @details 用于设置选区None。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int SetSelNone();
		/**
		 * @brief 获取Selection类型
		 * @details 用于获取Selection类型。具体行为由当前对象状态以及传入参数共同决定。
		 * @return WORD 返回对应的数值结果
		 */
		WORD GetSelectionType() const;
		/**
		 * @brief 获取缩放
		 * @details 用于获取缩放。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nNum [in,out] Num数值
		 * @param nDen [in,out] Den数值
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetZoom(int& nNum, int& nDen) const;
		/**
		 * @brief 设置缩放
		 * @details 用于设置缩放。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nNum [in] Num数值
		 * @param nDen [in] Den数值
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetZoom(int nNum, int nDen);
		/**
		 * @brief 设置缩放Off
		 * @details 用于设置缩放Off。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetZoomOff();
		/**
		 * @brief 获取AutoURLDetect
		 * @details 用于获取AutoURLDetect。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetAutoURLDetect() const;
		/**
		 * @brief 设置AutoURLDetect
		 * @details 用于设置AutoURLDetect。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bAutoDetect [in] 是否AutoDetect
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetAutoURLDetect(bool bAutoDetect = true);
		/**
		 * @brief 获取事件Mask
		 * @details 用于获取事件Mask。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetEventMask() const;
		/**
		 * @brief 设置事件Mask
		 * @details 用于设置事件Mask。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwEventMask [in] 事件Mask数值
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD SetEventMask(DWORD dwEventMask);
		/**
		 * @brief 获取文本Range
		 * @details 用于获取文本Range。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nStartChar [in] Start字符数值
		 * @param nEndChar [in] End字符数值
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetTextRange(long nStartChar, long nEndChar) const;
		/**
		 * @brief 隐藏Selection
		 * @details 用于隐藏Selection。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bHide [in] 是否隐藏
		 * @param bChangeStyle [in] 是否Change样式
		 */
		void HideSelection(bool bHide = true, bool bChangeStyle = false);
		/**
		 * @brief 滚动Caret
		 * @details 用于滚动Caret。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ScrollCaret();
		/**
		 * @brief 插入文本
		 * @details 用于插入文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nInsertAfterChar [in] 插入After字符数值
		 * @param lpstrText [in] lpstr文本参数
		 * @param bCanUndo [in] 是否检查是否可以撤销
		 * @return int 返回对应的数值结果
		 */
		int InsertText(long nInsertAfterChar, std::wstring_view lpstrText, bool bCanUndo = false);
		/**
		 * @brief 追加文本
		 * @details 用于追加文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lpstrText [in] lpstr文本参数
		 * @param bCanUndo [in] 是否检查是否可以撤销
		 * @return int 返回对应的数值结果
		 */
		int AppendText(std::wstring_view lpstrText, bool bCanUndo = false);
		/**
		 * @brief 获取默认字符Format
		 * @details 用于获取默认字符Format。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cf [in,out] cf参数
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDefaultCharFormat(CHARFORMAT2& cf) const;
		/**
		 * @brief 设置默认字符Format
		 * @details 用于设置默认字符Format。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cf [in,out] cf参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetDefaultCharFormat(CHARFORMAT2& cf);
		/**
		 * @brief 获取Selection字符Format
		 * @details 用于获取Selection字符Format。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cf [in,out] cf参数
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSelectionCharFormat(CHARFORMAT2& cf) const;
		/**
		 * @brief 设置Selection字符Format
		 * @details 用于设置Selection字符Format。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cf [in,out] cf参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetSelectionCharFormat(CHARFORMAT2& cf);
		/**
		 * @brief 设置Word字符Format
		 * @details 用于设置Word字符Format。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cf [in,out] cf参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetWordCharFormat(CHARFORMAT2& cf);
		/**
		 * @brief 获取ParaFormat
		 * @details 用于获取ParaFormat。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pf [in,out] pf参数
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetParaFormat(PARAFORMAT2& pf) const;
		/**
		 * @brief 设置ParaFormat
		 * @details 用于设置ParaFormat。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pf [in,out] pf参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetParaFormat(PARAFORMAT2& pf);
		/**
		 * @brief 检查是否可以撤销
		 * @details 用于检查是否可以撤销。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool CanUndo();
		/**
		 * @brief 检查是否可以重做
		 * @details 用于检查是否可以重做。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool CanRedo();
		/**
		 * @brief 检查是否可以粘贴
		 * @details 用于检查是否可以粘贴。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool CanPaste();
		/**
		 * @brief 执行 Redo 操作
		 * @details 用于执行 Redo 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Redo();
		/**
		 * @brief 执行 Undo 操作
		 * @details 用于执行 Undo 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Undo();
		/**
		 * @brief 执行 Clear 操作
		 * @details 用于执行 Clear 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Clear();
		/**
		 * @brief 执行 Copy 操作
		 * @details 用于执行 Copy 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Copy();
		/**
		 * @brief 执行 Cut 操作
		 * @details 用于执行 Cut 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Cut();
		/**
		 * @brief 执行 Paste 操作
		 * @details 用于执行 Paste 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Paste();
		/**
		 * @brief 获取行数量
		 * @details 用于获取行数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetLineCount() const;
		/**
		 * @brief 获取行
		 * @details 用于获取行。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nIndex [in] 索引值
		 * @param nMaxLength [in] 最大Length数值
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetLine(int nIndex, int nMaxLength) const;
		/**
		 * @brief 执行 LineIndex 操作
		 * @details 用于执行 LineIndex 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nLine [in] 行数值
		 * @return int 返回对应的数值结果
		 */
		int LineIndex(int nLine = -1) const;
		/**
		 * @brief 执行 LineLength 操作
		 * @details 用于执行 LineLength 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nLine [in] 行数值
		 * @return int 返回对应的数值结果
		 */
		int LineLength(int nLine = -1) const;
		/**
		 * @brief 执行 LineScroll 操作
		 * @details 用于执行 LineScroll 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nLines [in] Lines数值
		 * @param nChars [in] Chars数值
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool LineScroll(int nLines, int nChars = 0);
		/**
		 * @brief 获取字符位置
		 * @details 用于获取字符位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lChar [in] l字符参数
		 * @return 返回对应的几何结果
		 */
		CDuiPoint GetCharPos(long lChar) const;
		/**
		 * @brief 执行 LineFromChar 操作
		 * @details 用于执行 LineFromChar 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nIndex [in] 索引值
		 * @return long 返回 执行 LineFromChar 操作 的结果
		 */
		long LineFromChar(long nIndex) const;
		/**
		 * @brief 执行 PosFromChar 操作
		 * @details 用于执行 PosFromChar 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nChar [in] 字符数值
		 * @return 返回对应的几何结果
		 */
		CDuiPoint PosFromChar(UINT nChar) const;
		/**
		 * @brief 执行 CharFromPos 操作
		 * @details 用于执行 CharFromPos 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 * @return int 返回对应的数值结果
		 */
		int CharFromPos(CDuiPoint pt) const;
		/**
		 * @brief 执行 EmptyUndoBuffer 操作
		 * @details 用于执行 EmptyUndoBuffer 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void EmptyUndoBuffer();
		/**
		 * @brief 设置撤销Limit
		 * @details 用于设置撤销Limit。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nLimit [in] Limit数值
		 * @return UINT 返回对应的数值结果
		 */
		UINT SetUndoLimit(UINT nLimit);
		/**
		 * @brief 执行 StreamIn 操作
		 * @details 用于执行 StreamIn 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nFormat [in] Format数值
		 * @param es [in,out] es参数
		 * @return long 返回 执行 StreamIn 操作 的结果
		 */
		long StreamIn(int nFormat, EDITSTREAM& es);
		/**
		 * @brief 执行 StreamOut 操作
		 * @details 用于执行 StreamOut 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nFormat [in] Format数值
		 * @param es [in,out] es参数
		 * @return long 返回 执行 StreamOut 操作 的结果
		 */
		long StreamOut(int nFormat, EDITSTREAM& es);
		/**
		 * @brief 设置AccumulateDBCMode
		 * @details 用于设置AccumulateDBCMode。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bDBCMode [in] 是否DBCMode
		 */
		void SetAccumulateDBCMode(bool bDBCMode);
		/**
		 * @brief 判断是否AccumulateDBCMode
		 * @details 用于判断是否AccumulateDBCMode。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsAccumulateDBCMode();

		/**
		 * @brief 获取文本内边距
		 * @details 用于获取文本内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetTextPadding() const;
		/**
		 * @brief 设置文本内边距
		 * @details 用于设置文本内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 */
		void SetTextPadding(RECT rc);
		/**
		 * @brief 获取Normal图像
		 * @details 用于获取Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetNormalImage();
		/**
		 * @brief 设置Normal图像
		 * @details 用于设置Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetNormalImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取热状态图像
		 * @details 用于获取热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetHotImage();
		/**
		 * @brief 设置热状态图像
		 * @details 用于设置热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetHotImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取Focused图像
		 * @details 用于获取Focused图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetFocusedImage();
		/**
		 * @brief 设置Focused图像
		 * @details 用于设置Focused图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetFocusedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取Disabled图像
		 * @details 用于获取Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetDisabledImage();
		/**
		 * @brief 设置Disabled图像
		 * @details 用于设置Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetDisabledImage(std::wstring_view pStrImage);
		/**
		 * @brief 绘制状态图像
		 * @details 用于绘制状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintStatusImage(CPaintRenderContext& renderContext) override;

		/**
		 * @brief 设置提示值
		 * @details 用于设置提示值。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrTipValue [in] Str提示值对象
		 */
		void SetTipValue(std::wstring_view pStrTipValue);
		/**
		 * @brief 获取提示值
		 * @details 用于获取提示值。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetTipValue();
		/**
		 * @brief 设置提示值颜色
		 * @details 用于设置提示值颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrColor [in] Str颜色对象
		 */
		void SetTipValueColor(std::wstring_view pStrColor);
		/**
		 * @brief 获取提示值颜色
		 * @details 用于获取提示值颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetTipValueColor();
		/**
		 * @brief 设置提示值对齐方式
		 * @details 用于设置提示值对齐方式。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uAlign [in] 对齐方式标志
		 */
		void SetTipValueAlign(UINT uAlign);
		/**
		 * @brief 获取提示值对齐方式
		 * @details 用于获取提示值对齐方式。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetTipValueAlign();
		/**
		 * @brief 设置提示值内边距
		 * @details 用于设置提示值内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcTipValuePadding [in] 提示值内边距矩形区域
		 */
		void SetTipValuePadding(RECT rcTipValuePadding);
		/**
		 * @brief 获取提示值内边距
		 * @details 用于获取提示值内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetTipValuePadding();

		/**
		 * @brief 执行初始化逻辑
		 * @details 用于执行初始化逻辑。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void DoInit() override;
		/**
		 * @brief 设置拖放Accept文件
		 * @details 用于设置拖放Accept文件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bAccept [in] 是否Accept
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetDropAcceptFile(bool bAccept);
		/**
		 * @brief 执行 TxSendMessage 操作
		 * @details 用于执行 TxSendMessage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param msg [in] msg参数
		 * @param wparam [in] wparam参数
		 * @param lparam [in] lparam参数
		 * @param plresult [in] plresult参数
		 * @return HRESULT 返回 执行 TxSendMessage 操作 的结果
		 */
		HRESULT TxSendMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT* plresult) const;
		/**
		 * @brief 获取Tx拖放目标
		 * @details 用于获取Tx拖放目标。具体行为由当前对象状态以及传入参数共同决定。
		 * @return IDropTarget* 返回结果对象指针，失败时返回 nullptr
		 */
		IDropTarget* GetTxDropTarget();
		/**
		 * @brief 执行 OnTxViewChanged 操作
		 * @details 用于执行 OnTxViewChanged 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool OnTxViewChanged();
		/**
		 * @brief 执行 OnTxNotify 操作
		 * @details 用于执行 OnTxNotify 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iNotify [in] 通知值
		 * @param pv [in] pv参数
		 */
		void OnTxNotify(DWORD iNotify, void* pv);

		/**
		 * @brief 获取Natural尺寸
		 * @details 用于获取Natural尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param width [in] 宽度参数
		 * @param height [in] 高度参数
		 * @return 返回对应的几何结果
		 */
		CDuiSize GetNaturalSize(LONG width, LONG height);
		/**
		 * @brief 获取文本Content高度
		 * @details 用于获取文本Content高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nRichEditWidth [in] 富文本Edit宽度数值
		 * @return int 返回对应的数值结果
		 */
		int GetTextContentHeight(int nRichEditWidth = 0);
		/**
		 * @brief 设置滚动位置
		 * @details 用于设置滚动位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szPos [in] 位置尺寸参数
		 * @param bMsg [in] 是否Msg
		 * @param bScroolVisible [in] 是否Scrool可见状态
		 */
		void SetScrollPos(SIZE szPos, bool bMsg = true, bool bScroolVisible = true) override;
		/**
		 * @brief 执行 LineUp 操作
		 * @details 用于执行 LineUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bScroolVisible [in] 是否Scrool可见状态
		 */
		void LineUp(bool bScroolVisible = true) override;
		/**
		 * @brief 执行 LineDown 操作
		 * @details 用于执行 LineDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bScroolVisible [in] 是否Scrool可见状态
		 */
		void LineDown(bool bScroolVisible = true) override;
		/**
		 * @brief 执行 PageUp 操作
		 * @details 用于执行 PageUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void PageUp() override;
		/**
		 * @brief 执行 PageDown 操作
		 * @details 用于执行 PageDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void PageDown() override;
		/**
		 * @brief 执行 HomeUp 操作
		 * @details 用于执行 HomeUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void HomeUp() override;
		/**
		 * @brief 执行 EndDown 操作
		 * @details 用于执行 EndDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void EndDown() override;
		/**
		 * @brief 执行 LineLeft 操作
		 * @details 用于执行 LineLeft 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void LineLeft() override;
		/**
		 * @brief 执行 LineRight 操作
		 * @details 用于执行 LineRight 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void LineRight() override;
		/**
		 * @brief 执行 PageLeft 操作
		 * @details 用于执行 PageLeft 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void PageLeft() override;
		/**
		 * @brief 执行 PageRight 操作
		 * @details 用于执行 PageRight 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void PageRight() override;
		/**
		 * @brief 执行 HomeLeft 操作
		 * @details 用于执行 HomeLeft 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void HomeLeft() override;
		/**
		 * @brief 执行 EndRight 操作
		 * @details 用于执行 EndRight 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void EndRight() override;

		/**
		 * @brief 计算预估尺寸
		 * @details 用于计算预估尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szAvailable [in] 可用尺寸
		 * @return 返回对应的几何结果
		 */
		SIZE EstimateSize(SIZE szAvailable) override;
		/**
		 * @brief 设置位置
		 * @details 用于设置位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		/**
		 * @brief 执行 Move 操作
		 * @details 用于执行 Move 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szOffset [in] 位移尺寸
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		void Move(SIZE szOffset, bool bNeedInvalidate = true) override;
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event) override;
		/**
		 * @brief 执行绘制
		 * @details 用于执行绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue) override;

		/**
		 * @brief 处理窗口消息
		 * @details 用于处理窗口消息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;
		/**
		 * @brief 预处理字符消息
		 * @details 用于预处理字符消息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in,out] Msg标志
		 * @param wParam [in,out] wParam参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool PreprocessCharMessage(UINT& uMsg, WPARAM& wParam);
		/**
		 * @brief 完成消息分发收尾
		 * @details 用于完成消息分发收尾。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param hr [in] hr参数
		 * @param bWasHandled [in] 是否WasHandled
		 * @param bHandled [in,out] 是否Handled
		 */
		void FinalizeMessageDispatch(UINT uMsg, HRESULT hr, bool bWasHandled, bool& bHandled) const;

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CRichEditUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CRichEditUI* Clone() override;
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		void CopyData(CRichEditUI* pControl);

	protected:
		enum { DEFAULT_TIMERID = 20 };

	private:
		struct TextLine
		{
			size_t start = 0;
			size_t length = 0;
			int width = 0;
		};

		/**
		 * @brief 重置CaretBlink
		 * @details 用于重置CaretBlink。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ResetCaretBlink();
		/**
		 * @brief 执行 PushUndoState 操作
		 * @details 用于执行 PushUndoState 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void PushUndoState();
		/**
		 * @brief 执行 MarkTextChanged 操作
		 * @details 用于执行 MarkTextChanged 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bCanUndo [in] 是否检查是否可以撤销
		 */
		void MarkTextChanged(bool bCanUndo);
		/**
		 * @brief 执行 NormalizeText 操作
		 * @details 用于执行 NormalizeText 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in,out] 文本内容
		 */
		void NormalizeText(std::wstring& text) const;
		/**
		 * @brief 替换Range
		 * @details 用于替换Range。具体行为由当前对象状态以及传入参数共同决定。
		 * @param start [in] start参数
		 * @param end [in] end参数
		 * @param replacement [in] replacement参数
		 * @param bCanUndo [in] 是否检查是否可以撤销
		 */
		void ReplaceRange(size_t start, size_t end, std::wstring_view replacement, bool bCanUndo);
		/**
		 * @brief 删除Selection
		 * @details 用于删除Selection。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bCanUndo [in] 是否检查是否可以撤销
		 */
		void DeleteSelection(bool bCanUndo);
		/**
		 * @brief 移动Caret
		 * @details 用于移动Caret。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pos [in] 位置参数
		 * @param bKeepAnchor [in] 是否KeepAnchor
		 */
		void MoveCaret(size_t pos, bool bKeepAnchor);
		/**
		 * @brief 移动Caret垂直
		 * @details 用于移动Caret垂直。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lineDelta [in] 行Delta参数
		 * @param bKeepAnchor [in] 是否KeepAnchor
		 */
		void MoveCaretVertical(int lineDelta, bool bKeepAnchor);
		/**
		 * @brief 确保布局
		 * @details 用于确保布局。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void EnsureLayout() const;
		/**
		 * @brief 执行 InvalidateLayout 操作
		 * @details 用于执行 InvalidateLayout 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void InvalidateLayout() const;
		/**
		 * @brief 获取Edit矩形
		 * @details 用于获取Edit矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetEditRect() const;
		/**
		 * @brief 获取视图矩形
		 * @details 用于获取视图矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetViewRect() const;
		/**
		 * @brief 获取行高度
		 * @details 用于获取行高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetLineHeight() const;
		/**
		 * @brief 获取Display文本
		 * @details 用于获取Display文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetDisplayText() const;
		/**
		 * @brief 测量文本宽度
		 * @details 用于测量文本宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @return int 返回对应的数值结果
		 */
		int MeasureTextWidth(std::wstring_view text) const;
		/**
		 * @brief 执行 HitTest 操作
		 * @details 用于执行 HitTest 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 * @return size_t 返回对应的数值结果
		 */
		size_t HitTest(CDuiPoint pt) const;
		/**
		 * @brief 执行 CharPos 操作
		 * @details 用于执行 CharPos 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 * @return 返回对应的几何结果
		 */
		CDuiPoint CharPos(size_t index) const;
		/**
		 * @brief 执行 CurrentLineIndex 操作
		 * @details 用于执行 CurrentLineIndex 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return size_t 返回对应的数值结果
		 */
		size_t CurrentLineIndex() const;
		/**
		 * @brief 更新滚动Bars
		 * @details 用于更新滚动Bars。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void UpdateScrollBars();
		/**
		 * @brief 应用滚动Bar样式
		 * @details 用于应用滚动Bar样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pScrollBar [in] 滚动Bar对象
		 * @param styleName [in] 样式名称
		 * @param defaultName [in] 默认样式名称
		 */
		void ApplyScrollBarStyle(CScrollBarUI* pScrollBar, std::wstring_view styleName, std::wstring_view defaultName);
		/**
		 * @brief 滚动ToCaret
		 * @details 用于滚动ToCaret。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ScrollToCaret();
		/**
		 * @brief 执行 TryGetCachedCaretPoint 操作
		 * @details 用于执行 TryGetCachedCaretPoint 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param ptCaret [in,out] Caret坐标点
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool TryGetCachedCaretPoint(POINT& ptCaret) const;
		/**
		 * @brief 执行 TryGetCaretInvalidateRect 操作
		 * @details 用于执行 TryGetCaretInvalidateRect 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcCaret [in,out] Caret矩形区域
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool TryGetCaretInvalidateRect(RECT& rcCaret) const;
		/**
		 * @brief 执行 InvalidateCaretRect 操作
		 * @details 用于执行 InvalidateCaretRect 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void InvalidateCaretRect();
		/**
		 * @brief 更新ImeComposition窗口
		 * @details 用于更新ImeComposition窗口。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void UpdateImeCompositionWindow();
		/**
		 * @brief 绘制Selection
		 * @details 用于绘制Selection。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcView [in] 视图矩形区域
		 */
		void DrawSelection(CPaintRenderContext& renderContext, const RECT& rcView);
		/**
		 * @brief 绘制文本Lines
		 * @details 用于绘制文本Lines。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcView [in] 视图矩形区域
		 */
		void DrawTextLines(CPaintRenderContext& renderContext, const RECT& rcView);
		/**
		 * @brief 绘制Caret
		 * @details 用于绘制Caret。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcView [in] 视图矩形区域
		 */
		void DrawCaret(CPaintRenderContext& renderContext, const RECT& rcView);
		/**
		 * @brief 绘制提示文本
		 * @details 用于绘制提示文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcView [in] 视图矩形区域
		 */
		void DrawTipText(CPaintRenderContext& renderContext, const RECT& rcView);
		/**
		 * @brief 显示上下文菜单
		 * @details 用于显示上下文菜单。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 */
		void ShowContextMenu(CDuiPoint pt);
		/**
		 * @brief 获取Clipboard文本
		 * @details 用于获取Clipboard文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetClipboardText() const;
		/**
		 * @brief 设置Clipboard文本
		 * @details 用于设置Clipboard文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 */
		void SetClipboardText(std::wstring_view text) const;
		/**
		 * @brief 处理KeyDown
		 * @details 用于处理KeyDown。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void HandleKeyDown(TEventUI& event);
		/**
		 * @brief 处理字符
		 * @details 用于处理字符。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void HandleChar(TEventUI& event);

	protected:
		std::wstring m_strLimitText;
		bool m_bWantTab;
		bool m_bWantReturn;
		bool m_bWantCtrlReturn;
		bool m_bTransparent;
		bool m_bRich;
		bool m_bReadOnly;
		bool m_bPasswordMode;
		bool m_bWordWrap;
		bool m_bHideSelection;
		bool m_bAutoURLDetect;
		bool m_bModified;
		bool m_bDrawCaret;
		bool m_bInited;
		bool m_bDraggingSelection;
		bool m_fAccumulateDBC;
		UINT m_chLeadByte;
		DWORD m_dwTextColor;
		DWORD m_dwEventMask;
		int m_iFont;
		int m_iLimitText;
		LONG m_lTwhStyle;
		UINT m_uUndoLimit;
		int m_nZoomNum;
		int m_nZoomDen;
		size_t m_nCaret;
		size_t m_nAnchor;
		int m_nPreferredCaretX;
		RECT m_rcTextPadding;
		UINT m_uButtonState;
		std::wstring m_sNormalImage;
		std::wstring m_sHotImage;
		std::wstring m_sFocusedImage;
		std::wstring m_sDisabledImage;
		std::wstring m_sTipValue;
		DWORD m_dwTipValueColor;
		UINT m_uTipValueAlign;
		RECT m_rcTipValuePadding;
		CHARFORMAT2W m_defaultCharFormat;
		CHARFORMAT2W m_selectionCharFormat;
		PARAFORMAT2 m_paraFormat;
		std::vector<std::wstring> m_undoStack;
		std::vector<std::wstring> m_redoStack;
		mutable bool m_bLayoutDirty;
		mutable int m_nLayoutWidth;
		mutable int m_nContentWidth;
		mutable int m_nContentHeight;
		mutable std::vector<TextLine> m_lines;
	};
}
