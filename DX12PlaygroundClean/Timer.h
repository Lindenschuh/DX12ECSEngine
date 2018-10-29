#pragma once
#include "Default.h"

class GameTimer
{
public:
	float mDeltaTime;

	GameTimer();
	void Tick();
	void Reset();
	float GetGameTime();
private:
	s64 _baseTime;
	s64 _prevTime;
	s64 _currentTime;
	double _secoundsPerCount;
};

class StopWatch
{
public:

	StopWatch();
	float GetDuration();
	void Start();
	void Stop();
	void Reset();
private:
	bool _isStopped;
	double _secoundsPerCount;
	s64 _startTime;
	s64 _endTime;
};