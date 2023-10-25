 #include "stdafx.h"
#include "math.h"
#include "VCad.h"
#include "VCadDoc.h"
#include "VCadView.h"
#include "MainFrm.h"
#include "Entity.h"
#include "Command.h"
#include "CreateCmd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL IsImport(Position & pos)
{
	POSITION	POS = g_pDoc->m_EntityList.GetHeadPosition();
	while(POS!=NULL)
	{
		MEntity*	pEntity = (MEntity *) g_pDoc->m_EntityList.GetNext(POS);
		pEntity = pEntity->GetShowEnt();
		if(pEntity == NULL || pEntity->m_nOperationNum ==0)
			continue;

		if(pEntity->GetType() == etBlock) {
			Position pt = ((MBlock*)pEntity)->GetImportPos();
			if(pt.Distance(pos) < 5) {
				pos = pt;
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL IsOutport(Position & pos)
{
	POSITION	POS = g_pDoc->m_EntityList.GetHeadPosition();
	while(POS!=NULL)
	{
		MEntity*	pEntity = (MEntity *) g_pDoc->m_EntityList.GetNext(POS);
		pEntity = pEntity->GetShowEnt();
		if(pEntity == NULL || pEntity->m_nOperationNum ==0)
			continue;

		if(pEntity->GetType() == etBlock) {
			Position pt = ((MBlock*)pEntity)->GetOutportPos();
			if(pt.Distance(pos) < 5) {
				pos = pt;
				return TRUE;
			}
		}
	}

	return FALSE;
}

CCreateConnect::CCreateConnect()
	: m_begin(0,0), m_end(0,0)
{
	m_nStep = 0; // ��ʼ��������Ϊ 0
	m_pPositions = NULL;

	m_bImport = 0;
}

CCreateConnect::~CCreateConnect()
{
	if(m_pPositions) {
		delete [] m_pPositions;
		m_pPositions = NULL;
	}
}

int CCreateConnect::GetType()
{
	return ctCreateConnect; 
}

int	CCreateConnect::OnLButtonDown(UINT nFlags, const Position& pos1) 
{
	Position pos = pos1;
	m_nStep ++; // ÿ�ε���������ʱ�������� 1 

	int i;
	//����㼯
	if(m_nStep > 0) {
		Position * pNew = new Position[m_nStep];
		for( i=0; i<m_nStep-1; i++)
			pNew[i] = m_pPositions[i];
		pNew[i] = pos;
		if(m_nStep > 1 && m_pPositions)
			delete [] m_pPositions;
		m_pPositions = pNew;
	}

	switch(m_nStep) // ���ݲ�����ִ����Ӧ�Ĳ���
	{
		case 1: 
		{
			//����ǿ�������/�����
			if(IsImport(pos) || IsOutport(pos)) {
				if(IsImport(pos))
					m_bImport = 1;
				else
					m_bImport = -1;

				m_begin = m_end = pos;
				::Prompt("�����������ߵ���һ�㣺");
			}
			else
				m_nStep = 0;
			break;
		}
		default:
		{
			CDC*	pDC = g_pView->GetDC(); // �õ��豸����ָ�� 
			// �������϶�״̬ʱ��ʾ�����һ����
			//MLines*	pTempLine = new MLines(m_begin,m_end); 
			//pTempLine->Draw(pDC, dmDrag);
			//delete pTempLine;

			// ����ڰ��������Ĺ�����ͬʱ������Shift����
			//	��ô������굥��λ�û���ˮƽ�߻���������
			if( nFlags & MK_SHIFT ){ 
				double dx = pos.x - m_begin.x;
				double dy = pos.y - m_begin.y;
				if(fabs(dx) <= fabs(dy)) // �����굥��λ����X���򿿽����
					m_end.Set(m_begin.x, pos.y); // ��ô�յ��x������������ͬ
				else
					m_end.Set(pos.x,m_begin.y);
			}
			else
				m_end = pos; // ���δ����Shift��, ���յ�Ϊ��굥��λ��			

			if( (IsImport(pos) && m_bImport == -1) || (IsOutport(pos) && m_bImport == 1) ) {
				MLines*	pTempLine = new MLines(m_begin,m_end); 
				pTempLine->Draw(pDC, dmDrag);
				delete pTempLine;
				
				m_pPositions[m_nStep-1] = pos;
				MConnect*	pNewLine = new MConnect(m_nStep,m_pPositions);// ���������յ㴴��������
				pNewLine->Draw(pDC,dmNormal); // ����������
				g_pDoc->m_EntityList.AddTail(pNewLine); // ��������ָ�����ӵ�ͼԪ����
				g_pDoc->SetModifiedFlag(TRUE);// set modified flag ;
				pNewLine->m_nOperationNum = g_pView->m_nCurrentOperation;
				m_nStep = 0;  // ������������Ϊ 0
				::Prompt("�����������ߵ���㣺");
			}

			m_begin = pos;
			g_pView->ReleaseDC(pDC);
			break;
		}		
	}
	
	//���²����
	if(m_nStep > 0)
		m_pPositions[m_nStep-1] = pos;

	return 0;
}

//������˫����������
int CCreateConnect::OnLButtonDblClk(UINT nFlags, const Position& pos)
{
	CDC*	pDC = g_pView->GetDC(); // �õ��豸����ָ�� 
	
	// �������϶�״̬ʱ��ʾ�����һ����
	if(m_nStep>=2) {
		MLines*	pTempLine = new MLines(m_begin,m_end); 
		pTempLine->Draw(pDC, dmDrag);
		delete pTempLine;
	}
	
	// ����ڰ��������Ĺ�����ͬʱ������Shift����
	//	��ô������굥��λ�û���ˮƽ�߻���������
	if( nFlags & MK_SHIFT ){ 
		double dx = pos.x - m_begin.x;
		double dy = pos.y - m_begin.y;
		if(fabs(dx) <= fabs(dy)) // �����굥��λ����X���򿿽����
			m_end.Set(m_begin.x, pos.y); // ��ô�յ��x������������ͬ
		else
			m_end.Set(pos.x,m_begin.y);
	}
	else
		m_end = pos; // ���δ����Shift��, ���յ�Ϊ��굥��λ��			
	
	if(m_nStep >= 2) {
		//�����Ի���ѡ��
		CString sMsg;
		if(m_bImport == 1)
			sMsg = _T("��������ڵ���");
		else
			sMsg = _T("��������ڵ���");

		if(AfxMessageBox(sMsg, MB_OKCANCEL) == IDOK)
		{
			MConnect*	pNewLine = new MConnect(m_nStep,m_pPositions);// ���������յ㴴��������
			pNewLine->Draw(pDC,dmNormal); // ����������
			g_pDoc->m_EntityList.AddTail(pNewLine); // ��������ָ�����ӵ�ͼԪ����

			double width = 30;
			double height = 30;
			Position p1, p2;
			p1.x = pos.x;
			p1.y = pos.y + height/2;
			p2.x = pos.x + width;
			p2.y = pos.y - height/2;

			MBlock * pBlk = new MBlock(p1, p2, (CBitmap*)NULL);
			pBlk->Draw(pDC,dmNormal); // ���ƿ�
			g_pDoc->m_EntityList.AddTail(pBlk); // ����ָ�����ӵ�ͼԪ����

			g_pDoc->SetModifiedFlag(TRUE);// set modified flag ;
			pNewLine->m_nOperationNum = g_pView->m_nCurrentOperation;			
			pBlk->m_nOperationNum = g_pView->m_nCurrentOperation;			
		}
	}
	
	m_nStep = 0;  // ������������Ϊ 0
	g_pView->ReleaseDC(pDC); // �ͷ��豸����ָ��
	::Prompt("�����������ߵ���㣺");
	
	return 0;
}

int	CCreateConnect::OnMouseMove(UINT nFlags, const Position& pos)
{
	::SetCursor(AfxGetApp()->LoadCursor(IDC_DRAW_CONNECT));

	// ��һ��̬����nPreRefresh��¼����OnMouseMove״̬ʱ��ˢ�´���
	static	int nPreRefresh = g_nRefresh; 
	// ��������bRefresh˵����OnMouseMove�������Ӵ��Ƿ�ˢ��
	BOOL	bRefresh = FALSE; 
	// nCurRefresh���ڼ�¼��ǰ��ˢ�´���
	int		nCurRefresh = g_nRefresh; 
	// ���nCurRefresh��nPreRefresh����ȣ�˵���Ӵ�����ˢ�¹�
	if(nCurRefresh != nPreRefresh){ 
		bRefresh = TRUE;
		nPreRefresh = nCurRefresh; 
	}

	switch(m_nStep)
	{
		case 0:
			::Prompt("�����������ߵ���㣺");
			break;
		default:
		{
			Position	prePos, curPos;
			prePos = m_end; // ���������ڵ�ǰһ��λ��
			
			// ����ڰ��������Ĺ�����ͬʱ������Shift����
			//	��ô������굥��λ�û���ˮƽ�߻���������
			if( nFlags & MK_SHIFT ){
				double dx = pos.x - m_begin.x;
				double dy = pos.y - m_begin.y;
				if(fabs(dx)>=fabs(dy))
					curPos.Set(pos.x,m_begin.y);
				else
					curPos.Set(m_begin.x, pos.y);
			}
			else
				curPos = pos;

			CDC*	pDC = g_pView->GetDC(); // �õ��豸����ָ��
			
			// ������ʱ���������һ����Ƥ��
			MLines*	pTempLine1 = new MLines(m_begin, prePos);
			if(!bRefresh) // ���Ӵ�û�б�ˢ��ʱ���ػ�ԭ������Ƥ��ʹ�䱻����
				pTempLine1->Draw(pDC, dmDrag);
			delete pTempLine1;
			// ������ʱ���󣬸��ݵ�ǰλ�û���һ����Ƥ��
			MLines*	pTempLine2 = new MLines(m_begin, curPos);	
			pTempLine2->Draw(pDC, dmDrag);
			delete pTempLine2;
 
			g_pView->ReleaseDC(pDC); // �ͷ��豸����ָ��			

			m_end = curPos; // ����ǰλ������Ϊ�������յ㣬�Ա���һ������ƶ�ʱ��
			break;
		}
	}
	return 0;
}

// ��������Ҽ�ȡ����ǰ�Ĳ���
int	CCreateConnect::OnRButtonDown(UINT nFlags, const Position& pos) 
{
	// �����ǰ�Ĳ�����Ϊ 1 ����ôҪ�ڽ������β���ǰ�����ϴ�����ƶ�ʱ���Ƶ���Ƥ��
	if(m_nStep == 1){ 
		CDC*	pDC = g_pView->GetDC(); // �õ��豸����ָ��
		Position	prePos = m_end; // ���������ڵ�ǰһ��λ��
		MLines*	pTempLine = new MLines(m_begin, prePos); 
		pTempLine->Draw(pDC, dmDrag); // ������һ�λ��Ƶ���Ƥ��
		delete pTempLine;
		g_pView->ReleaseDC(pDC); // �ͷ��豸����ָ��
	}
	m_nStep = 0; // ������������Ϊ 0 
	::Prompt("�����������ߵ���㣺");
	return 0;
}
// ����Cancel ����ȡ�����β���
int CCreateConnect::Cancel()
{
	// �����ǰ�Ĳ�����Ϊ 1 ����ôҪ�ڽ������β���ǰ�����ϴ�����ƶ�ʱ���Ƶ���Ƥ��
	if(m_nStep == 1){ 
		CDC*	pDC = g_pView->GetDC(); // �õ��豸����ָ��
		Position	prePos = m_end; // ���������ڵ�ǰһ��λ��
		MLines*	pTempLine = new MLines(m_begin, prePos); 
		pTempLine->Draw(pDC, dmDrag); // ������һ�λ��Ƶ���Ƥ��
		delete pTempLine;
		g_pView->ReleaseDC(pDC); // �ͷ��豸����ָ��
	}
	m_nStep = 0; // ������������Ϊ 0 
	::Prompt("����"); // �ȴ���ʾ�����͵��������
	return 0 ;
}
