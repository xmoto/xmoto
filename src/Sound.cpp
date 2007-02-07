/*=============================================================================
XMOTO
Copyright (C) 2005-2006 Rasmus Neckelmann (neckelmann@gmail.com)

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

/* 
 *  Sound system.
 */
#include "VFileIO.h"
#include "UserConfig.h"
#include "Sound.h"

/* SDL_mixer / nosound is the only ones supported!!! I should really clean up this 
   file :) */
#define USE_SDL_MIXER     1
#define USE_OPENAL        0
#define USE_NO_SOUND      0

namespace vapp {

  /*===========================================================================
  Globals
  ===========================================================================*/
  bool Sound::m_bEnable;               
  int Sound::m_nSampleRate;            
  int Sound::m_nSampleBits;            
  int Sound::m_nChannels;              
      
  //SDL_AudioSpec Sound::m_ASpec;
  //
  //SoundPlayer *Sound::m_pPlayers[16];
  std::vector<SoundSample *> Sound::m_Samples;
  Mix_Music *Sound::m_pMenuMusic;

  /*===========================================================================
  Init and uninit
  ===========================================================================*/
  void Sound::init(UserConfig *pConfig) {
    #if USE_NO_SOUND
      m_bEnable = false;
      return;
    #endif
  
    /* Get user configuration */
    if(pConfig->getBool("AudioEnable")) {
      m_bEnable = true;
      
      int n;

      switch(n=pConfig->getInteger("AudioSampleRate")) {
        case 11025:
        case 22050:
        case 44100:
          m_nSampleRate = n;
          break;
        default:
          Log("** Warning ** : invalid audio sample rate, falling back to 22050");
          m_nSampleRate = 22050;
          break;
      }      

      switch(n=pConfig->getInteger("AudioSampleBits")) {
        case 8:
        case 16:
          m_nSampleBits = n;
          break;
        default:
          Log("** Warning ** : invalid audio sample bits, falling back to 16");
          m_nSampleRate = 16;
          break;
      }      
      
      std::string s = pConfig->getString("AudioChannels");
      if(s == "Mono") m_nChannels = 1;
      else if(s == "Stereo") m_nChannels = 2;
      else {
        Log("** Warning ** : invalid number of audio channels, falling back to mono");
        m_nChannels = 1;
      }
        
    }  
    else {
      m_bEnable = false;
      return;
    }
    
    #if USE_OPENAL
      try {
//        sal::Global::init("default",m_nSampleRate,0,sal::SYNC_DEFAULT);
      }
      catch(const char *e) {
        Log("** Warning ** : failed to initialize OpenAL (%s)",e);
        m_bEnable = false;
        return;
      }
    #else
      ///* Clear stuff */
      //for(int i=0;i<16;i++) {
      //  m_pPlayers[i] = NULL;
      //}    
          
      /* Init SDL stuff */
      if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        Log("** Warning ** : failed to initialize SDL audio (%s)",SDL_GetError());
        m_bEnable = false;
        return;
      }
      
      #if USE_SDL_MIXER
        int nFormat;
        if(m_nSampleBits == 8) nFormat = AUDIO_S8;
        else nFormat = AUDIO_S16;
        
        if(Mix_OpenAudio(m_nSampleRate,nFormat,m_nChannels,2048) < 0) {
          Log("** Warning ** : failed to open mixer device (%s)",Mix_GetError());
          SDL_QuitSubSystem(SDL_INIT_AUDIO);
          m_bEnable = false;
          return;
        }
        
        Mix_AllocateChannels(64);
        
      #else /* Not using SDL_mixer */
        /* Open audio device */
        m_ASpec.freq = m_nSampleRate;
        if(m_nSampleBits == 8)
          m_ASpec.format = AUDIO_S8;
        else
          m_ASpec.format = AUDIO_S16;
        m_ASpec.samples = 1024; /* buffered samples */    
        m_ASpec.callback = audioCallback;
        m_ASpec.channels = m_nChannels;
        m_ASpec.userdata = NULL;
        
        if(SDL_OpenAudio(&m_ASpec,NULL) < 0) {
          Log("** Warning ** : failed to open audio device (%s)",SDL_GetError());
          SDL_QuitSubSystem(SDL_INIT_AUDIO);
          m_bEnable = false;
          return;
        }
        
        /* Start playing */
        SDL_PauseAudio(0);
      #endif
    #endif
	
	m_pMenuMusic = NULL;
  }
  
  void Sound::uninit(void) {  
    #if USE_OPENAL
      sal::Global::quit();
    #else
      #if USE_SDL_MIXER
        if(isEnabled()) {
          Mix_CloseAudio();
        }      

        /* Free loaded samples */
        for(int i=0;i<m_Samples.size();i++) {
          Mix_FreeChunk(m_Samples[i]->pChunk);
          delete m_Samples[i];
        }
        m_Samples.clear();
      #else /* Not using SDL_mixer */
        /* Free loaded samples */
        for(int i=0;i<m_Samples.size();i++) {
          delete [] m_Samples[i]->pcBuf;
          delete m_Samples[i];
        }
        m_Samples.clear();
      #endif
      
      /* Quit sound system if enabled */
      if(isEnabled()) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
      }
    #endif

    if(m_pMenuMusic != NULL) {
      Mix_FreeMusic(m_pMenuMusic);
    }
  }
  
  /*==============================================================================
  Update sound system
  ==============================================================================*/
  void Sound::update(void) {
    #if USE_SDL_MIXER
      /* ... */
    #else /* Not using SDL_mixer */
      /* Walk through all player slots, and update them */
      SDL_LockAudio();
      for(int i=0;i<16;i++) {
        if(m_pPlayers[i] != NULL) {
          m_pPlayers[i]->update();
                  
          /* Do it want to die? */
          if(m_pPlayers[i]->isDone()) {
            m_pPlayers[i]->shutdownPlayer();        
            delete m_pPlayers[i];
            m_pPlayers[i] = NULL;
          }
        }      
      }    
      SDL_UnlockAudio();
    #endif
  }
      
  /*==============================================================================
  Sound sample playing
  ==============================================================================*/
  //int SamplePlayer::fillBuffer(unsigned char *pcBuffer,int nSize) {
  //  int nRem = m_pSample->nBufSize - m_nPosition;
  //  if(nRem <= 0) {
  //    setDone(true);    
  //    return 0;            
  //  }
  //  
  //  int nCopy = nRem < nSize ? nRem : nSize;
  //  memcpy(pcBuffer,&m_pSample->pcBuf[m_nPosition],nCopy);
  //  m_nPosition += nCopy;
  //      
  //  return nCopy;
  //}
  //
  //void SamplePlayer::update(void) {
  //}
  //
  //void SamplePlayer::shutdownPlayer(void) {
  //}

  //void SamplePlayer::initSample(SoundSample *pSample) {
  //  m_pSample = pSample;
  //  m_nPosition = 0;
  //}

  int Sound::RWops_seek(SDL_RWops *context,int offset,int whence) {
    FileHandle *pf = (FileHandle *)context->hidden.unknown.data1;
    switch(whence) {
      case SEEK_SET: FS::setOffset(pf,offset); break;
      case SEEK_END: FS::setOffset(pf,FS::getLength(pf)); break;
      case SEEK_CUR: FS::setOffset(pf,FS::getOffset(pf) + offset); break;
    }
    return FS::getOffset(pf);    
  }
  
  int Sound::RWops_read(SDL_RWops *context,void *ptr,int size,int maxnum) {
    FileHandle *pf = (FileHandle *)context->hidden.unknown.data1;
    if(FS::isEnd(pf)) return 0;
    
    int nRemaining = (FS::getLength(pf) - FS::getOffset(pf)) / size;
    
    int nToRead = nRemaining < maxnum ? nRemaining : maxnum;
    
    if(!FS::readBuf(pf,(char *)ptr,size*nToRead))
      return 0;
      
    return nToRead;
  }

  int Sound::RWops_write(SDL_RWops *context,const void *ptr,int size,int num) {
    return num;
  }
  
  int Sound::RWops_close(SDL_RWops *context) {    
    return 0;
  }
 
  SoundSample *Sound::loadSample(const std::string &File) {      
    if(!Sound::isEnabled())
      throw Exception("Can't load sample, sound is disabled!");
    
    #if USE_SDL_MIXER
      /* Allocate sample */
      SoundSample *pSample = new SoundSample;
      pSample->Name = File;
      
      /* Setup a RW_ops struct */
      SDL_RWops *pOps = SDL_AllocRW();
      pOps->close = RWops_close;
      pOps->read = RWops_read;
      pOps->seek = RWops_seek;
      pOps->write = RWops_write;
      pOps->type = 1000;
      
      /* Open */
      FileHandle *pf = FS::openIFile(File);
      if(pf == NULL) {
        SDL_FreeRW(pOps);
        throw Exception("failed to open sample file");
      }
      
      pOps->hidden.unknown.data1 = (void *)pf;
      
      /* Loadit */
      pSample->pChunk = Mix_LoadWAV_RW(pOps,1);
      
      /* Close file */
      FS::closeFile(pf);
      
      m_Samples.push_back(pSample);
      return pSample;            
      
    #else /* Not using SDL_mixer */
      /* Start loading sample */
      VorbisSound vs;
      vs.openFile(File);
      
      /* Allocate sample */
      SoundSample *pSample = new SoundSample;
      
      /* Build conversion structure */    
      Uint16 nSrcFormat,nDstFormat;
      Uint8 nSrcChannels,nDstChannels;
      int nSrcRate,nDstRate;
      
      if(Sound::getSampleBits() == 8) nDstFormat = AUDIO_S8;
      else nDstFormat = AUDIO_S16;
      
      nDstChannels = Sound::getChannels();
      nDstRate = Sound::getSampleRate();
      
      if(vs.getSampleBits() == 8) nSrcFormat = AUDIO_S8;
      else nSrcFormat = AUDIO_S16;
      
      nSrcChannels = vs.getChannels();
      nSrcRate = vs.getSampleRate();
                     
      if(SDL_BuildAudioCVT(&pSample->cvt,nSrcFormat,nSrcChannels,nSrcRate,nDstFormat,nDstChannels,nDstRate) < 0) {
        vs.closeFile();
        delete pSample;
        Log("** Warning ** : Sound::loadSample() : Failed to prepare audio conversion for '%s'!",File.c_str());
        throw Exception("unsupported audio format");
      } 
      
      /* Decode entire .ogg */
      unsigned char *pcRaw = new unsigned char[vs.getLength()];    
      vs.decode(pcRaw,vs.getLength());
              
      /* Convert */
      pSample->nBufSize = (int)((float)vs.getLength() * pSample->cvt.len_ratio);
      pSample->pcBuf = new unsigned char[pSample->nBufSize * 4]; /* stupid, but i'm lazy */
      memcpy(pSample->pcBuf,pcRaw,vs.getLength());
      pSample->cvt.buf = (Uint8 *)pSample->pcBuf;
      pSample->cvt.len = vs.getLength();
      
      pSample->Name = File;
      
      delete [] pcRaw;
      
      /* Register and return */    
      vs.closeFile();
      m_Samples.push_back(pSample);
      
      return pSample;
    #endif
  }
  
  void Sound::playSample(SoundSample *pSample,float fVolume) {
    if(pSample == NULL || !isEnabled()) return;

    #if USE_SDL_MIXER      
      /* WHY OH WHY does this pause the game thread for 200-300 ms on linux??? :( */
      int nChannel = Mix_PlayChannel(-1,pSample->pChunk,0);
      if(nChannel >= 0) {
        Mix_Volume(nChannel,(int)(fVolume * MIX_MAX_VOLUME));
      }     
      
    #else /* Not using SDL_mixer */  
      int nSlot = _GetFreePlayerSlot();
      if(nSlot<0) return;
      
      SamplePlayer *pPlayer = new SamplePlayer;
      pPlayer->initSample(pSample);
      m_pPlayers[nSlot] = (SoundPlayer *)pPlayer;
    #endif
  }
  
  SoundSample *Sound::findSample(const std::string &File) {
    for(int i=0;i<m_Samples.size();i++) {
      if(m_Samples[i]->Name == File)
        return m_Samples[i];
    }
    return NULL;
  }
  
  void Sound::playSampleByName(const std::string &Name,float fVolume) {
    SoundSample *pSample = findSample(Name);
    if(pSample != NULL) {
      playSample(pSample,fVolume);
    }
  }

  /*==============================================================================
  Engine sound simulator
  ==============================================================================*/
  void EngineSoundSimulator::update(float fTime) {
    if(Sound::isEnabled()) {
      if(m_fRPM > 100.0f) {
        /* Calculate the delay between the samples */
        float fInterval = (60.0f / m_fRPM) * 1.0f;
        
        if(fTime - m_fLastBangTime > fInterval) {
          /* Stroke! Determine a random sample to use */
          float x = ((float)rand()) / (float)RAND_MAX; /* linux likes insanely high RAND_MAX'es */
          int i = (int)(((float)m_BangSamples.size())*x);
          if(i<0) i = 0;
          if(i>=m_BangSamples.size()) i = m_BangSamples.size()-1;
          
          #if defined(USE_SDL_MIXER)
            /* Play it */
            Mix_PlayChannel(-1,m_BangSamples[i]->pChunk,0);
          #endif
          
          m_fLastBangTime = fTime;
        }
      }
    }
  }

  void Sound::playMusic(std::string i_musicPath) {
    if(Mix_PlayingMusic() || Mix_FadingMusic()) {
      stopMusic();
    }

    if(m_pMenuMusic != NULL) {
      Mix_FreeMusic(m_pMenuMusic);
      m_pMenuMusic = NULL;
    }

    /* No music available, try loading */
#if defined(WIN32) /* this works around a bug in SDL_mixer 1.2.7 on Windows */
    SDL_RWops *rwfp;
    rwfp = SDL_RWFromFile(i_musicPath.c_str(), "rb");
    m_pMenuMusic = Mix_LoadMUS_RW(rwfp);
#else
    m_pMenuMusic = Mix_LoadMUS(i_musicPath.c_str());
#endif

    if(m_pMenuMusic == NULL) {
      throw Exception("No music played !");
    }

    if(Mix_PlayMusic(m_pMenuMusic, -1) < 0) {
      throw Exception("No music played !");
    }
  }

  void Sound::stopMusic() {
    if(m_pMenuMusic != NULL && Mix_PlayingMusic()) {
      Mix_HaltMusic();
    }
  }

  bool Sound::isPlayingMusic() {
    return Mix_PlayingMusic();
  }
}

