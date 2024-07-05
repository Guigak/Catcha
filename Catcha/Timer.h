#pragma once
#include "common.h"

class Timer {
private:
	bool m_performance_counter;

	double m_seconds_per_count;
	double m_elapsed_time;

	__int64 m_base_time;
	__int64 m_paused_time;
	__int64 m_stop_time;
	__int64 m_previous_time;
	__int64 m_current_time;

	bool m_stopped;

public:
	Timer();
	~Timer() {}

	float Get_Total_Time();
	float Get_Elapsed_Time();

	void Reset();
	void Start();
	void Stop();
	void Tick();
};

