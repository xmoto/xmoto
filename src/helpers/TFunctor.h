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

#ifndef __TFUNCTOR_H_
#define __TFUNCTOR_H_

template<class TClass>
class TFunctor {
public:
  virtual void call(TClass *) = 0;
};

template<class TClass, typename TArg1>
class TFunctor1A : public TFunctor<TClass> {
private:
  void (TClass::*m_fpt)(TArg1);
  TArg1 m_argument1;

public:
  TFunctor1A(void (TClass::*i_fpt)(TArg1), TArg1 i_argument1) {
    m_fpt = i_fpt;
    m_argument1 = i_argument1;
  }

  virtual ~TFunctor1A() {}

  void call(TClass *i_object) { (*i_object.*m_fpt)(m_argument1); }
};

template<class TClass, typename TArg1>
class TFunctor1ARef : public TFunctor<TClass> {
private:
  void (TClass::*m_fpt)(const TArg1 &);
  TArg1 m_argument1;

public:
  TFunctor1ARef(void (TClass::*i_fpt)(const TArg1 &),
                const TArg1 &i_argument1) {
    m_fpt = i_fpt;
    m_argument1 = i_argument1;
  }

  virtual ~TFunctor1ARef() {}

  void call(TClass *i_object) { (*i_object.*m_fpt)(m_argument1); }
};

// Two arguments with the 2nd a reference
template<class TClass, typename TArg1, typename TArg2>
class TFunctor1A2ARef : public TFunctor<TClass> {
private:
  void (TClass::*m_fpt)(TArg1, const TArg2 &);
  TArg1 m_argument1;
  TArg2 m_argument2;

public:
  TFunctor1A2ARef(void (TClass::*i_fpt)(TArg1, const TArg2 &),
                  TArg1 i_argument1,
                  const TArg2 &i_argument2) {
    m_fpt = i_fpt;
    m_argument1 = i_argument1;
    m_argument2 = i_argument2;
  }

  virtual ~TFunctor1A2ARef() {}

  void call(TClass *i_object) { (*i_object.*m_fpt)(m_argument1, m_argument2); }
};

// Three arguments with the 2nd a reference
template<class TClass, typename TArg1, typename TArg2, typename TArg3>
class TFunctor1A2ARef3A : public TFunctor<TClass> {
private:
  void (TClass::*m_fpt)(TArg1, const TArg2 &, TArg3);
  TArg1 m_argument1;
  TArg2 m_argument2;
  TArg3 m_argument3;

public:
  TFunctor1A2ARef3A(void (TClass::*i_fpt)(TArg1, const TArg2 &, TArg3),
                    TArg1 i_argument1,
                    const TArg2 &i_argument2,
                    TArg3 i_argument3) {
    m_fpt = i_fpt;
    m_argument1 = i_argument1;
    m_argument2 = i_argument2;
    m_argument3 = i_argument3;
  }

  virtual ~TFunctor1A2ARef3A() {}

  void call(TClass *i_object) {
    (*i_object.*m_fpt)(m_argument1, m_argument2, m_argument3);
  }
};

#endif
