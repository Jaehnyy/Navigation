// ChildView.cpp : CChildView 클래스의 구현
//

#include "pch.h"
#include "framework.h"

#ifndef SHARED_HANDLERS
#include "Navigation.h"
#endif

#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CChildView, CView)

BEGIN_MESSAGE_MAP(CChildView, CView)
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()




CChildView::CChildView() noexcept
{
    m_selectedNodeIndex = -1;
    m_dijkstraStartIdx = -1;
}

CChildView::~CChildView()
{
}




void CChildView::ResizeMatrix()
{
    // 행렬 크기 늘리기
    int nSize = (int)m_vertexList.size();
    m_graphMatrix.resize(nSize);

    for (int i = 0; i < nSize; ++i)
    {
        m_graphMatrix[i].resize(nSize, INFINITE_DIST); 
        m_graphMatrix[i][i] = 0;                    
    }
}

int CChildView::GetClosestVertex(CPoint ptInput, int nRange)
{
    int nFoundIdx = -1;
    long lMinDistSq = nRange * nRange; // 거리 제곱 비교

    for (int i = 0; i < (int)m_vertexList.size(); ++i)
    {
        long lDx = m_vertexList[i].ptLocation.x - ptInput.x;
        long lDy = m_vertexList[i].ptLocation.y - ptInput.y;
        long lDistSq = lDx * lDx + lDy * lDy;

        if (lDistSq <= lMinDistSq)
        {
            lMinDistSq = lDistSq;
            nFoundIdx = i;
        }
    }
    return nFoundIdx;
}

void CChildView::ExecuteShortestPath(int nStart, int nEnd)
{
    int nV = (int)m_vertexList.size();

    if (nStart < 0 || nEnd < 0 || nStart >= nV || nEnd >= nV)
        return;

    // Dijkstra 알고리즘 
    std::vector<int> vecDist(nV, INFINITE_DIST);
    std::vector<int> vecPrev(nV, -1);
    std::vector<bool> vecVisited(nV, false);

    vecDist[nStart] = 0;

    for (int i = 0; i < nV; ++i)
    {
        int u = -1;
        int nMinVal = INFINITE_DIST;

        for (int j = 0; j < nV; ++j)
        {
            if (!vecVisited[j] && vecDist[j] < nMinVal)
            {
                nMinVal = vecDist[j];
                u = j;
            }
        }

        if (u == -1 || u == nEnd) break;

        vecVisited[u] = true;

        for (int v = 0; v < nV; ++v)
        {
            if (m_graphMatrix[u][v] != INFINITE_DIST)
            {
                int nNewDist = vecDist[u] + m_graphMatrix[u][v];
                if (nNewDist < vecDist[v])
                {
                    vecDist[v] = nNewDist;
                    vecPrev[v] = u;
                }
            }
        }
    }

    m_resultPath.clear();
    if (vecDist[nEnd] == INFINITE_DIST) return; // 경로 없음

    int nCurrent = nEnd;
    while (nCurrent != -1)
    {
        m_resultPath.push_back(nCurrent);
        nCurrent = vecPrev[nCurrent];
    }

    // 출발 -> 도착
    std::reverse(m_resultPath.begin(), m_resultPath.end());
}



void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
    // 키보드 입력 확인
    bool bCtrlPressed = (nFlags & MK_CONTROL) != 0;
    bool bAltPressed = (GetKeyState(VK_MENU) & 0x8000) != 0;

    // Alt + 클릭: 최단 경로
    if (bAltPressed)
    {
        int nTargetIdx = GetClosestVertex(point);
        if (nTargetIdx == -1) return;

        if (m_dijkstraStartIdx == -1)
        {
            m_dijkstraStartIdx = nTargetIdx; // 시작점 설정
            m_resultPath.clear();            // 기존 경로 제거
        }
        else
        {
            ExecuteShortestPath(m_dijkstraStartIdx, nTargetIdx); // 경로 계산
            m_dijkstraStartIdx = -1;
        }
        Invalidate();
        return;
    }

    // Ctrl + 클릭: 점-점 연결
    if (bCtrlPressed)
    {
        int nTargetIdx = GetClosestVertex(point);
        if (nTargetIdx == -1) return;

        if (m_selectedNodeIndex == -1)
        {
            m_selectedNodeIndex = nTargetIdx; // 첫 번째 점 선택
        }
        else
        {
            int nFirst = m_selectedNodeIndex;
            int nSecond = nTargetIdx;
            m_selectedNodeIndex = -1;

            if (nFirst != nSecond)
            {
                // 거리 계산 (Euclidean Distance)
                long dx = m_vertexList[nFirst].ptLocation.x - m_vertexList[nSecond].ptLocation.x;
                long dy = m_vertexList[nFirst].ptLocation.y - m_vertexList[nSecond].ptLocation.y;
                int nDist = (int)sqrt(dx * dx + dy * dy);

                ResizeMatrix();

                // 양방향 그래프 연결
                m_graphMatrix[nFirst][nSecond] = nDist;
                m_graphMatrix[nSecond][nFirst] = nDist;
            }
        }
        Invalidate();
        return;
    }

    // 좌클릭: 점 추가
    Vertex newV;
    newV.nIndex = (int)m_vertexList.size();
    newV.ptLocation = point;

    m_vertexList.push_back(newV);
    ResizeMatrix();

    Invalidate();
}

void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
    // 초기화 확인
    if (MessageBox(_T("모든 지도 데이터를 초기화하시겠습니까?"),
        _T("Reset Map"), MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        // 데이터 클리어
        m_vertexList.clear();
        m_graphMatrix.clear();
        m_resultPath.clear();

        // 선택 상태 리셋
        m_selectedNodeIndex = -1;
        m_dijkstraStartIdx = -1;

        Invalidate();
    }

    CView::OnRButtonDown(nFlags, point);
}



// 화면 그리기 및 초기화

void CChildView::OnInitialUpdate()
{
    CView::OnInitialUpdate();

    // 리소스에서 비트맵 로드
    m_bgImage.LoadBitmap(IDB_BITMAP1);

    BITMAP bm;
    m_bgImage.GetBitmap(&bm);
    m_imgSize.cx = bm.bmWidth;
    m_imgSize.cy = bm.bmHeight;
}

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}

void CChildView::OnDraw(CDC* pDC)
{


    // 배경 그리기
    if (m_bgImage.GetSafeHandle())
    {
        CDC memDC;
        memDC.CreateCompatibleDC(pDC);
        CBitmap* pOldBmp = memDC.SelectObject(&m_bgImage);

        pDC->BitBlt(0, 0, m_imgSize.cx, m_imgSize.cy, &memDC, 0, 0, SRCCOPY);

        memDC.SelectObject(pOldBmp);
    }

    // 스타일 정의
    int nRadius = 7; // 점 크기
    CPen bluePen(PS_SOLID, 3, RGB(0, 0, 255));
    CPen redPen(PS_SOLID, 5, RGB(255, 0, 0));
    CBrush blueBrush(RGB(0, 0, 255));
    CBrush redBrush(RGB(255, 0, 0));

    // 경로 그리기
    pDC->SelectObject(&bluePen);

    int nNodeCount = (int)m_vertexList.size();
    for (int i = 0; i < nNodeCount; ++i)
    {
        for (int j = i + 1; j < nNodeCount; ++j)
        {

            if (m_graphMatrix.size() > i && m_graphMatrix[i].size() > j)
            {
                if (m_graphMatrix[i][j] < INFINITE_DIST)
                {
                    pDC->MoveTo(m_vertexList[i].ptLocation);
                    pDC->LineTo(m_vertexList[j].ptLocation);
                }
            }
        }
    }

    pDC->SelectObject(&blueBrush);
    pDC->SelectStockObject(NULL_PEN); // 테두리 제거

    for (const auto& v : m_vertexList)
    {
        CRect rectNode(v.ptLocation.x - nRadius, v.ptLocation.y - nRadius,
            v.ptLocation.x + nRadius, v.ptLocation.y + nRadius);
        pDC->Ellipse(rectNode);
    }

    // 최단경로
    if (m_resultPath.size() >= 2)
    {
        pDC->SelectObject(&redPen);
        pDC->SelectObject(&redBrush);

        // 경로 선
        for (size_t k = 0; k < m_resultPath.size() - 1; ++k)
        {
            int u = m_resultPath[k];
            int v = m_resultPath[k + 1];
            pDC->MoveTo(m_vertexList[u].ptLocation);
            pDC->LineTo(m_vertexList[v].ptLocation);
        }

        // 경로 점
        for (int idx : m_resultPath)
        {
            CRect rectPathNode(m_vertexList[idx].ptLocation.x - (nRadius + 1),
                m_vertexList[idx].ptLocation.y - (nRadius + 1),
                m_vertexList[idx].ptLocation.x + (nRadius + 1),
                m_vertexList[idx].ptLocation.y + (nRadius + 1));
            pDC->Ellipse(rectPathNode);
        }
    }
}


#ifdef _DEBUG
void CChildView::AssertValid() const
{
    CView::AssertValid();
}

void CChildView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}
#endif