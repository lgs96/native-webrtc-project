#ifndef RTC_STATS_COLLECTOR_H_
#define RTC_STATS_COLLECTOR_H_

#include <fstream>
#include <mutex>
#include <string>
#include <vector>
#include "api/peer_connection_interface.h"
#include "api/stats/rtc_stats.h"
#include "api/stats/rtc_stats_collector_callback.h"
#include "rtc_base/thread.h"
#include <thread>
#include <condition_variable>


// Add persistent stats structure
struct PersistentStats {
    int64_t frame_timing_count_ = 0;
    int64_t last_render_time_ms_ = -1;
    int64_t last_timestamp_ = -1;
};

class RTCStatsCollectorCallback : public webrtc::RTCStatsCollectorCallback {
public:
    RTCStatsCollectorCallback(
        std::ofstream& per_frame_stats_file,
        std::ofstream& average_stats_file,
        std::mutex& stats_mutex,
        PersistentStats& persistent_stats);  // Add persistent stats
    ~RTCStatsCollectorCallback();


protected:
    void OnStatsDelivered(
        const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) override;

private:
    void OnStatsDeliveredOnSignalingThread(
        rtc::scoped_refptr<const webrtc::RTCStatsReport> report);
    
    void ProcessInboundRTPStats(const webrtc::RTCStats& stats);

    std::ofstream& per_frame_stats_file_;
    std::ofstream& average_stats_file_;
    std::mutex& stats_mutex_;
    PersistentStats& persistent_stats_;  // Reference to persistent stats

    const int kFrameTimingLogCount = 60; // 60 frames per second
};

class RTCStatsCollector {
public:
    RTCStatsCollector();
    ~RTCStatsCollector();

    bool Start(const std::string& filename,
               rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection);
    void Stop();

    bool IsRunning () { return is_running_;}

private:
    void CollectStats();
    void ThreadLoop();

    bool OpenStatsFile(const std::string& filename);
    void CloseStatsFile();

    std::ofstream per_frame_stats_file_;
    std::ofstream average_stats_file_;

    std::thread stats_thread_;          // Use std::thread instead of rtc::Thread
    std::mutex stats_mutex_;            // Mutex for thread safety
    std::condition_variable stop_cv_;   // To signal the thread to stop
    bool should_collect_ = false;       // Flag to control the thread loop
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

    bool is_running_ = false;
    const int kStatsIntervalMs = 16; // Collection interval in milliseconds

    PersistentStats persistent_stats_;
};
#endif  // RTC_STATS_COLLECTOR_H_