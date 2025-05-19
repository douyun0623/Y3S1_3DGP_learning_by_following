#pragma once

#include <memory>

// �������� ���⿡ include ���ߴµ� FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT�� ������
// �����;��Ұ� ���Ƽ� ���⿡ �߰���
// ������ cpp���Ͽ� �߰��ȴ�. 
#include "stdafx.h"
#include "Mesh.h"

class CViewport
{
public:
	//�̴� ���ʿ��� �⺻�� �ʱ�ȭ�� ���� ������ ���� �� �ֽ��ϴ�.
	// const ��� ������ ������ ��� ������ ���� �������� �ʱ�ȭ�� �� ���� ������, 
	// �ݵ�� �ʱ�ȭ ����Ʈ�� ����ؾ� �մϴ�.
	// �����Ϸ� ����ȭ�� �� �̷������ ������ ���ɻ� ����
	// ���ʿ��� �⺻ �ʱ�ȭ�� �߻����� ������, �̴� ���� �� ����
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
	virtual ~CCamera() = default;	// ����Ʈ �����Ͱ� �ڵ����� �����ϹǷ� Ư���� �Ҹ��ڴ� �ʿ� ����
	// { if (m_pViewport) delete m_pViewport; }

private:
	//ī�޶��� ��ġ(���� ��ǥ��)
	float m_fxPosition = 0.0f;
	float m_fyPosition = 0.0f;
	float m_fzPosition = 0.0f;
	//ī�޶��� ȸ��(ī�޶� ��ǥ��)
	float m_fxRotation = 0.0f;
	float m_fyRotation = 0.0f;
	float m_fzRotation = 0.0f;
	//ī�޶��� �þ߰�, ���� �簢�������� �Ÿ�
	float m_fFOVAngle = 90.0f;
	float m_fProjectRectDistance = 1.0f;
	//����Ʈ
	// ����Ʈ �����ͷ� ���� �� ī�޶� ��ü�� �Ҹ��Ҷ� ���� �����
	// CViewport* m_pViewport = NULL;
	std::unique_ptr<CViewport> m_pViewport{};
	//����Ʈ�� ���� ���̿� ���� ������ ����(��Ⱦ��: Aspect ratio)
	// constepr�� ����� float(FRAMEBUFFER_WIDTH)�̷��� ���� �ʾƵ� �ȴ�. 
	float m_fAspectRatio = FRAMEBUFFER_WIDTH / FRAMEBUFFER_HEIGHT;
public:
	//ī�޶� ��ȯ, ���� ��ȯ, ȭ�� ��ȯ�� �����Ѵ�.
	CPoint3D CameraTransform(CPoint3D& f3World);
	CPoint3D ProjectionTransform(CPoint3D& f3Camera);
	CPoint3D ScreenTransform(CPoint3D& f3Projection);

	// ī�޶��� ��ġ ����
	void SetPosition(float x, float y, float z) {
		m_fxPosition = x;
		m_fyPosition = y; 
		m_fzPosition = z;
	}
	// ī�޶��� ȸ�� ����
	void SetRotation(float fPitch, float fYaw, float fRoll) {
		m_fxRotation = fPitch; 
		m_fyRotation = fYaw; 
		m_fzRotation = fRoll;
	}
	//ī�޶��� ����Ʈ�� �þ߰��� �����Ѵ�.
	void SetViewport(int xStart, int yStart, int nWidth, int nHeight);
	void SetFOVAngle(float fFOVAngle);

	//ī�޶� �̵��ϰ� ȸ���Ѵ�.
	void Move(float x, float y, float z);
	void Rotate(float fPitch, float fYaw, float fRoll);
};

