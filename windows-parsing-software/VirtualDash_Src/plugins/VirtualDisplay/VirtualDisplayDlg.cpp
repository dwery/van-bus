// VirtualDisplayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VirtualDisplay.h"
#include "VirtualDisplayDlg.h"

#include "../../common/inc/plugins/plugins_defs.h"

char BIG_FONT_TXT[] = "0123456789- ";
char MED_FONT_TXT[] = "0123456789- ";
char SMALL_FONT_TXT[] = "0123456789- abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.";

// CVirtualDisplayDlg dialog

IMPLEMENT_DYNAMIC(CVirtualDisplayDlg, CDialog)
CVirtualDisplayDlg::CVirtualDisplayDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVirtualDisplayDlg::IDD, pParent)
{
	m_MemDC.CreateCompatibleDC(NULL);

	m_Click = false;
	m_x = m_y = 0;

	for(int i=0;i<MAX_ENTRIES;i++)
	{
		tab_elem[i].x = -1;
		tab_elem[i].y = -1;
		tab_elem[i].l = -1;
		tab_elem[i].w = -1;
		tab_elem[i].font = -1;
		tab_elem[i].option1 = -1;
		tab_elem[i].option2 = -1;
	}
}

CVirtualDisplayDlg::~CVirtualDisplayDlg()
{
}

void CVirtualDisplayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVirtualDisplayDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CVirtualDisplayDlg message handlers


void CVirtualDisplayDlg::exitDlg()
{
	OutputDebugString("Exiting VirtualDisplayDlg\n");
	
	EndDialog(0);
	
}

BOOL CVirtualDisplayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	LoadSkin("skins\\test.skz");
	SetWindowText("VirtualDisplay v0.1");

	SetWindowLong(this->m_hWnd, GWL_STYLE, GetWindowLong(this->m_hWnd, GWL_STYLE) | WS_MINIMIZEBOX);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

int CVirtualDisplayDlg::read_JPEG_file (FILE* infile, CBitmap* pBmp, struct jpeg_decompress_struct *cinfo)
{
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */

	unsigned char* buff;

	CDC dc;
	dc.Attach(::GetDC(NULL));
	//CDC dcMem;
	BITMAPINFO info;
	
	(void) jpeg_read_header(cinfo, TRUE);
	(void) jpeg_start_decompress(cinfo);

	row_stride = cinfo->output_width * cinfo->output_components;
	buffer = (*cinfo->mem->alloc_sarray)((j_common_ptr) cinfo, JPOOL_IMAGE, row_stride, 1);


	int index = 0;
	int cx = cinfo->output_width;
	int cy = cinfo->output_height;
	buff = new unsigned char [cx*cy*cinfo->output_components];

	index = (cy-1)*row_stride;

	if(pBmp->m_hObject != NULL)
		DeleteObject(pBmp->Detach());
	pBmp->CreateCompatibleBitmap(&dc, cx, cy);
	ZeroMemory(&info, sizeof(BITMAPINFO));
	info.bmiHeader.biBitCount = (WORD)(8*cinfo->output_components)/*(8*nb_bytes/(cx*cy))*/;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = cx;
	info.bmiHeader.biHeight = cy;
	info.bmiHeader.biCompression = BI_RGB;
	
	while (cinfo->output_scanline < cinfo->output_height)
	{
		(void) jpeg_read_scanlines(cinfo, buffer, 1);
		memcpy(buff+index, buffer[0], row_stride);
		index -= row_stride;
	}

	SetDIBits(NULL, pBmp->operator HBITMAP(), 0, cy, buff, &info, DIB_RGB_COLORS);
	pBmp->SetBitmapDimension(cx, cy);
	delete [] buff;

	(void) jpeg_finish_decompress(cinfo);

	::ReleaseDC(NULL, dc.Detach());
	
	return 1;
}

void CVirtualDisplayDlg::LoadSkin(char* file)
{
	char deb[100];
	int i;
	FILE* f;
	f=fopen(file,"rb");
	if(!f)
	{
		OutputDebugString("Cannot open skin file\n");
		return;
	}

	if(fread(&m_skin_header, sizeof(skin_header_t), 1, f) != 1)
	{
		OutputDebugString("***ERROR :VirtualDisplayDlg: Invalid skin file!\n");
		fclose(f);
		return;
	}

	strcpy(m_skin_header.magic, "VDASH_PSA");
	m_skin_header.version=0.1f;

	if(strcmp(m_skin_header.magic, "VDASH_PSA"))
	{
		OutputDebugString("***ERROR :VirtualDisplayDlg: Invalid skin file!\n");
		fclose(f);
		return ;
	}
	if(m_skin_header.version != 0.1f)
	{
		char deb[50];
		sprintf(deb, "VirtualDisplayDlg: Invalid skin file version %.2f\n", m_skin_header.version);
		OutputDebugString(deb);
		return;
	}

	while(1)
	{
		if(fread(&i, sizeof(int), 1, f) != 1)
			break;
		if(i == 1234321)
			break;
		if(i>=MAX_ENTRIES)
		{
			sprintf(deb, "**ERROR : VirtualDisplayDlg : Invalid index %d\n", i);
			OutputDebugString(deb);
			sprintf(deb, "File index %d\n", ftell(f));
			OutputDebugString(deb);
			break;
		}

		fread(&(tab_elem[i].x), sizeof(int), 1, f);
		fread(&(tab_elem[i].y), sizeof(int), 1, f);
		fread(&(tab_elem[i].font), sizeof(int), 1, f);
		fread(&(tab_elem[i].l), sizeof(int), 1, f);
		fread(&(tab_elem[i].w), sizeof(int), 1, f);
		fread(&(tab_elem[i].option1), sizeof(int), 1, f);
		fread(&(tab_elem[i].option2), sizeof(int), 1, f);
	}

	CSize sz;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, f);

	if(!read_JPEG_file(f, &m_Backbmp, &cinfo))
	{
		OutputDebugString("VirtualDisplayDlg: Decode error\n");
		AfxMessageBox("Error, invalid skin file", MB_ICONERROR);
		EndDialog(-1);
		return;
	}
		

	OutputDebugString("VirtualDisplayDlg: loading big font...\n");
	
	

	for(i=0;i<strlen(BIG_FONT_TXT);i++)
	{
		if(!read_JPEG_file(f, &(big_font[BIG_FONT_TXT[i]]), &cinfo))
		{
			OutputDebugString("VirtualDisplayDlg: Decode error\n");
			AfxMessageBox("Error, invalid skin file", MB_ICONERROR);
			EndDialog(-2);
			return;
		}
	}

	OutputDebugString("VirtualDisplayDlg: Loading medium font...\n");

	for(i=0;i<strlen(MED_FONT_TXT);i++)
	{
		if(!read_JPEG_file(f, &(medium_font[MED_FONT_TXT[i]]), &cinfo))
		{
			OutputDebugString("Decode error\n");
			AfxMessageBox("Error, invalid skin file", MB_ICONERROR);
			EndDialog(-3);
			return;
		}
	}

	OutputDebugString("VirtualDisplayDlg: Loading small font...\n");

	for(i=0;i<strlen(SMALL_FONT_TXT);i++)
	{
		if(!read_JPEG_file(f, &(small_font[SMALL_FONT_TXT[i]]), &cinfo))
		{
			OutputDebugString("Decode error\n");
			AfxMessageBox("Error, invalid skin file", MB_ICONERROR);
			EndDialog(-4);
			return;
		}
	}

	OutputDebugString("VirtualDisplayDlg: Font loaded\n");
	
	OutputDebugString("VirtualDisplayDlg: Loading images...\n");



	for(i=0;i<MAX_ENTRIES;i++)
	{
		if(tab_elem[i].x >= 0)
		{
			if(i<=BOOT_INDEX)
			{
				if(!read_JPEG_file(f, &(tab_elem[i].bmp), &cinfo))
				{
					OutputDebugString("VirtualDisplayDlg: Decode error\n");
					AfxMessageBox("Error, invalid skin file", MB_ICONERROR);
					EndDialog(-5);
					return;
				}
			}
			
		}
	}

	jpeg_destroy_decompress(&cinfo);

	fclose(f);

	sz = m_Backbmp.GetBitmapDimension();
	SetWindowPos(NULL, 0, 0, sz.cx, sz.cy, SWP_NOZORDER | SWP_NOMOVE);
}

BOOL CVirtualDisplayDlg::OnEraseBkgnd(CDC* pDC)
{
	CSize sz;

	sz = m_Backbmp.GetBitmapDimension();

	CBitmap* bmp=m_MemDC.SelectObject(&m_Backbmp);
	pDC->BitBlt(0,0,sz.cx,sz.cy,&m_MemDC,0,0,SRCCOPY);
	m_MemDC.SelectObject(bmp);

	//return CDialog::OnEraseBkgnd(pDC);
	return 1;
}


void CVirtualDisplayDlg::PutText(CDC* pDC, unsigned int index, char* txt)
{
	CBitmap* pfont;
	unsigned int x;
	unsigned int y;
	int l;
	int w;
	int opt1;
	int opt2;
	int font;

	int len;
	unsigned int offset;
	unsigned int i;
	CSize sz;

	x = tab_elem[index].x;
	y = tab_elem[index].y;
	l = tab_elem[index].l;
	w = tab_elem[index].w;
	opt1 = tab_elem[index].option1;
	opt2 = tab_elem[index].option2;
	font = tab_elem[index].font;

	if(strlen(txt) == 0)
		return;

	if(font == SMALL_FONT_SIZE)
		pfont = small_font;
	else if(font == MEDIUM_FONT_SIZE)
		pfont = medium_font;
	else if(font == BIG_FONT_SIZE)
		pfont = big_font;
	else
	{
		OutputDebugString("Invalid font!\n");
		OutputDebugString("Using small font as default\n");
		pfont = small_font;
		//return;
	}

	len = 0;
	for(i=0;i<strlen(txt);i++)
	{
		if(pfont[txt[i]].m_hObject != NULL)
		{
			sz = pfont[txt[i]].GetBitmapDimension();
			len += sz.cx;
		}
	}

	if((len>l) && (l>0))
	{
		OutputDebugString("**** Text too big, cannot display.*****\n");
	}
	else
	{
		if(l != -1)
		{
			if((opt1 == CENTER_ALIGNMENT_OPTION) || (opt2 == CENTER_ALIGNMENT_OPTION))
				offset = x + (l - len)/2;
			else if((opt1 == LEFT_ALIGNMENT_OPTION) || (opt2 == LEFT_ALIGNMENT_OPTION))
				offset = x;
			else
			{
				OutputDebugString("Unknown text alignment!\n");
				//OutputDebugString("Using 'center' alignment as default.\n");
				//offset = x + (l - len)/2;
				OutputDebugString("Using 'left' alignment as default.\n");
				offset = x;
				
				//return;
			}
		}
		else
			offset = x;

		CBitmap* old_bmp = NULL;
		
		old_bmp = m_MemDC.SelectObject(&m_Backbmp);
		pDC->BitBlt(x, y, l, w, &m_MemDC, x, y, SRCCOPY); // erase background

		for(i=0;i<strlen(txt);i++)
		{
			if(pfont[txt[i]].m_hObject != NULL)
			{
				sz = pfont[txt[i]].GetBitmapDimension();
		
				if(txt[i] == ' ')
				{
					m_MemDC.SelectObject(&m_Backbmp);
					pDC->BitBlt(offset, y, sz.cx, sz.cy, &m_MemDC, offset, y, SRCCOPY); // erase background
				}
				else
				{
					m_MemDC.SelectObject(&(pfont[txt[i]]));
					pDC->BitBlt(offset, y, sz.cx, sz.cy, &m_MemDC, 0, 0, SRCCOPY);
				}

				offset += sz.cx;
			}
		}

		if(old_bmp)
			m_MemDC.SelectObject(old_bmp);

	}

}

void CVirtualDisplayDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_Click = true;
	m_pt_Mouse.x = point.x;
	m_pt_Mouse.y = point.y;
	SetCapture();
	ClientToScreen(&point);
	m_OldPoint = point;

	CDialog::OnLButtonDown(nFlags, point);
}

void CVirtualDisplayDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(m_Click) 
	{
		m_Click = false;
		ClientToScreen(&point);
		m_x = point.x - m_pt_Mouse.x;
		m_y = point.y - m_pt_Mouse.y;
		ReleaseCapture();
	}

	CDialog::OnLButtonUp(nFlags, point);
}

void CVirtualDisplayDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_Click)
	{
		ClientToScreen(&point);
		if(point != m_OldPoint)
		{
			SetWindowPos(NULL, point.x - m_pt_Mouse.x, point.y - m_pt_Mouse.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			m_OldPoint = point;
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CVirtualDisplayDlg::DrawBmp(CDC* pDC, int index)
{
	CBitmap* old_bmp = NULL;
	CSize sz;

	if(tab_elem[index].x != -1)
	{
		if(tab_elem[index].bmp.m_hObject != NULL)
		{
			sz = tab_elem[index].bmp.GetBitmapDimension();
			old_bmp = m_MemDC.SelectObject(&(tab_elem[index].bmp));
			pDC->BitBlt(tab_elem[index].x, tab_elem[index].y, sz.cx, sz.cy, &m_MemDC, 0, 0, SRCCOPY);
		}
	}

	if(old_bmp)
		m_MemDC.SelectObject(old_bmp);
}

void CVirtualDisplayDlg::ClearBmp(CDC* pDC, int index)
{
	CBitmap* old_bmp = NULL;
	CSize sz;

	if(tab_elem[index].x != -1)
	{
		if(tab_elem[index].bmp.m_hObject != NULL)
		{
			sz = tab_elem[index].bmp.GetBitmapDimension();
			old_bmp = m_MemDC.SelectObject(&m_Backbmp);
			pDC->BitBlt(tab_elem[index].x, tab_elem[index].y, sz.cx, sz.cy, &m_MemDC, tab_elem[index].x, tab_elem[index].y, SRCCOPY);
		}
	}

	if(old_bmp)
		m_MemDC.SelectObject(old_bmp);
}

int CVirtualDisplayDlg::ProcessMessage(int addr, unsigned int msgType, void* msg)
{
	CDC* pDC;
	char tmp[100];

	pDC = GetDC();

	switch(msgType)
	{
		case ENGINE_MSG:
			{
				ENGINE_MSG_T* evt = (ENGINE_MSG_T*)msg;
				if(evt->engine)
					DrawBmp(pDC, ENGINE_INDEX);
				else
					ClearBmp(pDC, ENGINE_INDEX);

				if(!evt->economy)
					DrawBmp(pDC, BATTERY_INDEX);
				else
					ClearBmp(pDC, BATTERY_INDEX);

				sprintf(tmp, "%02d", evt->ext_temp);
				PutText(pDC, EXT_TEMP_INDEX, tmp);

				sprintf(tmp, "%03d", evt->fuel_level);
				PutText(pDC, FUEL_INDEX, tmp);

				sprintf(tmp, "%03d", evt->oil_level);
				PutText(pDC, OIL_LEVEL_INDEX, tmp);

				sprintf(tmp, "%03d", evt->oil_temp);
				PutText(pDC, OIL_TEMP_INDEX, tmp);

				if(evt->water_temp<127)
					sprintf(tmp, "%03d", evt->water_temp);
				else
					sprintf(tmp, "---");
				PutText(pDC, WATER_TEMP_INDEX, tmp);
			}
			break;

		case DASHBOARD_MSG:
			{
				DASHBOARD_MSG_T* evt = (DASHBOARD_MSG_T*)msg;
				if(evt->rpm<655)
					sprintf(tmp, "%d", evt->rpm);
				else
					sprintf(tmp, "000");
				PutText(pDC, RPM_INDEX, tmp);

				if(evt->speed<655)
					sprintf(tmp, "%d", evt->speed);
				else
					sprintf(tmp, "000");
				PutText(pDC, SPEED_INDEX, tmp);

			}
			break;

		case RADIO_INFOS_MSG:
			{
				RADIO_INFOS_MSG_T* evt = (RADIO_INFOS_MSG_T*)msg;
				
				PutText(pDC, SOURCE_NAME_INDEX, "Radio");
				PutText(pDC, INFO_TXT_INDEX, evt->rds_txt);

			}
			break;

		case CD_INFOS_MSG:
			{
				CD_INFOS_MSG_T* evt = (CD_INFOS_MSG_T*)msg;

				sprintf(tmp, "CD %02d:%02d", evt->total_cd_minutes, evt->total_cd_seconds);
				PutText(pDC, SOURCE_NAME_INDEX, tmp);

				sprintf(tmp, "T %02d-%02d, %02d:%02d", evt->track_num, evt->track_count, evt->minutes, evt->total_cd_seconds);
				PutText(pDC, INFO_TXT_INDEX, tmp);

			}
			break;

		case CAR_STATUS1_MSG:
			{
				CAR_STATUS1_MSG_T* evt = (CAR_STATUS1_MSG_T*)msg;

				sprintf(tmp, "%03d", evt->avg_speed);
				PutText(pDC, AVG_SPEED_INDEX, tmp);

				if(evt->avg_consumption < 25)
					sprintf(tmp, "%02.01f", evt->avg_consumption);
				else
					sprintf(tmp, "---");
				PutText(pDC, AVG_CONSUMPTION_INDEX, tmp);

				if(evt->inst_consumption < 200)
					sprintf(tmp, "%02.01f", evt->inst_consumption);
				else
					sprintf(tmp, "--");
				PutText(pDC, INSTANT_CONSUMPTION_INDEX, tmp);

				sprintf(tmp, "%05d", evt->range);
				PutText(pDC, RANGE_INDEX, tmp);

			}
			break;

		case LIGHTS_STATUS_MSG:
			{
				LIGHTS_STATUS_MSG_T* evt = (LIGHTS_STATUS_MSG_T*)msg;

				if(evt->lights_status & LIGHT_HEAD_LOW)
					DrawBmp(pDC, LOW_LIGHT_INDEX);
				else
					ClearBmp(pDC, LOW_LIGHT_INDEX);

				if(evt->lights_status & LIGHT_HEAD_HIGH)
					DrawBmp(pDC, HIGH_LIGHT_INDEX);
				else
					ClearBmp(pDC, HIGH_LIGHT_INDEX);

				if(evt->lights_status & LIGHT_FOG_REAR)
					DrawBmp(pDC, REAR_FOG_LIGHT_INDEX);
				else
					ClearBmp(pDC, REAR_FOG_LIGHT_INDEX);


			}
			break;
	}

	ReleaseDC(pDC);
	return 0;
}

