#ifndef VIDEO_ANDROID_ANDROID_STATS_OBSERVER_H_
#define VIDEO_ANDROID_ANDROID_STATS_OBSERVER_H_

#include "webrtc/api/android/jni/jni_helpers.h"

#include "video/stats_observer.h"
#include "video/stats_report.h"

#include <memory>
#include <vector>


class AndroidStatsObserver: public twilio::video::StatsObserver {
public:
    AndroidStatsObserver(JNIEnv *env, jobject j_stats_observer):
        j_stats_observer_(env, j_stats_observer),
        j_stats_observer_class_(env, webrtc_jni::GetObjectClass(env, *j_stats_observer_)),
        j_array_list_class_(env, env->FindClass("java/util/ArrayList")),
        j_stats_report_class_(env, env->FindClass("com/twilio/video/StatsReport")),
        j_local_audio_track_stats_class_(env, env->FindClass("com/twilio/video/LocalAudioTrackStats")),
        j_local_video_track_stats_class_(env, env->FindClass("com/twilio/video/LocalVideoTrackStats")),
        j_audio_track_stats_class_(env, env->FindClass("com/twilio/video/AudioTrackStats")),
        j_video_track_stats_class_(env, env->FindClass("com/twilio/video/VideoTrackStats")),
        j_video_dimensions_class_(env, env->FindClass("com/twilio/video/VideoDimensions")),
        j_on_stats_id_(
            webrtc_jni::GetMethodID(env,
                                    *j_stats_observer_class_,
                                    "onStats",
                                    "(Ljava/util/List;)V")),
        j_array_list_ctor_id_(
            webrtc_jni::GetMethodID(env,
                                    *j_array_list_class_,
                                    "<init>",
                                    "()V")),
        j_array_list_add_(
            webrtc_jni::GetMethodID(env,
                                    *j_array_list_class_,
                                    "add",
                                    "(Ljava/lang/Object;)Z")),
        j_stats_report_ctor_id_(
            webrtc_jni::GetMethodID(env,
                                    *j_stats_report_class_,
                                    "<init>",
                                    "(Ljava/lang/String;)V")),
        j_stats_report_add_local_audio_id_(
            webrtc_jni::GetMethodID(env,
                                    *j_stats_report_class_,
                                    "addLocalAudioTrackStats",
                                    "(Lcom/twilio/video/LocalAudioTrackStats;)V")),
        j_stats_report_add_local_video_id_(
            webrtc_jni::GetMethodID(env,
                                    *j_stats_report_class_,
                                    "addLocalVideoTrackStats",
                                    "(Lcom/twilio/video/LocalVideoTrackStats;)V")),
        j_stats_report_add_audio_id_(
            webrtc_jni::GetMethodID(env,
                                    *j_stats_report_class_,
                                    "addAudioTrackStats",
                                    "(Lcom/twilio/video/AudioTrackStats;)V")),
        j_stats_report_add_video_id_(
            webrtc_jni::GetMethodID(env,
                                    *j_stats_report_class_,
                                    "addVideoTrackStats",
                                    "(Lcom/twilio/video/VideoTrackStats;)V")),
        j_local_audio_track_stats_ctor_id_(
            webrtc_jni::GetMethodID(env,
                                    *j_local_audio_track_stats_class_,
                                    "<init>",
                                    "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;DJIJIII)V")),
        j_local_video_track_stats_ctor_id_(
            webrtc_jni::GetMethodID(env,
                                    *j_local_video_track_stats_class_,
                                    "<init>",
                                    "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;DJIJLcom/twilio/video/VideoDimensions;Lcom/twilio/video/VideoDimensions;II)V")),
        j_audio_track_stats_ctor_id_(
            webrtc_jni::GetMethodID(env,
                                    *j_audio_track_stats_class_,
                                    "<init>",
                                    "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;DJIIII)V")),
        j_video_track_stats_ctor_id_(
            webrtc_jni::GetMethodID(env,
                                    *j_video_track_stats_class_,
                                    "<init>",
                                    "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;DJIILcom/twilio/video/VideoDimensions;I)V")),
        j_video_dimensions_ctor_id_(
            webrtc_jni::GetMethodID(env,
                                    *j_video_dimensions_class_,
                                    "<init>",
                                    "(II)V"))
        {

    }
    virtual ~AndroidStatsObserver() {
        TS_CORE_LOG_MODULE(twilio::video::kTSCoreLogModulePlatform,
                           twilio::video::kTSCoreLogLevelDebug,
                           "~AndroidStatsObserver");
    }

    void setObserverDeleted() {
        rtc::CritScope cs(&deletion_lock_);
        observer_deleted_ = true;
        TS_CORE_LOG_MODULE(twilio::video::kTSCoreLogModulePlatform,
                           twilio::video::kTSCoreLogLevelDebug,
                           "room observer deleted");
    }

protected:
    virtual void onStats(
            const std::vector<std::unique_ptr<twilio::video::StatsReport>> &stats_reports) {
        webrtc_jni::ScopedLocalRefFrame local_ref_frame(jni());
        std::string func_name = std::string(__FUNCTION__);
        TS_CORE_LOG_MODULE(twilio::video::kTSCoreLogModulePlatform,
                           twilio::video::kTSCoreLogLevelDebug,
                           "%s", func_name.c_str());

        // Create ArrayList<StatsReport>
        jobject j_stats_reports = jni()->NewObject(*j_array_list_class_, j_array_list_ctor_id_);
        for (auto const &stats_report : stats_reports) {
            webrtc_jni::ScopedLocalRefFrame stats_iteration_ref_frame(jni());
            jstring j_peerconnection_id =
                webrtc_jni::JavaStringFromStdString(jni(), stats_report->getPeerConnectionId());
            jobject j_stats_report = jni()->NewObject(*j_stats_report_class_,
                                                      j_stats_report_ctor_id_,
                                                      j_peerconnection_id);
            processLocalAudioTrackStats(j_stats_report,
                                        stats_report->getLocalAudioTrackStats());
            processLocalVideoTrackStats(j_stats_report,
                                        stats_report->getLocalVideoTrackStats());
            processAudioTrackStats(j_stats_report, stats_report->getAudioTrackStats());
            processVideoTrackStats(j_stats_report, stats_report->getVideoTrackStats());

            jni()->CallBooleanMethod(j_stats_reports, j_array_list_add_, j_stats_report);
        }

        {
            rtc::CritScope cs(&deletion_lock_);
            if (!isObserverValid(func_name)) {
                return;
            }

            jni()->CallVoidMethod(*j_stats_observer_, j_on_stats_id_, j_stats_reports);
        }
    }

private:
    JNIEnv *jni() {
        return webrtc_jni::AttachCurrentThreadIfNeeded();
    }

    bool isObserverValid(const std::string &callbackName) {
        if (observer_deleted_) {
            TS_CORE_LOG_MODULE(twilio::video::kTSCoreLogModulePlatform,
                               twilio::video::kTSCoreLogLevelWarning,
                               "stats observer is marked for deletion, skipping %s callback",
                               callbackName.c_str());
            return false;
        };
        if (webrtc_jni::IsNull(jni(), *j_stats_observer_)) {
            TS_CORE_LOG_MODULE(twilio::video::kTSCoreLogModulePlatform,
                               twilio::video::kTSCoreLogLevelWarning,
                               "stats observer reference has been destroyed, skipping %s callback",
                               callbackName.c_str());
            return false;
        }
        return true;
    }

    void processLocalAudioTrackStats(jobject j_stats_report,
                                     const std::vector<std::unique_ptr<twilio::media::LocalAudioTrackStats>> &local_audio_tracks_stats) {
        for(auto const &track_stats : local_audio_tracks_stats) {
            webrtc_jni::ScopedLocalRefFrame local_ref_frame(jni());
            jstring j_track_id =
                webrtc_jni::JavaStringFromStdString(jni(), track_stats->getTrackId());
            jstring j_codec_name =
                webrtc_jni::JavaStringFromStdString(jni(), track_stats->getCodecName());
            jstring j_ssrc =
                webrtc_jni::JavaStringFromStdString(jni(), track_stats->getSsrc());
            jobject j_local_audio_track_stats =
                jni()->NewObject(*j_local_audio_track_stats_class_,
                                 j_local_audio_track_stats_ctor_id_,
                                 j_track_id,
                                 track_stats->getPacketsLost(),
                                 j_codec_name,
                                 j_ssrc,
                                 track_stats->getUnixTimestamp(),
                                 track_stats->getBytesSent(),
                                 track_stats->getPacketsSent(),
                                 track_stats->getRoundTripTime(),
                                 track_stats->getAudioInputLevel(),
                                 track_stats->getJitterReceived(),
                                 track_stats->getJitterBufferMs());
            jni()->CallVoidMethod(j_stats_report,
                                  j_stats_report_add_local_audio_id_,
                                  j_local_audio_track_stats);
        }
    }

    void processLocalVideoTrackStats(jobject j_stats_report,
                                     const std::vector<std::unique_ptr<twilio::media::LocalVideoTrackStats>> &local_video_tracks_stats) {
        for(auto const &track_stats : local_video_tracks_stats) {
            webrtc_jni::ScopedLocalRefFrame local_ref_frame(jni());
            jstring j_track_id =
                webrtc_jni::JavaStringFromStdString(jni(), track_stats->getTrackId());
            jstring j_codec_name =
                webrtc_jni::JavaStringFromStdString(jni(), track_stats->getCodecName());
            jstring j_ssrc =
                webrtc_jni::JavaStringFromStdString(jni(), track_stats->getSsrc());
            jobject j_capture_dimensions =
                jni()->NewObject(*j_video_dimensions_class_,
                                 j_video_dimensions_ctor_id_,
                                 track_stats->getCapturedDimensions().getWidth(),
                                 track_stats->getCapturedDimensions().getHeight());
            jobject j_sent_dimensions =
                jni()->NewObject(*j_video_dimensions_class_,
                                 j_video_dimensions_ctor_id_,
                                 track_stats->getSentDimensions().getWidth(),
                                 track_stats->getSentDimensions().getHeight());
            jobject j_local_video_track_stats =
                jni()->NewObject(*j_local_video_track_stats_class_,
                                 j_local_video_track_stats_ctor_id_,
                                 j_track_id,
                                 track_stats->getPacketsLost(),
                                 j_codec_name,
                                 j_ssrc,
                                 track_stats->getUnixTimestamp(),
                                 track_stats->getBytesSent(),
                                 track_stats->getPacketsSent(),
                                 track_stats->getRoundTripTime(),
                                 j_capture_dimensions,
                                 j_sent_dimensions,
                                 track_stats->getCapturedFrameRate(),
                                 track_stats->getSentFrameRate());
            jni()->CallVoidMethod(j_stats_report,
                                  j_stats_report_add_local_video_id_,
                                  j_local_video_track_stats);
        }
    }

    void processAudioTrackStats(jobject j_stats_report,
                                const std::vector<std::unique_ptr<twilio::media::AudioTrackStats>> &audio_tracks_stats) {
        for(auto const &track_stats : audio_tracks_stats) {
            webrtc_jni::ScopedLocalRefFrame local_ref_frame(jni());
            jstring j_track_id =
                webrtc_jni::JavaStringFromStdString(jni(), track_stats->getTrackId());
            jstring j_codec_name =
                webrtc_jni::JavaStringFromStdString(jni(), track_stats->getCodecName());
            jstring j_ssrc =
                webrtc_jni::JavaStringFromStdString(jni(), track_stats->getSsrc());
            jobject j_audio_track_stats =
                jni()->NewObject(*j_audio_track_stats_class_,
                                 j_audio_track_stats_ctor_id_,
                                 j_track_id,
                                 track_stats->getPacketsLost(),
                                 j_codec_name,
                                 j_ssrc,
                                 track_stats->getUnixTimestamp(),
                                 track_stats->getBytesReceived(),
                                 track_stats->getPacketsReceived(),
                                 track_stats->getJitterBuffer(),
                                 track_stats->getAudioOutputLevel(),
                                 track_stats->getJitterReceived());
            jni()->CallVoidMethod(j_stats_report,
                                  j_stats_report_add_audio_id_,
                                  j_audio_track_stats);
        }
    }

    void processVideoTrackStats(jobject j_stats_report,
                                const std::vector<std::unique_ptr<twilio::media::VideoTrackStats>> &video_tracks_stats) {
        for(auto const &track_stats : video_tracks_stats) {
            webrtc_jni::ScopedLocalRefFrame local_ref_frame(jni());
            jstring j_track_id =
                webrtc_jni::JavaStringFromStdString(jni(), track_stats->getTrackId());
            jstring j_codec_name =
                webrtc_jni::JavaStringFromStdString(jni(), track_stats->getCodecName());
            jstring j_ssrc =
                webrtc_jni::JavaStringFromStdString(jni(), track_stats->getSsrc());
            jobject j_received_dimensions =
                jni()->NewObject(*j_video_dimensions_class_,
                                 j_video_dimensions_ctor_id_,
                                 track_stats->getReceivedDimensions().getWidth(),
                                 track_stats->getReceivedDimensions().getHeight());
            jobject j_video_track_stats =
                jni()->NewObject(*j_video_track_stats_class_,
                                 j_video_track_stats_ctor_id_,
                                 j_track_id,
                                 track_stats->getPacketsLost(),
                                 j_codec_name,
                                 j_ssrc,
                                 track_stats->getUnixTimestamp(),
                                 track_stats->getBytesReceived(),
                                 track_stats->getPacketsReceived(),
                                 track_stats->getJitterBuffer(),
                                 j_received_dimensions,
                                 track_stats->getReceivedFrameRate());
            jni()->CallVoidMethod(j_stats_report,
                                  j_stats_report_add_video_id_,
                                  j_video_track_stats);
        }
    }

    bool observer_deleted_ = false;
    mutable rtc::CriticalSection deletion_lock_;

    const webrtc_jni::ScopedGlobalRef<jobject> j_stats_observer_;
    const webrtc_jni::ScopedGlobalRef<jclass> j_stats_observer_class_;
    const webrtc_jni::ScopedGlobalRef<jclass> j_array_list_class_;
    const webrtc_jni::ScopedGlobalRef<jclass> j_stats_report_class_;
    const webrtc_jni::ScopedGlobalRef<jclass> j_local_audio_track_stats_class_;
    const webrtc_jni::ScopedGlobalRef<jclass> j_local_video_track_stats_class_;
    const webrtc_jni::ScopedGlobalRef<jclass> j_audio_track_stats_class_;
    const webrtc_jni::ScopedGlobalRef<jclass> j_video_track_stats_class_;
    const webrtc_jni::ScopedGlobalRef<jclass> j_video_dimensions_class_;
    jmethodID j_on_stats_id_;
    jmethodID j_array_list_ctor_id_;
    jmethodID j_array_list_add_;
    jmethodID j_stats_report_ctor_id_;
    jmethodID j_stats_report_add_local_audio_id_;
    jmethodID j_stats_report_add_local_video_id_;
    jmethodID j_stats_report_add_audio_id_;
    jmethodID j_stats_report_add_video_id_;
    jmethodID j_local_audio_track_stats_ctor_id_;
    jmethodID j_local_video_track_stats_ctor_id_;
    jmethodID j_audio_track_stats_ctor_id_;
    jmethodID j_video_track_stats_ctor_id_;
    jmethodID j_video_dimensions_ctor_id_;

};

#endif // VIDEO_ANDROID_ANDROID_STATS_OBSERVER_H_
