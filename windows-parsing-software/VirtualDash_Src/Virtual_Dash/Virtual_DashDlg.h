// Virtual_DashDlg.h : header file
//

#pragma once

#include "../common/inc/plugins/plugins_defs.h"
#include "../common/inc/plugins/parser_plugins_defs.h"

#include "vansource.h"
#include "afxwin.h"

#define MAX_PLUGINS			20
#define MAX_PLUGIN_NAME_LEN	50
#define MAX_SKIN_NAME_LEN	50



typedef struct _plugin_t
{
	char				file_name[MAX_PLUGIN_NAME_LEN];
	HMODULE 			handle;

	TFGetPluginType			GetPluginType;
	TFInitPlugin			InitPlugin;
	TFClosePlugin			ClosePlugin;
	TFProcessMessage		ProcessMessage;
	TFProcessRawVANPacket	ProcessRawVANPacket;
} plugin_t;

typedef struct _parser_plugin_t
{
	char				file_name[MAX_PLUGIN_NAME_LEN];
	HMODULE 			handle;

	TFInit_Parser			Init_Parser;
	TFClose_Parser			Close_Parser;
	TFOnNewByte				OnNewByte;
} parser_plugin_t;

class CVirtual_DashDlg;

class CParserInterface : public CParserPlugin
{
	CVirtual_DashDlg* dlg;

public:
	CParserInterface(CVirtual_DashDlg* ptr);
	virtual ~CParserInterface(void);

	virtual void deliverMsgToPlugins(int addr, int msg_type, void* msg);
	virtual void deliverRAWPacketToPlugins(int addr, uint8* pkt, uint16 len);
};

// CVirtual_DashDlg dialog
class CVirtual_DashDlg : public CDialog
{
// Construction
public:
	CVirtual_DashDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CVirtual_DashDlg();

// Dialog Data
	enum { IDD = IDD_VIRTUAL_DASH_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	void ParseIniFile(FILE *f);
	bool ParseLine(char* sline);
	int LoadParser(void);
	void UnLoadParser(void);
	void LoadPlugins(void);
	void UnLoadPlugins(bool clean);

	char m_skin_file_name[MAX_SKIN_NAME_LEN];
	int m_num_plugins;
	plugin_t m_plugin_list[MAX_PLUGINS];

	parser_plugin_t m_parser_plugin;
	CRITICAL_SECTION m_parser_plugin_crit_sec;

	CRITICAL_SECTION m_plugin_crit_sec;
	HANDLE	m_parserThread_Handle;
	bool m_exit_thread;

	CRITICAL_SECTION m_van_src_crit_sec;
	CVANSource* m_van_src;
	static DWORD WINAPI ParserThread(LPVOID lpParameter);
	
	CParserInterface* m_ParseInterface;

	virtual void deliverMsgToPlugins(int addr, int msg_type, void* msg);
	virtual void deliverRAWPacketToPlugins(int addr, uint8* pkt, uint16 len);
	

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnOpenLogfile();
	afx_msg void OnOpenComport();
};
