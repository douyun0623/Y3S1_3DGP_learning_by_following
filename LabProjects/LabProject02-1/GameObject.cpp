#include "stdafx.h"
#include "GameObject.h"

// CGameObject 클래스의 소멸자를 다음과 같이 정의한다.
// CGameObject::~CGameObject(void)
//{
//    //이 게임 객체는 더 이상 메쉬를 참조하지 않으므로 메쉬의 참조값을 1 감소한다.
//    if (m_pMesh) m_pMesh->Release();
//}

// 당 오브젝트의 회전, 이동, 스케일 등을 월드 좌표계로 변환
CPoint3D CGameObject::WorldTransform(CPoint3D& f3Model)
{
    float fPitch = DegreeToRadian(m_fxRotation);
    float fYaw = DegreeToRadian(m_fyRotation);
    float fRoll = DegreeToRadian(m_fzRotation);
    CPoint3D f3World = f3Model;
    CPoint3D f3Rotated = f3Model;
    //회전 변환
    if (fPitch != 0.0f)
    {
        f3Rotated.y = float(f3World.y * cos(fPitch) - f3World.z * sin(fPitch));
        f3Rotated.z = float(f3World.y * sin(fPitch) + f3World.z * cos(fPitch));
        f3World.y = f3Rotated.y;
        f3World.z = f3Rotated.z;
    }
    if (fYaw != 0.0f)
    {
        f3Rotated.x = float(f3World.x * cos(fYaw) + f3World.z * sin(fYaw));
        f3Rotated.z = float(-f3World.x * sin(fYaw) + f3World.z * cos(fYaw));
        f3World.x = f3Rotated.x;
        f3World.z = f3Rotated.z;
    }
    if (fRoll != 0.0f)
    {
        f3Rotated.x = float(f3World.x * cos(fRoll) - f3World.y * sin(fRoll));
        f3Rotated.y = float(f3World.x * sin(fRoll) + f3World.y * cos(fRoll));
        f3World.x = f3Rotated.x;
        f3World.y = f3Rotated.y;
    }
    //평행 이동 변환
    f3World.x += m_fxPosition;
    f3World.y += m_fyPosition;
    f3World.z += m_fzPosition;
    return(f3World);
}

// 애니메이션을 적용하는 함수로,
// 게임 오브젝트의 상태나 속성을 시간에 따라 변경하는데 사용
// 지금은 시간에 따라 회전
void CGameObject::Animate(float fElapsedTime)
{
    Rotate(m_fxRotationSpeed * fElapsedTime, m_fyRotationSpeed * fElapsedTime, 
            m_fzRotationSpeed * fElapsedTime);
}

// ::의 의미
// 전역 네임스페이스 범위
// 왜 ::를 사용하냐면: 이 코드에서 CreatePen이나 SelectObject라는 이름의 함수가 
// 코드 내에서 다른 용도로 사용될 가능성도 있기 때문에, 
// 확실하게 WinAPI의 함수들을 호출한다는 것을 명시하는 것입니다.
void CGameObject::Render(HDC hDCFrameBuffer)
{
    HPEN hPen = ::CreatePen(PS_SOLID, 0, m_dwColor);
    HPEN hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);

    if (m_pMesh) {
        m_pMesh->Render(hDCFrameBuffer);
    }

    ::SelectObject(hDCFrameBuffer, hOldPen);
    ::DeleteObject(hPen);
}
