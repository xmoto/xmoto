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

#ifndef __BIKECONTROLER_H__
#define __BIKECONTROLER_H__

class BikeController {
  public:
  BikeController();
  virtual ~BikeController();

  virtual void setBreak(float i_break)        = 0;
  virtual void setThrottle(float i_throttle)  = 0;
  virtual void setPull(float i_pull)          = 0;
  virtual void setChangeDir(bool i_changeDir) = 0;
  virtual void stopControls()                 = 0; // inform the contoller that controls must stop

  virtual bool isDriving() = 0;
};

class BikeControllerPlayer : public BikeController {
  public:
  BikeControllerPlayer();
  virtual ~BikeControllerPlayer();

  virtual void setBreak(float i_break);
  virtual void setThrottle(float i_throttle);
  virtual void setPull(float i_pull);
  virtual void setChangeDir(bool i_changeDir);
  virtual void stopControls();

  float Drive()    const;
  float Pull()     const;
  bool  ChangeDir() const;

  virtual bool isDriving();
  void breakBreaks();

  private:
  float m_drive;    /* Throttle [0; 1] or Brake [-1; 0] */
  float m_pull;     /* Pull back on the handle bar [0; 1] or push forward on the handle bar [-1; 0] */
  bool m_changeDir; /* Change direction */

  // store this two key pressed in order to be able to brake while
  // accelerating, and going on accelerating when you release the
  // brakes
  // throttle [0; 1], break [0; 1]
  float m_throttle;
  float m_break;

  // broken key
  bool m_brokenBreaks;
};

class BikeControllerNet : public BikeController {
  public:
  BikeControllerNet(int i_localNetId);
  virtual ~BikeControllerNet();

  virtual void setBreak(float i_break);
  virtual void setThrottle(float i_throttle);
  virtual void setPull(float i_pull);
  virtual void setChangeDir(bool i_changeDir);

  virtual bool isDriving();
  virtual void stopControls();
  void setLocalNetId(int i_value);

  private:
  int m_localNetId;
};

#endif /* BIKECONTROLER */
