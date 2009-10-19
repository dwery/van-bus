#pragma once


// CLogDlg dialog

class CLogDlg : public CDialog
{
	DECLARE_DYNAMIC(CLogDlg)

public:
	CLogDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLogDlg();

	void exitDlg();

// Dialog Data
	enum { IDD = IDD_LOG_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CRichEditCtrl m_log;
	CRITICAL_SECTION	m_str_crit_sec;
	CString m_str;
	int m_lines;
	
	CButton m_check_E24;
	uint8 m_old_E24[250];
	uint16 m_old_len_E24;

	CButton m_check_8A4;
	uint8 m_old_8A4[250];
	uint16 m_old_len_8A4;

	CButton m_check_9C4;
	uint8 m_old_9C4[250];
	uint16 m_old_len_9C4;

	CButton m_check_4FC;
	uint8 m_old_4FC[250];
	uint16 m_old_len_4FC;

	CButton m_check_8C4;
	uint8 m_old_8C4[250];
	uint16 m_old_len_8C4;

	CButton m_check_564;
	uint8 m_old_564[250];
	uint16 m_old_len_564;

	CButton m_check_524;
	uint8 m_old_524[250];
	uint16 m_old_len_524;

	CButton m_check_824;
	uint8 m_old_824[250];
	uint16 m_old_len_824;

	CButton m_check_664;
	uint8 m_old_664[250];
	uint16 m_old_len_664;

	CButton m_check_554;
	uint8 m_old_554[250];
	uint16 m_old_len_554;

	CButton m_check_984;
	uint8 m_old_984[250];
	uint16 m_old_len_984;

	CButton m_check_4D4;
	uint8 m_old_4D4[250];
	uint16 m_old_len_4D4;

	CButton m_check_5E4;
	uint8 m_old_5E4[250];
	uint16 m_old_len_5E4;

	CButton m_check_Unknown;

	CButton m_check_filter;
	
public:
	virtual BOOL OnInitDialog();
	void LogData(int addr, char* txt, uint16 len);

	afx_msg void OnBnClickedButtonClearLog();
	afx_msg void OnBnClickedButtonSaveLog();
};
