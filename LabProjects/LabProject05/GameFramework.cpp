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

// OnCreate() �Լ��� OnDestroy() �Լ��� ������ ���� �����Ѵ�.
// ���� �Լ��� ���� ���α׷��� ����Ǿ� �� �����찡 �����Ǹ� ȣ��ȴٴ� �Ϳ� �����϶�.
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
	//GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���.
	ReleaseObjects();
	//���� ��ü(���� ���� ��ü)�� �Ҹ��Ѵ�.
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

// CreateSwapChain() �Լ��� ������ ���� �����Ѵ�.
void CGameFramework::CreateSwapChain()
{
	// ���� ������ â�� Ŭ���̾�Ʈ ���� (����ǥ����, �׵θ� ������ ���� ����) ũ�⸦ ����
	// �� ũ�⸦ ������� ���� ü���� ���� ũ�Ⱑ �����ȴ�.
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

// CreateDirect3DDisplay() �Լ��� ������ ���� �����Ѵ�.
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
	// DXGI ���丮 ���� 
	// gpu ����� ����, ���� ü�� ���� � �̿�ȴ�.
	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);
	
	// �ý����� gpu ����͵��� ��ȸ�ϸ鼭 ���ǿ� �´� ���� ã�´�.
	IDXGIAdapter1* pd3dAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;
	}
	//��� �ϵ���� ����� ���Ͽ� Ư�� ���� 12.0�� �����ϴ� �ϵ���� ����̽��� �����Ѵ�.
	
	// �ϵ���� ����̽��� ���� ��� ����Ѵ�.
	// warp �����
	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIAdapter1), (void**)&pd3dAdapter);
		D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}
	//Ư�� ���� 12.0�� �����ϴ� �ϵ���� ����̽��� ������ �� ������ WARP ����̽��� �����Ѵ�.


	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4; //Msaa4x ���� ���ø�
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	//����̽��� �����ϴ� ���� ������ ǰ�� ������ Ȯ���Ѵ�.
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;
	//���� ������ ǰ�� ������ 1���� ũ�� ���� ���ø��� Ȱ��ȭ�Ѵ�.


	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence),
		(void**)&m_pd3dFence);
	m_nFenceValue = 0;
	//�潺�� �����ϰ� �潺 ���� 0���� �����Ѵ�.

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	/*�潺�� ����ȭ�� ���� �̺�Ʈ ��ü�� �����Ѵ�(�̺�Ʈ ��ü�� �ʱⰪ�� FALSE�̴�). �̺�Ʈ�� ����Ǹ�(Signal) ��
	��Ʈ�� ���� �ڵ������� FALSE�� �ǵ��� �����Ѵ�.*/

	m_d3dViewport.TopLeftX = 0;
	m_d3dViewport.TopLeftY = 0;
	m_d3dViewport.Width = static_cast<float>(m_nWndClientWidth);
	m_d3dViewport.Height = static_cast<float>(m_nWndClientHeight);
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;
	//����Ʈ�� �� �������� Ŭ���̾�Ʈ ���� ��ü�� �����Ѵ�.
	m_d3dScissorRect = { 0, 0, m_nWndClientWidth, m_nWndClientHeight };
	//���� �簢���� �� �������� Ŭ���̾�Ʈ ���� ��ü�� �����Ѵ�.
	if (pd3dAdapter) pd3dAdapter->Release();
}


// CreateCommandQueueAndList() �Լ��� ������ ���� �����Ѵ�.
void CGameFramework::CreateCommandQueueAndList()
{
	// ZeroMemory ����ü�� 0���� �ʱ�ȭ -> �������� ���� ���ɼ� Ȯ��
	// Flags = None -> Ư�� �÷��� ���� �Ϲ� ť�� ����
	// type = direct -> direct ��� ť�� ���� ( �׷��� / ��� / ���� �۾� ��� ����)
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;	// ��� ť ���� ����ü 
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;


	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc,
		_uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);
	//����(Direct) ��� ť�� �����Ѵ�.

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);
	//����(Direct) ��� �Ҵ��ڸ� �����Ѵ�.

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList);
	//����(Direct) ��� ����Ʈ�� �����Ѵ�.

	hResult = m_pd3dCommandList->Close();
	// ��� ����Ʈ�� �����Ǹ� ����(Open) �����̹Ƿ� ����(Closed) ���·� �����.
	// 
	// ��� ����Ʈ�� �����Ҷ� �ݵ�� "���� ����"���� �Ѵ�.
	// open : ��� ����� �� ���� ( ����� �׸� ���� ���� -> �ϴ� �����)
	// closed : ��� ����� �� ����, submit ����
	//
	// ���� ������ ����
	// reset -> draw -> close -> ����

	// Command Queue	 : gpu�� ��� ����Ʈ���� �����ϴ� â��
	// command Allocator : ��� ����Ʈ�� ���������� ��ɾ ������ ���� ����
	// Command List		 : ����� ��ɵ��� ����ϴ� ��ü
}

// CreateRtvAndDsvDescriptorHeaps() �Լ��� ������ ���� �����Ѵ�.
void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	// ��ũ����(Descriptor)�� GPU�� ���ҽ�(����, �ؽ�ó ��)�� ������ �� �ֵ��� ������ ���� ����ü�Դϴ�.
	// **��ũ���� ��(Descriptor Heap)** �� �׷� ��ũ���͵��� ���������� �����ϴ� �޸� �����Դϴ�.
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;	// ����ü�ο� ���۰� 2���� rtv�� 2�� �ʿ�
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // rtv ���� Ÿ��
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // ���̴����� �������� �����Ƿ� �÷��� ����
	d3dDescriptorHeapDesc.NodeMask = 0;


	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	//���� Ÿ�� ������ ��(�������� ������ ����ü�� ������ ����)�� �����Ѵ�.

	m_nRtvDescriptorIncrementSize =
		m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//���� Ÿ�� ������ ���� ������ ũ�⸦ �����Ѵ�.

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	//����-���ٽ� ������ ��(�������� ������ 1)�� �����Ѵ�.
	m_nDsvDescriptorIncrementSize =
		m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//����-���ٽ� ������ ���� ������ ũ�⸦ �����Ѵ�.
}

// CreateRenderTargetViews()�� CreateDepthStencilView() �Լ��� ������ ���� �����Ѵ�.
// ����ü���� �� �ĸ� ���ۿ� ���� ���� Ÿ�� �並 �����Ѵ�.
void CGameFramework::CreateRenderTargetViews()
{
	// RTV ��ũ���� ���� ���� �ּҸ� �����ɴϴ�. 		
	// RTV�� ������ ��ġ�� ù �����͸� �غ��մϴ�.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		// 1. ���� ü���� i��° ���۸� ������ (�� ����)
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dRenderTargetBuffers[i]);
		
		// 2. �ش� ���ۿ� ���� RTV ����
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i], NULL,
			d3dRtvCPUDescriptorHandle);

		// 3. ���� RTV ������ ���� ��ũ���� �����͸� ����
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
}


// ���� �׽�Ʈ : �ȼ��� ȭ�鿡 ��µǱ� ���� �󸶳� �ָ� �ִ����� �Ǵ�.
// ���ٽ� �׽�Ʈ : �ȼ� �������� �����ϴ� �߰� ������ ������ �� ���.
void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2d �ؽ�ó ���ҽ� ���� (���� ȭ��� ���� ��2d)
	d3dResourceDesc.Alignment = 0; // ���ҽ��� �޸� ���� ���� (�⺻ ���� ���)
	d3dResourceDesc.Width = m_nWndClientWidth;	 // â�� ũ�⿡ �°� ���� ���۸� ����
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;	// ���ҽ��� �Ѱ��� 2d �ؽ�ó��� �ǹ��̴�. (�迭 �ؽ�ó�� �ƴϹǷ� 1�� ����)
	d3dResourceDesc.MipLevels = 1;	// �θ� ���� �� �ǹ� ( ���� ���۴� �ʿ���� -> 1�� ����)
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// ���ҽ� ������ �����մϴ�. 24 ����, 8 ���ٽ�
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;	// msaa ���� 4 �ƴϸ� 1(�Ⱦ�)
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;	// m_nMsaa4xQualityLevels�� ����̽��� �����ϴ� ǰ�� ��ġ�̸�, -1�� ���� ���� ǰ�� ����
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;	// �޸� ���̾ƿ��� �ý��ۿ� �ñ�
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // �� ���ҽ��� ����/���ٽǿ����� ����� ���̴� ��� ���
	
	
	D3D12_HEAP_PROPERTIES d3dHeapProperties; // �� �Ӽ� ���� (���ҽ��� ��� �ø���)
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;	// unkown���� �ΰ� �ڵ�
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;		// unkown���� �ΰ� �ڵ�
	d3dHeapProperties.CreationNodeMask = 1;	// �̱� gpu ���� 1
	d3dHeapProperties.VisibleNodeMask = 1;	// �̱� gpu ���� 1


	D3D12_CLEAR_VALUE d3dClearValue;	// �ʱ�ȭ�� ����� �⺻��
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;	// �⺻ ���̰� (1.0 = ���� �ָ�)
	d3dClearValue.DepthStencil.Stencil = 0;		// �⺻ ���ٽ� ��

	// ���� ���� - ���ٽ� ���� ���ҽ��� GPU�� �����մϴ�.
	m_pd3dDevice->CreateCommittedResource(
		&d3dHeapProperties,						// �� �Ӽ�
		D3D12_HEAP_FLAG_NONE,					// Ư���� �� �÷��� ����
		&d3dResourceDesc,						// ���ҽ� �Ӽ�
		D3D12_RESOURCE_STATE_DEPTH_WRITE,		// �ʱ� ����: ���� ����
		&d3dClearValue,							// Ŭ���� ��
		__uuidof(ID3D12Resource),				// �������̽� ID
		(void**)&m_pd3dDepthStencilBuffer);		// ��� ����
	//����-���ٽ� ���۸� �����Ѵ�.


	// ����/���ٽ� �並 ���� ��ũ���� ���� ���� ��ġ�� �����ɴϴ�.
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();


	m_pd3dDevice->CreateDepthStencilView(
		m_pd3dDepthStencilBuffer,	// ��� ����
		NULL,						// �⺻ �� ����
		d3dDsvCPUDescriptorHandle);	// ��ũ���� �ڵ� ��ġ
	//����-���ٽ� ���� �並 �����Ѵ�.
}


// BuildObjects(), ReleaseObjects(), OnProcessingWindowMessage(), ProcessInput(),
// AnimateObjects(), OnProcessingMouseMessage(), OnProcessingKeyboardMessage() �Լ��� ������
// ���� �����Ѵ�.
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
			//��F9�� Ű�� �������� ������ ���� ��üȭ�� ����� ��ȯ�� ó���Ѵ�.
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

// WaitForGpuComplete() �Լ��� ������ ���� �����Ѵ�.
// CPU�� GPU�� �۾��� ���� ������ ��ٸ��� �Լ��Դϴ�.
void CGameFramework::WaitForGpuComplete()
{
	m_nFenceValue++;	// CPU�� ����� ���� ������ 1�� ����
	//CPU �潺�� ���� �����Ѵ�.

	const UINT64 nFence = m_nFenceValue;	// �� ���� GPU�� �Ϸ��ؾ� �� ��ǥ���� �˴ϴ�.
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFence);	// ��� ť�� �潺�� �����϶�(Signal) �� ����� �߰��մϴ�.
	// GPU�� �ռ� ��� ����� �Ϸ��� �Ŀ� �� �潺�� nFence�� �����մ�
	// GPU�� �潺�� ���� �����ϴ� ����� ��� ť�� �߰��Ѵ�.

	// GPU�� nFence���� �۾��� ��ġ�� OS �̺�Ʈ(m_hFenceEvent)�� ��ȣ(Signal) �ϵ��� �����մϴ�.
	if (m_pd3dFence->GetCompletedValue() < nFence)
	{
		//�潺�� ���� ���� ������ ������ ������ �潺�� ���� ���� ������ ���� �� ������ ��ٸ���.
		hResult = m_pd3dFence->SetEventOnCompletion(nFence, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
		// ������ ������ �̺�Ʈ�� Signal ���°� �� ������ ���� ���(INFINITE) �մϴ�.
		// ��, GPU�� �۾��� ��ĥ ������ CPU�� ���⼭ ���缭 ��ٸ��ϴ�.
	}
}

// FrameAdvance() �Լ��� ������ ���� �����Ѵ�.
// �Է� ó�� �� �ִϸ��̼� ó�� �� ������ �غ� �� �׸��� �� ���� Ÿ�� ���� ��ȯ �� GPU�� ���� �� GPU ��� �� ȭ�� ���(Present)
void CGameFramework::FrameAdvance()
{
	// CGameFramework�� Ŭ������ FrameAdvance() ��� �Լ��� ������ ���� �����Ѵ�.
	// Ÿ�̸��� �ð��� ���ŵǵ��� �ϰ� ������ ����Ʈ�� ����Ѵ�.
	m_GameTimer.Tick(0.0f);

	// �Է� �� �ִϸ��̼� ó��
	ProcessInput();
	AnimateObjects();

	// ��� ��ü �ʱ�ȭ
	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
	//��� �Ҵ��ڿ� ��� ����Ʈ�� �����Ѵ�.

	// �ĸ� ���۴� Present ���¿��µ�, ���� �������� ����Ϸ��� Render Target ���·� �ٲ�� �մϴ�.
	// �̰� Resource Barrier��� �մϴ�.
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
	/*���� ���� Ÿ�ٿ� ���� ������Ʈ�� �����⸦ ��ٸ���. ������Ʈ�� ������ ���� Ÿ�� ������ ���´� ������Ʈ ����
	(D3D12_RESOURCE_STATE_PRESENT)���� ���� Ÿ�� ����(D3D12_RESOURCE_STATE_RENDER_TARGET)�� ��
	�� ���̴�.*/

	m_pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	m_pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
	//����Ʈ�� ���� �簢���� �����Ѵ�.

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex *	m_nRtvDescriptorIncrementSize);
	//������ ���� Ÿ�ٿ� �ش��ϴ� �������� CPU �ּ�(�ڵ�)�� ����Ѵ�.

	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	// float pfClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle,
		pfClearColor/*Colors::Azure*/, 0, NULL);
	//���ϴ� �������� ���� Ÿ��(��)�� �����.

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//����-���ٽ� �������� CPU �ּҸ� ����Ѵ�.

	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	//���ϴ� ������ ����-���ٽ�(��)�� �����. 
	// (��ü ������ Ŭ���� �Ѵٴ� �ǹ�) 
	// ���� �����ӿ��� ���� ����/���ٽ� ���� ���� ������ ���� �����ӿ� ������ ������ ���ʿ��� ���� ������ ����ϴ�.

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE,
		&d3dDsvCPUDescriptorHandle);
	//���� Ÿ�� ��(������)�� ����-���ٽ� ��(������)�� ���-���� �ܰ�(OM)�� �����Ѵ�.
	//������ �ڵ�� ���⿡ �߰��� ���̴�.

	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
	/*���� ���� Ÿ�ٿ� ���� �������� �����⸦ ��ٸ���. GPU�� ���� Ÿ��(����)�� �� �̻� ������� ������ ���� Ÿ��
	�� ���´� ������Ʈ ����(D3D12_RESOURCE_STATE_PRESENT)�� �ٲ� ���̴�.*/

	hResult = m_pd3dCommandList->Close();
	//��� ����Ʈ�� ���� ���·� �����.
	// ����Ʈ�� ���������� ���� ����Ʈ�� ������ �� ����.

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	//��� ����Ʈ�� ��� ť�� �߰��Ͽ� �����Ѵ�.

	WaitForGpuComplete();
	//GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���.

	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;      // ������Ʈ�� ���� �� (0�̸� ��ü)
	dxgiPresentParameters.pDirtyRects = NULL;       // ������Ʈ�� ������ (NULL�̸� ��ü ȭ��)
	dxgiPresentParameters.pScrollRect = NULL;       // ��ũ�� ����ȭ �� ��
	dxgiPresentParameters.pScrollOffset = NULL;     // ��ũ�� �����µ� ����
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
	/*����ü���� ������Ʈ�Ѵ�. ������Ʈ�� �ϸ� ���� ���� Ÿ��(�ĸ����)�� ������ ������۷� �Ű����� ���� Ÿ�� ��
	������ �ٲ� ���̴�.*/

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	// ������ �� �ĸ� ������ �ε����� �˷��ִ� �ڵ� ( ���� �׸� ���ʰ� �� �ĸ� ������ �ε��� )

	/*������ ������ ����Ʈ�� ���ڿ��� �����ͼ� �� �������� Ÿ��Ʋ�� ����Ѵ�. m_pszBuffer ���ڿ���
	"LapProject ("���� �ʱ�ȭ�Ǿ����Ƿ� (m_pszFrameRate+12)�������� ������ ����Ʈ�� ���ڿ��� ���
	�Ͽ� �� FPS)�� ���ڿ��� ��ģ��.
	::_itow_s(m_nCurrentFrameRate, (m_pszFrameRate+12), 37, 10);
	::wcscat_s((m_pszFrameRate+12), 37, _T(" FPS)"));
	*/
	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}