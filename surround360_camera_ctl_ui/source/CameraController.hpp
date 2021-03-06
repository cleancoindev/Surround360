/**
* Copyright (c) 2016-present, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the license found in the
* LICENSE_camera_ctl_ui file in the root directory of this subproject.
*/

#pragma once

#include "CameraView.hpp"
#include "PointGrey.hpp"
#include "ProducerConsumer.h"
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <utility>
#include <memory>

namespace surround360 {
struct FramePacket{
  int frameNumber;
  int frameSize;
  int cameraNumber;
  int cameraSerial;
  void* imageBytes;
};

typedef ProducerConsumer<FramePacket, 100ULL> ConsumerBuffer;

class CameraController {
public:
  static CameraController& get(
    CameraView& camview,
    const unsigned int nprod = 1,
    const unsigned int ncons = 2);

  static CameraController& get();

  bool configureCameras(
    const float shutter,
    const float framerate,
    const float gain,
    const int bitsPerPixel);

  bool updateCameraParams(
    const float shutter,
    const float fps,
    const float gain,
    const int bpp);

  void startProducer(const unsigned int count);
  void startConsumers(const unsigned int count);
  void startSlaveCapture();
  void startMasterCapture();
  void setPreviewCamera(unsigned int i);
  void setPaths(const string path[2]);
  void startRecording(const bool oneshot = false);
  void stopRecording();

  ~CameraController();

private:
  CameraController(
    CameraView& camview,
    const unsigned int nproducers = 1,
    const unsigned int nconsumers = 2);
  void cameraProducer(const unsigned int id);
  void cameraConsumer(const unsigned int id);
  inline size_t frameSize() const {
    return (m_width * m_height * m_bitsPerPixel) / 8;
  }
  void ispThread();
  void writeCameraNames(const string& path);
  void writeHeader(const int fd, const uint32_t id);

private:
  std::vector<PointGreyCameraPtr> m_camera;
  std::vector<std::thread>       m_prodThread;
  std::vector<std::thread>       m_consThread;
  std::vector<ConsumerBuffer>    m_consumerBuf;

  int m_masterCameraSerial;
  int m_masterCameraIndex;

  std::size_t m_producerCount;
  std::size_t m_consumerCount;
  float  m_shutter;
  float  m_framerate;
  float  m_gain;
  float  m_bitsPerPixel;
  float  m_exposure;
  float  m_brightness;
  float  m_gamma;

  std::atomic<bool>   m_keepRunning;
  std::atomic<bool>   m_recording;
  std::atomic<bool>   m_startRecording;
  std::atomic<bool>   m_stopRecording;
  std::atomic<bool>   m_oneshot;
  std::vector<int8_t> m_oneshotCount;
  std::unique_ptr<std::vector<int>[]>    m_oneshotIdx;

  const int m_pinStrobe;
  const int m_pinTrigger;
  std::vector<std::mutex> m_mutex;
  std::vector<std::mutex> m_slaveMutex;

  std::vector<std::condition_variable> m_cond;
  std::vector<std::condition_variable> m_slaveCond;

  CameraView& m_cameraView;

  std::atomic<size_t> m_previewIndex;

  unsigned int m_width;
  unsigned int m_height;

  std::vector<std::size_t> m_sample;

  bool m_running;
  bool m_paramUpdatePending;

  enum param { paramShutter = 0, paramFramerate, paramGain, paramBits };
  std::vector<bool> update;
  string m_dirname[2];
};
}
