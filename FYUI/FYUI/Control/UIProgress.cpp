#include "pch.h"
#include "UIProgress.h"
#include "../Core/Render/UIRenderContext.h"

namespace FYUI
{
	IMPLEMENT_DUICONTROL(CProgressUI)

		CProgressUI::CProgressUI() : m_bShowText(false), m_bHorizontal(true), m_nMin(0), m_nMax(100), m_nValue(0), m_bStretchForeImage(true), m_szForeImage({0,0})
	{
		m_uTextStyle = DT_SINGLELINE | DT_CENTER;
		SetFixedHeight(12);
	}

	std::wstring_view CProgressUI::GetClass() const
	{
		return _T("ProgressUI");
	}

	LPVOID CProgressUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, DUI_CTR_PROGRESS) == 0 ) return static_cast<CProgressUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	bool CProgressUI::IsShowText()
	{
		return m_bShowText;
	}

	void CProgressUI::SetShowText(bool bShowText)
	{
		if( m_bShowText == bShowText ) return;
		m_bShowText = bShowText;
		if(!m_bShowText) SetText(_T(""));
	}

	bool CProgressUI::IsHorizontal()
	{
		return m_bHorizontal;
	}

	void CProgressUI::SetHorizontal(bool bHorizontal)
	{
		if( m_bHorizontal == bHorizontal ) return;

		m_bHorizontal = bHorizontal;
		Invalidate();
	}

	int CProgressUI::GetMinValue() const
	{
		return m_nMin;
	}

	void CProgressUI::SetMinValue(int nMin)
	{
		m_nMin = nMin;
		Invalidate();
	}

	int CProgressUI::GetMaxValue() const
	{
		return m_nMax;
	}

	void CProgressUI::SetMaxValue(int nMax)
	{
		m_nMax = nMax;
		Invalidate();
	}

	int CProgressUI::GetValue() const
	{
		return m_nValue;
	}

	void CProgressUI::SetValue(int nValue)
	{
		if(nValue == m_nValue || nValue<m_nMin || nValue > m_nMax) {
			return;
		}
		m_nValue = nValue;
		Invalidate();
		UpdateText();
	}

	void CProgressUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("hor")) == 0 ) SetHorizontal(StringUtil::ParseBool(pstrValue));
		else if( StringUtil::CompareNoCase(pstrName, _T("min")) == 0 ) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetMinValue(value);
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("max")) == 0 ) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetMaxValue(value);
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("value")) == 0 ) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetValue(value);
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("isstretchfore"))==0) SetStretchForeImage(StringUtil::ParseBool(pstrValue));
		else if( StringUtil::CompareNoCase(pstrName, _T("showtext"))==0) SetShowText(StringUtil::ParseBool(pstrValue));
		else if (StringUtil::CompareNoCase(pstrName, _T("foreimagepadding")) == 0) 
		{
			SIZE cxyForeimage = {0};
			if (StringUtil::TryParseSize(pstrValue, cxyForeimage)) {
				SetForeImagePadding(cxyForeimage);
			}
		}
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CProgressUI::PaintForeColor(CPaintRenderContext& renderContext)
	{
		if(m_dwForeColor == 0) return;

		if( m_nMax <= m_nMin ) m_nMax = m_nMin + 1;
		if( m_nValue > m_nMax ) m_nValue = m_nMax;
		if( m_nValue < m_nMin ) m_nValue = m_nMin;

		RECT rc = m_rcItem;
		if( m_bHorizontal ) {
			rc.right = m_rcItem.left + (m_nValue - m_nMin) * (m_rcItem.right - m_rcItem.left) / (m_nMax - m_nMin);
		}
		else {
			rc.bottom = m_rcItem.top + (m_rcItem.bottom - m_rcItem.top) * (m_nMax - m_nValue) / (m_nMax - m_nMin);

		}

		const SIZE cxyBorderRound = GetBorderRound();
		if (cxyBorderRound.cx > 0 || cxyBorderRound.cy > 0) {
			CRenderEngine::DrawRoundColor(renderContext, rc, cxyBorderRound.cx, cxyBorderRound.cy, GetAdjustColor(m_dwForeColor));
		}
		else {
			CRenderEngine::DrawColor(renderContext, rc, GetAdjustColor(m_dwForeColor));
		}
	}

	void CProgressUI::PaintForeImage(CPaintRenderContext& renderContext)
	{
		if( m_nMax <= m_nMin ) m_nMax = m_nMin + 1;
		if( m_nValue > m_nMax ) m_nValue = m_nMax;
		if( m_nValue < m_nMin ) m_nValue = m_nMin;

		RECT rc = {0};
		bool bHasCustomDest = false;

		// 鐏忔繆鐦憴锝嗙€?XML 娑擃厾鏁ら幋鐤殰鐎规矮绠熼惃鍕閺咁垰娴?dest
		int nDestPos = StringUtil::Find(m_sForeImage, L"dest='");
		if (nDestPos >= 0) {
			int nDestEnd = StringUtil::Find(m_sForeImage, L"'", nDestPos + 6);
			if (nDestEnd >= 0) {
				std::wstring sDest = StringUtil::Mid(m_sForeImage, nDestPos + 6, nDestEnd - nDestPos - 6);
				bHasCustomDest = StringUtil::TryParseRect(sDest, rc);
			}
		}

		// 鏉╂稑瀹崇拋锛勭暬閿涙碍鐗撮幑顔芥Ц閸氾箒鍤滅€规矮绠熸禍?dest 鐠ч绗夐崥灞藉瀻閺€?
		if (bHasCustomDest) {
			// 婵″倹鐏夐悽銊﹀煕閸愭瑤绨?dest閿涘奔浜?dest 娑撳搫鐔€閸戝棗灏崺鐔活吀缁犳绻樻惔锔芥蒋闂€璺ㄧ叚
			if (m_bHorizontal) {
				int nWidth = (m_nValue - m_nMin) * (rc.right - rc.left) / (m_nMax - m_nMin);
				rc.right = rc.left + nWidth;
			} else {
				int nHeight = (m_nValue - m_nMin) * (rc.bottom - rc.top) / (m_nMax - m_nMin);
				rc.top = rc.bottom - nHeight;
			}
		}
		else {
			// 婵″倹鐏夊▽鈩冩箒閸?dest閿涘矁铔嬮崢鐔告降閻?DPI 閸?Padding 閼奉亜濮╃拋锛勭暬闁槒绶?
			SIZE szForePadding = GetManager()->ScaleSize(m_szForeImage);
			int sw ;
			int sh ;

			if( m_bHorizontal ) {
				rc.right = (m_nValue - m_nMin) * (m_rcItem.right - m_rcItem.left) / (m_nMax - m_nMin);

				sh = GetManager()->UnscaleValue(m_rcItem.bottom - m_rcItem.top - rc.top - szForePadding.cx - szForePadding.cy);
				sw = GetManager()->UnscaleValue(rc.right - rc.left);

				rc.left = GetManager()->UnscaleValue(rc.left);
				rc.top = GetManager()->UnscaleValue(rc.top + szForePadding.cx);
				rc.right = rc.left + sw;
				rc.bottom = rc.top + sh;
			}
			else {
				rc.top = (m_rcItem.bottom - m_rcItem.top) * (m_nMax - m_nValue) / (m_nMax - m_nMin);
				rc.bottom = m_rcItem.bottom - m_rcItem.top;

				sw = GetManager()->UnscaleValue(m_rcItem.right - m_rcItem.left - rc.left - szForePadding.cx - szForePadding.cy);
				sh = GetManager()->UnscaleValue(rc.bottom - rc.top);

				rc.left = GetManager()->UnscaleValue(rc.left + szForePadding.cx);
				rc.top = GetManager()->UnscaleValue(rc.top);
				rc.right = rc.left + sw;
				rc.bottom = rc.top + sh;
			}
		}

		// 閺堚偓缂佸牏绮嶉崥鍫㈢帛閸?
		if( !m_sForeImage.empty() ) {
			m_sForeImageModify.clear();

			if (m_bStretchForeImage) {
				m_sForeImageModify = StringUtil::Format(L"dest='{},{},{},{}'", rc.left, rc.top, rc.right, rc.bottom);
			}
			else {
				// 瑜?isstretchfore="false" 閺冭绱漵ource 娑旂喕顩︾捄鐔烘絻鐟佷礁澹€閿涘奔浜掗梼鍙夊娴?
				m_sForeImageModify = StringUtil::Format(
					L"dest='{},{},{},{}' source='{},{},{},{}'",
					rc.left, rc.top, rc.right, rc.bottom,
					rc.left, rc.top, rc.right, rc.bottom);
			}

			if( !DrawImage(renderContext, m_sForeImage, m_sForeImageModify) ) {}
			else return;
		}
	}

	bool CProgressUI::IsStretchForeImage()
	{
		return m_bStretchForeImage;
	}

	void CProgressUI::SetStretchForeImage( bool bStretchForeImage /*= true*/ )
	{
		if (m_bStretchForeImage==bStretchForeImage)		return;
		m_bStretchForeImage=bStretchForeImage;
		Invalidate();
	}

	void CProgressUI::SetForeImagePadding(SIZE szForeImage) 
	{
		m_szForeImage = szForeImage;
	}
	CProgressUI* CProgressUI::Clone()
	{
		CProgressUI* pClone = new CProgressUI();
		pClone->CopyData(this);
		return pClone;
	}
	void CProgressUI::CopyData(CProgressUI* pControl)
	{
		m_bShowText = m_bShowText;
		m_bHorizontal = m_bHorizontal;
		m_bStretchForeImage = m_bStretchForeImage;
		m_nMax = m_nMax;
		m_nMin = m_nMin;
		m_nValue = m_nValue;
		m_szForeImage = m_szForeImage;
		m_sForeImageModify = m_sForeImageModify;
		__super::CopyData(pControl);
	}
	void CProgressUI::UpdateText()
	{
		if(m_bShowText) {
			std::wstring sText;
			sText = StringUtil::Format(L"{:.0f}%", (m_nValue - m_nMin) * 100.0f / (m_nMax - m_nMin));
			SetText(sText);
		}
	}
}


