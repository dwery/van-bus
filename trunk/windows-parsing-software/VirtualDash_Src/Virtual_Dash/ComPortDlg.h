#pragma once


// CComPortDlg dialog

class CComPortDlg : public CDialog
{
	DECLARE_DYNAMIC(CComPortDlg)

public:
	CComPortDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CComPortDlg();

// Dialog Data
	enum { IDD = IDD_COM_PORT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:

	CString m_com_port;
	UINT m_baud_rate;
	BOOL m_check_start;
};
