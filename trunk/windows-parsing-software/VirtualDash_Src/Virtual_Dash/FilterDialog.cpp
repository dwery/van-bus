// FilterDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Virtual_Dash.h"
#include "FilterDialog.h"


// CFilterDialog dialog

IMPLEMENT_DYNAMIC(CFilterDialog, CDialog)
CFilterDialog::CFilterDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFilterDialog::IDD, pParent)
	, m_filter(1)
{
}

CFilterDialog::~CFilterDialog()
{
}

void CFilterDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_COM_NO_FILTER, m_filter);
}


BEGIN_MESSAGE_MAP(CFilterDialog, CDialog)
END_MESSAGE_MAP()


// CFilterDialog message handlers
