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

#ifndef __SOUND_H__
#define __SOUND_H__

#include "VCommon.h"
#include "VApp.h"
#include "VFileIO.h"
#include "UserConfig.h"

//#include "vorbis/codec.h"
//#include "vorbis/vorbisfile.h"

namespace vapp {

	/*===========================================================================
	Sound sample
  ===========================================================================*/
  struct SoundSample {
    int nBufSize;
    unsigned char *pcBuf;
    SDL_AudioCVT cvt;
    std::string Name;
    
    /* Used by SDL_mixer */
    Mix_Chunk *pChunk;
  };

	/*===========================================================================
	Engine sound simulator (single cylinder, 4-stroke)
  ===========================================================================*/
  class EngineSoundSimulator {
    public:
      EngineSoundSimulator() {m_fRPM = 0.0f; m_fLastBangTime=0.0f;}
      
      /* Methods */
      void update(float fTime);
      
      /* Data interface */
      void setRPM(float f) {m_fRPM = f;}
      float getRPM(void) {return m_fRPM;}
      void addBangSample(SoundSample *pSample) {if(pSample!=NULL) m_BangSamples.push_back(pSample);}
    
    private:
      /* Data */
      std::vector<SoundSample *> m_BangSamples;
      float m_fRPM;
      float m_fLastBangTime;
  };
 
	/*===========================================================================
	Sound system object
  ===========================================================================*/  
  class Sound {
    public:
      /* Static functions */
      static void init(UserConfig *pConfig);
      static void uninit(void);
      
      static void update(void);
      
      //static void playStream(std::string File);
      static SoundSample *loadSample(const std::string &File);
      static void playSample(SoundSample *pSample,float fVolume=1.0f);
      static SoundSample *findSample(const std::string &File);
      static void playSampleByName(const std::string &Name,float fVolume=1.0f);
      
      /* Data interface */
      static bool isEnabled(void) {return m_bEnable;}
      static int getSampleRate(void) {return m_nSampleRate;}
      static int getSampleBits(void) {return m_nSampleBits;}
      static int getChannels(void) {return m_nChannels;}
      static int getNumSamples(void) {return m_Samples.size();}
    
      static void playMusic(std::string i_musicPath);
      static void stopMusic();
      static bool isPlayingMusic();

    private:
      /* SDL audio callback */
      //static void audioCallback(void *pvUserData,unsigned char *pcStream,int nLen); 
      
      /* SDL_mixer callbacks (RWops) */
      static int RWops_seek(SDL_RWops *context,int offset,int whence);
      static int RWops_read(SDL_RWops *context,void *ptr,int size,int maxnum);
      static int RWops_write(SDL_RWops *context,const void *ptr,int size,int num);
      static int RWops_close(SDL_RWops *context);      
    
      /* Data */
      static bool m_bEnable;               /* From config: AudioEnable */
      static int m_nSampleRate;            /* From config: AudioSampleRate */
      static int m_nSampleBits;            /* From config: AudioSampleBits */
      static int m_nChannels;              /* From config: AudioChannels */
      
      //static SDL_AudioSpec m_ASpec;
      //
      //static SoundPlayer *m_pPlayers[16];
      
      static std::vector<SoundSample *> m_Samples;
      
      static Mix_Music *m_pMenuMusic;

      /* Helpers */
      //static int _GetFreePlayerSlot(void);
  };

}

#endif

