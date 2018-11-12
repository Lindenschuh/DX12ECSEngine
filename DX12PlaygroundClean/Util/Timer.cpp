#include "Timer.h"

GameTimer::GameTimer()
	: _secoundsPerCount(0.0), mDeltaTime(-1.0), _baseTime(0),
	_prevTime(0), _currentTime(0)
{
	s64 countsPerSecound;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecound);
	_secoundsPerCount = 1.0 / (double)countsPerSecound;
	Reset();
}

void GameTimer::Tick()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&_currentTime);
	mDeltaTime = (float)(_currentTime - _prevTime) * _secoundsPerCount;
	_prevTime = _currentTime;
	if (mDeltaTime < 0.0)
		mDeltaTime = 0.0;
}

void GameTimer::Reset()
{
	s64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	_baseTime = currTime;
	_prevTime = currTime;
}

float GameTimer::GetGameTime()
{
	s64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	float gameTime = (float)(currTime - _baseTime) * _secoundsPerCount;
	return ((gameTime > 0.0f) ? gameTime : 0.0f);
}

StopWatch::StopWatch()
	: _secoundsPerCount(0.0), _endTime(0), _startTime(0)
{
	s64 countsPerSecound;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecound);
	_secoundsPerCount = 1.0 / (double)countsPerSecound;
}

float StopWatch::GetDuration()
{
	float duration;
	if (_isStopped)
	{
		duration = (float)(_endTime - _startTime) *_secoundsPerCount;
	}
	else
	{
		s64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		duration = (float)(currTime - _startTime) * _secoundsPerCount;
	}

	return ((duration > 0.0f) ? duration : 0.0f);
}

void StopWatch::Start()
{
	Reset();
	_isStopped = false;
}

void StopWatch::Stop()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&_endTime);
	_isStopped = true;
}

void StopWatch::Reset()
{
	s64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	_startTime = currTime;
	_endTime = currTime;
	_isStopped = true;
}