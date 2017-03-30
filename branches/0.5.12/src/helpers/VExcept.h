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

#ifndef __VEXCEPT_H__
#define __VEXCEPT_H__

#include <string>

/*===========================================================================
  Exceptions
  ===========================================================================*/
class Exception {
  public:
  Exception(const std::string &iMsg){
   setMsg(iMsg);
  }

  Exception(const char *pc){
    setMsg(std::string(pc));
  }

  std::string &getMsg() {return m_Msg;}

  protected:
  void setMsg(std::string message){ 
      m_Msg = message;
      // printf("Exception :  %s \n",message.c_str());
  };
  private: 
  std::string m_Msg;
};
  
 
class SyntaxError : public Exception {
 public:
  SyntaxError(const std::string &iMsg)
    : Exception(iMsg) {}
  SyntaxError(const char *pc)
    : Exception(std::string(pc)) {}
 private:
};

class InvalidSystemKeyException : public Exception {
 public:
  InvalidSystemKeyException() : Exception("Invalid Key") {}
 private:
};

class DisconnectedException : public Exception {
 public:
  DisconnectedException() : Exception("Disconnected") {}
 private:
};

#endif

