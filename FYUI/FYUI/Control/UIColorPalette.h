#pragma once

#include "../Core/Render/UIRenderSurface.h"

namespace FYUI 
{
	/////////////////////////////////////////////////////////////////////////////////////
	//
	class FYUI_API CColorPaletteUI : public CControlUI
	{
		DECLARE_DUICONTROL(CColorPaletteUI)
	public:
		/**
		 * @brief 构造 CColorPaletteUI 对象
		 * @details 用于构造 CColorPaletteUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CColorPaletteUI();
		/**
		 * @brief 析构 CColorPaletteUI 对象
		 * @details 用于析构 CColorPaletteUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~CColorPaletteUI();

		//鑾峰彇鏈€缁堣閫夋嫨鐨勯鑹诧紝鍙互鐩存帴鐢ㄤ簬璁剧疆duilib鑳屾櫙鑹?
		/**
		 * @brief 获取选中颜色
		 * @details 用于获取选中颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSelectColor();
		/**
		 * @brief 设置选中颜色
		 * @details 用于设置选中颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetSelectColor(DWORD dwColor);

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetClass() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		virtual LPVOID GetInterface(std::wstring_view pstrName);
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		virtual void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		//璁剧疆/鑾峰彇 Pallet锛堣皟鑹叉澘涓荤晫闈級鐨勯珮搴?
		/**
		 * @brief 设置Pallet高度
		 * @details 用于设置Pallet高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nHeight [in] 高度数值
		 */
		void SetPalletHeight(int nHeight);
		/**
		 * @brief 获取Pallet高度
		 * @details 用于获取Pallet高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int	GetPalletHeight() const;

		//璁剧疆/鑾峰彇 涓嬫柟Bar锛堜寒搴﹂€夋嫨鏍忥級鐨勯珮搴?
		/**
		 * @brief 设置Bar高度
		 * @details 用于设置Bar高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nHeight [in] 高度数值
		 */
		void SetBarHeight(int nHeight);
		/**
		 * @brief 获取Bar高度
		 * @details 用于获取Bar高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetBarHeight() const;

		//璁剧疆/鑾峰彇 閫夋嫨鍥炬爣鐨勮矾寰?
		/**
		 * @brief 设置Pallet图像
		 * @details 用于设置Pallet图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pszImage [in] psz图像参数
		 */
		void SetPalletImage(std::wstring_view pszImage);
		/**
		 * @brief 获取Pallet图像
		 * @details 用于获取Pallet图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetPalletImage() const;
		/**
		 * @brief 设置Bar图像
		 * @details 用于设置Bar图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pszImage [in] psz图像参数
		 */
		void SetBarImage(std::wstring_view pszImage);
		/**
		 * @brief 获取Bar图像
		 * @details 用于获取Bar图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetBarImage() const;
		/**
		 * @brief 设置颜色Bar图像
		 * @details 用于设置颜色Bar图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pszImage [in] psz图像参数
		 */
		void SetColorBarImage(std::wstring_view pszImage);
		/**
		 * @brief 获取颜色Bar图像
		 * @details 用于获取颜色Bar图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetColorBarImage() const;

		/**
		 * @brief 设置Pallet图像尺寸
		 * @details 用于设置Pallet图像尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szPalletImage [in] Pallet图像尺寸参数
		 */
		void SetPalletImageSize(SIZE szPalletImage);
		/**
		 * @brief 设置Bar图像尺寸
		 * @details 用于设置Bar图像尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szBarImage [in] Bar图像尺寸参数
		 */
		void SetBarImageSize(SIZE szBarImage);
		/**
		 * @brief 获取Pallet图像尺寸
		 * @details 用于获取Pallet图像尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetPalletImageSize();
		/**
		 * @brief 获取Bar图像尺寸
		 * @details 用于获取Bar图像尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetBarImageSize();

		/**
		 * @brief 设置BarHor移动
		 * @details 用于设置BarHor移动。具体行为由当前对象状态以及传入参数共同决定。
		 * @param move [in] 移动参数
		 */
		void SetBarHorMove(int move);
		/**
		 * @brief 设置BarVer移动
		 * @details 用于设置BarVer移动。具体行为由当前对象状态以及传入参数共同决定。
		 * @param move [in] 移动参数
		 */
		void SetBarVerMove(int move);

		/**
		 * @brief 设置位置
		 * @details 用于设置位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		virtual void SetPos(RECT rc, bool bNeedInvalidate = true);
		/**
		 * @brief 执行初始化逻辑
		 * @details 用于执行初始化逻辑。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void DoInit();
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		virtual void DoEvent(TEventUI& event);
		/**
		 * @brief 绘制前景图像
		 * @details 用于绘制前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintForeImage(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 绘制Pallet
		 * @details 用于绘制Pallet。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintPallet(CPaintRenderContext& renderContext);

		/**
		 * @brief 执行 NotifyColorChanged 操作
		 * @details 用于执行 NotifyColorChanged 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void NotifyColorChanged();



	protected:
		//鏇存柊鏁版嵁
		/**
		 * @brief 确保调色板绘制表面
		 * @details 用于确保调色板绘制表面。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool EnsurePaletteSurface();
		/**
		 * @brief 更新Pallet数据
		 * @details 用于更新Pallet数据。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void UpdatePalletData();
		/**
		 * @brief 更新Bar数据
		 * @details 用于更新Bar数据。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void UpdateBarData();

	private:
		CPaintRenderSurface m_paletteSurface;
		UINT		m_uButtonState;

		bool		m_activeHandleHueBar = true; // 榛樿閫変腑涓嬫柟 Hue 鏉?
		bool		m_bIsInBar;
		bool		m_bPaletteDataDirty;
		bool		m_bBarDataDirty;

		bool		m_bIsInPallet;
		bool        m_bSetColor =false;
		int			m_nCurH;
		int			m_nCurS;
		int			m_nCurB;

		int			m_nPalletHeight;
		int			m_nBarHeight;
		CDuiPoint		m_ptLastPalletMouse;
		CDuiPoint		m_ptLastBarMouse;
		std::wstring  m_strPalletImage;
		std::wstring  m_strBarImage;
		std::wstring  m_strColorBarImage;

		SIZE m_szPalletImage;
		SIZE m_szBarImage;
	};
}

