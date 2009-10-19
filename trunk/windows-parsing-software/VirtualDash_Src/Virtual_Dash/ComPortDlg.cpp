// ComPortDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Virtual_Dash.h"
#include "ComPortDlg.h"


// CComPortDlg dialog

IMPLEMENT_DYNAMIC(CComPortDlg, CDialog)
CComPortDlg::CComPortDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CComPortDlg::IDD, pParent)
	, m_com_port(_T("com1"))
	, m_baud_rate(115200)
	, m_check_start(FALSE)
{
}

CComPortDlg::~CComPortDlg()
{
}

void CComPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBString(pDX, IDC_COMBO_COM_LIST, m_com_port);
	DDV_MaxChars(pDX, m_com_port, 9);
	DDX_Text(pDX, IDC_EDIT_BAUD_RATE, m_baud_rate);
	DDV_MinMaxUInt(pDX, m_baud_rate, 0, 460800);
	DDX_Check(pDX, IDC_CHECK_START, m_check_start);

}


BEGIN_MESSAGE_MAP(CComPortDlg, CDialog)
END_MESSAGE_MAP()


// CComPortDlg message handlers
