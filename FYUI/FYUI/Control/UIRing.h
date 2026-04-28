#pragma once

namespace FYUI
{
	class FYUI_API CRingUI : public CLabelUI
	{
		enum
		{ 
			RING_TIMERID = 100,
		};
		DECLARE_DUICONTROL(CRingUI)
	public:
		CRingUI();
		~CRingUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		void SetBkImage(std::wstring_view pStrImage);	
		virtual void DoEvent(TEventUI& event);
		void PaintBkImage(CPaintRenderContext& renderContext) override;

		virtual CRingUI* Clone();
		virtual void CopyData(CRingUI* pControl) ;

	private:
		void InitImage();
		void DeleteImage();

	public:
		float m_fCurAngle;
		TImageInfo* m_pBkimageInfo;
	};
}
