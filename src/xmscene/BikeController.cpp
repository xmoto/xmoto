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

#include "BikeController.h"
#include "../helpers/VMath.h"
#include "../net/NetActions.h"
#include "../net/NetClient.h"

#define XM_MIN_DRIVE_DETECTION 0.1

BikeController::BikeController() {}

BikeController::~BikeController() {}

BikeControllerPlayer::BikeControllerPlayer() {
  m_brokenBreaks = false;
  stopControls();
}

BikeControllerPlayer::~BikeControllerPlayer() {}

float BikeControllerPlayer::Drive() const {
  return m_drive;
}

float BikeControllerPlayer::Pull() const {
  return m_pull;
}

bool BikeControllerPlayer::ChangeDir() const {
  return m_changeDir;
}

bool BikeControllerPlayer::isDriving() {
  return fabs(m_drive) > XM_MIN_DRIVE_DETECTION;
}

void BikeControllerPlayer::stopControls() {
  m_drive = 0.0;
  m_pull = 0.0;
  m_changeDir = false;
  m_throttle = 0.0;
  m_break = 0.0;
}

void BikeControllerPlayer::setThrottle(float i_throttle) {
  m_throttle = i_throttle;
  if (m_throttle > 0.0f) {
    m_drive = m_throttle;
  } else if (m_throttle == 0.0f) {
    m_drive = -m_break;
  }
}

void BikeControllerPlayer::setBreak(float i_break) {
  if (m_brokenBreaks) {
    return;
  }

  m_break = i_break;
  if (m_break > 0.0f) {
    m_drive = -m_break;
  } else if (m_break == 0.0f) {
    m_drive = m_throttle;
  }
}

void BikeControllerPlayer::setPull(float i_pull) {
  m_pull = i_pull;
}

void BikeControllerPlayer::setChangeDir(bool i_changeDir) {
  m_changeDir = i_changeDir;
}

void BikeControllerPlayer::breakBreaks() {
  m_break = 0.0;
  m_brokenBreaks = true;
}

BikeControllerNet::BikeControllerNet(int i_localNetId) {
  m_localNetId = i_localNetId;
}

BikeControllerNet::~BikeControllerNet() {}

void BikeControllerNet::setBreak(float i_break) {
  if (NetClient::instance()->isConnected()) {
    NA_playerControl na(PC_BRAKE, i_break);
    NetClient::instance()->send(&na, m_localNetId);
  }
}

void BikeControllerNet::setThrottle(float i_throttle) {
  if (NetClient::instance()->isConnected()) {
    NA_playerControl na(PC_THROTTLE, i_throttle);
    NetClient::instance()->send(&na, m_localNetId);
  }
}

void BikeControllerNet::setPull(float i_pull) {
  if (NetClient::instance()->isConnected()) {
    NA_playerControl na(PC_PULL, i_pull);
    NetClient::instance()->send(&na, m_localNetId);
  }
}

void BikeControllerNet::setChangeDir(bool i_changeDir) {
  if (NetClient::instance()->isConnected()) {
    NA_playerControl na(PC_CHANGEDIR, i_changeDir);
    NetClient::instance()->send(&na, m_localNetId);
  }
}

void BikeControllerNet::stopControls() {
  // nothing to reinitialize
}

void BikeControllerNet::setLocalNetId(int i_value) {
  m_localNetId = i_value;
}

bool BikeControllerNet::isDriving() {
  return false; // unable to get this information
}
