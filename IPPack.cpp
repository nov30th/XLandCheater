/////////////////////////////////////////////
// IPPack.cpp�ļ�


#include "IPPack.h"
#include "resource.h"
#include "EnumProcessDlg.h"


#include "IPPackLib.h"
#include "ShareMemory.h"

#pragma comment(lib, "WS2_32")
CMyApp theApp;

// dll��������
BOOL SetHook(BOOL bInstall, DWORD dwThreadId = 0, HWND hWndCaller = NULL)
{
	// ���嵼������������
	typedef (WINAPI *PFNSETHOOK)(BOOL, DWORD, HWND);
	// ����������DLL�ļ���������ʱ������Ϊ../IPPackLib/debug/IPPackLib.dll
	char szDll[] = "HOHOLib.dll";

	// ����DLLģ��
	BOOL bNeedFree = FALSE;
	HMODULE hModule = ::GetModuleHandle(szDll);
	if(hModule == NULL)
	{
		hModule = ::LoadLibrary(szDll);
		bNeedFree = TRUE;
	}
	

	// ��ȡSetHook�����ĵ�ַ
	PFNSETHOOK mSetHook = (PFNSETHOOK)::GetProcAddress(hModule, "SetHook");
	if(mSetHook == NULL) // �ļ�����ȷ?
	{
		if(bNeedFree)
			::FreeLibrary(hModule);
		return FALSE;
	}

	// ����SetHook����
	BOOL bRet = mSetHook(bInstall, dwThreadId, hWndCaller);

	// ���ж�أ��ͷ�������ص�ģ��
	if(!bInstall)
		::FreeLibrary(hModule);

	return bRet;
}

BOOL CMyApp::InitInstance()
{
	CMainDialog dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;	// ����FALSE��ֹ���������Ϣѭ��
}

CMainDialog::CMainDialog(CWnd* pParentWnd):CDialog(IDD_MAINDIALOG, pParentWnd)
{
}

BEGIN_MESSAGE_MAP(CMainDialog, CDialog)
// ��ť�ĵ����¼�
ON_BN_CLICKED(IDC_TARGET, OnTarget)
ON_BN_CLICKED(IDC_PAUSE, OnPause)
ON_BN_CLICKED(IDC_CLEAR, OnClear)
ON_BN_CLICKED(IDC_TOPMOST, OnTopMost)
ON_BN_CLICKED(IDC_CLOSE, OnClose)
// �б���ͼ�ĵ�����ɾ���¼�
ON_NOTIFY(NM_CLICK, IDC_LISTDATA, OnClickListData)
ON_NOTIFY(LVN_DELETEITEM, IDC_LISTDATA, OnDeleteItemList)  
// �����Զ�����Ϣ
ON_MESSAGE(HM_SEND, OnSend)
END_MESSAGE_MAP()

BOOL CMainDialog::OnInitDialog()
{
	// �ø�������ڲ���ʼ��
	CDialog::OnInitDialog();

	// ����ͼ��
	SetIcon(theApp.LoadIcon(IDI_MAIN), FALSE);

	// ����״̬���������������ԣ�CStatusBarCtrl���װ�˶�״̬���ؼ��Ĳ�����
	m_bar.Create(WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP, CRect(0, 0, 0, 0), this, 101);
	m_bar.SetBkColor(RGB(0xa6, 0xca, 0xf0));		// ����ɫ
	int arWidth[] = { 150, -1 };
	m_bar.SetParts(2, arWidth);				// ����
	m_bar.SetText(" HOHO`` �ξ��·�����װ��", 1, 0);	// �ڶ��������ı�

	// ȡ���б���ͼ���ڵĿ���Ȩ���������ķ���
	m_listData.SubclassWindow(::GetDlgItem(m_hWnd, IDC_LISTDATA));
	m_listData.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listData.InsertColumn(0, "���", LVCFMT_LEFT, 38);
	m_listData.InsertColumn(2, "����", LVCFMT_LEFT, 80);
	m_listData.InsertColumn(3, "����", LVCFMT_LEFT, 180);

	// ע���ȼ�CTRL��F9�����û������ȼ�ʱ����������ʾ��������
	if(!::RegisterHotKey(m_hWnd, 11, MOD_CONTROL, VK_F9))
	{
		MessageBox("ע���ȼ�CTRL��F9ʧ�ܣ�");
	}

	this->SetDlgItemText(IDC_EDITINFO,"   HOHO``�ξ��·�ϵͳ\r\n  �����ӹ���:�ڶԷ��ݳ���ʱ��,�����Ҫ�Է���̨,ֻ�迪���Ի����촰��,����HO������Ӣ�ĵ��ּ���.\r\n  ʹ�÷���,���������¼��Ϸ,Ȼ�󰴣�Ŀ�꣢��ť,�ҵ����������,����IE��iexplore.exe,��maxthon����Maxthon.exe.�ҵ�����֮��ѡ��,��ȷ��.\r\n  Ȼ����Ϳ�������ߵ��·������������·��ı��(��Ҫ���Լ�����),Ȼ������Ϸ���ѹ��㵱ǰ���·�,�㱣�漴��.ע��!!!�·�������㱣�����ʹ�����е��·���ʧ!\r\n  ����,����ʹ�װ���㱣��,��û��Ч����.����취��:��Ҫ�����·�����(���㱣��),Ȼ���ٵ�ȡ��,��󰴱��漴��.\r\n  �ó���ֻ���о���;,���Ƿ�ʹ��,һ�к���Ը�.");


	// ��ʼ��״̬����
	InitData();
	// �����û�����
	UIControl();

	return TRUE;
}

void CMainDialog::InitData()
{
	m_bOpen = FALSE;	// û�д�
	m_bPause = FALSE;	// û����ͣ
}

void CMainDialog::UIControl()
{
	if(m_bOpen)
	{
		GetDlgItem(IDC_PAUSE)->EnableWindow(TRUE);
		GetDlgItem(IDC_CLOSE)->EnableWindow(TRUE);

		if(!m_bPause)
			m_bar.SetText(" �ȴ�����...", 0, 0);
		else
			m_bar.SetText(" ��ͣ", 0, 0);
	}
	else
	{
		GetDlgItem(IDC_PAUSE)->EnableWindow(FALSE);
		GetDlgItem(IDC_CLOSE)->EnableWindow(FALSE);

		m_bar.SetText(" ��ѡ����Ϸ����", 0, 0);
	}

	if(m_bPause)
	{
		GetDlgItem(IDC_PAUSE)->SetWindowText("�ָ�");
	}
	else
	{
		GetDlgItem(IDC_PAUSE)->SetWindowText("��ͣ");
	}
}


void CMainDialog::OnTarget()
{
	CEnumProcessDlg dlg(this);
	// ����ѡ����̶Ի���
	if(dlg.DoModal() == IDOK)
	{
		// ���Ϊ�������̰�װ�˹��ӣ���ж��
		if(m_bOpen)	
		{
			SetHook(FALSE);
			InitData();
		}
		// Ϊ�û�ѡ����̵����̰߳�װ����
		if(SetHook(TRUE, dlg.m_dwThreadId, m_hWnd))
		{
			m_bOpen = TRUE;
			m_bPause = FALSE;
		}
		else
		{
			MessageBox(" ȱ��HOHOLib.Dll�ļ�.");
		}
	}
	// �����û�����
	UIControl();
}

long CMainDialog::OnSend(WPARAM wParam, LPARAM lParam)
{
	CShareMemory sm("IPPACK_SEND");
	CMessageData *pData = (CMessageData*)sm.GetBuffer();
	
	if(m_bPause)
		return 0;
	if (*pData->data()=='C' && *(pData->data() + 1)=='S' && *(pData->data() + 2) == 0x0a)
	{
		CString strNum[9];
		GetDlgItemText(IDC_EDITMZ,strNum[0]);
		GetDlgItemText(IDC_EDITSY,strNum[1]);
		GetDlgItemText(IDC_EDITKQ,strNum[2]);
		GetDlgItemText(IDC_EDITXZ,strNum[3]);
		GetDlgItemText(IDC_EDITSS,strNum[4]);
		GetDlgItemText(IDC_EDITTZ,strNum[5]);
		GetDlgItemText(IDC_EDITRW,strNum[7]);
		GetDlgItemText(IDC_EDITWP,strNum[8]);
		strNum[6] = "\012" + strNum[0] + "|" + strNum[1] + "|" +  strNum[2] + "|" +  strNum[3] + "|" +  strNum[4] + "|" + strNum[7] + "|" +  strNum[5] + "|" + strNum[8] + "|";
		
		int iStart = 0,iEnd = 0, iTemp = 1, iCount = 0;
		char buff[200];
		iStart = strstr(pData->data()+3,"\x0a") - pData->data();
		*(pData->data() + iStart) = '\0';
		strcpy(buff,pData->data());
		
		for(iTemp = iStart; iTemp < (pData->nDataLength); iTemp++)
		{
			if (*(pData->data() + iTemp) == '|')
			{
				iEnd = iTemp;
				if (++iCount == 8)
					break;
			}
		}
		
		*(pData->data() + iEnd) = '\0';
		strcat(buff,strNum[6].GetBuffer(0));
		strNum[6].ReleaseBuffer();
		strcat(buff,pData->data() + iEnd + 1);
		memcpy(pData->data(),buff,strlen(buff) + 1);
		pData->nDataLength = strlen(buff) + 1;
		m_bar.SetText(" �������...", 0, 0);
	}
	else if (*pData->data() == 0x50 && *(pData->data() + 1) == 0x4d && *(pData->data() + 2) == 0x0a)
	{
		char personChar[8];
		char spear[2] = {"\012"};
		int numStart = strstr(pData->data() + 4,spear) - pData->data();
		int numEnd = strstr(pData->data() + numStart + 1, spear) - pData->data();
		*(pData->data() + numEnd) = '\0';
		strcpy(personChar,pData->data() + numStart + 1);
		if (*(pData->data() + 1 + numEnd) == 'H' && *(pData->data() + 2 + numEnd) == 'O')
		{
			//��ʼת������������
			strcpy(pData->data(),"PL\012");
			strcat(pData->data(),personChar);
			strcat(pData->data(),"\012stageMC\012vote\012-99999");
			pData->nDataLength = strlen(pData->data()) + 1;
			m_bar.SetText(" ��̨���...", 0, 0);
		}
	}
	/*	
	BOOL bNoSend = IsDlgButtonChecked(IDC_CLOSESEND);

	if(!bNoSend)
	{
	int nIndex = m_listData.GetItemCount();
	if(nIndex == 100)
	return 0; 
	
	  // �������
	  char sz[32] = "";
	  itoa(nIndex+1, sz, 10);
	  m_listData.InsertItem(nIndex, sz, 0);
	  
		// ���������ı�
		char szText[128] = "";
		int nCopy = min(pData->nDataLength, 127);
		strncpy(szText, pData->data(), nCopy);
		m_listData.SetItemText(nIndex, 1, "����");
		m_listData.SetItemText(nIndex, 2, szText);
		
		  
			// ���湲���ڴ��е����ݵ����̶��У��Ա��û���ѯ
			int nTotal = sizeof(CMessageData) + pData->nDataLength;
			nTotal = min(nTotal, 500);
			BYTE* pByte = new BYTE[nTotal];
			memcpy(pByte, pData, nTotal);
			m_listData.SetItemData(nIndex, (DWORD)pByte);
			}
	*/
	return 0;
}

void CMainDialog::OnClickListData(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// pNMListView->iItem��ֵΪ�û�������Ŀ������
	int nIndex = pNMListView->iItem;	
	if (nIndex >= 0)
	{
			// ��16���Ƶ���ʽ��ʾ���ݵ��༭��
		CMessageData* pData = (CMessageData*)m_listData.GetItemData(nIndex);

		// Ϊ�ַ�������ռ�
		// һ���ֽ�Ҫռ��3���ַ���2��16��������1���ո񣩣��硰16 12 b2 c7��������Ҫ����3
		char* pBuf = new char[pData->nDataLength*3 + 1];

		// ת���ػ������Ϊ16�����ַ�������ʽ
		char* pTemp = pBuf;
		char* psz = pData->data();
		for(int i=0; i<pData->nDataLength; i++, psz++)
		{
			wsprintf(pTemp, "%02x ", (BYTE)(*psz));
			pTemp += 3;
		}
		*pTemp = '\0';
	
		// ��ʾ���ݵ��༭����
		GetDlgItem(IDC_SPECIFICDATA)->SetWindowText(pBuf);

		// �ͷ��ڴ�ռ�
		delete[] pBuf;
	}

	*pResult = 0;
}

void CMainDialog::OnClear()
{
	// ɾ���б���ͼ�е��������ձ༭��
	m_listData.DeleteAllItems();
	GetDlgItem(IDC_SPECIFICDATA)->SetWindowText("");
}

void CMainDialog::OnDeleteItemList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// ȡ��Ҫɾ����Ŀ�Ĺ������ݣ�Ҳ����������Ĵ���
	// 	int nIndex = pNMListView->iItem;
	//	pByte = (PBYTE)m_listData.GetItemData(nIndex);
	BYTE* pByte = (PBYTE)pNMListView->lParam;

	// �ͷ��ڴ�ռ�
	delete[] pByte;

	*pResult = 0;
}


void CMainDialog::OnCancel()
{
	// ȷ�����ӱ�ж��
	if(m_bOpen)
		SetHook(FALSE);
	CDialog::OnCancel();
}

void CMainDialog::OnClose()	
{
	// �û������رհ�ťʱ������������
	// ��������״̬
	if(m_bOpen)
	{
		SetHook(FALSE);
		InitData();
		OnClear();
	}
	UIControl();
}

void CMainDialog::OnPause()
{
	m_bPause = !m_bPause;
	UIControl();
}

void CMainDialog::OnTopMost()
{
	BOOL bTopMost = ::IsDlgButtonChecked(m_hWnd, IDC_TOPMOST);
	if(bTopMost)
	{
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, 
					SWP_NOMOVE|SWP_NOSIZE|SWP_NOREDRAW);
	}
	else
	{
		::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, 
					SWP_NOMOVE|SWP_NOSIZE|SWP_NOREDRAW);
	}
}