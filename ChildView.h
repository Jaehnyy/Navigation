// ChildView.h : CChildView 클래스의 인터페이스
//

#pragma once

#include <vector>
#include <algorithm>
#include <cmath>

class CcampusmapDoc;

class CChildView : public CView
{
public:
    CChildView() noexcept;
    DECLARE_DYNCREATE(CChildView)

public:

    // 점 구조체
    struct Vertex
    {
        int    nIndex;      // 점 번호
        CPoint ptLocation;  // 좌표 
    };

    // 점 저장 리스트
    std::vector<Vertex> m_vertexList;

    // 거리 저장
    std::vector<std::vector<int>> m_graphMatrix;

    // 계산된 최단 경로의 노드 인덱스 
    std::vector<int> m_resultPath;

    // 연결 없음 상태를 나타내는 상수
    static const int INFINITE_DIST = 1000000000;

    int m_selectedNodeIndex;  // Ctrl + 클릭 시 첫 번째 선택된 노드
    int m_dijkstraStartIdx;   // Alt + 클릭 시 경로 탐색 시작 노드

    CBitmap m_bgImage;        // 캠퍼스 지도 이미지
    CSize   m_imgSize;        // 해상도

protected:
    // 점 추가시 행렬 크기 조정
    void ResizeMatrix();

    // 클릭한 좌표에서 가까운 점 찾는
    int  GetClosestVertex(CPoint ptInput, int nRange = 15);

    // 최단 경로 탐색 (Dijkstra)
    void ExecuteShortestPath(int nStart, int nEnd);

public:


    virtual void OnDraw(CDC* pDC);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnInitialUpdate();
    virtual ~CChildView();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

    DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  
inline CcampusmapDoc* CChildView::GetDocument() const
{
    return reinterpret_cast<CcampusmapDoc*>(m_pDocument);
}
#endif