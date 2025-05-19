#pragma once

#include "GameObject.h"	
#include "Camera.h"	

// �÷��̾� ��ü�� ���� ��ü�̹Ƿ� CGameObject Ŭ�������� ����Ͽ� Ŭ������ �����Ѵ�.
class CPlayer : public CGameObject
{
public:
	CPlayer() = default;
	virtual ~CPlayer() = default;

private:
	// �÷��̾� ��ü�� ���Ե� ī�޶��̴�.
	std::unique_ptr<CCamera> m_pCamera{};

public:
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);

	void Move(float x, float y, float z);
	void Rotate(float fPitch, float fYaw, float fRoll);

	void SetCamera(std::unique_ptr<CCamera> pCamera) { m_pCamera = std::move(pCamera); }

	// **CCamera*** �� �����ϴ� ������ ī�޶� ��ü�� ���� ������ �����ϰ�,(
	// �������� �������� �ʱ� �����Դϴ�.
	CCamera* GetCamera() { return m_pCamera.get(); }
};

