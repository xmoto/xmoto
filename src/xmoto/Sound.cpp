/*=============================================================================
XMOTO

This file is part of XMOTO.

XMOTO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

XMOTO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XMOTO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

#include "Sound.h"
#include "common/VFileIO.h"
#include "common/XMSession.h"
#include "helpers/Log.h"
#include "helpers/VExcept.h"
#include "include/xm_SDL.h"

/*===========================================================================
  Globals
  ===========================================================================*/
int Sound::m_nSampleRate;
int Sound::m_nSampleBits;
int Sound::m_nChannels;
bool Sound::m_activ;

std::vector<SoundSample *> Sound::m_Samples;
Mix_Music *Sound::m_pMenuMusic;
bool Sound::m_isInitialized = false;

void Sound::init(XMSession *i_session) {
  /* Get user configuration */

  int n;
  switch (n = i_session->audioSampleRate()) {
    case 11025:
    case 22050:
    case 44100:
      m_nSampleRate = n;
      break;
    default:
      LogWarning("invalid audio sample rate, falling back to 22050");
      m_nSampleRate = 22050;
      break;
  }

  switch (n = i_session->audioSampleBits()) {
    case 8:
    case 16:
      m_nSampleBits = n;
      break;
    default:
      LogWarning("invalid audio sample bits, falling back to 16");
      m_nSampleRate = 16;
      break;
  }

  m_nChannels = i_session->audioChannels();

  /* Init SDL stuff */
  if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
    LogWarning("failed to initialize SDL audio (%s)", SDL_GetError());
    return;
  }

  int nFormat;
  if (m_nSampleBits == 8)
    nFormat = AUDIO_S8;
  else
    nFormat = AUDIO_S16;

  if (Mix_OpenAudio(m_nSampleRate, nFormat, m_nChannels, 2048) < 0) {
    LogWarning("failed to open mixer device (%s)", Mix_GetError());
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    return;
  }

  Mix_AllocateChannels(64);
  m_pMenuMusic = NULL;
  m_activ = i_session->enableAudio();
  m_isInitialized = true;
}

void Sound::uninit(void) {
  Mix_CloseAudio();

  /* Free loaded samples */
  for (unsigned int i = 0; i < m_Samples.size(); i++) {
    Mix_FreeChunk(m_Samples[i]->pChunk);
    delete m_Samples[i];
  }
  m_Samples.clear();

  /* Quit sound system if enabled */
  SDL_QuitSubSystem(SDL_INIT_AUDIO);

  if (m_pMenuMusic != NULL) {
    Mix_FreeMusic(m_pMenuMusic);
  }

  m_isInitialized = false;
}

bool Sound::isInitialized() {
  return m_isInitialized;
}

void Sound::update(void) {}

int64_t Sound::RWops_size(SDL_RWops *context) {
  FileHandle *pf = (FileHandle *)context->hidden.unknown.data1;
  int length = XMFS::getLength(pf);

  // This is technically valid, but the stb vorbis library
  // optionally used by SDL_mixer may not be able to decode the file
  // if the length can't be determined for whatever reason
  if (length < 0) {
    return -1;
  }

  return static_cast<int64_t>(length);
}

int64_t Sound::RWops_seek(SDL_RWops *context, int64_t offset, int whence) {
  FileHandle *pf = (FileHandle *)context->hidden.unknown.data1;
  switch (whence) {
    case SEEK_SET:
      XMFS::setOffset(pf, offset);
      break;
    case SEEK_END:
      XMFS::setOffset(pf, XMFS::getLength(pf));
      break;
    case SEEK_CUR:
      XMFS::setOffset(pf, XMFS::getOffset(pf) + offset);
      break;
  }
  return XMFS::getOffset(pf);
}

size_t Sound::RWops_read(SDL_RWops *context,
                         void *ptr,
                         size_t size,
                         size_t maxnum) {
  FileHandle *pf = (FileHandle *)context->hidden.unknown.data1;
  if (XMFS::isEnd(pf))
    return 0;

  int nRemaining = (XMFS::getLength(pf) - XMFS::getOffset(pf)) / size;

  int nToRead = nRemaining < maxnum ? nRemaining : maxnum;

  if (!XMFS::readBuf(pf, (char *)ptr, size * nToRead))
    return 0;

  return nToRead;
}

size_t Sound::RWops_write(SDL_RWops *context,
                          const void *ptr,
                          size_t size,
                          size_t num) {
  return num;
}

int Sound::RWops_close(SDL_RWops *context) {
  return 0;
}

SoundSample *Sound::loadSample(const std::string &File) {
  /* Allocate sample */
  SoundSample *pSample = new SoundSample;
  pSample->Name = File;

  /* Setup a RW_ops struct */
  SDL_RWops *pOps = SDL_AllocRW();
  pOps->close = RWops_close;
  pOps->read = RWops_read;
  pOps->seek = RWops_seek;
  pOps->size = RWops_size;
  pOps->write = RWops_write;
  pOps->type = 1000;

  /* Open */
  FileHandle *pf = XMFS::openIFile(FDT_DATA, File);
  if (pf == NULL) {
    SDL_FreeRW(pOps);
    delete pSample;
    throw Exception("failed to open sample file " + File);
  }

  pOps->hidden.unknown.data1 = (void *)pf;

  /* Loadit */
  pSample->pChunk = Mix_LoadWAV_RW(pOps, 1);

  /* Close file */
  XMFS::closeFile(pf);
  SDL_FreeRW(pOps);

  m_Samples.push_back(pSample);
  return pSample;
}

void Sound::playSample(SoundSample *pSample, float fVolume) {
  if (Sound::isActiv() == false)
    return;

  if (pSample == NULL)
    return;

  int nChannel = Mix_PlayChannel(-1, pSample->pChunk, 0);
  if (nChannel >= 0) {
    Mix_Volume(nChannel, (int)(fVolume * MIX_MAX_VOLUME));
  }
}

SoundSample *Sound::findSample(const std::string &File) {
  for (unsigned int i = 0; i < m_Samples.size(); i++) {
    if (m_Samples[i]->Name == File)
      return m_Samples[i];
  }

  return loadSample(File);
}

void Sound::playSampleByName(const std::string &Name, float fVolume) {
  if (Sound::isActiv() == false)
    return;

  SoundSample *pSample = findSample(Name);
  if (pSample != NULL) {
    playSample(pSample, fVolume);
  }
}

/*==============================================================================
  Engine sound simulator
  ==============================================================================*/
void EngineSoundSimulator::update(int i_time) {
  if (Sound::isActiv() == false)
    return;

  if (i_time < m_lastBangTime)
    m_lastBangTime = i_time; /* manage back in the past */

  if (m_BangSamples.size() > 0) {
    if (m_fRPM > 100.0f) {
      /* Calculate the delay between the samples */
      int v_interval = (int)(60.0 * 120.0 / m_fRPM);

      if (i_time - m_lastBangTime > v_interval) {
        /* Stroke! Determine a random sample to use */
        float x = ((float)rand()) /
                  (float)RAND_MAX; /* linux likes insanely high RAND_MAX'es */
        int i = (int)(((float)m_BangSamples.size()) * x);
        if (i < 0)
          i = 0;
        if ((unsigned int)i >= m_BangSamples.size())
          i = m_BangSamples.size() - 1;
        /* Play it */
        Mix_PlayChannel(-1, m_BangSamples[i]->pChunk, 0);
        m_lastBangTime = i_time;
      }
    }
  }
}

void Sound::playMusic(std::string i_musicPath) {
  if (Sound::isActiv() == false)
    return;

  if (Mix_PlayingMusic() || Mix_FadingMusic()) {
    stopMusic();
  }

  if (m_pMenuMusic != NULL) {
    Mix_FreeMusic(m_pMenuMusic);
    m_pMenuMusic = NULL;
  }

/* No music available, try loading */
#if 0 && defined(WIN32) || \
  defined(                 \
    __amigaos4__) /* this works around a bug in SDL_mixer 1.2.7 on Windows */
  SDL_RWops *rwfp;
  rwfp = SDL_RWFromFile(i_musicPath.c_str(), "rb");
  if (rwfp != NULL) {
    m_pMenuMusic = Mix_LoadMUS_RW(rwfp);
  }
#else
  m_pMenuMusic = Mix_LoadMUS(i_musicPath.c_str());
#endif

  if (m_pMenuMusic == NULL) {
    throw Exception("No music played !");
  }

  if (Mix_PlayMusic(m_pMenuMusic, -1) < 0) {
    throw Exception("No music played !");
  }
}

void Sound::togglePauseMusic() {
  if (m_pMenuMusic != NULL && Mix_PlayingMusic() &&
      Mix_PausedMusic() == false) {
    Mix_PauseMusic();
  } else if (m_pMenuMusic != NULL && Mix_PlayingMusic() && Mix_PausedMusic()) {
    Mix_ResumeMusic();
  }
}

void Sound::stopMusic() {
  if (m_pMenuMusic != NULL && Mix_PlayingMusic()) {
    Mix_HaltMusic();
  }
}

bool Sound::isPlayingMusic() {
  return Mix_PlayingMusic();
}

void Sound::setActiv(bool i_value) {
  m_activ = i_value;

  if (i_value == false && isPlayingMusic()) {
    stopMusic();
  }
}

bool Sound::isActiv() {
  return m_activ;
}
