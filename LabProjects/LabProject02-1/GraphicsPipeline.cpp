#include "stdafx.h"
#include "GraphicsPipeline.h"

//��CGraphicsPipeline�� Ŭ������ ���� ����� ������ ���� �����Ѵ�.
CGameObject* CGraphicsPipeline::m_pGameObject = NULL;
CCamera* CGraphicsPipeline::m_pCamera = NULL;

//��CGraphicsPipeline�� Ŭ������ Project() �Լ��� ������ ���� �����Ѵ�.
//Project() �Լ��� �� ��ǥ���� ���� ���� ��ȯ, ī�޶� ��ȯ, ���� ���� ��ȯ�� ������
//���� �����Ѵ�.���� ��ȯ�� ���� ��ü�� ����(��ġ�� ����)�� �ʿ��ϴ�.ī�޶� ��ȯ��
//ī�޶��� ����(��ġ�� ����)�� �ʿ��ϴ�.���� ���� ��ȯ�� ī�޶��� ����(�þ߰��� ��Ⱦ
//	��)�� �ʿ��ϴ�.
CPoint3D CGraphicsPipeline::Project(CPoint3D& f3Model)
{
	CPoint3D f3World = m_pGameObject->WorldTransform(f3Model);
	CPoint3D f3Camera = m_pCamera->CameraTransform(f3World);
	CPoint3D f3Projection = m_pCamera->ProjectionTransform(f3Camera);
	return(f3Projection);
}

//CGraphicsPipeline�� Ŭ������ ScreenTransform() �Լ��� ������ ���� �����Ѵ�.
//ScreenTransform() �Լ��� ���� ��ǥ���� ���� ȭ�� ��ȯ�� �����Ѵ�.ȭ�� ��ȯ�� ī
//�޶��� ����(����Ʈ)�� �ʿ��ϴ�.
CPoint3D CGraphicsPipeline::ScreenTransform(CPoint3D& f3Projection)
{
	CPoint3D f3Screen = m_pCamera->ScreenTransform(f3Projection);
	return(f3Screen);
}