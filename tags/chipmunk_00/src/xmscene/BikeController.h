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

  float Drive() const;
  float Pull() const;
  bool ChangeDir() const;

  // joystick
  void setDrive(float i_drive);

  // keyboard
  void setBreak(float i_break);
  void setThrottle(float i_throttle);

  void setPull(float i_pull);
  void setChangeDir(bool i_changeDir);

  void stopContols();

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

#endif /* BIKECONTROLER */
