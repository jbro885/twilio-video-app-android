#include <memory>

#include "android_video_capturer.h"
#include "webrtc/api/android/jni/native_handle_impl.h"
#include "webrtc/base/common.h"
#include "webrtc/base/timeutils.h"
#include "webrtc/media/engine/webrtcvideoframe.h"

AndroidVideoCapturer::AndroidVideoCapturer(
        const rtc::scoped_refptr<AndroidVideoCapturerDelegate>& delegate)
        : running_(false),
          delegate_(delegate) {
    thread_checker_.DetachFromThread();
    SetSupportedFormats(delegate_->GetSupportedFormats());
}

AndroidVideoCapturer::~AndroidVideoCapturer() {
    RTC_CHECK(!running_);
}

cricket::CaptureState AndroidVideoCapturer::Start(const cricket::VideoFormat& capture_format) {
    RTC_CHECK(thread_checker_.CalledOnValidThread());
    RTC_CHECK(!running_);
    const int fps = cricket::VideoFormat::IntervalToFps(capture_format.interval);
    LOG(LS_INFO) << " AndroidVideoCapturer::Start " << capture_format.width << "x"
                 << capture_format.height << "@" << fps;

    running_ = true;
    delegate_->Start(capture_format, this);
    SetCaptureFormat(&capture_format);
    return cricket::CS_STARTING;
}

void AndroidVideoCapturer::Stop() {
    LOG(LS_INFO) << " AndroidVideoCapturer::Stop ";
    RTC_CHECK(thread_checker_.CalledOnValidThread());
    RTC_CHECK(running_);
    running_ = false;
    SetCaptureFormat(NULL);

    delegate_->Stop();
    SetCaptureState(cricket::CS_STOPPED);
}

bool AndroidVideoCapturer::IsRunning() {
    RTC_CHECK(thread_checker_.CalledOnValidThread());
    return running_;
}

bool AndroidVideoCapturer::GetPreferredFourccs(std::vector<uint32_t>* fourccs) {
    RTC_CHECK(thread_checker_.CalledOnValidThread());
    fourccs->push_back(cricket::FOURCC_YV12);
    return true;
}

void AndroidVideoCapturer::OnCapturerStarted(bool success) {
    RTC_CHECK(thread_checker_.CalledOnValidThread());
    const cricket::CaptureState new_state =
            success ? cricket::CS_RUNNING : cricket::CS_FAILED;
    SetCaptureState(new_state);
}

void AndroidVideoCapturer::OnOutputFormatRequest(
        int width, int height, int fps) {
    RTC_CHECK(thread_checker_.CalledOnValidThread());
    cricket::VideoFormat format(width, height,
                                cricket::VideoFormat::FpsToInterval(fps), 0);
    video_adapter()->OnOutputFormatRequest(format);
}

bool AndroidVideoCapturer::GetBestCaptureFormat(
        const cricket::VideoFormat& desired,
        cricket::VideoFormat* best_format) {
    // Delegate this choice to VideoCapturer.startCapture().
    *best_format = desired;
    return true;
}
