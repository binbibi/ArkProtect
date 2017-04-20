// ProcessDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ArkProtectApp.h"
#include "ProcessDlg.h"
#include "afxdialogex.h"

#include "ArkProtectAppDlg.h"

// CProcessDlg �Ի���

IMPLEMENT_DYNAMIC(CProcessDlg, CDialogEx)


UINT32 CProcessDlg::m_SortColumn;
BOOL CProcessDlg::m_bSortOrder;

CProcessDlg::CProcessDlg(CWnd* pParent /*=NULL*/, ArkProtect::CGlobal *GlobalObject)
	: CDialogEx(IDD_PROCESS_DIALOG, pParent)
	, m_Global(GlobalObject)
	, m_Process(GlobalObject)
{
	m_SortColumn = 0;
	m_bSortOrder = FALSE;
}

CProcessDlg::~CProcessDlg()
{
}

void CProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROCESS_LIST, m_ProcessListCtrl);
}


BEGIN_MESSAGE_MAP(CProcessDlg, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_PROCESS_LIST, &CProcessDlg::OnLvnColumnclickProcessList)
	ON_NOTIFY(NM_RCLICK, IDC_PROCESS_LIST, &CProcessDlg::OnNMRClickProcessList)
	ON_COMMAND(ID_PROCESS_FRESHEN, &CProcessDlg::OnProcessFreshen)
	ON_COMMAND(ID_PROCESS_MODULE, &CProcessDlg::OnProcessModule)
	ON_WM_SIZE()
	
END_MESSAGE_MAP()


// CProcessDlg ��Ϣ��������


BOOL CProcessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ����Ӷ���ĳ�ʼ��
	// ����Ի���ָ��
	m_Global->ProcessDlg = this;

	// ����Iconͼ���б�
	UINT nIconSize = 20 * (UINT)(m_Global->iDpix / 96.0);
	m_ProcessIconList.Create(nIconSize, nIconSize, ILC_COLOR32 | ILC_MASK, 2, 2);
	ListView_SetImageList(m_ProcessListCtrl.m_hWnd, m_ProcessIconList.GetSafeHandle(), LVSIL_SMALL);

	// ��ʼ�������б�
	APInitializeProcessList();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CProcessDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴�������Ϣ�����������
	m_Global->iResizeX = cx;
	m_Global->iResizeY = cy;
}


void CProcessDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: �ڴ˴�������Ϣ�����������
	if (bShow == TRUE)
	{
		m_ProcessListCtrl.MoveWindow(0, 0, m_Global->iResizeX, m_Global->iResizeY);

		// ���¸�������Ϣ CurrentChildDlg �� ���õ�ǰ�Ӵ��ڵ�button
		((CArkProtectAppDlg*)(m_Global->AppDlg))->m_CurrentChildDlg = ArkProtect::cd_ProcessDialog;
		((CArkProtectAppDlg*)(m_Global->AppDlg))->m_ProcessButton.EnableWindow(FALSE);

		//// ���ؽ�����Ϣ�б�
		//CloseHandle(
		//	CreateThread(NULL, 0,
		//	(LPTHREAD_START_ROUTINE)ArkProtect::CProcessCore::QueryProcessInfoCallback, &m_ProcessListCtrl, 0, NULL)
		//);

		// ���ؽ�����Ϣ�б�
		APLoadProcessList();

		m_ProcessListCtrl.SetFocus();
	}
}


void CProcessDlg::OnLvnColumnclickProcessList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ����ӿؼ�֪ͨ�����������

	m_SortColumn = pNMLV->iSubItem;

	int iItemCount = m_ProcessListCtrl.GetItemCount();
	for (int i = 0; i < iItemCount; i++)
	{
		m_ProcessListCtrl.SetItemData(i, i);	// Set the data of each item to be equal to its index. 
	}

	m_ProcessListCtrl.SortItems((PFNLVCOMPARE)::APProcessListCompareFunc, (DWORD_PTR)&m_ProcessListCtrl);

	if (m_bSortOrder)
	{
		m_bSortOrder = FALSE;
	}
	else
	{
		m_bSortOrder = TRUE;
	}

	/*	for (int i = 0; i < iItemCount; i++)
	{
	if (_wcsnicmp(m_ProcessListCtrl.GetItemText(i, ArkProtect::pc_Company),
	L"Microsoft Corporation",
	wcslen(L"Microsoft Corporation")) == 0)
	{
	m_ProcessList.SetItemData(i, 1);
	}
	}
	*/

	*pResult = 0;
}


void CProcessDlg::OnNMRClickProcessList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ����ӿؼ�֪ͨ�����������

	CMenu Menu;
	Menu.LoadMenuW(IDR_PROCESS_MENU);
	CMenu* SubMenu = Menu.GetSubMenu(0);	// �Ӳ˵�

	CPoint Pt;
	GetCursorPos(&Pt);         // �õ����λ��

	int	iCount = SubMenu->GetMenuItemCount();

	// ���û��ѡ��,����ˢ�� ����ȫ��Disable
	if (m_ProcessListCtrl.GetSelectedCount() == 0)
	{
		for (int i = 0; i < iCount; i++)
		{
			SubMenu->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED); //�˵�ȫ�����
		}

		SubMenu->EnableMenuItem(ID_PROCESS_FRESHEN, MF_BYCOMMAND | MF_ENABLED);
	}

	SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Pt.x, Pt.y, this);

	*pResult = 0;
}


void CProcessDlg::OnProcessFreshen()
{
	// TODO: �ڴ�����������������
	// ���ؽ�����Ϣ�б�
	APLoadProcessList();
}



void CProcessDlg::OnProcessModule()
{
	// TODO: �ڴ�����������������

	// ��ʼ��ProcessInfoDlg������
	APInitializeProcessInfoDlg(ArkProtect::pik_Module);

}


/************************************************************************
*  Name : APInitializeProcessList
*  Param: void
*  Ret  : void
*  ��ʼ��ListControl
************************************************************************/
void CProcessDlg::APInitializeProcessList()
{
	m_Process.InitializeProcessList(&m_ProcessListCtrl);
}


/************************************************************************
*  Name : APLoadProcessList
*  Param: void
*  Ret  : void
*  ���ؽ�����Ϣ��ListControl
************************************************************************/
void CProcessDlg::APLoadProcessList()
{
	if (m_Global->m_bIsRequestNow == TRUE)
	{
		return;
	}

	while (m_ProcessIconList.Remove(0));

	m_ProcessListCtrl.DeleteAllItems();

	m_ProcessListCtrl.SetSelectedColumn(-1);

	// ���ؽ�����Ϣ�б�
	CloseHandle(
		CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)ArkProtect::CProcessCore::QueryProcessInfoCallback, &m_ProcessListCtrl, 0, NULL)
	);
}


/************************************************************************
*  Name : APInitializeProcessInfoDlg
*  Param: ProcessInfoKind            ��Ҫ�򿪵Ľ�����Ϣ����
*  Ret  : void
*  ����ProcessEntry�������µ��ӶԻ���������ʾĿ�������Ϣ
************************************************************************/
void CProcessDlg::APInitializeProcessInfoDlg(ArkProtect::eProcessInfoKind ProcessInfoKind)
{
	POSITION Pos = m_ProcessListCtrl.GetFirstSelectedItemPosition();

	while (Pos)
	{
		int iItem = m_ProcessListCtrl.GetNextSelectedItem(Pos);

		UINT32 ProcessId = _ttoi(m_ProcessListCtrl.GetItemText(iItem, ArkProtect::pc_ProcessId).GetBuffer());
		UINT32 ParentProcessId = _ttoi(m_ProcessListCtrl.GetItemText(iItem, ArkProtect::pc_ParentProcessId).GetBuffer());
		CString strEProcess = m_ProcessListCtrl.GetItemText(iItem, ArkProtect::pc_EProcess);

		UINT_PTR EProcess = 0;
		strEProcess = strEProcess.GetBuffer() + 2;	 // �ƹ�0x
		swscanf_s(strEProcess.GetBuffer(), L"%p", &EProcess);

		ArkProtect::PROCESS_ENTRY_INFORMATION ProcessEntry = { 0 };

		ProcessEntry.ProcessId = ProcessId;
		ProcessEntry.ParentProcessId = ParentProcessId;
		ProcessEntry.EProcess = EProcess;

		if (_wcsnicmp(m_ProcessListCtrl.GetItemText(iItem, ArkProtect::pc_UserAccess).GetBuffer(), L"�ܾ�", wcslen(L"�ܾ�")) == 0)
		{
			ProcessEntry.bUserAccess = FALSE;
		}
		else
		{
			ProcessEntry.bUserAccess = TRUE;
		}

		StringCchCopyW(ProcessEntry.wzCompanyName,
			m_ProcessListCtrl.GetItemText(iItem, ArkProtect::pc_Company).GetLength() + 1,
			m_ProcessListCtrl.GetItemText(iItem, ArkProtect::pc_Company).GetBuffer());
		StringCchCopyW(ProcessEntry.wzImageName,
			m_ProcessListCtrl.GetItemText(iItem, ArkProtect::pc_ImageName).GetLength() + 1,
			m_ProcessListCtrl.GetItemText(iItem, ArkProtect::pc_ImageName).GetBuffer());
		StringCchCopyW(ProcessEntry.wzFilePath,
			m_ProcessListCtrl.GetItemText(iItem, ArkProtect::pc_FilePath).GetLength() + 1,
			m_ProcessListCtrl.GetItemText(iItem, ArkProtect::pc_FilePath).GetBuffer());

		CProcessInfoDlg *ProcessViewDlg = new CProcessInfoDlg(this, ProcessInfoKind, m_Global, &ProcessEntry);
		ProcessViewDlg->DoModal();
	}
}





/************************************************************************
*  Name : AddProcessFileIcon
*  Param: wzProcessPath
*  Ret  : void
*  ��ѯ������Ϣ�Ļص�
************************************************************************/
void CProcessDlg::AddProcessFileIcon(WCHAR *wzProcessPath)
{
	SHFILEINFO ShFileInfo = { 0 };

	SHGetFileInfo(wzProcessPath, FILE_ATTRIBUTE_NORMAL,
		&ShFileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);

	HICON  hIcon = ShFileInfo.hIcon;

	m_ProcessIconList.Add(hIcon);
}


/************************************************************************
*  Name : APProcessListCompareFunc
*  Param: lParam1                   ��һ��
*  Param: lParam2                   �ڶ���
*  Param: lParamSort                ���Ӳ�����ListControl��
*  Ret  : void
*  ��ѯ������Ϣ�Ļص�
************************************************************************/
int CALLBACK APProcessListCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// �Ӳ�������ȡ��Ҫ�Ƚϵ���������

	int iRow1 = (int)lParam1;
	int iRow2 = (int)lParam2;

	CListCtrl* ListCtrl = (CListCtrl*)lParamSort;

	CString str1 = ListCtrl->GetItemText(iRow1, CProcessDlg::m_SortColumn);
	CString str2 = ListCtrl->GetItemText(iRow2, CProcessDlg::m_SortColumn);

	if (CProcessDlg::m_SortColumn == ArkProtect::pc_ProcessId ||
		CProcessDlg::m_SortColumn == ArkProtect::pc_ParentProcessId)
	{
		// int�ͱȽ�
		if (CProcessDlg::m_bSortOrder)
		{
			return _ttoi(str1) - _ttoi(str2);
		}
		else
		{
			return _ttoi(str2) - _ttoi(str1);
		}
	}
	else if (CProcessDlg::m_SortColumn == ArkProtect::pc_EProcess)
	{
		UINT_PTR p1 = 0, p2 = 0;

		str1 = str1.GetBuffer() + 2;	// ��0x
		str2 = str2.GetBuffer() + 2;

		swscanf_s(str1.GetBuffer(), L"%P", &p1);
		swscanf_s(str2.GetBuffer(), L"%P", &p2);

		if (CProcessDlg::m_bSortOrder)
		{
			return (int)(p1 - p2);
		}
		else
		{
			return (int)(p2 - p1);
		}
	}
	else
	{
		// �����ͱȽ�
		if (CProcessDlg::m_bSortOrder)
		{
			return str1.CompareNoCase(str2);
		}
		else
		{
			return str2.CompareNoCase(str1);
		}
	}

	return 0;
}















