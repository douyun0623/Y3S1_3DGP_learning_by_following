#include "stdafx.h"
#include "Scene.h"
#include "GraphicsPipeline.h"

// OE “CScene” 클래스의 BuildObjects()와 ReleaseObjects() 함수를 다음과 같이 정의한다.
void CScene::BuildObjects()
{
    auto pCubeMesh = std::make_shared<CCubeMesh>(8.0f, 8.0f, 8.0f);

    // 객체 1
    {
        auto pGameObject = std::make_unique<CGameObject>();
        pGameObject->SetMesh(pCubeMesh);   // 메쉬를 여러 오브젝트가 공유한다고 가정
        pGameObject->SetPosition(-8.5f, 0.0f, -14.0f);
        pGameObject->SetRotation(0.0f, 0.0f, 0.0f);
        pGameObject->SetRotationSpeed(5.0f, 30.0f, 9.0f);
        pGameObject->SetColor(RGB(255, 0, 0));
        m_ppObjects.push_back(std::move(pGameObject));
    }

    // 객체 2
    {
        auto pGameObject = std::make_unique<CGameObject>();
        pGameObject->SetMesh(pCubeMesh);
        pGameObject->SetPosition(+8.5f, 0.0f, -14.0f);
        pGameObject->SetRotation(0.0f, 0.0f, 0.0f);
        pGameObject->SetRotationSpeed(30.0f, 9.0f, 5.0f);
        pGameObject->SetColor(RGB(0, 0, 255));
        m_ppObjects.push_back(std::move(pGameObject));
    }
}


void CScene::Animate(float fElapsedTime)
{
    for (auto& object : m_ppObjects)
    {
        object->Animate(fElapsedTime);
    }
}

// CScene 클래스의 Render() 함수를 다음과 같이 정의한다.
void CScene::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
    //현재 카메라를 렌더링 파이프라인에 설정한다.
    if (pCamera) CGraphicsPipeline::SetCamera(pCamera);

    for (auto& object : m_ppObjects)
    {
        //현재 게임 객체를 렌더링 파이프라인에 설정한다.
        CGraphicsPipeline::SetGameObject(object.get());  // unique_ptr이므로 .get() 필요
        //현재 게임 객체를 렌더링한다.
        object->Render(hDCFrameBuffer);
    }
}