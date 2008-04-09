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
 *  Generic data buffer stuff.
 */
#include "DBuffer.h"

namespace vapp {

  void DBuffer::_FreeDBuffer(void) {
    /* Free stuff if anything */
    if(m_pcData != NULL && m_bOwnData) delete [] m_pcData;
    for(int i=0;i<m_Parts.size();i++) {
      delete [] m_Parts[i]->pcBuffer;
      delete m_Parts[i];
    }
  }

  void DBuffer::initOutput(int nPartSize) {
    /* Initialize buffer for output */
    m_bInit = true;
    m_bOutput = true; 
    m_nPartSize = nPartSize;
    m_nCurPart = 0;
  }

  void DBuffer::initInput(char *pcInput,int nInputSize) {
    /* Initialize buffer for input */
    m_bInit = true;
    m_bOutput = false;
    m_nReadPtr = 0;
    m_nSize = nInputSize;
    m_pcData = pcInput;
    m_bOwnData = false;
  }

  void DBuffer::writeBuf(const char *pcBuf,int nBufSize) {
    if(isOutput() && nBufSize > 0) {    
      /* Start writing into the part */
      int nToWrite = nBufSize,i = 0;
      while(nToWrite > 0) {
        /* Determine whether we need to create a new part */  
        bool bNewPart = false;
        if(m_Parts.empty()) bNewPart = true;
        else if (m_Parts[m_nCurPart]->nPtr == m_nPartSize) bNewPart = true;
        
        /* New part? */
        if(bNewPart) {
          _NewPart();
        }
        
        /* What's left? */
        int nRem = m_nPartSize - m_Parts[m_nCurPart]->nPtr;
        int nWrite = nRem < nBufSize-i ? nRem : nBufSize-i;
        
        /* Write it */
        memcpy(&m_Parts[m_nCurPart]->pcBuffer[m_Parts[m_nCurPart]->nPtr],&pcBuf[i],nWrite);
        i += nWrite;
        m_Parts[m_nCurPart]->nPtr += nWrite;
        nToWrite -= nWrite;      
      }
    }   
  }

  void DBuffer::_NewPart(void) {
    DBufferPart *p = new DBufferPart;
    p->nPtr = 0;
    p->pcBuffer = new char [m_nPartSize];
    m_nCurPart = m_Parts.size();
    m_Parts.push_back( p );
  }

  void DBuffer::readBuf(char *pcBuf,int nBufSize) {
    if(isInput() && nBufSize > 0) {
      /* Remaining in input buffer? */
      if(m_nSize - m_nReadPtr < nBufSize) {
        /* TODO: error */
        memset(pcBuf,0,nBufSize);
      }
      else {
        /* Read and advance ptr */
        memcpy(pcBuf,&m_pcData[m_nReadPtr],nBufSize);
        m_nReadPtr += nBufSize;
      }
    }
  }

  int DBuffer::numRemainingBytes(void) {  
    if(isInput()) {
      return m_nSize - m_nReadPtr;
    }
    return 0;
  }

  const char *DBuffer::convertOutputToInput(void) {
    if(isOutput()) {
      /* Build full buffer */
      m_nSize = 0;
      for(int i=0;i<m_Parts.size();i++) {
        if(i == m_Parts.size() - 1) m_nSize += m_Parts[i]->nPtr;
        else m_nSize += m_nPartSize;
      }

      m_pcData = new char [m_nSize];
      int k = 0;
      for(int i=0;i<m_Parts.size();i++) {
        int w;
        if(i == m_Parts.size() - 1) w = m_Parts[i]->nPtr;
        else w = m_nPartSize;
        memcpy(&m_pcData[k],m_Parts[i]->pcBuffer,w);
        k+=w;
      }
      
      /* Set up params */
      m_bOutput = false;
      m_nReadPtr = 0;
      m_bOwnData = true;
      
      return (const char *)m_pcData;
    }
    return NULL;
  }

};
