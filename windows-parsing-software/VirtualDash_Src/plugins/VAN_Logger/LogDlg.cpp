// LogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VAN_Logger.h"
#include "LogDlg.h"

#define MAX_LOG_LINES	27
// CLogDlg dialog

IMPLEMENT_DYNAMIC(CLogDlg, CDialog)
CLogDlg::CLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLogDlg::IDD, pParent)
{
	InitializeCriticalSection(&m_str_crit_sec);
}

CLogDlg::~CLogDlg()
{
	DeleteCriticalSection(&m_str_crit_sec);
}

void CLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_RICHEDIT2_LOG, m_log);

	DDX_Control(pDX, IDC_CHECK_FILTER_E24, m_check_E24);
	DDX_Control(pDX, IDC_CHECK_FILTER_8A4, m_check_8A4);
	DDX_Control(pDX, IDC_CHECK_FILTER_9C4, m_check_9C4);
	DDX_Control(pDX, IDC_CHECK_FILTER_4FC, m_check_4FC);
	DDX_Control(pDX, IDC_CHECK_FILTER_8C4, m_check_8C4);
	DDX_Control(pDX, IDC_CHECK_FILTER_564, m_check_564);
	DDX_Control(pDX, IDC_CHECK_FILTER_524, m_check_524);
	DDX_Control(pDX, IDC_CHECK_FILTER_824, m_check_824);
	DDX_Control(pDX, IDC_CHECK_FILTER_664, m_check_664);
	DDX_Control(pDX, IDC_CHECK_FILTER_554, m_check_554);
	DDX_Control(pDX, IDC_CHECK_FILTER_984, m_check_984);
	DDX_Control(pDX, IDC_CHECK_FILTER_4D4, m_check_4D4);
	DDX_Control(pDX, IDC_CHECK_FILTER_5E4, m_check_5E4);

	DDX_Control(pDX, IDC_CHECK_FILTER_UNKNOWN, m_check_Unknown);

	DDX_Control(pDX, IDC_CHECK_FILTER, m_check_filter);
}

void CLogDlg::exitDlg()
{
	OutputDebugString("Exiting LogDlg\n");
	

	EndDialog(0);
	
}

BEGIN_MESSAGE_MAP(CLogDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_LOG, OnBnClickedButtonClearLog)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_LOG, OnBnClickedButtonSaveLog)
END_MESSAGE_MAP()


// CLogDlg message handlers

BOOL CLogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_check_E24.SetCheck(1);
	m_check_8A4.SetCheck(1);
	m_check_9C4.SetCheck(1);
	m_check_4FC.SetCheck(1);
	m_check_8C4.SetCheck(1);
	m_check_564.SetCheck(1);
	m_check_524.SetCheck(1);
	m_check_824.SetCheck(1);
	m_check_664.SetCheck(1);
	m_check_554.SetCheck(1);
	m_check_984.SetCheck(1);
	m_check_4D4.SetCheck(1);
	m_check_5E4.SetCheck(1);

	m_check_Unknown.SetCheck(1);
	m_check_filter.SetCheck(1);

	m_old_len_E24 = 0; m_old_len_8A4 = 0;
	m_old_len_9C4 = 0; m_old_len_4FC = 0;
	m_old_len_8C4 = 0; m_old_len_564 = 0;
	m_old_len_524 = 0; m_old_len_824 = 0;
	m_old_len_664 = 0; m_old_len_554 = 0;
	m_old_len_984 = 0; m_old_len_4D4 = 0;
	m_old_len_5E4 = 0;

	m_str = "{\\rtf1{\\fonttbl{\\f0\\fmodern\\fprq1\\fcharset0 Courier New;}}{\\colortbl ;\\red255\\green0\\blue0;}\\fs16\\f0 }"; //100
	m_lines = 0;
	m_log.SetUndoLimit(0);

	//{\fonttbl{\f0\fswiss\fcharset0 Arial;}}
	//m_log.SetWindowText("{\\rtf1{\\colortbl ;\\red255\\green0\\blue0;}\\fs20 t\\cf1 es\\cf0 t\\par toto}");
	//m_log.SetWindowText("{\\rtf1{\\colortbl ;\\red255\\green0\\blue0;}\\fs20 toto\\par\n}"); //46
	m_log.SetWindowText(m_str);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CLogDlg::LogData(int addr, char* txt, uint16 len)
{
	if(addr == 0xE24)
	{
		if(m_check_E24.GetCheck() == 0)
			return;
	}
	else if(addr == 0x8A4)
	{
		if(m_check_8A4.GetCheck() == 0)
			return;
	}
	else if(addr == 0x9C4)
	{
		if(m_check_9C4.GetCheck() == 0)
			return;
	}
	else if(addr == 0x4FC)
	{
		if(m_check_4FC.GetCheck() == 0)
			return;
	}
	else if(addr == 0x8C4)
	{
		if(m_check_8C4.GetCheck() == 0)
			return;
	}
	else if(addr == 0x564)
	{
		if(m_check_564.GetCheck() == 0)
			return;
	}
	else if(addr == 0x524)
	{
		if(m_check_524.GetCheck() == 0)
			return;
	}
	else if(addr == 0x824)
	{
		if(m_check_824.GetCheck() == 0)
			return;
	}
	else if(addr == 0x664)
	{
		if(m_check_664.GetCheck() == 0)
			return;
	}
	else if(addr == 0x554)
	{
		if(m_check_554.GetCheck() == 0)
			return;
	}
	else if(addr == 0x984)
	{
		if(m_check_984.GetCheck() == 0)
			return;
	}
	else if(addr == 0x4D4)
	{
		if(m_check_4D4.GetCheck() == 0)
			return;
	}
	else if(addr == 0x5E4)
	{
		if(m_check_5E4.GetCheck() == 0)
			return;
	}
	else
	{
		if(m_check_Unknown.GetCheck() == 0)
			return;
	}

	EnterCriticalSection(&m_str_crit_sec);

	if(m_lines == MAX_LOG_LINES)
	{
		int ind;
		ind = m_str.Find("\\par", 100);
		if(ind!=-1)
			m_str.Delete(101, ind-101+5);

		--m_lines;
	}

	
	
	if(m_check_filter.GetCheck() == 0)
	{
	
		m_str.Delete(m_str.GetLength()-1,1);

		m_str.Append(txt, len);

		m_str.Append("\\par\n}");
		++m_lines;

		m_log.SetWindowText(m_str);
	}
	else
	{
		uint8* old_txt;
		uint16 old_len;

		if(addr == 0xE24)
		{
			old_txt = m_old_E24;	old_len = m_old_len_E24;
		}
		else if(addr == 0x8A4)
		{
			old_txt = m_old_8A4;	old_len = m_old_len_8A4;
		}
		else if(addr == 0x9C4)
		{
			old_txt = m_old_9C4;	old_len = m_old_len_9C4;
		}
		else if(addr == 0x4FC)
		{
			old_txt = m_old_4FC;	old_len = m_old_len_4FC;
		}
		else if(addr == 0x8C4)
		{
			old_txt = m_old_8C4;	old_len = m_old_len_8C4;
		}
		else if(addr == 0x564)
		{
			old_txt = m_old_564;	old_len = m_old_len_564;
		}
		else if(addr == 0x524)
		{
			old_txt = m_old_524;	old_len = m_old_len_524;
		}
		else if(addr == 0x824)
		{
			old_txt = m_old_824;	old_len = m_old_len_824;
		}
		else if(addr == 0x664)
		{
			old_txt = m_old_664;	old_len = m_old_len_664;
		}
		else if(addr == 0x554)
		{
			old_txt = m_old_554;	old_len = m_old_len_554;
		}
		else if(addr == 0x984)
		{
			old_txt = m_old_984;	old_len = m_old_len_984;
		}
		else if(addr == 0x4D4)
		{
			old_txt = m_old_4D4;	old_len = m_old_len_4D4;
		}
		else if(addr == 0x5E4)
		{
			old_txt = m_old_5E4;	old_len = m_old_len_5E4;
		}
		else
			old_len = 1;

		//Set in red the bytes that are different...
		if(old_len == 0) // first time
		{
			m_str.Delete(m_str.GetLength()-1,1);
			m_str.Append(txt, len);
			m_str.Append("\\par\n}");
			++m_lines;
			m_log.SetWindowText(m_str);
		}
		else if(old_len == 1) // unknown addr!
		{
			m_str.Delete(m_str.GetLength()-1,1);
			
			m_str.Append("\\cf1 ");
			m_str.Append(txt, len);
			m_str.Append("\\cf0 ");

			m_str.Append("\\par\n}");
			++m_lines;
			m_log.SetWindowText(m_str);
		}
		else
		{
			if(old_len != len) // no the same len as the old packet!
			{
				m_str.Delete(m_str.GetLength()-1,1);
			
				m_str.Append("\\cf1 ");
				m_str.Append(txt, len);
				m_str.Append("\\cf0 ");

				m_str.Append("\\par\n}");
				++m_lines;
				m_log.SetWindowText(m_str);
			}
			else
			{
				uint16 i;
				uint8 st = 0; // same
				uint8 all_same = 1;
				
				CString tmp_str = "";

				
				for(i=0;i<len;++i)
				{
					if((old_txt[i] != txt[i]) && (st == 0))
					{
						st = 1;//different;
						all_same = 0;
						tmp_str.Append("\\cf1 ");
					}
					else if((old_txt[i] == txt[i]) && (st == 1))
					{
						st = 0;//different;
						tmp_str.Append("\\cf0 ");
					}
					
					tmp_str.AppendChar(txt[i]);
				}

				if(st)
					tmp_str.Append("\\cf0 ");

				if(!all_same)
				{
					m_str.Delete(m_str.GetLength()-1,1);
					m_str.Append(tmp_str);
					m_str.Append("\\par\n}");
					++m_lines;
					m_log.SetWindowText(m_str);

			
				}
			}
		}

		if(addr == 0xE24)
		{
			memcpy(m_old_E24, txt, len);	m_old_len_E24 = len;
		}
		else if(addr == 0x8A4)
		{
			memcpy(m_old_8A4, txt, len);	m_old_len_8A4 = len;
		}
		else if(addr == 0x9C4)
		{
			memcpy(m_old_9C4, txt, len);	m_old_len_9C4 = len;
		}
		else if(addr == 0x4FC)
		{
			memcpy(m_old_4FC, txt, len);	m_old_len_4FC = len;
		}
		else if(addr == 0x8C4)
		{
			memcpy(m_old_8C4, txt, len);	m_old_len_8C4 = len;
		}
		else if(addr == 0x564)
		{
			memcpy(m_old_564, txt, len);	m_old_len_564 = len;
		}
		else if(addr == 0x524)
		{
			memcpy(m_old_524, txt, len);	m_old_len_524 = len;
		}
		else if(addr == 0x824)
		{
			memcpy(m_old_824, txt, len);	m_old_len_824 = len;
		}
		else if(addr == 0x664)
		{
			memcpy(m_old_664, txt, len);	m_old_len_664 = len;
		}
		else if(addr == 0x554)
		{
			memcpy(m_old_554, txt, len);	m_old_len_554 = len;
		}
		else if(addr == 0x984)
		{
			memcpy(m_old_984, txt, len);	m_old_len_984 = len;
		}
		else if(addr == 0x4D4)
		{
			memcpy(m_old_4D4, txt, len);	m_old_len_4D4 = len;
		}
		else if(addr == 0x5E4)
		{
			memcpy(m_old_5E4, txt, len);	m_old_len_5E4 = len;
		}
		
	}

	LeaveCriticalSection(&m_str_crit_sec);
}

void CLogDlg::OnBnClickedButtonClearLog()
{
	EnterCriticalSection(&m_str_crit_sec);
	
	m_str = "{\\rtf1{\\fonttbl{\\f0\\fmodern\\fprq1\\fcharset0 Courier New;}}{\\colortbl ;\\red255\\green0\\blue0;}\\fs16\\f0 }"; //100
	m_lines = 0;
	m_log.SetWindowText(m_str);

	LeaveCriticalSection(&m_str_crit_sec);
}

void CLogDlg::OnBnClickedButtonSaveLog()
{
	CFileDialog file_dlg(false, ".rtf", NULL, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, "RTF file (*.rtf) |*.rtf||", this);
	
	int rep = (int)file_dlg.DoModal();
	if(rep != IDOK)
		return;

	
	FILE *fp;

	fp = fopen((LPCSTR)(file_dlg.GetPathName()), "w+b");
	if(fp == NULL)
	{
		AfxMessageBox("Error : cannot create file\n", MB_ICONERROR);
		return;
	}

	EnterCriticalSection(&m_str_crit_sec);

	fwrite((LPCSTR)m_str, sizeof(char), m_str.GetLength(), fp);
	LeaveCriticalSection(&m_str_crit_sec);

	fclose(fp);
}
