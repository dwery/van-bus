// Virtual_DashDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Virtual_Dash.h"
#include "Virtual_DashDlg.h"
#include "van_UartSource.h"
#include "van_FileSource.h"
#include "ComPortDlg.h"
#include "FilterDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CVirtual_DashDlg dialog
CVirtual_DashDlg::CVirtual_DashDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVirtual_DashDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVirtual_DashDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVirtual_DashDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_OPEN_LOGFILE, OnOpenLogfile)
	ON_COMMAND(ID_OPEN_COMPORT, OnOpenComport)
END_MESSAGE_MAP()


// CVirtual_DashDlg message handlers

BOOL CVirtual_DashDlg::OnInitDialog()
{
	int i;
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetWindowLong(this->m_hWnd, GWL_STYLE, GetWindowLong(this->m_hWnd, GWL_STYLE) | WS_MINIMIZEBOX);

	m_ParseInterface = NULL;

	m_num_plugins = 0;
	for(i=0;i<MAX_PLUGINS;i++)
	{
		m_plugin_list[i].handle = NULL;
		m_plugin_list[i].ClosePlugin = NULL;
		m_plugin_list[i].GetPluginType = NULL;
		m_plugin_list[i].InitPlugin = NULL;
		m_plugin_list[i].ProcessMessage = NULL;
		m_plugin_list[i].ProcessRawVANPacket = NULL;
		m_plugin_list[i].file_name[0] = 0;
	}

	m_parser_plugin.handle = NULL;
	m_parser_plugin.file_name[0] = 0;
	m_parser_plugin.Init_Parser = NULL;
	m_parser_plugin.Close_Parser = NULL;
	m_parser_plugin.OnNewByte = NULL;

	InitializeCriticalSection(&m_parser_plugin_crit_sec);

	InitializeCriticalSection(&m_plugin_crit_sec);
	InitializeCriticalSection(&m_van_src_crit_sec);
	m_van_src = NULL;

	m_parserThread_Handle = INVALID_HANDLE_VALUE;

	FILE* f = fopen("settings.ini", "rt");
	if(f)
	{
		ParseIniFile(f);
		fclose(f);
	}

	if(LoadParser())
	{
		m_ParseInterface = new CParserInterface(this);

		EnterCriticalSection(&m_parser_plugin_crit_sec);
		m_parser_plugin.Init_Parser(m_ParseInterface);
		LeaveCriticalSection(&m_parser_plugin_crit_sec);

		LoadPlugins();
		
		m_exit_thread = false;
		m_parserThread_Handle = CreateThread(NULL,0,CVirtual_DashDlg::ParserThread,this,0,0);
	}
	else
	{
		EndDialog(-1); // no parser
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

//return 1 if no error
int CVirtual_DashDlg::LoadParser(void)
{
	char file[100];

	OutputDebugString("loading parser...\n");

	EnterCriticalSection(&m_parser_plugin_crit_sec);

	if(m_parser_plugin.file_name[0] == 0)
	{
		AfxMessageBox("No parser specified in settings.ini\n", MB_ICONERROR);
		LeaveCriticalSection(&m_parser_plugin_crit_sec);
		return 0;
	}
	strcpy(file,"parsers\\");
	strcat(file, m_parser_plugin.file_name);
	m_parser_plugin.handle = LoadLibrary(file);
	if(m_parser_plugin.handle == NULL)
	{
		sprintf(file,"Error : cannot load plugin '%s'!", m_parser_plugin.file_name);
		AfxMessageBox(file, MB_ICONERROR);

		LeaveCriticalSection(&m_parser_plugin_crit_sec);
		return 0;
	}

	m_parser_plugin.Init_Parser = (TFInit_Parser)::GetProcAddress(m_parser_plugin.handle, "Init_Parser");
	if(m_parser_plugin.Init_Parser == NULL)
	{
		AfxMessageBox("Invalid parser plugin : Init_Parser function not found!\n");
		FreeLibrary(m_parser_plugin.handle);
		LeaveCriticalSection(&m_parser_plugin_crit_sec);
		return 0;
	}

	m_parser_plugin.Close_Parser = (TFClose_Parser)::GetProcAddress(m_parser_plugin.handle, "Close_Parser");
	if(m_parser_plugin.Close_Parser == NULL)
	{
		AfxMessageBox("Invalid plugin : Close_Parser function not found!\n");
		FreeLibrary(m_parser_plugin.handle);
		LeaveCriticalSection(&m_parser_plugin_crit_sec);
		return 0;
	}

	m_parser_plugin.OnNewByte = (TFOnNewByte)::GetProcAddress(m_parser_plugin.handle, "OnNewByte");
	if(m_parser_plugin.OnNewByte == NULL)
	{
		AfxMessageBox("Invalid plugin : OnNewByte function not found!\n");
		FreeLibrary(m_parser_plugin.handle);
		LeaveCriticalSection(&m_parser_plugin_crit_sec);
		return 0;
	}

	LeaveCriticalSection(&m_parser_plugin_crit_sec);
	OutputDebugString("Parser loaded.\n");

	return 1;
}

void CVirtual_DashDlg::UnLoadParser(void)
{
	OutputDebugString("Unloading parser...\n");

	EnterCriticalSection(&m_parser_plugin_crit_sec);

	if(m_parser_plugin.Close_Parser)
		m_parser_plugin.Close_Parser();

	if(m_parser_plugin.handle != NULL)
	{
		FreeLibrary(m_parser_plugin.handle);
	}
	m_parser_plugin.handle = NULL;
	m_parser_plugin.Close_Parser = NULL;
	m_parser_plugin.Init_Parser = NULL;
	m_parser_plugin.OnNewByte = NULL;

	LeaveCriticalSection(&m_parser_plugin_crit_sec);

	OutputDebugString("Parser Unloaded.\n");
}

void CVirtual_DashDlg::LoadPlugins(void)
{
	int i;
	int type;
	char file[100];

	OutputDebugString("loading plugins...\n");

	EnterCriticalSection(&m_plugin_crit_sec);
	for(i=0;i<m_num_plugins;i++)
	{
		strcpy(file,"plugins\\");
		strcat(file, m_plugin_list[i].file_name);
		m_plugin_list[i].handle = LoadLibrary(file);
		if(m_plugin_list[i].handle == NULL)
		{
			sprintf(file,"Error : cannot load plugin '%s'!", m_plugin_list[i].file_name);
			AfxMessageBox(file, MB_ICONERROR);
		}
		else
		{
			m_plugin_list[i].GetPluginType = (TFGetPluginType)::GetProcAddress(m_plugin_list[i].handle, "Get_PluginType");

			if(m_plugin_list[i].GetPluginType == NULL)
			{
				OutputDebugString("Invalid plugin : Get_PluginType function not found!\n");
			}
			else
			{
				type = m_plugin_list[i].GetPluginType();
				
				m_plugin_list[i].ClosePlugin = (TFClosePlugin)::GetProcAddress(m_plugin_list[i].handle, "Close_Plugin");
				if(m_plugin_list[i].ClosePlugin == NULL)
					OutputDebugString("ERROR : Close_Plugin function not found\n");

				if(type == LOGGER_PLUGIN)
				{
					m_plugin_list[i].ProcessRawVANPacket = (TFProcessRawVANPacket)::GetProcAddress(m_plugin_list[i].handle, "Process_Raw_VAN_Packet");
					if(m_plugin_list[i].ProcessRawVANPacket == NULL)
						OutputDebugString("ERROR : Process_Raw_VAN_Packet function not found\n");
				}
				else
				{
					m_plugin_list[i].ProcessMessage = (TFProcessMessage)::GetProcAddress(m_plugin_list[i].handle, "Process_Message");
					if(m_plugin_list[i].ProcessMessage == NULL)
						OutputDebugString("ERROR : Process_Message function not found\n");
				}
				
				
				m_plugin_list[i].InitPlugin = (TFInitPlugin)::GetProcAddress(m_plugin_list[i].handle, "Init_Plugin");
				if(m_plugin_list[i].InitPlugin == NULL)
					OutputDebugString("ERROR : Init_Plugin function not found\n");
				else
					m_plugin_list[i].InitPlugin(this);
			}
		}
	}

	LeaveCriticalSection(&m_plugin_crit_sec);
	OutputDebugString("Plugins loaded.\n");
}

void CVirtual_DashDlg::UnLoadPlugins(bool clean)
{
	int i;
	if(m_num_plugins == 0)
		return;

	OutputDebugString("Unloading plugins...\n");

	EnterCriticalSection(&m_plugin_crit_sec);

	for(i=0;i<m_num_plugins;i++)
	{
		if(m_plugin_list[i].ClosePlugin)
			m_plugin_list[i].ClosePlugin();

		if(m_plugin_list[i].handle != NULL)
		{
			FreeLibrary(m_plugin_list[i].handle);
		}
		m_plugin_list[i].handle = NULL;
		m_plugin_list[i].ClosePlugin = NULL;
		m_plugin_list[i].GetPluginType = NULL;
		m_plugin_list[i].InitPlugin = NULL;
		m_plugin_list[i].ProcessMessage = NULL;
		m_plugin_list[i].ProcessRawVANPacket = NULL;
		//m_plugin_list[i].file_name[0] = 0;
	}
	//m_num_plugins = 0;

	if(clean)
		m_num_plugins = 0; // we'll reload a new list of plugins...

	LeaveCriticalSection(&m_plugin_crit_sec);
	OutputDebugString("Plugins Unloaded.\n");
}

#define MAX_LINE_SIZE 255
void CVirtual_DashDlg::ParseIniFile(FILE *f)
{
	char sline[MAX_LINE_SIZE];
	int i;
	int err=0;
	char deb[50];

	OutputDebugString("Parsing file...\n");
	i=1;

	EnterCriticalSection(&m_plugin_crit_sec);
	while(fgets(sline, MAX_LINE_SIZE, f) != NULL)
	{
		//sprintf(deb,"Line %d:\n", i);
		//OutputDebugString(deb);
		//OutputDebugString(sline);
		if(strlen(sline) > 2)
		{
			if(((sline[0] != '/') || (sline[1] != '/')) && (sline[0] != ';') && (sline[0] != '#')) // not a comment? (// or ; or #)
			{
				if(ParseLine(sline))
				{
					err = 1;
					sprintf(deb,"Syntax error on line %d\n", i);
					OutputDebugString(deb);
				}
			}
		}
		else
		{
			if(sline[0] != '\n')
			{
				err = 1;
				sprintf(deb, "Syntax Error at line %d\n", i);
				OutputDebugString(deb);
			}
		}
		i++;
	}
	OutputDebugString("Parsing done.\n");

	LeaveCriticalSection(&m_plugin_crit_sec);

	if(err)
	{
		AfxMessageBox("There are errors in the file!\n");
		return;
	}
}

bool CVirtual_DashDlg::ParseLine(char* sline)
{
	//first remove section under comment
	unsigned int i;
	unsigned int j;
	char tmp[100];
	char* args;
	unsigned char found = 0;
	
		
	for(i=0;i<(strlen(sline)-1);i++) // the last char is always '\n', except the last line
	{
		if(((sline[i] == '/') && (sline[i+1] == '/')) || (sline[i] == ';') || (sline[i] == '#')) // a comment? (// or ; or #)
		{
			sline[i] = 0;
			break;
		}
	}
	if(sline[strlen(sline)-1] == '\n')
		sline[strlen(sline)-1] = 0; // remove '\n' at the end

	//OutputDebugString(sline);
	//OutputDebugString("\n");

	j=0;
	for(i=0;i<strlen(sline);++i)
	{
		if(sline[i] == '=')
		{
			found = 1;
			break;
		}
		if(sline[i] != ' ')
		{
			tmp[j] = sline[i];
			j++;
		}
	}

	if(!found)
		return true;

	tmp[j] = '\0';
	
	if(strlen(tmp) == 0)
		return true;

	args = sline + i + 1; // the arguments, just after '='

	

	if(strlen(args) == 0) // oups! no argument found
	{
		OutputDebugString("WARNING : No arguments : ");
		return true;
	}

	// convert to lower case
	_strlwr(tmp);


	if(!strncmp(tmp, "skin", 4))
	{		
		if(strlen(args) > MAX_SKIN_NAME_LEN)
		{
			OutputDebugString("ERROR : skin file name too big...\n");
			m_skin_file_name[0] = 0;
			return true;
		}

		strcpy(m_skin_file_name, args);
		
		OutputDebugString("skin:");	OutputDebugString(m_skin_file_name); OutputDebugString("\n");

		return false;
	}
	else if(!strncmp(tmp, "plugin", 6))
	{		
		if(strlen(args) > MAX_PLUGIN_NAME_LEN)
		{
			OutputDebugString("ERROR : plugin file name too big...\n");
			return true;
		}

		strcpy(m_plugin_list[m_num_plugins].file_name, args);
				
		OutputDebugString("plugin:"); OutputDebugString(m_plugin_list[m_num_plugins].file_name); OutputDebugString("\n");

		m_num_plugins++;

		return false;
	}
	else if(!strncmp(tmp, "parser", 6))
	{
		if(strlen(args) > MAX_PLUGIN_NAME_LEN)
		{
			OutputDebugString("ERROR : parser file name too big...\n");
			return true;
		}
		
		if(m_parser_plugin.file_name[0] != 0)
		{
			OutputDebugString("WARNING : parser plugin already specified in ini file...\n");
			return true;
		}

		strcpy(m_parser_plugin.file_name, args);
				
		OutputDebugString("parser:"); OutputDebugString(m_parser_plugin.file_name); OutputDebugString("\n");

		return false;
	}
	return true;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVirtual_DashDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVirtual_DashDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVirtual_DashDlg::OnDestroy()
{
	CDialog::OnDestroy();

	OutputDebugString("Waiting ParserThread to exit\n");
	m_exit_thread = true;
	if(m_van_src)
		m_van_src->AbortRead();

	if(m_parserThread_Handle != INVALID_HANDLE_VALUE)
	{
		WaitForSingleObject(m_parserThread_Handle, 10000);
		CloseHandle(m_parserThread_Handle);
		m_parserThread_Handle = INVALID_HANDLE_VALUE;
	}
	
	UnLoadPlugins(true);

	UnLoadParser();

	OutputDebugString("Exiting application\n");
}

CVirtual_DashDlg::~CVirtual_DashDlg()
{
	OutputDebugString("CVirtual_DashDlg destructor\n");

	EnterCriticalSection(&m_van_src_crit_sec);
	if(m_van_src)
		delete m_van_src;
	m_van_src = NULL;
	LeaveCriticalSection(&m_van_src_crit_sec);

	if(m_ParseInterface)
		delete m_ParseInterface;

	DeleteCriticalSection(&m_plugin_crit_sec);
	DeleteCriticalSection(&m_parser_plugin_crit_sec);
	DeleteCriticalSection(&m_van_src_crit_sec);

	OutputDebugString("CVirtual_DashDlg destructor :: END\n");
}

void CVirtual_DashDlg::deliverMsgToPlugins(int addr, int msg_type, void* msg)
{
	int i;

	EnterCriticalSection(&m_plugin_crit_sec);
	for(i=0;i<m_num_plugins;i++)
	{
		if(m_plugin_list[i].ProcessMessage)
		{
			m_plugin_list[i].ProcessMessage(addr, msg_type, msg);
		}
	}

	LeaveCriticalSection(&m_plugin_crit_sec);
}

void CVirtual_DashDlg::deliverRAWPacketToPlugins(int addr, uint8* pkt, uint16 len)
{
	int i;

	EnterCriticalSection(&m_plugin_crit_sec);
	for(i=0;i<m_num_plugins;i++)
	{
		if(m_plugin_list[i].ProcessRawVANPacket) //only logger plugins have a valid ProcessRawVANPacket function...
		{
			m_plugin_list[i].ProcessRawVANPacket(addr, pkt, len);
		}
	}

	LeaveCriticalSection(&m_plugin_crit_sec);
}


DWORD WINAPI CVirtual_DashDlg::ParserThread(LPVOID lpParameter)
{
	CVirtual_DashDlg* dlg = (CVirtual_DashDlg*)lpParameter;
	uint8 c;
	
	OutputDebugString("Starting parser thread...\n");

	
	while(!dlg->m_exit_thread)
	{
		EnterCriticalSection(&(dlg->m_van_src_crit_sec));
		if(dlg->m_van_src)
		{
			c = dlg->m_van_src->ReadByte();
			LeaveCriticalSection(&(dlg->m_van_src_crit_sec));
		}
		else
		{
			LeaveCriticalSection(&(dlg->m_van_src_crit_sec));
			Sleep(1000);
			continue;
		}
		

		EnterCriticalSection(&(dlg->m_parser_plugin_crit_sec));

		if(dlg->m_parser_plugin.OnNewByte)
			dlg->m_parser_plugin.OnNewByte(c);

		LeaveCriticalSection(&(dlg->m_parser_plugin_crit_sec));
	}

	OutputDebugString("Exiting parser thread...\n");
	return 0;
}

void CVirtual_DashDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	CMenu pmenu;

	pmenu.LoadMenu(IDR_GENERAL_MENU);
	CMenu* pm = pmenu.GetSubMenu(0);

	ClientToScreen(&point);
	pm->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);

	DestroyMenu(pmenu);

	CDialog::OnRButtonUp(nFlags, point);
}

void CVirtual_DashDlg::OnOpenLogfile()
{
	CFileDialog file(true, ".txt", NULL,OFN_FILEMUSTEXIST, "VAN Log file (*.txt) |*.txt||", this);
	
	int rep = (int)file.DoModal();
	if(rep == IDOK)
	{
		CFilterDialog filterDlg;
		uint32 filter;

		if(m_van_src)
			m_van_src->AbortRead();

		EnterCriticalSection(&m_van_src_crit_sec);
		
		if(m_van_src)
		{
			m_van_src->CloseSource();
			delete m_van_src;

		}
		
		m_van_src = new CVAN_FileSource();

		filterDlg.DoModal();
		if(filterDlg.m_filter == 0)
		{
			OutputDebugString("VAN_FILE_HEX\n");
			filter = VAN_FILE_HEX; //to be used with rev3a of van_mon (output in hexa) (dump file in raw binary)
		}
		else if(filterDlg.m_filter == 1)
		{
			OutputDebugString("VAN_FILE_ASCII_ONLY\n");
			filter = VAN_FILE_ASCII_ONLY; //to be used with rev3 of Van_Mon (standard version, output in ASCII)
		}
		else if(filterDlg.m_filter == 2)
		{
			OutputDebugString("VAN_FILE_ASCII_TO_HEX\n");
			filter = VAN_FILE_ASCII_TO_HEX; //to be used with rev3a of VAN_Mon (output in hexa, file was saved in text format (for example using SDT))
		}

		if(m_van_src->OpenSource((char*)((LPCSTR)file.GetPathName()), NULL, filter))
		{
			delete m_van_src;
			m_van_src = NULL;
			AfxMessageBox("Connot open log file!\n", MB_ICONERROR);
		}

		LeaveCriticalSection(&m_van_src_crit_sec);
	}
}


void CVirtual_DashDlg::OnOpenComport()
{
	CComPortDlg dlg;
	char arg[100];

	if(dlg.DoModal() == IDOK)
	{
		if(m_van_src)
			m_van_src->AbortRead();

		EnterCriticalSection(&m_van_src_crit_sec);
		
		if(m_van_src)
		{
			m_van_src->CloseSource();
			delete m_van_src;
		}
		
		m_van_src = new CVAN_UartSource();	
		sprintf(arg,"baud=%u parity=N data=8 stop=1", dlg.m_baud_rate);
		OutputDebugString(arg);
		OutputDebugString("\n");

		if(m_van_src->OpenSource((char*)((LPCSTR)dlg.m_com_port), arg, SER32_NO_FLOW_CONTROL))
		{
			delete m_van_src;
			m_van_src = NULL;
			sprintf(arg, "Cannot open com port %s", (char*)((LPCSTR)dlg.m_com_port));
			AfxMessageBox(arg, MB_ICONERROR);
		}
		else
		{
			if(dlg.m_check_start)
			{
				OutputDebugString("Send 'c' to the pic16F\n");
				m_van_src->WriteData((uint8*)"c\x0d", 2);
			}
		}

		LeaveCriticalSection(&m_van_src_crit_sec);
	}
}

CParserInterface::CParserInterface(CVirtual_DashDlg* ptr)
{
	dlg = ptr;
}

CParserInterface::~CParserInterface(void)
{
}

void CParserInterface::deliverMsgToPlugins(int addr, int msg_type, void* msg)
{
	dlg->deliverMsgToPlugins(addr, msg_type, msg);
}

void CParserInterface::deliverRAWPacketToPlugins(int addr, uint8* pkt, uint16 len)
{
	dlg->deliverRAWPacketToPlugins(addr, pkt, len);
}

