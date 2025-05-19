#pragma once

#include <Windows.h>
#include <memory>
#include <vector>


// ����Ʈ �����͸� ����ؼ� �ڿ� ������ �ڵ����� �Ҽ� �ְ� �����Ͽ���.

// 3D �������� ���� ǥ���ϴ� Ŭ�����Դϴ�.
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

// 3D �������� �ϳ��� ������ ǥ���ϴ� Ŭ�����Դϴ�. 
// �� Ŭ������ CPoint3D�� �̿��� ���� �����մϴ�.
// ������ ����, ������ ���� ����, �ؽ�ó ��ǥ ��� ���� �߰����� ������ ������ �� �ֽ�
class CVertex
{
public:
	CVertex() = default;

	CVertex(float x, float y, float z)
	: m_f3Position(x,y,z) {}

	virtual ~CVertex() = default;

	CPoint3D m_f3Position;
};

// 3D �������� �ٰ���(������)�� �����ϴ� Ŭ�����Դϴ�. 
// �� Ŭ������ �������� ����Ʈ�� �����Ͽ� ���� �����մϴ�.
class CPolygon
{
public:
	CPolygon() = default;

	CPolygon(int nVertices)
		:m_nVertices(nVertices),
		m_pVertices(std::make_unique<CVertex[]>(nVertices)) {

	}
	// �Ҹ��� ���� ���� (unique_ptr�� �ڵ����� �޸� ��������)

	//�ٰ���(��)�� �����ϴ� �������� ����Ʈ�̴�
	int m_nVertices{};								// �ٰ����� �����ϴ� ������ ����
	std::unique_ptr<CVertex[]> m_pVertices{};		// �ٰ����� �����ϴ� ������ ����Ų��.

	// ������ �ڵ� 
	// void SetVertex(int nIndex, CVertex vertex);
	void SetVertex(int nIndex, const CVertex& vertex);
};

// �޽�(3D ��)�� �����͸� �����ϴ� Ŭ�����Դϴ�. 
// �޽��� �ٰ���(������)�� �����̸�, ���� ���� ��ü���� �̸� ������ �� �ֽ��ϴ�.
class CMesh
{
public:
	CMesh() = default;
	CMesh(int nPolygons);

	// delete this;�� ȣ���Ϸ��� �Ҹ��ڰ� ���� �Լ��� ���ǵǾ�� �Ѵ�.
	// virtual ~CMesh() = default;�� ���� ������� �Ҹ��ڰ� �ʿ�
	// Release()�� ���� ���� ���� ������ �����ϴ� ���, 
	// �Ҹ��ڴ� ������ ��������� ������ �ʿ�
	virtual ~CMesh() = default;

	// void SetPolygon(int nIndex, CPolygon* pPolygon);
	void SetPolygon(int nIndex, std::unique_ptr<CPolygon> pPolygon);

	virtual void Render(HDC hDCFrameBuffer);

private:
	//�޽��� �����ϴ� �ٰ���(��)���� ����Ʈ�̴�. 
	int m_nPolygons{};				// �޽��� �����ϴ� �ٰ����� ����
	// CPolygon** m_ppPolygons{};		// �ٰ������� ����Ű�� ������ �迭
	std::vector<std::unique_ptr<CPolygon>> m_vecPolygons;	// ����Ʈ ������ ����

	// std::shared_ptr<CMesh> mesh = std::make_shared<CCubeMesh>();
	// shared_ptr�� ����ؼ� �ν��Ͻ��� �Ѵ�.
	// ���� �Ʒ��� ���� �ڵ带 �ּ� ó����

	/*
	//private:
	//	//�ν��Ͻ�(Instancing)�� ���Ͽ� �޽��� ���� ��ü�鿡 ������ �� �ִ�. 
	//	//���� ������(Reference Count)�� �޽��� �����Ǵ� ���� ��ü�� ������ ��Ÿ����.
	//	int m_nReferences = 1;
	//
	//public:
	//	//�޽��� ���� ��ü�� ������ ������ �������� 1�� ������Ų��. 
	//	void AddRef() { ++m_nReferences; }
	//
	//	//�޽��� �����ϴ� ���� ��ü�� �Ҹ�� ������ �������� 1�� ���ҽ�Ų��. 
	//	//�������� 0�̵Ǹ� �޽��� �Ҹ��Ų��. 
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