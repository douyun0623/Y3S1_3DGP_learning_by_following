#pragma once

#include <Windows.h>
#include <memory>
#include <vector>


// 스마트 포인터를 사용해서 자원 해제를 자동으로 할수 있게 수정하였다.

// 3D 공간에서 점을 표현하는 클래스입니다.
class CPoint3D
{
public:
	CPoint3D() = default;

	CPoint3D(float x, float y, float z)
		: x(x), y(y), z(z) {}

	virtual ~CPoint3D() = default;

	float x{};
	float y{};
	float z{};
};

// 3D 공간에서 하나의 정점을 표현하는 클래스입니다. 
// 이 클래스는 CPoint3D를 이용해 점을 정의합니다.
// 정점의 색상, 정점의 법선 벡터, 텍스처 좌표 등과 같은 추가적인 정보를 포함할 수 있습
class CVertex
{
public:
	CVertex() = default;

	CVertex(float x, float y, float z)
	: m_f3Position(x,y,z) {}

	virtual ~CVertex() = default;

	CPoint3D m_f3Position;
};

// 3D 공간에서 다각형(폴리곤)을 정의하는 클래스입니다. 
// 이 클래스는 정점들의 리스트를 포함하여 면을 구성합니다.
class CPolygon
{
public:
	CPolygon() = default;

	CPolygon(int nVertices)
		:m_nVertices(nVertices),
		m_pVertices(std::make_unique<CVertex[]>(nVertices)) {

	}
	// 소멸자 생략 가능 (unique_ptr이 자동으로 메모리 해제해줌)

	//다각형(면)을 구성하는 정점들의 리스트이다
	int m_nVertices{};								// 다각형을 구성하는 정점의 개수
	std::unique_ptr<CVertex[]> m_pVertices{};		// 다각형을 구성하는 정점을 가르킨다.

	// 교수님 코드 
	// void SetVertex(int nIndex, CVertex vertex);
	void SetVertex(int nIndex, const CVertex& vertex);
};

// 메쉬(3D 모델)의 데이터를 관리하는 클래스입니다. 
// 메쉬는 다각형(폴리곤)의 집합이며, 여러 게임 객체들이 이를 공유할 수 있습니다.
class CMesh
{
public:
	CMesh() = default;
	CMesh(int nPolygons);

	// delete this;를 호출하려면 소멸자가 가상 함수로 정의되어야 한다.
	// virtual ~CMesh() = default;와 같은 명시적인 소멸자가 필요
	// Release()와 같은 수동 삭제 로직을 포함하는 경우, 
	// 소멸자는 여전히 명시적으로 정의할 필요
	virtual ~CMesh() = default;

	// void SetPolygon(int nIndex, CPolygon* pPolygon);
	void SetPolygon(int nIndex, std::unique_ptr<CPolygon> pPolygon);

	virtual void Render(HDC hDCFrameBuffer);

private:
	//메쉬를 구성하는 다각형(면)들의 리스트이다. 
	int m_nPolygons{};				// 메쉬를 구성하는 다각형의 개수
	// CPolygon** m_ppPolygons{};		// 다각형들을 가르키는 포인터 배열
	std::vector<std::unique_ptr<CPolygon>> m_vecPolygons;	// 스마트 포인터 벡터

	// std::shared_ptr<CMesh> mesh = std::make_shared<CCubeMesh>();
	// shared_ptr을 사용해서 인스턴싱을 한다.
	// 따라서 아래와 같은 코드를 주석 처리함

	/*
	//private:
	//	//인스턴싱(Instancing)을 위하여 메쉬는 게임 객체들에 공유될 수 있다. 
	//	//다음 참조값(Reference Count)은 메쉬가 공유되는 게임 객체의 개수를 나타낸다.
	//	int m_nReferences = 1;
	//
	//public:
	//	//메쉬가 게임 객체에 공유될 때마다 참조값을 1씩 증가시킨다. 
	//	void AddRef() { ++m_nReferences; }
	//
	//	//메쉬를 공유하는 게임 객체가 소멸될 때마다 참조값을 1씩 감소시킨다. 
	//	//참조값이 0이되면 메쉬를 소멸시킨다. 
	//	void Release() {
	//		--m_nReferences;
	//		if (m_nReferences <= 0) {
	//			delete this;		
	//		}
	//	}
	*/
};

class CCubeMesh : public CMesh
{
public:
	CCubeMesh(float fwidth = 4.0f, float fHeight = 4.0f, float fDepth = 4.0f);
	
	virtual ~CCubeMesh() = default;
};