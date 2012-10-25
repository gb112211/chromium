// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_MEDIA_CRYPTO_PPAPI_CLEAR_KEY_CDM_H_
#define WEBKIT_MEDIA_CRYPTO_PPAPI_CLEAR_KEY_CDM_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/synchronization/lock.h"
#include "media/base/decryptor_client.h"
#include "media/crypto/aes_decryptor.h"
#include "webkit/media/crypto/ppapi/content_decryption_module.h"

// Enable this to use the fake decoder for testing.
#if 0
#define CLEAR_KEY_CDM_USE_FAKE_AUDIO_DECODER
#endif

#if 0
#define CLEAR_KEY_CDM_USE_FAKE_VIDEO_DECODER
#endif

#if defined(CLEAR_KEY_CDM_USE_FAKE_VIDEO_DECODER)
#undef CLEAR_KEY_CDM_USE_FFMPEG_DECODER
#endif

namespace media {
class DecoderBuffer;
}

namespace webkit_media {

class FFmpegCdmVideoDecoder;

// Clear key implementation of the cdm::ContentDecryptionModule interface.
class ClearKeyCdm : public cdm::ContentDecryptionModule {
 public:
  explicit ClearKeyCdm(cdm::Allocator* allocator, cdm::CdmHost*);
  virtual ~ClearKeyCdm();

  // ContentDecryptionModule implementation.
  virtual cdm::Status GenerateKeyRequest(
      const uint8_t* init_data,
      int init_data_size,
      cdm::KeyMessage* key_request) OVERRIDE;
  virtual cdm::Status AddKey(const char* session_id,
                             int session_id_size,
                             const uint8_t* key,
                             int key_size,
                             const uint8_t* key_id,
                             int key_id_size) OVERRIDE;
  virtual cdm::Status CancelKeyRequest(const char* session_id,
                                       int session_id_size) OVERRIDE;
  virtual void TimerExpired(cdm::KeyMessage* msg, bool* populated) OVERRIDE;
  virtual cdm::Status Decrypt(const cdm::InputBuffer& encrypted_buffer,
                              cdm::DecryptedBlock* decrypted_block) OVERRIDE;
  virtual cdm::Status InitializeAudioDecoder(
      const cdm::AudioDecoderConfig& audio_decoder_config) OVERRIDE;
  virtual cdm::Status InitializeVideoDecoder(
      const cdm::VideoDecoderConfig& video_decoder_config) OVERRIDE;
  virtual void DeinitializeDecoder(cdm::StreamType decoder_type) OVERRIDE;
  virtual void ResetDecoder(cdm::StreamType decoder_type) OVERRIDE;
  virtual cdm::Status DecryptAndDecodeFrame(
      const cdm::InputBuffer& encrypted_buffer,
      cdm::VideoFrame* video_frame) OVERRIDE;
  virtual cdm::Status DecryptAndDecodeSamples(
      const cdm::InputBuffer& encrypted_buffer,
      cdm::AudioFrames* audio_frames) OVERRIDE;

 private:
  class Client : public media::DecryptorClient {
   public:
    enum Status {
      kKeyAdded,
      kKeyError,
      kKeyMessage,
      kNeedKey
    };

    Client();
    virtual ~Client();

    Status status() { return status_; }
    const std::string& session_id() { return session_id_; }
    const uint8* key_message() { return key_message_.get(); }
    int key_message_length() { return key_message_length_; }
    const std::string& default_url() { return default_url_; }

    // Resets the Client to a clean state.
    void Reset();

    // media::DecryptorClient implementation.
    virtual void KeyAdded(const std::string& key_system,
                          const std::string& session_id) OVERRIDE;
    virtual void KeyError(const std::string& key_system,
                          const std::string& session_id,
                          media::Decryptor::KeyError error_code,
                          int system_code) OVERRIDE;
    virtual void KeyMessage(const std::string& key_system,
                            const std::string& session_id,
                            scoped_array<uint8> message,
                            int message_length,
                            const std::string& default_url) OVERRIDE;
    virtual void NeedKey(const std::string& key_system,
                         const std::string& session_id,
                         scoped_array<uint8> init_data,
                         int init_data_length) OVERRIDE;

   private:
    Status status_;
    std::string session_id_;
    scoped_array<uint8> key_message_;
    int key_message_length_;
    std::string default_url_;
  };

  // Decrypts the |encrypted_buffer| and puts the result in |decrypted_buffer|.
  // Returns cdm::kSuccess if decryption succeeded. The decrypted result is
  // put in |decrypted_buffer|. If |encrypted_buffer| is empty, the
  // |decrypted_buffer| is set to an empty (EOS) buffer.
  // Returns cdm::kNoKey if no decryption key was available. In this case
  // |decrypted_buffer| should be ignored by the caller.
  // Returns cdm::kDecryptError if any decryption error occurred. In this case
  // |decrypted_buffer| should be ignored by the caller.
  cdm::Status DecryptToMediaDecoderBuffer(
      const cdm::InputBuffer& encrypted_buffer,
      scoped_refptr<media::DecoderBuffer>* decrypted_buffer);

#if defined(CLEAR_KEY_CDM_USE_FAKE_AUDIO_DECODER)
  // Generates fake video frames with |last_timestamp_| and |last_duration_|.
  void GenerateFakeAudioFrames(cdm::AudioFrames* audio_frames);
#endif  // CLEAR_KEY_CDM_USE_FAKE_VIDEO_DECODER

#if defined(CLEAR_KEY_CDM_USE_FAKE_VIDEO_DECODER)
  // Generate a fake video frame with |video_size_| and |timestamp|.
  void GenerateFakeVideoFrame(base::TimeDelta timestamp,
                              cdm::VideoFrame* video_frame);
#endif  // CLEAR_KEY_CDM_USE_FAKE_VIDEO_DECODER

  Client client_;
  media::AesDecryptor decryptor_;

  // Protects the |client_| from being accessed by the |decryptor_|
  // simultaneously.
  base::Lock client_lock_;

  cdm::Allocator* const allocator_;

#if defined(CLEAR_KEY_CDM_USE_FAKE_AUDIO_DECODER)
  int channel_count_;
  int bits_per_channel_;
  int samples_per_second_;
  base::TimeDelta last_timestamp_;
  base::TimeDelta last_duration_;
#endif  // CLEAR_KEY_CDM_USE_FAKE_AUDIO_DECODER

#if defined(CLEAR_KEY_CDM_USE_FFMPEG_DECODER)
  scoped_ptr<FFmpegCdmVideoDecoder> video_decoder_;
#endif  // CLEAR_KEY_CDM_USE_FFMPEG_DECODER

#if defined(CLEAR_KEY_CDM_USE_FAKE_VIDEO_DECODER)
  cdm::Size video_size_;
#endif  // CLEAR_KEY_CDM_USE_FAKE_VIDEO_DECODER
};

}  // namespace webkit_media

#endif  // WEBKIT_MEDIA_CRYPTO_PPAPI_CLEAR_KEY_CDM_H_
