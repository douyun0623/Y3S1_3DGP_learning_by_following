#include "stdafx.h"
#include "GameFramework.h"

CGameFramework::CGameFramework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;
	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;
	m_pd3dPipelineState = NULL;
	m_pd3dCommandList = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_ppd3dRenderTargetBuffers[i] = NULL;
	m_pScene = NULL;
	m_d3dViewport = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	m_pd3dRtvDescriptorHeap = NULL;
	m_nRtvDescriptorIncrementSize = 0;
	m_pd3dDepthStencilBuffer = NULL;
	m_pd3dDsvDescriptorHeap = NULL;
	m_nDsvDescriptorIncrementSize = 0;
	m_nSwapChainBufferIndex = 0;
	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	// m_nFenceValue = 0;
	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;
	_tcscpy_s(m_pszFrameRate, _T("LapProject ("));
}

CGameFramework::~CGameFramework()
{
}

// OnCreate() 함수와 OnDestroy() 함수를 다음과 같이 정의한다.
// 다음 함수는 응용 프로그램이 실행되어 주 윈도우가 생성되면 호출된다는 것에 유의하라.
bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;
	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();
	BuildObjects();
	return(true);
}
void CGameFramework::OnDestroy()
{
	WaitForGpuComplete();
	//GPU가 모든 명령 리스트를 실행할 때 까지 기다린다.
	ReleaseObjects();
	//게임 객체(게임 월드 객체)를 소멸한다.
	::CloseHandle(m_hFenceEvent);
	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dRenderTargetBuffers[i])
		m_ppd3dRenderTargetBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();
	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dPipelineState) m_pd3dPipelineState->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();
	if (m_pd3dFence) m_pd3dFence->Release();
	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();
#if defined(_DEBUG)
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
		DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif
}

// CreateSwapChain() 함수를 다음과 같이 정의한다.
void CGameFramework::CreateSwapChain()
{
	// 현재 윈도우 창의 클라이언트 영약 (제목표시줄, 테두리 제외한 내부 영역) 크기를 구함
	// 이 크기를 기반으로 스왑 체인의 버퍼 크기가 결정된다.
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));

	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue,
		&dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiSwapChain);
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRenderTargetViews();
#endif
}

// CreateDirect3DDisplay() 함수를 다음과 같이 정의한다.
void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;
	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	// DXGI 팩토리 생성 
	// gpu 어댑터 열거, 스왑 체인 생성 등에 이용된다.
	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);
	
	// 시스템의 gpu 어댑터들을 순회하면서 조건에 맞는 것을 찾는다.
	IDXGIAdapter1* pd3dAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;
	}
	//모든 하드웨어 어댑터 대하여 특성 레벨 12.0을 지원하는 하드웨어 디바이스를 생성한다.
	
	// 하드웨어 디바이스가 없는 경우 사용한다.
	// warp 어댑터
	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIAdapter1), (void**)&pd3dAdapter);
		D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}
	//특성 레벨 12.0을 지원하는 하드웨어 디바이스를 생성할 수 없으면 WARP 디바이스를 생성한다.


	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4; //Msaa4x 다중 샘플링
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	//디바이스가 지원하는 다중 샘플의 품질 수준을 확인한다.
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;
	//다중 샘플의 품질 수준이 1보다 크면 다중 샘플링을 활성화한다.


	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence),
		(void**)&m_pd3dFence);
	m_nFenceValue = 0;
	//펜스를 생성하고 펜스 값을 0으로 설정한다.

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	/*펜스와 동기화를 위한 이벤트 객체를 생성한다(이벤트 객체의 초기값을 FALSE이다). 이벤트가 실행되면(Signal) 이
	벤트의 값을 자동적으로 FALSE가 되도록 생성한다.*/

	m_d3dViewport.TopLeftX = 0;
	m_d3dViewport.TopLeftY = 0;
	m_d3dViewport.Width = static_cast<float>(m_nWndClientWidth);
	m_d3dViewport.Height = static_cast<float>(m_nWndClientHeight);
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;
	//뷰포트를 주 윈도우의 클라이언트 영역 전체로 설정한다.
	m_d3dScissorRect = { 0, 0, m_nWndClientWidth, m_nWndClientHeight };
	//씨저 사각형을 주 윈도우의 클라이언트 영역 전체로 설정한다.
	if (pd3dAdapter) pd3dAdapter->Release();
}


// CreateCommandQueueAndList() 함수를 다음과 같이 정의한다.
void CGameFramework::CreateCommandQueueAndList()
{
	// ZeroMemory 구조체를 0으로 초기화 -> 안전성과 예측 가능성 확보
	// Flags = None -> 특별 플레그 없이 일반 큐로 설정
	// type = direct -> direct 명령 큐로 설정 ( 그래픽 / 계산 / 복사 작업 모두 가능)
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;	// 명령 큐 설정 구조체 
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;


	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc,
		_uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);
	//직접(Direct) 명령 큐를 생성한다.

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);
	//직접(Direct) 명령 할당자를 생성한다.

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList);
	//직접(Direct) 명령 리스트를 생성한다.

	hResult = m_pd3dCommandList->Close();
	// 명령 리스트는 생성되면 열린(Open) 상태이므로 닫힌(Closed) 상태로 만든다.
	// 
	// 명령 리스트를 제출할때 반드시 "닫힌 상태"여야 한다.
	// open : 명령 기록할 수 있음 ( 명령을 그릴 시즘에 연다 -> 일단 덮어둠)
	// closed : 명령 기록할 수 없음, submit 가능
	//
	// 이후 렌더링 루프
	// reset -> draw -> close -> 제출

	// Command Queue	 : gpu에 명령 리스트들을 제출하는 창구
	// command Allocator : 명령 리스트가 내부적으로 명령어를 저장할 공간 제공
	// Command List		 : 실행될 명령들을 기록하는 객체
}

// CreateRtvAndDsvDescriptorHeaps() 함수를 다음과 같이 정의한다.
void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	// 디스크립터(Descriptor)는 GPU가 리소스(버퍼, 텍스처 등)에 접근할 수 있도록 정보를 담은 구조체입니다.
	// **디스크립터 힙(Descriptor Heap)** 은 그런 디스크립터들을 연속적으로 저장하는 메모리 공간입니다.
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;	// 스왑체인에 버퍼가 2개면 rtv도 2개 필요
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // rtv 전용 타입
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // 셰이더에서 접근하지 않으므로 플래그 없음
	d3dDescriptorHeapDesc.NodeMask = 0;


	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	//렌더 타겟 서술자 힙(서술자의 개수는 스왑체인 버퍼의 개수)을 생성한다.

	m_nRtvDescriptorIncrementSize =
		m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//렌더 타겟 서술자 힙의 원소의 크기를 저장한다.

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	//깊이-스텐실 서술자 힙(서술자의 개수는 1)을 생성한다.
	m_nDsvDescriptorIncrementSize =
		m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//깊이-스텐실 서술자 힙의 원소의 크기를 저장한다.
}

// CreateRenderTargetViews()와 CreateDepthStencilView() 함수를 다음과 같이 정의한다.
// 스왑체인의 각 후면 버퍼에 대한 렌더 타겟 뷰를 생성한다.
void CGameFramework::CreateRenderTargetViews()
{
	// RTV 디스크립터 힙의 시작 주소를 가져옵니다. 		
	// RTV를 생성할 위치의 첫 포인터를 준비합니다.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		// 1. 스왑 체인의 i번째 버퍼를 가져옴 (백 버퍼)
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dRenderTargetBuffers[i]);
		
		// 2. 해당 버퍼에 대한 RTV 생성
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i], NULL,
			d3dRtvCPUDescriptorHandle);

		// 3. 다음 RTV 생성을 위해 디스크립터 포인터를 증가
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
}


// 깊이 테스트 : 픽셀이 화면에 출력되기 전에 얼마나 멀리 있는지를 판단.
// 스텐실 테스트 : 픽셀 렌더링을 제어하는 추가 조건을 적용할 때 사용.
void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2d 텍스처 리소스 생성 (보통 화면과 같으 ㄴ2d)
	d3dResourceDesc.Alignment = 0; // 리소스의 메모리 정렬 설정 (기본 정렬 사용)
	d3dResourceDesc.Width = m_nWndClientWidth;	 // 창의 크기에 맞게 깊이 버퍼를 설정
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;	// 리소스가 한개의 2d 텍스처라는 의미이다. (배열 텍스처가 아니므로 1로 설정)
	d3dResourceDesc.MipLevels = 1;	// 민맵 레벨 수 의미 ( 깊이 버퍼는 필요없음 -> 1로 설정)
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 리소스 포맷을 지정합니다. 24 깊이, 8 스텐실
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;	// msaa 사용시 4 아니면 1(안씀)
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;	// m_nMsaa4xQualityLevels는 디바이스가 지원하는 품질 수치이며, -1은 가장 높은 품질 설정
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;	// 메모리 레이아웃은 시스템에 맡김
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // 이 리소스는 깊이/스텐실용으로 사용할 것이다 라고 명시
	
	
	D3D12_HEAP_PROPERTIES d3dHeapProperties; // 힙 속성 정의 (리소스를 어디에 올릴지)
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;	// unkown으로 두고 자동
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;		// unkown으로 두고 자동
	d3dHeapProperties.CreationNodeMask = 1;	// 싱글 gpu 기준 1
	d3dHeapProperties.VisibleNodeMask = 1;	// 싱글 gpu 기준 1


	D3D12_CLEAR_VALUE d3dClearValue;	// 초기화시 사용할 기본값
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;	// 기본 깊이값 (1.0 = 가장 멀리)
	d3dClearValue.DepthStencil.Stencil = 0;		// 기본 스텐실 값

	// 실제 깊이 - 스텐실 버퍼 리소스를 GPU에 생성합니다.
	m_pd3dDevice->CreateCommittedResource(
		&d3dHeapProperties,						// 힙 속성
		D3D12_HEAP_FLAG_NONE,					// 특별한 힙 플래그 없음
		&d3dResourceDesc,						// 리소스 속성
		D3D12_RESOURCE_STATE_DEPTH_WRITE,		// 초기 상태: 깊이 쓰기
		&d3dClearValue,							// 클리어 값
		__uuidof(ID3D12Resource),				// 인터페이스 ID
		(void**)&m_pd3dDepthStencilBuffer);		// 결과 저장
	//깊이-스텐실 버퍼를 생성한다.


	// 깊이/스텐실 뷰를 위한 디스크립터 힙의 시작 위치를 가져옵니다.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();


	m_pd3dDevice->CreateDepthStencilView(
		m_pd3dDepthStencilBuffer,	// 대상 버퍼
		NULL,						// 기본 뷰 설정
		d3dDsvCPUDescriptorHandle);	// 디스크립터 핸들 위치
	//깊이-스텐실 버퍼 뷰를 생성한다.
}


// BuildObjects(), ReleaseObjects(), OnProcessingWindowMessage(), ProcessInput(),
// AnimateObjects(), OnProcessingMouseMessage(), OnProcessingKeyboardMessage() 함수를 다음과
// 같이 정의한다.
void CGameFramework::BuildObjects()
{
}
void CGameFramework::ReleaseObjects()
{
}
void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam,
	LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
			//“F9” 키가 눌려지면 윈도우 모드와 전체화면 모드의 전환을 처리한다.
		case VK_F9:
			ChangeSwapChainState();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}
void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM
	wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F8:
			break;
		case VK_F9:
			ChangeSwapChainState();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}
LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID,
	WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_SIZE:
	{
		m_nWndClientWidth = LOWORD(lParam);
		m_nWndClientHeight = HIWORD(lParam);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}
void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();
	BOOL bFullScreenState = FALSE;
	m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);
	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);
	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dRenderTargetBuffers[i])
		m_ppd3dRenderTargetBuffers[i]->Release();
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth,
		m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	CreateRenderTargetViews();
}
void CGameFramework::ProcessInput()
{
}
void CGameFramework::AnimateObjects()
{
}

// WaitForGpuComplete() 함수를 다음과 같이 정의한다.
// CPU가 GPU의 작업이 끝날 때까지 기다리는 함수입니다.
void CGameFramework::WaitForGpuComplete()
{
	m_nFenceValue++;	// CPU가 명령을 보낼 때마다 1씩 증가
	//CPU 펜스의 값을 증가한다.

	const UINT64 nFence = m_nFenceValue;	// 이 값은 GPU가 완료해야 할 목표값이 됩니다.
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFence);	// 명령 큐에 펜스를 설정하라(Signal) 는 명령을 추가합니다.
	// GPU는 앞선 모든 명령을 완료한 후에 이 펜스를 nFence로 설정합니
	// GPU가 펜스의 값을 설정하는 명령을 명령 큐에 추가한다.

	// GPU가 nFence까지 작업을 마치면 OS 이벤트(m_hFenceEvent)를 신호(Signal) 하도록 설정합니다.
	if (m_pd3dFence->GetCompletedValue() < nFence)
	{
		//펜스의 현재 값이 설정한 값보다 작으면 펜스의 현재 값이 설정한 값이 될 때까지 기다린다.
		hResult = m_pd3dFence->SetEventOnCompletion(nFence, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
		// 위에서 설정한 이벤트가 Signal 상태가 될 때까지 무한 대기(INFINITE) 합니다.
		// 즉, GPU가 작업을 마칠 때까지 CPU가 여기서 멈춰서 기다립니다.
	}
}

// FrameAdvance() 함수를 다음과 같이 정의한다.
// 입력 처리 → 애니메이션 처리 → 렌더링 준비 → 그리기 → 렌더 타겟 상태 전환 → GPU에 제출 → GPU 대기 → 화면 출력(Present)
void CGameFramework::FrameAdvance()
{
	// CGameFramework“ 클래스의 FrameAdvance() 멤버 함수를 다음과 같이 변경한다.
	// 타이머의 시간이 갱신되도록 하고 프레임 레이트를 계산한다.
	m_GameTimer.Tick(0.0f);

	// 입력 및 애니메이션 처리
	ProcessInput();
	AnimateObjects();

	// 명령 객체 초기화
	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
	//명령 할당자와 명령 리스트를 리셋한다.

	// 후면 버퍼는 Present 상태였는데, 이제 렌더링에 사용하려면 Render Target 상태로 바꿔야 합니다.
	// 이걸 Resource Barrier라고 합니다.
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
	/*현재 렌더 타겟에 대한 프리젠트가 끝나기를 기다린다. 프리젠트가 끝나면 렌더 타겟 버퍼의 상태는 프리젠트 상태
	(D3D12_RESOURCE_STATE_PRESENT)에서 렌더 타겟 상태(D3D12_RESOURCE_STATE_RENDER_TARGET)로 바
	뀔 것이다.*/

	m_pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	m_pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
	//뷰포트와 씨저 사각형을 설정한다.

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex *	m_nRtvDescriptorIncrementSize);
	//현재의 렌더 타겟에 해당하는 서술자의 CPU 주소(핸들)를 계산한다.

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	// float pfClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle,
		pfClearColor/*Colors::Azure*/, 0, NULL);
	//원하는 색상으로 렌더 타겟(뷰)을 지운다.

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//깊이-스텐실 서술자의 CPU 주소를 계산한다.

	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	//원하는 값으로 깊이-스텐실(뷰)을 지운다. 
	// (전체 영역을 클리어 한다는 의미) 
	// 이전 프레임에서 사용된 깊이/스텐실 값이 남아 있으면 다음 프레임에 렌더링 오류나 불필요한 가림 현상이 생깁니다.

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE,
		&d3dDsvCPUDescriptorHandle);
	//렌더 타겟 뷰(서술자)와 깊이-스텐실 뷰(서술자)를 출력-병합 단계(OM)에 연결한다.
	//렌더링 코드는 여기에 추가될 것이다.

	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
	/*현재 렌더 타겟에 대한 렌더링이 끝나기를 기다린다. GPU가 렌더 타겟(버퍼)을 더 이상 사용하지 않으면 렌더 타겟
	의 상태는 프리젠트 상태(D3D12_RESOURCE_STATE_PRESENT)로 바뀔 것이다.*/

	hResult = m_pd3dCommandList->Close();
	//명령 리스트를 닫힌 상태로 만든다.
	// 리스트가 열려있으면 상태 리스트를 실행할 수 없다.

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	//명령 리스트를 명령 큐에 추가하여 실행한다.

	WaitForGpuComplete();
	//GPU가 모든 명령 리스트를 실행할 때 까지 기다린다.

	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;      // 업데이트할 영역 수 (0이면 전체)
	dxgiPresentParameters.pDirtyRects = NULL;       // 업데이트할 영역들 (NULL이면 전체 화면)
	dxgiPresentParameters.pScrollRect = NULL;       // 스크롤 최적화 안 함
	dxgiPresentParameters.pScrollOffset = NULL;     // 스크롤 오프셋도 없음
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
	/*스왑체인을 프리젠트한다. 프리젠트를 하면 현재 렌더 타겟(후면버퍼)의 내용이 전면버퍼로 옮겨지고 렌더 타겟 인
	덱스가 바뀔 것이다.*/

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	// 다음에 쓸 후면 버퍼의 인덱스를 알려주는 코드 ( 현재 그릴 차례가 된 후면 버퍼의 인덱스 )

	/*현재의 프레임 레이트를 문자열로 가져와서 주 윈도우의 타이틀로 출력한다. m_pszBuffer 문자열이
	"LapProject ("으로 초기화되었으므로 (m_pszFrameRate+12)에서부터 프레임 레이트를 문자열로 출력
	하여 “ FPS)” 문자열과 합친다.
	::_itow_s(m_nCurrentFrameRate, (m_pszFrameRate+12), 37, 10);
	::wcscat_s((m_pszFrameRate+12), 37, _T(" FPS)"));
	*/
	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}