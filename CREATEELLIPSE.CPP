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

CCreateEllipse::CCreateEllipse()
	: m_LeftTop(0,0), m_RightBottum(0,0)
{
	m_nStep = 0; // ��ʼ��������Ϊ 0	
}

CCreateEllipse::~CCreateEllipse()
{
}

int CCreateEllipse::GetType()
{
	return ctCreateEllipse; 
}

int	CCreateEllipse::OnLButtonDown(UINT nFlags, const Position& pos) 
{
	m_nStep ++; // ÿ�ε���������ʱ�������� 1 
	switch(m_nStep) // ���ݲ�����ִ����Ӧ�Ĳ���
	{
		case 1: 
		{
			m_LeftTop = m_RightBottum = pos;
			::Prompt("��������Բ�����µ㣺");
			break;
		}
		case 2:
		{
			CDC*	pDC = g_pView->GetDC(); // �õ��豸����ָ�� 

			// �������϶�״̬ʱ��ʾ����Ƥ��
			MLines*		pTempLine = new MLines(m_LeftTop, m_RightBottum);
			MEllipse*	pTempCircle = new MEllipse(m_LeftTop,m_RightBottum); 
			pTempLine->Draw(pDC, dmDrag);
			pTempCircle->Draw(pDC, dmDrag);
			delete pTempLine;
			delete pTempCircle;

			m_RightBottum = pos;
						
			MEllipse*	pNewCircle = new MEllipse(m_LeftTop,m_RightBottum);
			pNewCircle->Draw(pDC,dmNormal); 
			g_pDoc->m_EntityList.AddTail(pNewCircle); 
			g_pDoc->SetModifiedFlag(TRUE);// set modified flag ;
			pNewCircle->m_nOperationNum = g_pView->m_nCurrentOperation;
			g_pView->ReleaseDC(pDC); // �ͷ��豸����ָ��
			
			m_nStep = 0;  // ������������Ϊ 0
			::Prompt("��������Բ�����ϵ㣺") ;
			break;
		}
		
	}
	return 0;
}

int	CCreateEllipse::OnMouseMove(UINT nFlags, const Position& pos)
{
	::SetCursor(AfxGetApp()->LoadCursor(IDC_DRAW_ELLIPSE));

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
			::Prompt("��������Բ�����ϵ㣺") ;
			break;
		case 1:
		{
			Position	prePos, curPos;
			prePos = m_RightBottum; // ���������ڵ�ǰһ��λ��
		
			curPos = pos;

			CDC*	pDC = g_pView->GetDC(); // �õ��豸����ָ��
			
			// ������ʱ���������һ�ε���Ƥ��
			MLines*		pTempLine1 = new MLines(m_LeftTop, prePos);
			MEllipse*	pTempCircle1 = new MEllipse(m_LeftTop, prePos);
			if(!bRefresh){ // ���Ӵ�û�б�ˢ��ʱ���ػ�ԭ������Ƥ��ʹ�䱻����
				pTempLine1->Draw(pDC, dmDrag);
				pTempCircle1->Draw(pDC, dmDrag);
			}
			delete pTempCircle1;
			delete pTempLine1;
			// ������ʱ���󣬸��ݵ�ǰλ�û�����Ƥ��
			MLines*		pTempLine2 = new MLines(m_LeftTop, curPos);
			MEllipse*	pTempCircle2 = new MEllipse(m_LeftTop, curPos);
			pTempLine2->Draw(pDC, dmDrag);
			pTempCircle2->Draw(pDC, dmDrag);
			delete pTempCircle2;
			delete pTempLine2;
 
			g_pView->ReleaseDC(pDC); // �ͷ��豸����ָ��			

			m_RightBottum = curPos; // ����ǰλ������Ϊֱ���յ㣬�Ա���һ������ƶ�ʱ��
			break;
		}
	}
	return 0;
}
// ��������Ҽ�ȡ����ǰ�Ĳ���
int	CCreateEllipse::OnRButtonDown(UINT nFlags, const Position& pos) 
{
	// �����ǰ�Ĳ�����Ϊ 1 ����ôҪ�ڽ������β���ǰ�����ϴ�����ƶ�ʱ���Ƶ���Ƥ��
	if(m_nStep == 1){ 
		CDC*	pDC = g_pView->GetDC(); // �õ��豸����ָ��
		Position	prePos = m_RightBottum; // ���������ڵ�ǰһ��λ��
		MLines*		pTempLine = new MLines(m_LeftTop, prePos);
		MEllipse*	pTempCircle = new MEllipse(m_LeftTop, prePos); 
		pTempLine->Draw(pDC, dmDrag); // ������һ�λ��Ƶ���Ƥ��
		pTempCircle->Draw(pDC, dmDrag); 
		delete pTempCircle;
		g_pView->ReleaseDC(pDC); // �ͷ��豸����ָ��
	}
	m_nStep = 0; // ������������Ϊ 0 
	::Prompt("��������Բ�����ϵ㣺") ;
	return 0;
}
// ����Cancel ����ȡ�����β���
int CCreateEllipse::Cancel()
{
	// �����ǰ�Ĳ�����Ϊ 1 ����ôҪ�ڽ������β���ǰ�����ϴ�����ƶ�ʱ���Ƶ���Ƥ��
	if(m_nStep == 1){ 
		CDC*	pDC = g_pView->GetDC(); // �õ��豸����ָ��
		Position	prePos = m_RightBottum; // ���������ڵ�ǰһ��λ��
		MLines*		pTempLine = new MLines(m_LeftTop, prePos);
		MEllipse*	pTempCircle = new MEllipse(m_LeftTop, prePos); 
		pTempLine->Draw(pDC, dmDrag); // ������һ�λ��Ƶ���Ƥ��
		pTempCircle->Draw(pDC, dmDrag); 
		delete pTempCircle;
		g_pView->ReleaseDC(pDC); // �ͷ��豸����ָ��
	}
	m_nStep = 0; // ������������Ϊ 0 
	::Prompt("����"); // �ȴ���ʾ�����͵��������
	return 0 ;
}
