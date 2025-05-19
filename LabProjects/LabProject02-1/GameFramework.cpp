#include "stdafx.h"
#include "GameFramework.h"

//CGameFramework” 클래스의 OnCreate()와 OnDestroy() 함수를 다음과 같이 정의
//한다.OnCreate() 함수는 주 윈도우가 생성되면 호출되고, OnDestroy() 함수는 응용프
//로그램이 종료될 때 호출된다.

void CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;
	//렌더링 화면을 생성한다.
	BuildFrameBuffer();
	//플레이어와 게임 세계(씬)을 생성한다.
	BuildObjects();
}
void CGameFramework::OnDestroy()
{
	if (m_hBitmapFrameBuffer) ::DeleteObject(m_hBitmapFrameBuffer);
	if (m_hDCFrameBuffer) ::DeleteDC(m_hDCFrameBuffer);
}

//“CGameFramework” 클래스의 BuildFrameBuffer(), ClearFrameBuffer(), 그리고
//PresentFrameBuffer() 함수를 다음과 같이 정의한다.화면 깜박임을 줄이기 위하여 윈
//도우의 클라이언트 영역의 디바이스 컨텍스트를 사용하여 직접 클라이언트 표면에 그리
//지 않는다.BuildFrameBuffer() 함수는 클라이언트 표면을 대신할 비트맵 생성하고 이
//비트맵을 메모리 디바이스 컨텍스트로 생성한다.ClearFrameBuffer() 함수는 렌더링을
//시작하기 전에 비트맵 표면을 원하는 색상으로 지운다.씬을 화면(클라이언트 영역)으로
//렌더링하기 위하여 먼저 씬의 게임 객체들을 비트맵 표면으로 렌더링한다.
//PresentFrameBuffer() 함수는 비트맵 표면으로 렌더링된 비트맵을 클라이언트 영역으
//로 옮긴다.

void CGameFramework::BuildFrameBuffer()
{
	::GetClientRect(m_hWnd, &m_rcClient);
	HDC hDC = ::GetDC(m_hWnd);
	m_hDCFrameBuffer = ::CreateCompatibleDC(hDC);
	m_hBitmapFrameBuffer = ::CreateCompatibleBitmap(hDC,
		m_rcClient.right - m_rcClient.left, m_rcClient.bottom -
		m_rcClient.top);
	::SelectObject(m_hDCFrameBuffer, m_hBitmapFrameBuffer);
	::ReleaseDC(m_hWnd, hDC);
	::SetBkMode(m_hDCFrameBuffer, TRANSPARENT);
}
void CGameFramework::ClearFrameBuffer(DWORD dwColor)
{
	HPEN hPen = ::CreatePen(PS_SOLID, 0, dwColor);
	HPEN hOldPen = (HPEN)::SelectObject(m_hDCFrameBuffer, hPen);
	HBRUSH hBrush = ::CreateSolidBrush(dwColor);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(m_hDCFrameBuffer,
		hBrush);
	::Rectangle(m_hDCFrameBuffer, m_rcClient.left, m_rcClient.top,
		m_rcClient.right, m_rcClient.bottom);
	::SelectObject(m_hDCFrameBuffer, hOldBrush);
	::SelectObject(m_hDCFrameBuffer, hOldPen);
	::DeleteObject(hPen);
	::DeleteObject(hBrush);
}
void CGameFramework::PresentFrameBuffer()
{
	HDC hDC = ::GetDC(m_hWnd);
	::BitBlt(hDC, m_rcClient.left, m_rcClient.top, m_rcClient.right -
		m_rcClient.left, m_rcClient.bottom - m_rcClient.top,
		m_hDCFrameBuffer, m_rcClient.left, m_rcClient.top, SRCCOPY);
	::ReleaseDC(m_hWnd, hDC);
}


// 3“CGameFramework” 클래스의 BuildObjects()와 ReleaseObjects() 함수를 다음과 같
//이 정의한다.BuildObjects() 함수는 카메라를 생성하여 설정하고, 플레이어 객체를 생
//성하고, 그리고 씬(게임 세계)을 생성한다.ReleaseObjects() 함수는 BuildObjects() 함
//수에서 생성한 객체들을 소멸한다.

void CGameFramework::BuildObjects()
{
	//카메라를 생성하고 뷰포트와 시야각(FOV)를 설정한다.
	std::unique_ptr<CCamera> pCamera = std::make_unique<CCamera>();
	//* pCamera = new CCamera();
	pCamera->SetViewport(0, 0, FRAMEBUFFER_WIDTH,
		FRAMEBUFFER_HEIGHT);
	pCamera->SetFOVAngle(60.0f);

	//플레이어 게임 객체를 생성하고 카메라와 위치를 설정한다.
	m_pPlayer = std::make_unique<CPlayer>();
	m_pPlayer->SetCamera(std::move(pCamera));
	m_pPlayer->SetPosition(0.0f, 3.0f, -40.0f);

	//씬 객체를 생성하고 게임 객체들을 생성한다.
	// m_pScene = new CScene();
	m_pScene = std::make_unique<CScene>();
	m_pScene->BuildObjects();
}

//“CGameFramework” 클래스의 ProcessInput() 함수를 다음과 같이 정의한다.
//ProcessInput() 함수는 키보드 입력을 처리하여 플레이어 객체를 이동한다.

void CGameFramework::ProcessInput()
{
	static UCHAR pKeyBuffer[256];
	if (::GetKeyboardState(pKeyBuffer))
	{
		float cxKeyDelta = 0.0f, cyKeyDelta = 0.0f, czKeyDelta = 0.0f;
		if (pKeyBuffer[VK_UP] & 0xF0) czKeyDelta = +0.125f;
		if (pKeyBuffer[VK_DOWN] & 0xF0) czKeyDelta = -0.125f;
		if (pKeyBuffer[VK_LEFT] & 0xF0) cxKeyDelta = -0.125f;
		if (pKeyBuffer[VK_RIGHT] & 0xF0) cxKeyDelta = +0.125f;
		if (pKeyBuffer[VK_PRIOR] & 0xF0) cyKeyDelta = +0.125f;
		if (pKeyBuffer[VK_NEXT] & 0xF0) cyKeyDelta = -0.125f;
		m_pPlayer->Move(cxKeyDelta, cyKeyDelta, czKeyDelta);
	}
}

//“CGameFramework” 클래스의 AnimateObjects() 함수를 다음과 같이 정의한다.
//AnimateObjects() 함수는 씬의 게임 객체들을 애니메이션한다.
void CGameFramework::AnimateObjects()
{
	if (m_pScene) {
		m_pScene->Animate(1.0f / 60.0f);
	}
}

/*“CGameFramework” 클래스의 FrameAdvance() 함수를 다음과 같이 정의한다.
FrameAdvance() 함수는 윈도우 메시지 루프에서 반복적으로 호출된다.즉, 응용프로그
램이 실행되는 동안 이 함수가 반복적으로 계속 실행된다.이 함수는 사용자 입력을 받
아 플레이어 또는 게임 세계의 게임 객체들을 움직이고 그 결과에 따라 게임 세계를 화
면으로 렌더링한다*/

void CGameFramework::FrameAdvance()
{
	//사용자 입력을 처리한다.
	ProcessInput();
	//게임 세계를 애니메이션(움직이게)한다.
	AnimateObjects();
	//렌더링을 할 대상 화면(비트맵)을 지운다.
	ClearFrameBuffer(RGB(90, 103, 224));
	//씬을 렌더링한다.
	CCamera* pCamera = m_pPlayer->GetCamera();
	if (m_pScene) m_pScene->Render(m_hDCFrameBuffer, pCamera);
	//렌더링을 한 화면(비트맵)을 클라이언트 영역으로 복사한다.
	PresentFrameBuffer();
}