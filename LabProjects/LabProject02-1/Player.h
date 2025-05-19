#pragma once

#include "GameObject.h"	
#include "Camera.h"	

// 플레이어 객체도 게임 객체이므로 CGameObject 클래스에서 상속하여 클래스를 생성한다.
class CPlayer : public CGameObject
{
public:
	CPlayer() = default;
	virtual ~CPlayer() = default;

private:
	// 플레이어 객체에 포함된 카메라이다.
	std::unique_ptr<CCamera> m_pCamera{};

public:
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);

	void Move(float x, float y, float z);
	void Rotate(float fPitch, float fYaw, float fRoll);

	void SetCamera(std::unique_ptr<CCamera> pCamera) { m_pCamera = std::move(pCamera); }

	// **CCamera*** 를 리턴하는 이유는 카메라 객체에 대한 접근을 제공하고,(
	// 소유권을 이전하지 않기 위함입니다.
	CCamera* GetCamera() { return m_pCamera.get(); }
};

