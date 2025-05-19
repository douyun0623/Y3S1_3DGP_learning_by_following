#pragma once

#include <memory>

// 교수님은 여기에 include 안했는데 FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT를 쓰려면
// 가져와야할거 같아서 여기에 추가함
// 원래는 cpp파일에 추가된다. 
#include "stdafx.h"
#include "Mesh.h"

class CViewport
{
public:
	//이는 불필요한 기본값 초기화와 대입 연산을 피할 수 있습니다.
	// const 멤버 변수나 참조형 멤버 변수는 대입 연산으로 초기화할 수 없기 때문에, 
	// 반드시 초기화 리스트를 사용해야 합니다.
	// 컴파일러 최적화가 잘 이루어지기 때문에 성능상 이점
	// 불필요한 기본 초기화가 발생하지 않으며, 이는 성능 상 이점
	CViewport(int nLeft, int nTop, int nWidth, int nHeight)
		: m_nLeft(nLeft), m_nTop(nTop), m_nWidth(nWidth), m_nHeight(nHeight){}

	virtual ~CViewport() = default;
	int m_nLeft;
	int m_nTop;
	int m_nWidth;
	int m_nHeight;
};

class CCamera
{
public:
	CCamera() = default;
	virtual ~CCamera() = default;	// 스마트 포인터가 자동으로 관리하므로 특별한 소멸자는 필요 없다
	// { if (m_pViewport) delete m_pViewport; }

private:
	//카메라의 위치(월드 좌표계)
	float m_fxPosition = 0.0f;
	float m_fyPosition = 0.0f;
	float m_fzPosition = 0.0f;
	//카메라의 회전(카메라 좌표계)
	float m_fxRotation = 0.0f;
	float m_fyRotation = 0.0f;
	float m_fzRotation = 0.0f;
	//카메라의 시야각, 투영 사각형까지의 거리
	float m_fFOVAngle = 90.0f;
	float m_fProjectRectDistance = 1.0f;
	//뷰포트
	// 스마트 포인터로 변경 이 카메라 객체가 소멸할때 같이 사라짐
	// CViewport* m_pViewport = NULL;
	std::unique_ptr<CViewport> m_pViewport{};
	//뷰포트의 가로 길이와 세로 길이의 비율(종횡비: Aspect ratio)
	// constepr로 만들어 float(FRAMEBUFFER_WIDTH)이렇게 하지 않아도 된다. 
	float m_fAspectRatio = FRAMEBUFFER_WIDTH / FRAMEBUFFER_HEIGHT;
public:
	//카메라 변환, 투영 변환, 화면 변환을 수행한다.
	CPoint3D CameraTransform(CPoint3D& f3World);
	CPoint3D ProjectionTransform(CPoint3D& f3Camera);
	CPoint3D ScreenTransform(CPoint3D& f3Projection);

	// 카메라의 위치 설정
	void SetPosition(float x, float y, float z) {
		m_fxPosition = x;
		m_fyPosition = y; 
		m_fzPosition = z;
	}
	// 카메라의 회전 설정
	void SetRotation(float fPitch, float fYaw, float fRoll) {
		m_fxRotation = fPitch; 
		m_fyRotation = fYaw; 
		m_fzRotation = fRoll;
	}
	//카메라의 뷰포트와 시야각을 설정한다.
	void SetViewport(int xStart, int yStart, int nWidth, int nHeight);
	void SetFOVAngle(float fFOVAngle);

	//카메라를 이동하고 회전한다.
	void Move(float x, float y, float z);
	void Rotate(float fPitch, float fYaw, float fRoll);
};

