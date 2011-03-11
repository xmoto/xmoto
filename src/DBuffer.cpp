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

/* 
 *  Generic data buffer stuff.
 */
#include "DBuffer.h"
#include "helpers/SwapEndian.h"
#include <algorithm>

  void DBuffer::_FreeDBuffer(void) {
    /* Free stuff if anything */
    if(m_pcData != NULL && m_bOwnData) delete [] m_pcData;
    for(unsigned int i=0;i<m_Parts.size();i++) {
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

  template <typename _ConstIter>
  void DBuffer::writeBuf(_ConstIter pcBuf,int nBufSize) {
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
        std::copy(pcBuf, pcBuf + nWrite,
          &m_Parts[m_nCurPart]->pcBuffer[m_Parts[m_nCurPart]->nPtr]);
        pcBuf += nWrite;
        i += nWrite;
        m_Parts[m_nCurPart]->nPtr += nWrite;
        nToWrite -= nWrite;      
      }
    }   
  }
  
  // Instantiations
  template void DBuffer::writeBuf(const char *, int);
  template void DBuffer::writeBuf(char *, int);
  
  void DBuffer::writeBuf_LE(const char *pcBuf,int nBufSize) {
    if (SwapEndian::bigendien) {
      writeBuf(std::reverse_iterator<const char *>(pcBuf + nBufSize), nBufSize);
    } else {
      writeBuf(pcBuf, nBufSize);
    }
  }

  void DBuffer::_NewPart(void) {
    DBufferPart *p = new DBufferPart;
    p->nPtr = 0;
    p->pcBuffer = new char [m_nPartSize];
    m_nCurPart = m_Parts.size();
    m_Parts.push_back( p );
  }

  template <typename _Iter>
  void DBuffer::readBuf(_Iter pcBuf,int nBufSize) {
    if(numRemainingBytes() < nBufSize) {
      throw Exception("Unable to read the data");
    }

    if(isInput() && nBufSize > 0) {
      /* Remaining in input buffer? */
      if(m_nSize - m_nReadPtr < nBufSize) {
        /* TODO: error */
        std::fill_n(pcBuf, nBufSize, 0);
      }
      else {
        /* Read and advance ptr */
        std::copy(&m_pcData[m_nReadPtr], &m_pcData[m_nReadPtr] + nBufSize,
          pcBuf);
        m_nReadPtr += nBufSize;
      }
    }
  }

  // Instantiations
  template void DBuffer::readBuf(char *, int);

  void DBuffer::readBuf_LE(char *pcBuf,int nBufSize) {
    readBuf(SwapEndian::LittleIter(pcBuf, nBufSize), nBufSize);
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
      for(unsigned int i=0;i<m_Parts.size();i++) {
        if(i == m_Parts.size() - 1) m_nSize += m_Parts[i]->nPtr;
        else m_nSize += m_nPartSize;
      }

      m_pcData = new char [m_nSize];
      int k = 0;
      for(unsigned int i=0;i<m_Parts.size();i++) {
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

  void DBuffer::operator <<(bool n) {
    unsigned char c;
    c = static_cast<unsigned char>(n);
    writeBuf_LE((char*) &c, 1);
  }
  
  void DBuffer::operator >>(bool &n) {
    unsigned char c;
    readBuf_LE((char*) &c, 1);
    n = (c != 0x00);
  }
  
  void DBuffer::operator <<(int n) {
    int32_t nv = static_cast<int32_t>(n);  
    writeBuf_LE((char *)&nv, 4);
  }
  
  void DBuffer::operator >>(int &n) {
    int32_t nv;
    readBuf_LE((char *)&nv, 4);
    n = static_cast<int>(nv);
  }

  void DBuffer::operator <<(unsigned char c) {
    writeBuf((char *)&c,sizeof(c));
  }

  void DBuffer::operator >>(unsigned char &c) {
    readBuf((char*) &c, 1);
  }
 
  void DBuffer::operator <<(unsigned int n) {
    int sn;
    sn = (int) (n);
    *this << sn;
  }
  
  void DBuffer::operator >>(unsigned int &n) {
    int sn;
    *this >> sn;
    n = (unsigned int) sn;
  }
  
  void DBuffer::operator <<(unsigned long n) {
      int sn;
      sn = (int) (n);
      *this << sn;
  }
  
  void DBuffer::operator >>(unsigned long &n) {
      int sn;
      *this >> sn;
      n = (unsigned long) sn;
  }
  
  void DBuffer::operator <<(float n) {
    writeBuf_LE((char *)&n, sizeof(float));
  }
  
  void DBuffer::operator >>(float &n) {
    readBuf_LE((char *)&n, sizeof(float));
  }

  void DBuffer::operator <<(std::string s) {
    *this << (unsigned int)(s.length());
    this->writeBuf(s.c_str(), s.length());
  }
   
  void DBuffer::operator >>(std::string &s) {
    int n;
    char c[512];
    *this >> n;

    if(n <= 0) {
      throw Exception("Unable to read the string !");
    }
    if(n >= 512) {
      throw Exception("Unable to read the string (max allowed length is 512)");
    }

    this->readBuf(c, n);
    c[n] = '\0';
    s = c;
  }

void DBuffer::clear() {
  /* keep only the first part */
  while(m_Parts.size() > 1) {
    delete [] m_Parts[1]->pcBuffer;
    delete m_Parts[1];
    m_Parts.erase(m_Parts.begin() + 1);
  }

  if(m_Parts.size() == 1) {
    m_Parts[0]->nPtr = 0; // clear the first part
    m_nCurPart = 0;
  }
}

int DBuffer::copyTo(char* i_str, int maxLen) {
  int v_cur = 0;

  for(unsigned int i=0; i<m_Parts.size(); i++) {
    if(m_Parts[i]->nPtr != 0) {
      if(v_cur + m_Parts[i]->nPtr > maxLen) {
	throw Exception("DBuffer::copyTo : destination too small");
      }
      memcpy(i_str, m_Parts[i]->pcBuffer, m_Parts[i]->nPtr);
      v_cur += m_Parts[i]->nPtr;
   }
  }

  return v_cur;
}

bool DBuffer::isEmpty() const {
  for(unsigned int i=0; i<m_Parts.size(); i++) {
    if(m_Parts[i]->nPtr != 0) {
      return false;
    }
  }
  
  return true;
}
