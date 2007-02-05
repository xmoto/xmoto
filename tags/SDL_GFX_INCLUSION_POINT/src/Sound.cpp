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
  }
  
  ///*===========================================================================
  //Audio callback (from seperate thread, so please be careful)
  //===========================================================================*/
  //void Sound::audioCallback(void *pvUserData,unsigned char *pcStream,int nLen) {    
  //  /* Go trough player slots... */
  //  for(int i=0;i<16;i++) {
  //    if(m_pPlayers[i] != NULL) {
  //      m_pPlayers[i]->fillBuffer(pcStream,nLen);
  //      break;
  //    }
  //  }
  //}
  //
  ///*===========================================================================
  //Opening and closing of vorbis file
  //===========================================================================*/
  //void VorbisSound::openFile(std::string File) {    
  //  /* Open source */
  //  m_pfh = FS::openIFile(File);
  //  if(m_pfh == NULL) throw Exception("(1) failed to open ogg stream");
  //  
  //  /* Create decoding handle */
  //  VorbisSoundHandle *pHandle = new VorbisSoundHandle;
  //  pHandle->pfh = m_pfh;
  //  pHandle->pVorbis = this;
  //  pHandle->pvf = &m_vf;
  //  m_ph = pHandle;
  //  
  //  memset(&m_vf,0,sizeof(m_vf));
  //  
  //  /* Init vorbis libraries */
  //	if(ov_open_callbacks(pHandle,&m_vf,NULL,0,m_Callbacks) < 0) {
  //	  delete pHandle;
  //	  Log("** Warning ** : invalid ogg stream '%s'",File.c_str());
  //	  throw Exception("(2) failed to open ogg stream");
  //	}    
  //	
  //	m_nCurrentSection = 0;

	 // /* Extract info */
	 // vorbis_info *vi=ov_info(&m_vf,-1);
  //	
	 // m_nSampleBits=16;
	 // m_nChannels=vi->channels;
	 // m_nSampleRate=vi->rate;
	 // m_nAvgBitRate=vi->bitrate_nominal;

	 // /* Calculate length */
	 // int nBytesPerSample=m_nChannels * (m_nSampleBits/8);
	 // m_nLength=ov_pcm_total(&m_vf,-1) * nBytesPerSample;

  //  /* We are now ready to stream */	  
  //  m_bEnd = false;
  //}
  //
  //void VorbisSound::closeFile(void) {
  //  if(m_ph) {
  //    delete m_ph;
  //  }    
  //	ov_clear(&m_vf);  	
  //	if(m_pfh) {
  //	  FS::closeFile(m_pfh);  	    	  
  //	}
  //	
  //	m_ph = NULL;
  //	m_pfh = NULL;
  //	m_bEnd = true;
  //}
  //
  ///*==============================================================================
  //Decoding
  //==============================================================================*/
  //int VorbisSound::decode(unsigned char *pcBuf,int nBufSize) {
  //  int nRem = nBufSize;
  //  int i = 0;
  //  while(nRem>0) {
  //    int nNumRead = ov_read(&m_vf,(char *)&pcBuf[i],nRem,0,2,1,&m_nCurrentSection);
  //    if(nNumRead == 0) {
  //      /* EOF */
  //      m_bEnd = true;
  //      break;
  //    }
  //    else if(nNumRead < 0) {
  //      /* Error */
  //      m_bEnd = true;
  //      break;
  //    }
  //    else {
  //      nRem-=nNumRead;
  //      i+=nNumRead;
  //    }
  //  }
  //  //printf("got %d bytes from stream\n",i);
  //  return i;
  //}
  //
  //bool VorbisSound::endOfStream(void) {
  //  return m_bEnd;
  //}

  ///*==============================================================================
  //Vorbis callbacks
  //==============================================================================*/
  //size_t VorbisSound_readc(void *pvPtr,size_t Size,size_t Blocks,void *pvDataSource) {
  //  VorbisSoundHandle *pHandle = (VorbisSoundHandle *)pvDataSource;
  //  if(pHandle != NULL) {
  //    int nRem = FS::getLength(pHandle->pfh) - FS::getOffset(pHandle->pfh);
  //    int nRead = nRem < Blocks ? nRem : Blocks;
  //    if(FS::readBuf(pHandle->pfh,(char *)pvPtr,Size*nRead)) {
  //      return nRead;
  //    }
  //  }
	 // return 0;
  //}

  //int VorbisSound_seekc(void *pvDataSource,ogg_int64_t Offset,int nWhence) {
  //  VorbisSoundHandle *pHandle = (VorbisSoundHandle *)pvDataSource;
  //  if(pHandle != NULL) {
		//  switch(nWhence) {
		//	  case SEEK_SET:
		//	    FS::setOffset(pHandle->pfh,Offset);
		//		  break;
		//	  case SEEK_END:
		//	    FS::setOffset(pHandle->pfh,FS::getLength(pHandle->pfh) + Offset);
		//		  break;
		//	  case SEEK_CUR:
		//	    FS::setOffset(pHandle->pfh,FS::getOffset(pHandle->pfh) + Offset);
		//		  break;
		//  }
		//  return 0;
  //  }
	 // return -1;	
  //}

  //int VorbisSound_closec(void *pvDataSource) {
  //  VorbisSoundHandle *pHandle = (VorbisSoundHandle *)pvDataSource;
  //  if(pHandle != NULL) {
  //  }    
	 // return 0;
  //}

  //long VorbisSound_tellc(void *pvDataSource) {
  //  VorbisSoundHandle *pHandle = (VorbisSoundHandle *)pvDataSource;
  //  if(pHandle != NULL) {
  //    return FS::getOffset(pHandle->pfh);
  //  }
	 // return 0;
  //}
  //
  ///* Callbacks struct */
  //ov_callbacks VorbisSound::m_Callbacks={  
  //  VorbisSound_readc,
  //  VorbisSound_seekc,
  //  VorbisSound_closec,
  //  VorbisSound_tellc
  //};
  
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
      
//  /*==============================================================================
//  Stream playing
//  ==============================================================================*/
//  void Sound::playStream(std::string File) {
//    #if USE_SDL_MIXER
//      /* .. */
//    #else /* Not using SDL_mixer */
//      int nSlot = _GetFreePlayerSlot();
//      if(nSlot<0) return; /* TODO: add request to some kind of queue, so we can
//                                  play it later */    
//      StreamPlayer *pStream = new StreamPlayer;
//      pStream->initStream(File,1024*100);
//      m_pPlayers[nSlot] = (SoundPlayer *)pStream;
//    #endif
//  }
//
//  void StreamPlayer::initStream(std::string File,int nBufSize) {
//    m_vs.openFile(File);
//    //printf("Streaming '%s'...\n",File.c_str());
//    m_pcBuf = new unsigned char[nBufSize];
//    m_nWritePos = m_nReadPos = 0;
//    m_nBufSize = nBufSize;
//    m_bNew = true;
//    
//    /* Build conversion structure */    
//    Uint16 nSrcFormat,nDstFormat;
//    Uint8 nSrcChannels,nDstChannels;
//    int nSrcRate,nDstRate;
//    
//    if(Sound::getSampleBits() == 8) nDstFormat = AUDIO_S8;
//    else nDstFormat = AUDIO_S16;
//    
//    nDstChannels = Sound::getChannels();
//    nDstRate = Sound::getSampleRate();
//    
//    if(m_vs.getSampleBits() == 8) nSrcFormat = AUDIO_S8;
//    else nSrcFormat = AUDIO_S16;
//    
//    nSrcChannels = m_vs.getChannels();
//    nSrcRate = m_vs.getSampleRate();
//    
//    //printf("STREAM:  [%d-bit, %dhz, %d-channel] -> [%d-bit, %dhz, %d-channel]\n",
//    //        nSrcFormat&0xff,nSrcRate,nSrcChannels,nDstFormat&0xff,nDstRate,nDstChannels);
//            
//    if(SDL_BuildAudioCVT(&m_cvt,nSrcFormat,nSrcChannels,nSrcRate,nDstFormat,nDstChannels,nDstRate) < 0) {
//      m_vs.closeFile();
//      Log("** Warning ** : StreamPlayer::initStream() : Failed to prepare audio conversion for '%s'!",File.c_str());
//      throw Exception("unsupported audio format");
//    }    
//    
//    /* Fill buffer */
//    update();
//  }
//
//  int StreamPlayer::fillBuffer(unsigned char *pcBuffer,int nSize) {    
//    int nRem = m_nBufSize - m_nReadPos;
//    
//    if(nSize <= nRem) {
//      memcpy(pcBuffer,&m_pcBuf[m_nReadPos],nSize);
//      m_nReadPos += nSize;
//      m_nReadPos = m_nReadPos % m_nBufSize;
//      return nSize;
//    }
//    
//    memcpy(pcBuffer,&m_pcBuf[m_nReadPos],nRem);
//    memcpy(&pcBuffer[nRem],&m_pcBuf[0],nSize - nRem);
//    m_nReadPos = nSize - nRem; 
//    return nSize;
//  }
//  
//  void StreamPlayer::update(void) {
//    /* Determine how much we should decode, if anything */
//    int nDecode;    
//    if(m_nWritePos <= m_nReadPos) {
//      nDecode = m_nReadPos - m_nWritePos;
//    }
//    else {
//      nDecode = m_nBufSize - m_nWritePos + m_nReadPos;
//    }
//    
//    if(m_bNew) {
//      nDecode = m_nBufSize;
//      m_bNew = false;
//    }
//    
//    static unsigned char cLocalBuffer[400000]; /* static because ::update() of
//                                                  streams are not called at the same time */
//    if(nDecode > 0) {
//      /* How much SOURCE data should we fetch then? */
//      int nSrcDecode = nDecode / m_cvt.len_ratio;
//    
//      int nDecodedSrc = m_vs.decode(cLocalBuffer,nSrcDecode);
//      
//      m_cvt.buf = (Uint8 *)cLocalBuffer;
//      m_cvt.len = nDecodedSrc;
//      
//      SDL_ConvertAudio(&m_cvt);
//      
//      int nActualNewData = nDecodedSrc * m_cvt.len_ratio;
////      printf("got %d bytes in dest-format\n",nActualNewData);
//      
//      int nRem = m_nBufSize - m_nWritePos;
//      if(nActualNewData <= nRem) {
//        memcpy(&m_pcBuf[m_nWritePos],cLocalBuffer,nActualNewData);
//        m_nWritePos += nActualNewData;
//        m_nWritePos = m_nWritePos % m_nBufSize;
//      }
//      else {
//        memcpy(&m_pcBuf[m_nWritePos],cLocalBuffer,nRem);
//        memcpy(&m_pcBuf[0],&cLocalBuffer[nRem],nActualNewData-nRem);
//        m_nWritePos = nActualNewData-nRem;
//      }            
//    }
//  }  
//  
//  void StreamPlayer::shutdownPlayer(void) {
//    delete [] m_pcBuf;
//    m_vs.closeFile();    
//  }
//  
//  /*==============================================================================
//  Helpers
//  ==============================================================================*/
//  int Sound::_GetFreePlayerSlot(void) {
//    for(int i=0;i<16;i++) {
//      if(m_pPlayers[i] == NULL) return i;
//    }
//    return -1;
//  }

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
  
}

