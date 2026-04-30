#pragma once
#include "UIButton.h"

namespace FYUI
{
	class FYUI_API COptionUI : public CButtonUI
	{
		DECLARE_DUICONTROL(COptionUI)
	public:
		/**
		 * @brief 构造 COptionUI 对象
		 * @details 用于构造 COptionUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		COptionUI();
		/**
		 * @brief 析构 COptionUI 对象
		 * @details 用于析构 COptionUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~COptionUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName);

		/**
		 * @brief 设置管理器和父级关系
		 * @details 用于设置管理器和父级关系。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pManager [in] 管理器对象
		 * @param pParent [in] 父级控件对象
		 * @param bInit [in] 是否执行初始化
		 */
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

		/**
		 * @brief 激活对象
		 * @details 用于激活对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Activate();

		/**
		 * @brief 获取Selected图像
		 * @details 用于获取Selected图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetSelectedImage();
		/**
		 * @brief 设置Selected图像
		 * @details 用于设置Selected图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetSelectedImage(std::wstring_view pStrImage);

		/**
		 * @brief 获取Selected热状态图像
		 * @details 用于获取Selected热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetSelectedHotImage();
		/**
		 * @brief 设置Selected热状态图像
		 * @details 用于设置Selected热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetSelectedHotImage(std::wstring_view pStrImage);

		/**
		 * @brief 获取SelectedPushed图像
		 * @details 用于获取SelectedPushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetSelectedPushedImage();
		/**
		 * @brief 设置SelectedPushed图像
		 * @details 用于设置SelectedPushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetSelectedPushedImage(std::wstring_view pStrImage);

		/**
		 * @brief 设置Selected文本颜色
		 * @details 用于设置Selected文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetSelectedTextColor(DWORD dwTextColor);
		/**
		 * @brief 获取Selected文本颜色
		 * @details 用于获取Selected文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSelectedTextColor();

		/**
		 * @brief 设置Selected背景颜色
		 * @details 用于设置Selected背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBkColor [in] 背景颜色值
		 */
		void SetSelectedBkColor(DWORD dwBkColor);
		/**
		 * @brief 获取选中背景颜色
		 * @details 用于获取选中背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSelectBkColor();

		/**
		 * @brief 设置SelectedDisable背景颜色
		 * @details 用于设置SelectedDisable背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBkColor [in] 背景颜色值
		 */
		void SetSelectedDisableBkColor(DWORD dwBkColor);
		/**
		 * @brief 获取SelectedDisable背景颜色
		 * @details 用于获取SelectedDisable背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSelectedDisableBkColor();

		/**
		 * @brief 获取SelectedFored图像
		 * @details 用于获取SelectedFored图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetSelectedForedImage();
		/**
		 * @brief 设置SelectedFored图像
		 * @details 用于设置SelectedFored图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetSelectedForedImage(std::wstring_view pStrImage);

		/**
		 * @brief 设置Selected状态数量
		 * @details 用于设置Selected状态数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nCount [in] 数量数值
		 */
		void SetSelectedStateCount(int nCount);
		/**
		 * @brief 获取Selected状态数量
		 * @details 用于获取Selected状态数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetSelectedStateCount() const;
		/**
		 * @brief 获取Selected状态图像
		 * @details 用于获取Selected状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetSelectedStateImage();
		/**
		 * @brief 设置Selected状态图像
		 * @details 用于设置Selected状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		virtual void SetSelectedStateImage(std::wstring_view pStrImage);

		/**
		 * @brief 设置Selected字体
		 * @details 用于设置Selected字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 */
		void SetSelectedFont(int index);
		/**
		 * @brief 获取Selected字体
		 * @details 用于获取Selected字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetSelectedFont() const;

		/**
		 * @brief 获取Group
		 * @details 用于获取Group。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetGroup() const;
		/**
		 * @brief 设置Group
		 * @details 用于设置Group。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrGroupName [in] StrGroup名称对象
		 */
		void SetGroup(std::wstring_view pStrGroupName = {});
		/**
		 * @brief 获取Group类型
		 * @details 用于获取Group类型。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetGroupType() const;
		/**
		 * @brief 设置Group类型
		 * @details 用于设置Group类型。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrGroupType [in] StrGroup类型对象
		 */
		void SetGroupType(std::wstring_view pStrGroupType = {});

		/**
		 * @brief 判断是否Selected
		 * @details 用于判断是否Selected。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsSelected() const;
		/**
		 * @brief 选中ed
		 * @details 用于选中ed。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSelected [in] 是否Selected
		 * @param bMsg [in] 是否Msg
		 */
		virtual void Selected(bool bSelected, bool bMsg = true);

		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		/**
		 * @brief 绘制背景颜色
		 * @details 用于绘制背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintBkColor(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 绘制状态图像
		 * @details 用于绘制状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintStatusImage(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 绘制前景图像
		 * @details 用于绘制前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintForeImage(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 绘制文本内容
		 * @details 用于绘制文本内容。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintText(CPaintRenderContext& renderContext) override;

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return COptionUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual COptionUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(COptionUI* pControl) ;

	protected:
		bool			m_bSelected;
		std::wstring		m_sGroupName;
		std::wstring		m_sGroupType;

		int				m_iSelectedFont;

		DWORD			m_dwSelectedBkColor;
		DWORD			m_dwSelectedDisableBkColor;
		DWORD			m_dwSelectedTextColor;

		std::wstring		m_sSelectedImage;
		std::wstring		m_sSelectedHotImage;
		std::wstring		m_sSelectedPushedImage;
		std::wstring		m_sSelectedForeImage;

		int m_nSelectedStateCount;
		std::wstring m_sSelectedStateImage;
	};

	class FYUI_API CCheckBoxUI : public COptionUI
	{
		DECLARE_DUICONTROL(CCheckBoxUI)
	public:
		/**
		 * @brief 构造 CCheckBoxUI 对象
		 * @details 用于构造 CCheckBoxUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CCheckBoxUI();

	public:
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
		 * @brief 设置Check
		 * @details 用于设置Check。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bCheck [in] 是否Check
		 */
		void SetCheck(bool bCheck);
		/**
		 * @brief 获取Check
		 * @details 用于获取Check。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetCheck() const;

	public:
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		virtual void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		/**
		 * @brief 设置AutoCheck
		 * @details 用于设置AutoCheck。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		void SetAutoCheck(bool bEnable);
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		virtual void DoEvent(TEventUI& event);
		/**
		 * @brief 选中ed
		 * @details 用于选中ed。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSelected [in] 是否Selected
		 * @param bMsg [in] 是否Msg
		 */
		virtual void Selected(bool bSelected, bool bMsg = true);

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CCheckBoxUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CCheckBoxUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CCheckBoxUI* pControl) ;

	protected:
		bool m_bAutoCheck; 
	};
};

