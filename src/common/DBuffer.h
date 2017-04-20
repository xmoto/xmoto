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

#ifndef __DBUFFER_H__
#define __DBUFFER_H__

#include "VCommon.h"
#include "helpers/VExcept.h"
#include <vector>

  /*===========================================================================
  Output buffer section
  ===========================================================================*/
  struct DBufferPart {
    char *pcBuffer;
    int nPtr;
  };

  /*===========================================================================
  I/O buffer class
  ===========================================================================*/
  class DBuffer {
    public:
      DBuffer() {
        m_bOwnData = m_bInit = m_bOutput = false;
        m_nPartSize = m_nSize = 0;
        m_pcData = NULL;
      }
      virtual ~DBuffer() {_FreeDBuffer();}
    
      /* Methods */
      void initOutput(int nPartSize);
      void initInput(char *pcInput,int nInputSize);
      
      template<typename _ConstIter>
      void writeBuf(_ConstIter pcBuf,int nBufSize);
      
      template<typename _Iter>
      void readBuf(_Iter pcBuf,int nBufSize);
      
      void writeBuf_LE(const char *pcBuf,int nBufSize);    
      void readBuf_LE(char *pcBuf,int nBufSize);
      int numRemainingBytes(void);
      const char *convertOutputToInput(void);
      
      /* Some I/O */
      void operator <<(bool n);
      void operator >>(bool &n);
      void operator <<(int n);
      void operator >>(int &n);
      void operator <<(unsigned char c);
      void operator >>(unsigned char &c);
      void operator <<(unsigned int n);
      void operator >>(unsigned int &n);
      void operator <<(unsigned long n);
      void operator >>(unsigned long &n);
      void operator <<(float n);
      void operator >>(float &n);
      void operator <<(std::string s);
      void operator >>(std::string &s); 
    
      /* Data interface */
      bool isOutput(void) {if(m_bInit && m_bOutput) return true; return false;}
      bool isInput(void) {if(m_bInit && !m_bOutput) return true; return false;}

      void clear();
      // copy the buffer to i_str ; throw en exception if maxLen is not big enough ; return the number of bytes copied
      int copyTo(char* i_str, int maxLen);
      bool isEmpty() const;

    private:
      /* Data */
      bool m_bInit;
      bool m_bOutput;

      /* Data - for output */    
      int m_nPartSize;
      std::vector<DBufferPart *> m_Parts;
      int m_nCurPart;    
      
      /* Data - for input */    
      int m_nSize;
      int m_nReadPtr;
      char *m_pcData;
      bool m_bOwnData;
      
      /* Helpers */
      void _FreeDBuffer(void);
      void _NewPart(void);
  };

#endif

