#pragma once

// 프레임 레이트(Frame Rate)의 계산은 일정한 횟수 동안 각 프레임을 렌더링하는 시간을
// 평균하여 계산한다.이 횟수를 나타내는 상수를 정의한다.
const ULONG MAX_SAMPLE_COUNT = 50; // 50회의 프레임 처리시간을 누적하여 평균한다.

class CGameTimer
{
public:
	CGameTimer();
	virtual ~CGameTimer();

	// 클래스의 멤버 함수를 다음과 같이 선언한다.
	void Start() {}
	void Stop() {}
	void Reset();
	void Tick(float fLockFPS = 0.0f);
	unsigned long GetFrameRate(LPTSTR lpszString = NULL, int nCharacters = 0);
	float GetTimeElapsed();
	// Tick 타이머의 시간을 갱신한다.
	// GetFrameRate 프레임 레이트를 반환한다.
	// GetTimeElapsed 프레임의 평균 경과 시간을 반환한다.

	// 클래스의 멤버 변수를 다음과 같이 선언한다.
private:
	bool m_bHardwareHasPerformanceCounter;
	float m_fTimeScale;
	float m_fTimeElapsed;
	__int64 m_nCurrentTime;
	__int64 m_nLastTime;
	__int64 m_nPerformanceFrequency;
	float m_fFrameTime[MAX_SAMPLE_COUNT];
	ULONG m_nSampleCount;
	unsigned long m_nCurrentFrameRate;
	unsigned long m_nFramesPerSecond;
	float m_fFPSTimeElapsed;
	bool m_bStopped;
	// m_bHardwareHasPerformanceCounter 컴퓨터가 Performance Counter를 가지고 있는 가
	// m_fTimeScale Scale Counter의 양
	// m_fTimeElapsed 마지막 프레임 이후 지나간 시간
	// m_nCurrentTime 현재의 시간
	// m_nLastTime 마지막 프레임의 시간
	// m_nPerformanceFrequency 컴퓨터의 Performance Frequency
	// m_fFrameTime 프레임 시간을 누적하기 위한 배열
	// m_nSampleCount 누적된 프레임 횟수
	// m_nCurrentFrameRate 현재의 프레임 레이트
	// m_nFramesPerSecond 초당 프레임 수
	// m_fFPSTimeElapsed 프레임 레이트 계산 소요 시간
};