#pragma once


#include "GameObject.h"
#include "Camera.h"

#include <vector>
#include <memory>

class CScene
{
public:
	CScene() {}
	virtual ~CScene() {}
private:
	//���� ��ü���� ������ ���� ��ü���� ����Ʈ(�迭)�̴�.
	int m_nObjects = 0;
	// CGameObject** m_ppObjects = NULL;
	std::vector<std::unique_ptr<CGameObject>> m_ppObjects;
public:
	//���� ��ü���� �����ϰ� �Ҹ��Ѵ�.
	virtual void BuildObjects();
	// virtual void ReleaseObjects();
	//���� ��ü���� �ִϸ��̼��Ѵ�.
	virtual void Animate(float fElapsedTime);
	//���� ��ü���� �������Ѵ�.
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
};

