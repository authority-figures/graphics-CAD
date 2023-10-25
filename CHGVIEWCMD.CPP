 #include "stdafx.h"
#include "math.h"

#include "Entity.h"

#include "VCad.h"
#include "VCadDoc.h"
#include "VCadView.h"
#include "MainFrm.h"

#include "Command.h"
#include "ChgViewCmd.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////
/*	
 *	CLASS	CViewPanCmd
 */
CViewPanCmd::CViewPanCmd()
{
	m_nStep = 0; // ��ʼ��������Ϊ 0
}

CViewPanCmd::~CViewPanCmd()
{
}

int	CViewPanCmd::GetType()
{
	return ctViewPan;
}

int	CViewPanCmd::OnLButtonDown(UINT nFlags, const Position& pos) 
{
	m_nStep ++ ;
	
	CRect rc ;
	g_pView->GetClientRect(&rc) ;

	switch(m_nStep)
	{
		case 1:
			m_basePos = m_desPos = pos;
			::Prompt("������ƽ�ƻ����Ŀ��㣺") ;
			break;
		case 2:
		{
			m_desPos = pos;
			double dx = m_basePos.x - m_desPos.x;
			double dy = m_basePos.y - m_desPos.y ;

			g_pView->m_dOrgX += dx ;
			g_pView->m_dOrgY += dy ;
			g_pView->Invalidate() ;	
			
//			g_pDoc->SetModifiedFlag(TRUE);

			//	���ò���Ϊ0
			m_nStep = 0;
			break;
		}
		default:
			break;
	}
	return 0;
}

int	CViewPanCmd::OnMouseMove(UINT nFlags, const Position& pos)
{
	switch(m_nStep)
	{
		case 1:
		{
			CRect	rect;
			g_pView->GetClientRect(&rect); 
				
			Position	prePos, curPos;
			prePos = m_basePos ;//
			curPos = pos ;
			//����λ�Ʋ�
			Position dp = prePos - curPos ;
			//����ƽ�ƺ����������ԭ��
			g_pView->m_dOrgX += dp.x ;
			g_pView->m_dOrgY += dp.y ;
			//����ÿ�ƶ�һ�Σ�����ԭ�㶼Ҫ�ı䣬
			//���ԣ���ʼ���λ��Ҳ�����仯
			m_basePos.x = pos.x + dp.x;
			m_basePos.y = pos.y + dp.y ;
			
			g_pView->Invalidate(TRUE) ;
			break;
		}
		default:
			break;
	}
	return 0;
}

// Click right button to cancel the operation
int	CViewPanCmd::OnRButtonDown(UINT nFlags, const Position& pos) 
{
	m_nStep = 0;
	return 0;
}

int CViewPanCmd::Cancel()
{
	return 0 ;
}
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/*	
 *	CLASS	CZoomRgnCmd
 */
CZoomRgnCmd::CZoomRgnCmd()
{
	m_nStep = 0; // ��ʼ��������Ϊ 0
}

CZoomRgnCmd::~CZoomRgnCmd()
{
}

int CZoomRgnCmd::GetType()
{
	return ctViewZoomRgn;
}

int	CZoomRgnCmd::OnLButtonDown(UINT nFlags, const Position& pos) 
{
	m_nStep ++;
	switch(m_nStep)
	{
		case 1:
		{
			m_basePos = m_desPos = pos;
			Prompt("���������ž��ε����½ǵ㣺") ;
			break;
		}
		case 2:
		{
			m_desPos = pos;
			// ��õ�ǰ���豸����
			CDC*	pDC = g_pView->GetDC(); 
			CRect rc ;
			g_pView->GetClientRect(&rc) ;
			// ת��Ϊ�߼�����ϵ
			pDC->DPtoLP(&rc) ;
			// ����������ϵ��ԭ��
			g_pView->m_dOrgX = (m_basePos.x + m_desPos.x)/2. ;
			g_pView->m_dOrgY = -(m_basePos.y + m_desPos.y)/2. ;
			// ����������ϵ���豸����ı�������
			double scalex = fabs((m_basePos.x - m_desPos.x)/
				(rc.right-rc.left)) ;
			double scaley = fabs((m_basePos.y - m_desPos.y)/
				(rc.bottom-rc.top )) ;
			if(scalex>scaley)
				g_pView->scale = scalex ;
			else
				g_pView->scale = scaley ;
			// ���ñ������ӵ�ȡֵ��Χ
			if(fabs( float(g_pView->scale) )<1e-6||
				fabs( float(g_pView->scale))>1e+6 )
				g_pView->scale = 1. ;
			
			g_pDoc->UpdateAllViews(NULL) ;
			// �����޸ı��
//			g_pDoc->SetModifiedFlag(TRUE);
			// �ͷŵ�ǰ���豸����
			g_pView->ReleaseDC(pDC); 
			
			m_nStep = 0;
			break;
		}
		default:
			m_nStep = 0;
			break;		
	}
	return 0;
}

int	CZoomRgnCmd::OnMouseMove(UINT nFlags, const Position& pos)
{
	switch(m_nStep)
	{
		case 1:
		{
			Position	prePos, curPos;
			prePos = m_desPos;
			curPos = pos;
			// ��õ�ǰ���豸����
			CDC*	pDC = g_pView->GetDC(); 
			CPoint prep, curp, bp ;
			// ת������Ļ�����
			g_pView->WorldtoScreen(prePos, prep) ;
			g_pView->WorldtoScreen(curPos, curp) ;
			g_pView->WorldtoScreen(m_basePos, bp) ;
			
			pDC->SelectStockObject(NULL_BRUSH) ;
			int ndrawmode = pDC->SetROP2(R2_NOT) ;
			// ���Ʊ��Ŵ�����
			pDC->Rectangle(bp.x, bp.y, prep.x, prep.y) ;
			pDC->Rectangle(bp.x, bp.y, curp.x, curp.y) ;
			pDC->SetROP2(ndrawmode) ;
			// �ͷŵ�ǰ���豸����
			g_pView->ReleaseDC(pDC); 
			m_desPos = pos;
			break;
		}
	}
	return 0;
}

int	CZoomRgnCmd::OnRButtonDown(UINT nFlags, const Position& pos) 
{

	::ClipCursor(NULL);

	m_nStep = 0;
	return 0;
}

int CZoomRgnCmd::Cancel()
{
	return 0 ;
}