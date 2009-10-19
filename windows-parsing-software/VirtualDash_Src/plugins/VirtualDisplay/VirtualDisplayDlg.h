#pragma once

extern "C"
{
#include <jpeglib.h>
#include <setjmp.h>
}
#include "../../common/inc/skin.h"

typedef struct _elem_t
{
	int x;
	int y;
	int l;
	int w;
	int font;
	int option1;
	int option2;
	CBitmap bmp;
} elem_t;

// CVirtualDisplayDlg dialog

class CVirtualDisplayDlg : public CDialog
{
	DECLARE_DYNAMIC(CVirtualDisplayDlg)

public:
	CVirtualDisplayDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVirtualDisplayDlg();

// Dialog Data
	enum { IDD = IDD_MAIN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	bool m_Click;
	CPoint m_pt_Mouse;
	CPoint m_OldPoint;
	int m_x;
	int m_y;


	CDC m_MemDC;
	elem_t tab_elem[MAX_ENTRIES];
	CBitmap small_font[256];
	CBitmap medium_font[256];
	CBitmap big_font[256];
	CBitmap m_Backbmp;
	skin_header_t m_skin_header;

	int read_JPEG_file (FILE* infile, CBitmap* pBmp, struct jpeg_decompress_struct *cinfo);
	void LoadSkin(char* file);
	void PutText(CDC* pDC, unsigned int index, char* txt);

	void DrawBmp(CDC* pDC, int index);
	void ClearBmp(CDC* pDC, int index);

public:
	void exitDlg();
	int ProcessMessage(int addr, unsigned int msgType, void* msg);
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
