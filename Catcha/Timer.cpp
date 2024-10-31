#include "Timer.h"

Timer::Timer() {
    __int64 counts_per_second;

    if (QueryPerformanceFrequency((LARGE_INTEGER*)&counts_per_second)) {
        m_performance_counter = true;
        m_seconds_per_count = 1.0 / (double)counts_per_second;
    }
    else {
        m_performance_counter = false;
        m_seconds_per_count = 0.001;
    }

    m_elapsed_time = -1.0;

    m_base_time = 0;
    m_paused_time = 0;
    m_stop_time = 0;
    m_previous_time = 0;
    m_current_time = 0;

    m_stopped = false;
}

float Timer::Get_Total_Time() {
    if (m_stopped) {
        return (float)(((m_stop_time - m_paused_time) - m_base_time) * m_seconds_per_count);
    }
    else {
        return (float)(((m_current_time - m_paused_time) - m_base_time) * m_seconds_per_count);
    }
}

float Timer::Get_Elapsed_Time() {
    return (float)m_elapsed_time;
}

void Timer::Reset() {
    __int64 reset_time;

    if (m_performance_counter) {
        QueryPerformanceCounter((LARGE_INTEGER*)&reset_time);
    }
    else {
        reset_time = timeGetTime();
    }

    m_base_time = reset_time;
    m_previous_time = reset_time;
    m_stop_time = 0;
    m_stopped = false;
}

void Timer::Start() {
    __int64 start_time;

    if (m_performance_counter) {
        QueryPerformanceCounter((LARGE_INTEGER*)&start_time);
    }
    else {
        start_time = timeGetTime();
    }

    if (m_stopped) {
        m_paused_time += start_time - m_stop_time;

        m_previous_time = start_time;
        m_stop_time = 0;
        m_stopped = false;
    }
}

void Timer::Stop() {
	if (!m_stopped) {
        __int64 current_time;

        if (m_performance_counter) {
            QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
        }
        else {
            current_time = timeGetTime();
        }

        m_stop_time = current_time;
        m_stopped = true;
	}
}

void Timer::Tick(float fps_limit) {
    if (m_stopped) {
        m_elapsed_time = 0.0;

        return;
    }

    __int64 current_time;

    if (m_performance_counter) {
        QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
    }
    else {
        current_time = timeGetTime();
    }

    m_current_time = current_time;
    m_elapsed_time = (m_current_time - m_previous_time) * m_seconds_per_count;

    if (fps_limit > 0.0f) {
        while (m_elapsed_time < (1.0f / fps_limit)) {
            if (m_performance_counter) {
                QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
            }
            else {
                current_time = timeGetTime();
            }

            m_current_time = current_time;
            m_elapsed_time = float((m_current_time - m_previous_time) * m_seconds_per_count);
        }
    }

    m_previous_time = m_current_time;

    if (m_elapsed_time < 0.0) {
        m_elapsed_time = 0.0;
    }
}
