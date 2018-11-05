#include "stdafx.h"
#include "PepperList.h"

constexpr auto TIMER_TOOLTIP = 0x01;

IMPLEMENT_DYNAMIC(CPepperList, CMFCListCtrl)

BEGIN_MESSAGE_MAP(CPepperList, CMFCListCtrl)
	ON_WM_SETCURSOR()
	ON_WM_KILLFOCUS()
	ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, &CPepperList::OnHdnDividerdblclick)
	ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, &CPepperList::OnHdnDividerdblclick)
	ON_WM_HSCROLL()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
END_MESSAGE_MAP()

CPepperList::CPepperList()
{
	LOGFONT lf { };
	lf.lfHeight = 16;
	StringCchCopyW(lf.lfFaceName, 18, L"Consolas");
	if (!m_fontList.CreateFontIndirectW(&lf))
	{
		StringCchCopyW(lf.lfFaceName, 18, L"Times New Roman");
		m_fontList.CreateFontIndirectW(&lf);
	}

	m_hwndToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
		TTS_BALLOON | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL);

	m_stToolInfo.cbSize = TTTOOLINFOW_V1_SIZE;
	m_stToolInfo.uFlags = TTF_TRACK;
	m_stToolInfo.uId = 1;

	m_colorListToolTipSubitem = RGB(170, 170, 230);
	m_colorListSelected = RGB(0, 120, 215);

	::SendMessage(m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
	::SendMessage(m_hwndToolTip, TTM_SETTIPBKCOLOR, (WPARAM)RGB(0, 132, 132), 0);
	::SendMessage(m_hwndToolTip, TTM_SETTIPTEXTCOLOR, (WPARAM)RGB(255, 255, 255), 0);
	::SendMessage(m_hwndToolTip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)400); //to allow using of newline \n.
}

BOOL CPepperList::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	SetFocus();

	return CMFCListCtrl::OnSetCursor(pWnd, nHitTest, message);
}

void CPepperList::OnKillFocus(CWnd* pNewWnd)
{
}

BOOL CPepperList::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}

void CPepperList::OnPaint()
{
	CPaintDC dc(this);

	//To avoid flickering.
	//Drawing to CMemDC, excluding CListHeader area (rect).
	CRect rcClient, rcHdr;
	GetClientRect(&rcClient);
	GetHeaderCtrl().GetClientRect(rcHdr);
	rcClient.top += rcHdr.Height();
	CMemDC memDC(dc, rcClient);

	CRect clip;
	memDC.GetDC().GetClipBox(&clip);
	memDC.GetDC().FillSolidRect(clip, GetSysColor(COLOR_WINDOW));

	DefWindowProc(WM_PAINT, (WPARAM)memDC.GetDC().m_hDC, (LPARAM)0);
}

void CPepperList::OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	//	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	*pResult = 0;
}

void CPepperList::InitHeader()
{
	m_PepperListHeader.SubclassDlgItem(0, this);
}

void CPepperList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CMFCListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);

	/*	GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_ALL);

		int max = m_stScrollInfo.nMax - m_stScrollInfo.nPage + 1;
		if (m_stScrollInfo.nPos >= max)
		{
			m_fEraseBkgnd = true;
			Invalidate();
		}*/
}

void CPepperList::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	GetHeaderCtrl().Invalidate();
	GetHeaderCtrl().UpdateWindow();

	CMFCListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CPepperList::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	/*	GetScrollInfo(SB_VERT, &m_stScrollInfo, SIF_ALL);

		int max = m_stScrollInfo.nMax - m_stScrollInfo.nPage + 1;
		if (m_stScrollInfo.nPos >= max)
		{
			m_fEraseBkgnd = true;
			Invalidate();
		}
		*/
	GetHeaderCtrl().Invalidate();
	GetHeaderCtrl().UpdateWindow();

	return CMFCListCtrl::OnMouseWheel(nFlags, zDelta, pt);
}

void CPepperList::DrawItem(LPDRAWITEMSTRUCT pDIS)
{
	if (pDIS->itemID == -1)
		return;

	CDC* pDC = CDC::FromHandle(pDIS->hDC);
	pDC->SelectObject(&m_penForRect);
	pDC->SelectObject(&m_fontList);
	CRect rect;

	switch (pDIS->itemAction)
	{
	case ODA_SELECT:
	case ODA_DRAWENTIRE:
		if (HasToolTip(pDIS->itemID, 0))
			pDC->FillSolidRect(&rect, m_colorListToolTipSubitem);
		else
			pDC->FillSolidRect(&pDIS->rcItem, RGB(255, 255, 255));

		GetItemRect(pDIS->itemID, &rect, LVIR_LABEL);

		if (pDIS->itemState & ODS_SELECTED)
		{
			pDIS->rcItem.left = rect.left;
			pDC->SetTextColor(RGB(255, 255, 255));
			pDC->FillSolidRect(&pDIS->rcItem, m_colorListSelected);
		}
		else
			pDC->SetTextColor(RGB(0, 0, 0));

		rect.left += 3;
		pDC->DrawText(GetItemText(pDIS->itemID, 0), &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		rect.left -= 3;

		//Drawing Item's rect lines. 
		pDC->MoveTo(rect.left, rect.top);
		pDC->LineTo(rect.right, rect.top);
		pDC->MoveTo(rect.left, rect.top);
		pDC->LineTo(rect.left, rect.bottom);
		pDC->MoveTo(rect.left, rect.bottom);
		pDC->LineTo(rect.right, rect.bottom);
		pDC->MoveTo(rect.right, rect.top);
		pDC->LineTo(rect.right, rect.bottom);

		for (int i = 1; i < GetHeaderCtrl().GetItemCount(); i++)
		{
			GetSubItemRect(pDIS->itemID, i, LVIR_BOUNDS, rect);

			if (HasToolTip(pDIS->itemID, i))
				pDC->FillSolidRect(&rect, m_colorListToolTipSubitem);

			CString textItem = GetItemText(pDIS->itemID, i);
			rect.left += 3; //Drawing text +-3 px from rect bounds
			ExtTextOutW(pDC->m_hDC, rect.left, rect.top, ETO_CLIPPED, rect, textItem, textItem.GetLength(), nullptr);
			rect.left -= 3;

			//Drawing Subitem's rect lines. 
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.right, rect.top);
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.left, rect.bottom);
			pDC->MoveTo(rect.left, rect.bottom);
			pDC->LineTo(rect.right, rect.bottom);
			pDC->MoveTo(rect.right, rect.top);
			pDC->LineTo(rect.right, rect.bottom);
		}

		break;

	case ODA_FOCUS:
		break;
	}
}

void CPepperList::SetItemTooltip(int nItem, int nSubitem, const std::wstring& strTipText, const std::wstring& strTipCaption)
{
	//If there is no tooltip for such item/subitem we just set it.
	if (m_unmapTooltip.find(nItem) == m_unmapTooltip.end())
	{
		std::unordered_map<int, std::tuple< std::wstring, std::wstring>> mapInner;
		mapInner.insert({ nSubitem, { strTipText, strTipCaption } });
		m_unmapTooltip.insert({ nItem, mapInner });
	}
	else
	{	//If there is Item's tooltip but no Subitem's tooltip
		//inserting new Subitem into inner map.
		if (m_unmapTooltip.at(nItem).find(nSubitem) == m_unmapTooltip.at(nItem).end())
		{
			std::unordered_map<int, std::tuple< std::wstring, std::wstring>> &mapInner=m_unmapTooltip.at(nItem);
			mapInner.insert({ nSubitem, { strTipText, strTipCaption } });
			m_unmapTooltip.at(nItem) = mapInner;
		}
		else //If there is already such an Item-Subitem's tooltip, just change it.
		{
			std::unordered_map<int, std::tuple< std::wstring, std::wstring>> &mapInner = m_unmapTooltip.at(nItem);
			mapInner.at(nSubitem) = { strTipText, strTipCaption };
			m_unmapTooltip.at(nItem) = mapInner;
		}
	}

	m_fToolTip = true;
}

bool CPepperList::HasToolTip(int nItem, int nSubitem, std::wstring& strTipText, std::wstring& strTipCaption)
{
	if (m_unmapTooltip.find(nItem) != m_unmapTooltip.end())
		if (m_unmapTooltip.at(nItem).find(nSubitem) != m_unmapTooltip.at(nItem).end())
		{
			strTipText = std::get<0>(m_unmapTooltip.at(nItem).at(nSubitem));
			strTipCaption = std::get<1>(m_unmapTooltip.at(nItem).at(nSubitem));
			return true;
		}

	return false;
}

bool CPepperList::HasToolTip(int nItem, int nSubItem)
{
	if (m_unmapTooltip.find(nItem) != m_unmapTooltip.end())
		if (m_unmapTooltip.at(nItem).find(nSubItem) != m_unmapTooltip.at(nItem).end())
			return true;

	return false;
}

void CPepperList::OnLButtonDown(UINT nFlags, CPoint point)
{
	LVHITTESTINFO hitInfo { };
	hitInfo.pt = point;
	ListView_SubItemHitTest(m_hWnd, &hitInfo);
	if (hitInfo.iSubItem == -1 || hitInfo.iItem == -1)
		return;

	CMFCListCtrl::OnLButtonDown(nFlags, point);
}

void CPepperList::OnRButtonDown(UINT nFlags, CPoint point)
{
	LVHITTESTINFO hitInfo { };
	hitInfo.pt = point;
	ListView_SubItemHitTest(m_hWnd, &hitInfo);
	if (hitInfo.iSubItem == -1 || hitInfo.iItem == -1)
		return;

	CMFCListCtrl::OnRButtonDown(nFlags, point);
}

void CPepperList::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_fToolTip)
	{
		LVHITTESTINFO hitInfo { };
		hitInfo.pt = point;
		ListView_SubItemHitTest(m_hWnd, &hitInfo);

		std::wstring strTipText { }, strTipCaption { };
		bool fTip = HasToolTip(hitInfo.iItem, hitInfo.iSubItem, strTipText, strTipCaption);

		if (fTip)
		{
			//Check if cursor is still in the same cell's rect. If so - just leave.
			if (m_stCurrentSubItem.iItem == hitInfo.iItem && m_stCurrentSubItem.iSubItem == hitInfo.iSubItem)
				return;

			m_fToolTipShow = true;

			m_stCurrentSubItem.iItem = hitInfo.iItem;
			m_stCurrentSubItem.iSubItem = hitInfo.iSubItem;
			m_stToolInfo.lpszText = const_cast<LPWSTR>(strTipText.c_str());

			ClientToScreen(&point);
			::SendMessage(m_hwndToolTip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(point.x, point.y));
			::SendMessage(m_hwndToolTip, TTM_SETTITLE, (WPARAM)TTI_NONE, (LPARAM)strTipCaption.c_str());
			::SendMessage(m_hwndToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
			::SendMessage(m_hwndToolTip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);

			//Timer to check whether mouse left subitem rect.
			SetTimer(TIMER_TOOLTIP, 200, 0);
		}
		else
		{
			m_stCurrentSubItem.iItem = hitInfo.iItem;
			m_stCurrentSubItem.iSubItem = hitInfo.iSubItem;

			//If there is showed tooltip window.
			if (m_fToolTipShow)
			{
				m_fToolTipShow = false;
				::SendMessage(m_hwndToolTip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
			}
		}
	}
}

void CPepperList::OnTimer(UINT_PTR nIDEvent)
{
	//Checking if mouse left list's subitem rect,
	//if so — hiding tooltip and killing timer

	LVHITTESTINFO hitInfo { };
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	hitInfo.pt = point;
	ListView_SubItemHitTest(m_hWnd, &hitInfo);

	//If cursor is still hovers subitem then do nothing
	if (m_stCurrentSubItem.iItem == hitInfo.iItem && m_stCurrentSubItem.iSubItem == hitInfo.iSubItem)
		return;
	else
	{	//If it left
		m_fToolTipShow = false;
		::SendMessage(m_hwndToolTip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)(LPTOOLINFO)&m_stToolInfo);
		KillTimer(TIMER_TOOLTIP);
		m_stCurrentSubItem.iItem = hitInfo.iItem;
		m_stCurrentSubItem.iSubItem = hitInfo.iSubItem;
	}

	CMFCListCtrl::OnTimer(nIDEvent);
}