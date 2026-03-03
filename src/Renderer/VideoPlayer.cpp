#include "VideoPlayer.h"
#include "../Core/Logger.h"
#include <glad/glad.h>
#include <iostream>
#include <cstdlib>
#include <filesystem>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

struct VideoPlayer::FFmpegContext {
    AVFormatContext* formatCtx = nullptr;
    AVCodecContext* codecCtx = nullptr;
    const AVCodec* codec = nullptr;
    AVFrame* frame = nullptr;
    AVFrame* frameRGB = nullptr;
    AVPacket* packet = nullptr;
    SwsContext* swsCtx = nullptr;
    
    int videoStreamIndex = -1;
    uint8_t* buffer = nullptr;
    double timeBase = 0.0;
    double currentTime = 0.0;
};

VideoPlayer::VideoPlayer() : m_TextureID(0), m_Playing(false), m_Looping(false), m_Ctx(nullptr) {}

VideoPlayer::~VideoPlayer() {
    Close();
}

bool VideoPlayer::Open(const std::string& path) {
    Close();
    m_Ctx = new FFmpegContext();
    
    
    std::string audioPath = path + "_audio.wav";
    if (!std::filesystem::exists(audioPath)) {
        Logger::AddLog("Extracting audio from video: %s", path.c_str());
        std::string cmd = "ffmpeg -y -i \"" + path + "\" -vn -acodec pcm_s16le -ar 44100 -ac 2 \"" + audioPath + "\" > /dev/null 2>&1";
        system(cmd.c_str());
    }

    if (avformat_open_input(&m_Ctx->formatCtx, path.c_str(), nullptr, nullptr) != 0) {
        Logger::AddLog("[VideoPlayer] Failed to open video: %s", path.c_str());
        Close();
        return false;
    }

    if (avformat_find_stream_info(m_Ctx->formatCtx, nullptr) < 0) {
        Logger::AddLog("[VideoPlayer] Failed to find stream info.");
        Close();
        return false;
    }

    for (unsigned int i = 0; i < m_Ctx->formatCtx->nb_streams; i++) {
        if (m_Ctx->formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_Ctx->videoStreamIndex = i;
            break;
        }
    }

    if (m_Ctx->videoStreamIndex == -1) {
        Logger::AddLog("[VideoPlayer] No video stream found.");
        Close();
        return false;
    }

    AVCodecParameters* codecPar = m_Ctx->formatCtx->streams[m_Ctx->videoStreamIndex]->codecpar;
    m_Ctx->codec = avcodec_find_decoder(codecPar->codec_id);
    if (!m_Ctx->codec) {
        Logger::AddLog("[VideoPlayer] Unsupported codec.");
        Close();
        return false;
    }

    m_Ctx->codecCtx = avcodec_alloc_context3(m_Ctx->codec);
    avcodec_parameters_to_context(m_Ctx->codecCtx, codecPar);

    if (avcodec_open2(m_Ctx->codecCtx, m_Ctx->codec, nullptr) < 0) {
        Logger::AddLog("[VideoPlayer] Failed to open codec.");
        Close();
        return false;
    }

    m_Ctx->frame = av_frame_alloc();
    m_Ctx->frameRGB = av_frame_alloc();
    m_Ctx->packet = av_packet_alloc();

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, m_Ctx->codecCtx->width, m_Ctx->codecCtx->height, 1);
    m_Ctx->buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

    av_image_fill_arrays(m_Ctx->frameRGB->data, m_Ctx->frameRGB->linesize, m_Ctx->buffer, AV_PIX_FMT_RGBA, m_Ctx->codecCtx->width, m_Ctx->codecCtx->height, 1);

    m_Ctx->swsCtx = sws_getContext(
        m_Ctx->codecCtx->width, m_Ctx->codecCtx->height, m_Ctx->codecCtx->pix_fmt,
        m_Ctx->codecCtx->width, m_Ctx->codecCtx->height, AV_PIX_FMT_RGBA,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    AVRational tb = m_Ctx->formatCtx->streams[m_Ctx->videoStreamIndex]->time_base;
    m_Ctx->timeBase = av_q2d(tb);

    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Ctx->codecCtx->width, m_Ctx->codecCtx->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_Playing = true;
    m_Ctx->currentTime = 0.0;
    
    return true;
}

void VideoPlayer::Close() {
    if (m_TextureID != 0) {
        glDeleteTextures(1, &m_TextureID);
        m_TextureID = 0;
    }
    
    if (m_Ctx) {
        if (m_Ctx->buffer) av_freep(&m_Ctx->buffer);
        if (m_Ctx->frameRGB) av_frame_free(&m_Ctx->frameRGB);
        if (m_Ctx->frame) av_frame_free(&m_Ctx->frame);
        if (m_Ctx->packet) av_packet_free(&m_Ctx->packet);
        if (m_Ctx->codecCtx) avcodec_free_context(&m_Ctx->codecCtx);
        if (m_Ctx->formatCtx) avformat_close_input(&m_Ctx->formatCtx);
        if (m_Ctx->swsCtx) sws_freeContext(m_Ctx->swsCtx);
        
        delete m_Ctx;
        m_Ctx = nullptr;
    }
    m_Playing = false;
}

int VideoPlayer::GetWidth() const {
    return m_Ctx && m_Ctx->codecCtx ? m_Ctx->codecCtx->width : 0;
}

int VideoPlayer::GetHeight() const {
    return m_Ctx && m_Ctx->codecCtx ? m_Ctx->codecCtx->height : 0;
}

void VideoPlayer::Update(float dt) {
    if (!m_Ctx || !m_Playing) return;
    
    m_Ctx->currentTime += dt;
    bool frameReady = false;

    
    while (av_read_frame(m_Ctx->formatCtx, m_Ctx->packet) >= 0) {
        if (m_Ctx->packet->stream_index == m_Ctx->videoStreamIndex) {
            if (avcodec_send_packet(m_Ctx->codecCtx, m_Ctx->packet) == 0) {
                while (avcodec_receive_frame(m_Ctx->codecCtx, m_Ctx->frame) == 0) {
                    double pts = m_Ctx->frame->pts * m_Ctx->timeBase;
                    
                    if (pts >= m_Ctx->currentTime) {
                        sws_scale(m_Ctx->swsCtx, (uint8_t const * const *)m_Ctx->frame->data,
                                  m_Ctx->frame->linesize, 0, m_Ctx->codecCtx->height,
                                  m_Ctx->frameRGB->data, m_Ctx->frameRGB->linesize);
                        frameReady = true;
                        av_packet_unref(m_Ctx->packet);
                        break; 
                    }
                }
            }
        }
        av_packet_unref(m_Ctx->packet);
        if (frameReady) break;
    }
    
    
    if (!frameReady) {
        if (m_Looping) {
            av_seek_frame(m_Ctx->formatCtx, -1, 0, AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(m_Ctx->codecCtx);
            m_Ctx->currentTime = 0.0;
        } else {
            m_Playing = false;
        }
    }

    if (frameReady && m_TextureID != 0) {
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Ctx->codecCtx->width, m_Ctx->codecCtx->height, GL_RGBA, GL_UNSIGNED_BYTE, m_Ctx->frameRGB->data[0]);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
