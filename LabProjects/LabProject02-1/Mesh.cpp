#include "stdafx.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"

void CPolygon::SetVertex(int nIndex, const CVertex& vertex)
{
	if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices) {
		m_pVertices[nIndex] = vertex;
	}
}

CMesh::CMesh(int nPolygons)
	// 벡터의 크기 설정
	: m_nPolygons(nPolygons), m_vecPolygons(nPolygons)
{
	//이 부분은 벡터에 있는 각 항목을 초기화합니다. 
	// std::make_unique<CPolygon>()는 CPolygon 객체를 동적으로 할당하고,
	// 그 주소를 std::unique_ptr로 반환합니다. 
	// 즉, 각 unique_ptr 객체가 CPolygon 객체를 소유하게 됩니다. 
	// 이 코드는 벡터의 모든 요소를 unique_ptr로 초기화하여, 
	// 안전하게 메모리를 관리하도록 합니다.
	for (int i = 0; i < nPolygons; ++i) {
		m_vecPolygons[i] = std::make_unique<CPolygon>();
	}
}

void CMesh::SetPolygon(int nIndex, std::unique_ptr<CPolygon> pPolygon)
{
	// if ((0 <= nIndex) && (nIndex < m_nPolygons))
	//		m_ppPolygons[nIndex] = pPolygon; 

	//std::move는 객체의 소유권을 이동시키는 것이지, 
	// 객체를 복사하는 것이 아닙니다.
	// std::move를 사용한 후에는 원본 스마트 포인터(pPolygon)는 더 이상 유효하지 않으며,
	// 이를 사용하려면 다시 새로운 값을 할당해야 합니다.
	if (nIndex >= 0 && nIndex < m_nPolygons) {
		m_vecPolygons[nIndex] = std::move(pPolygon);
	}
}

// -------------------------------나중에 풀기!---------------------------------

//// Draw2DLine() 전역 함수를 다음과 같이 정의한다.
void Draw2DLine(HDC hDCFrameBuffer, CPoint3D& f3PreviousProject,
    CPoint3D& f3CurrentProject)
{
    //투영 좌표계의 2점을 화면 좌표계로 변환하고 변환된 두 점(픽셀)을 선분으로 그린다.
    CPoint3D f3Previous =
        CGraphicsPipeline::ScreenTransform(f3PreviousProject);
    CPoint3D f3Current =
        CGraphicsPipeline::ScreenTransform(f3CurrentProject);
    ::MoveToEx(hDCFrameBuffer, (long)f3Previous.x,
        (long)f3Previous.y, NULL);
    ::LineTo(hDCFrameBuffer, (long)f3Current.x, (long)f3Current.y);
}

void CMesh::Render(HDC hDCFrameBuffer)
{
    CPoint3D f3InitialProject, f3PreviousProject, f3Intersect;
    bool bPreviousInside = false, bInitialInside = false,
        bCurrentInside = false, bIntersectInside = false;
    //메쉬를 구성하는 모든 다각형들을 렌더링한다.
    for (int j = 0; j < m_nPolygons; j++)
    {
        int nVertices = m_vecPolygons[j]->m_nVertices;
        CVertex* pVertices = m_vecPolygons[j]->m_pVertices.get();  // 스마트 포인터로 접근
        //다각형의 첫 번째 정점을 원근 투영 변환한다.
        f3PreviousProject = f3InitialProject =
            CGraphicsPipeline::Project(pVertices[0].m_f3Position);
        //변환된 점이 투영 사각형에 포함되는 가를 계산한다.
        bPreviousInside = bInitialInside = (-1.0f <=
            f3InitialProject.x) && (f3InitialProject.x <= 1.0f) && (-1.0f <=
                f3InitialProject.y) && (f3InitialProject.y <= 1.0f);
        //다각형을 구성하는 모든 정점들을 원근 투영 변환하고 선분으로 렌더링한다.
        for (int i = 1; i < nVertices; i++)
        {
            CPoint3D f3CurrentProject =
                CGraphicsPipeline::Project(pVertices[i].m_f3Position);
            //변환된 점이 투영 사각형에 포함되는 가를 계산한다.
            bCurrentInside = (-1.0f <= f3CurrentProject.x) &&
                (f3CurrentProject.x <= 1.0f) && (-1.0f <= f3CurrentProject.y) &&
                (f3CurrentProject.y <= 1.0f);
            //변환된 점이 투영 사각형에 포함되면 이전 점과 현재 점을 선분으로 그린다.
            if (((f3PreviousProject.z >= 0.0f) || (f3CurrentProject.z >=
                0.0f)) && ((bCurrentInside || bPreviousInside)))
                ::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
            f3PreviousProject = f3CurrentProject;
            bPreviousInside = bCurrentInside;
        }
        //다각형의 마지막 정점과 다각형의 시작점을 선분으로 그린다.
        if (((f3PreviousProject.z >= 0.0f) || (f3InitialProject.z >=
            0.0f)) && ((bInitialInside || bPreviousInside)))
            ::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3InitialProject);
    }
}

// CCubeMesh 클래스의 생성자와 소멸자를 다음과 같이 정의한다.
CCubeMesh::CCubeMesh(float fWidth, float fHeight, float fDepth) :
	CMesh(6)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;

    // 각 면(FrontFace, TopFace, 등)의 처리가 독립적이므로, 
    // 지역 블록 {}을 사용하여 각 블록에서 생성된 객체가 그 범위 내에서만 유효하도록 했습니다.
    // 이렇게 하면 각 CPolygon 객체가 그 블록을 벗어나면 자동으로 소멸됩니다.
    // 이로 인해 메모리 관리를 명확하게 할 수 있습니다.
    
    // 각 면을 처리하는 지역 블록
    {
        auto pFrontFace = std::make_unique<CPolygon>(4);
        pFrontFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
        pFrontFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
        pFrontFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
        pFrontFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
        SetPolygon(0, std::move(pFrontFace));
    }

    {
        auto pTopFace = std::make_unique<CPolygon>(4);
        pTopFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
        pTopFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
        pTopFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
        pTopFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
        SetPolygon(1, std::move(pTopFace));
    }

    {
        auto pBackFace = std::make_unique<CPolygon>(4);
        pBackFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
        pBackFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
        pBackFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
        pBackFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
        SetPolygon(2, std::move(pBackFace));
    }

    {
        auto pBottomFace = std::make_unique<CPolygon>(4);
        pBottomFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
        pBottomFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
        pBottomFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
        pBottomFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
        SetPolygon(3, std::move(pBottomFace));
    }

    {
        auto pLeftFace = std::make_unique<CPolygon>(4);
        pLeftFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
        pLeftFace->SetVertex(1, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
        pLeftFace->SetVertex(2, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
        pLeftFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
        SetPolygon(4, std::move(pLeftFace));
    }

    {
        auto pRightFace = std::make_unique<CPolygon>(4);
        pRightFace->SetVertex(0, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
        pRightFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
        pRightFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
        pRightFace->SetVertex(3, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
        SetPolygon(5, std::move(pRightFace));
    }
}