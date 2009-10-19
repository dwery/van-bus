#pragma once


// CFilterDialog dialog

class CFilterDialog : public CDialog
{
	DECLARE_DYNAMIC(CFilterDialog)

public:
	CFilterDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFilterDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_FILE_FILTER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_filter;
};
