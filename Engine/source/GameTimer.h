#pragma once
#include "EngineHeader.h"

#ifndef GAMETIMER_H
#define GAMETINER_H
namespace Engine 
{
	class D3D_API GameTimer
	{
	public:
		GameTimer();

		float TotalTime() const; // 애플리케이션 시작 이후 경과된 전체 시간(초)
		float DeltaTime() const; // 마지막 프레임 이후 경과된 시간(초)

		void Reset();	// 타이머 리셋
		void Start();	// 타이머 시작
		void Stop();	// 타이머 정지
		void Tick();	// 매 프레임마다 호출되어, 시간 갱신

	private:
		double mSecondsPerCount; // 카운터 당 초
		double mDeltaTime;       // 마지막 프레임 이후 경과된 시간(초)
		__int64 mBaseTime;       // 타이머가 시작된 시점
		__int64 mPausedTime;     // 타이머가 정지된 동안의 누적 시간
		__int64 mStopTime;       // 타이머가 마지막으로 정지된 시점
		__int64 mPrevTime;       // 직전 Tick 함수가	호출된 시점
		__int64 mCurrTime;       // 현재 Tick 함수가 호출된 시점

		bool mStopped;           // 타이머 정지 여부
	};
}
#endif // GAMETIMER_H



