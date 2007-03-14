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
 *  Physics simulation part (using ODE)
 */
#include "MotoGame.h"
#include "PhysSettings.h"

namespace vapp {

  /*===========================================================================
  Init physics
  ===========================================================================*/
  void MotoGame::_InitPhysics(void) {
    m_bFirstPhysicsUpdate = true;
  
    /* Setup ODE */
    m_WorldID = dWorldCreate();
    dWorldSetERP(m_WorldID,PHYS_WORLD_ERP);
    dWorldSetCFM(m_WorldID,PHYS_WORLD_CFM);
        
    dWorldSetGravity(m_WorldID,m_PhysGravity.x,m_PhysGravity.y,0);    
    
    m_ContactGroup = dJointGroupCreate(0);
    
    dWorldSetQuickStepNumIterations(m_WorldID,PHYS_QSTEP_ITERS);

    /* Set default bike parameters */
    m_BikeS.Parameters().setDefaults();
  }

  /*===========================================================================
  Uninit physics
  ===========================================================================*/
  void MotoGame::_UninitPhysics(void) {
    dJointGroupDestroy(m_ContactGroup);
    dWorldDestroy(m_WorldID);
    dCloseODE();
  }

  /*===========================================================================
  Determine and return the engine RPMs
  ===========================================================================*/
  float MotoGame::getBikeEngineRPM(void) {
    return m_BikeS.fBikeEngineRPM;
  }

  float MotoGame::getBikeEngineSpeed() {
    float fWheelAngVel;
    float speed;

    if(m_BikeS.Dir == DD_RIGHT) {
      fWheelAngVel = dBodyGetAngularVel(m_RearWheelBodyID)[2];
    } else {
      fWheelAngVel = dBodyGetAngularVel(m_FrontWheelBodyID)[2];
    }

    speed = (fWheelAngVel * PHYS_WHEEL_RADIUS * 3.6);
    return speed >= 0.0 ? speed : -speed;
  }

  /*===========================================================================
  Update physics
  ===========================================================================*/
  void MotoGame::_UpdatePhysics(float fTimeStep) {
    /* No wheel spin per default */
    m_bWheelSpin = false;

    /* Update gravity vector */
    dWorldSetGravity(m_WorldID,m_PhysGravity.x,m_PhysGravity.y,0);    

    /* Should we disable stuff? Ok ODE has an autodisable feature, but i'd rather
      roll my own here. */
    const dReal *pfFront = dBodyGetLinearVel(m_FrontWheelBodyID);
    const dReal *pfRear = dBodyGetLinearVel(m_RearWheelBodyID);
    const dReal *pfFrame = dBodyGetLinearVel(m_FrameBodyID);
    #define SLEEP_EPS PHYS_SLEEP_EPS
    if(fabs(pfFront[0]) < SLEEP_EPS && fabs(pfFront[1]) < SLEEP_EPS &&
      fabs(pfRear[0]) < SLEEP_EPS && fabs(pfRear[1]) < SLEEP_EPS &&
      fabs(pfFrame[0]) < SLEEP_EPS && fabs(pfFrame[1]) < SLEEP_EPS) {
      m_nStillFrames++;
    }
    else {
      m_nStillFrames=0;
    }
    bool bSleep = false;        
    
    //printf("{%d}\n",m_Collision.isDynamicTouched());
    //printf("]",m_nStillFrames);        
    if(m_nStillFrames > PHYS_SLEEP_FRAMES && !m_Collision.isDynamicTouched()) {
      bSleep = true;
      dBodyDisable(m_FrontWheelBodyID);
      dBodyDisable(m_RearWheelBodyID);
      dBodyDisable(m_FrameBodyID);
    }
    else {
      if(!dBodyIsEnabled(m_FrontWheelBodyID)) dBodyEnable(m_FrontWheelBodyID);
      if(!dBodyIsEnabled(m_RearWheelBodyID)) dBodyEnable(m_RearWheelBodyID);
      if(!dBodyIsEnabled(m_FrameBodyID)) dBodyEnable(m_FrameBodyID);
    }
    
    //printf("%d",bSleep);

    if(!bSleep) {
      /* Update front suspension */
      Vector2f Fq = m_BikeS.RFrontWheelP - m_BikeS.FrontWheelP;
      Vector2f Fqv = Fq - m_BikeS.PrevFq;
      Vector2f FSpring = Fq * PHYS_SUSP_SPRING; 
      Vector2f FDamp = Fqv * PHYS_SUSP_DAMP;
      Vector2f FTotal = FSpring + FDamp; 
      if(m_bodyDetach == false || m_BikeS.Dir == DD_LEFT) { 
  dBodyAddForce(m_FrontWheelBodyID,FTotal.x,FTotal.y,0);
  dBodyAddForceAtPos(m_FrameBodyID,-FTotal.x,-FTotal.y,0,m_BikeS.RFrontWheelP.x,m_BikeS.RFrontWheelP.y,0);
      }
      m_BikeS.PrevFq = Fq;

      /* Update rear suspension */
      Vector2f Rq = m_BikeS.RRearWheelP - m_BikeS.RearWheelP;
      Vector2f Rqv = Rq - m_BikeS.PrevRq;
      Vector2f RSpring = Rq * PHYS_SUSP_SPRING; 
      Vector2f RDamp = Rqv * PHYS_SUSP_DAMP;
      Vector2f RTotal = RSpring + RDamp;   
      if(m_bodyDetach == false || m_BikeS.Dir == DD_RIGHT) {
  dBodyAddForce(m_RearWheelBodyID,RTotal.x,RTotal.y,0);
  dBodyAddForceAtPos(m_FrameBodyID,-RTotal.x,-RTotal.y,0,m_BikeS.RRearWheelP.x,m_BikeS.RRearWheelP.y,0);
      }
      m_BikeS.PrevRq = Rq;
      
      /* Have any of the suspensions reached the "squeek-point"? (rate of compression/decompression at 
         which they will make squeeky noises) */
      if(Fqv.length() > PHYS_SUSP_SQUEEK_POINT || Rqv.length() > PHYS_SUSP_SQUEEK_POINT) {
        /* Calculate how large a sqeek it should be */
        float fSqueekSize1 = Fqv.length() - PHYS_SUSP_SQUEEK_POINT;
        float fSqueekSize2 = Rqv.length() - PHYS_SUSP_SQUEEK_POINT;
        float fSqueekSize = fSqueekSize1 > fSqueekSize2 ? fSqueekSize1 : fSqueekSize2;
        float fMaxSqueek = 0.11f;        
        float fL = fSqueekSize / fMaxSqueek;
        if(fL > 1.0f) fL = 1.0f;
        m_fHowMuchSqueek = fL;                
        //printf("%f\n",fL);
                
        m_bSqueeking = true;
      }
    }    

    /* Apply attitude control (SIMPLISTIC!) */
    if((m_BikeC.Pull() != 0.0f) && (getTime() > m_fNextAttitudeCon)) {
      m_fAttitudeCon = m_BikeC.Pull() * PHYS_RIDER_ATTITUDE_TORQUE;
      m_fNextAttitudeCon = getTime() + (0.6f * fabsf(m_BikeC.Pull()));
    }
   
    if(m_fAttitudeCon != 0.0f) {
      m_nStillFrames=0;
      dBodyEnable(m_FrontWheelBodyID);
      dBodyEnable(m_RearWheelBodyID);
      dBodyEnable(m_FrameBodyID);
      dBodyAddTorque(m_FrameBodyID,0,0,m_fAttitudeCon);      
      
      //printf("AttitudeCon %f\n",m_fAttitudeCon);
    }
    
    m_fAttitudeCon *= PHYS_ATTITUDE_DEFACTOR;
    if(fabs(m_fAttitudeCon) < 100) { /* make sure we glue to zero */
      m_fAttitudeCon = 0.0f;
    }

    float fRearWheelAngVel = dBodyGetAngularVel(m_RearWheelBodyID)[2];
    float fFrontWheelAngVel = dBodyGetAngularVel(m_FrontWheelBodyID)[2];

    /* Misc */
    if(!bSleep) {
      if(fRearWheelAngVel > -PHYS_MAX_ROLL_VELOCITY && fRearWheelAngVel < PHYS_MAX_ROLL_VELOCITY)    
        dBodyAddTorque(m_RearWheelBodyID,0,0,-dBodyGetAngularVel(m_RearWheelBodyID)[2]*PHYS_ROLL_RESIST);
      else
        dBodyAddTorque(m_RearWheelBodyID,0,0,-dBodyGetAngularVel(m_RearWheelBodyID)[2]*PHYS_ROLL_RESIST_MAX);
      
      if(fFrontWheelAngVel > -PHYS_MAX_ROLL_VELOCITY && fFrontWheelAngVel < PHYS_MAX_ROLL_VELOCITY)    
        dBodyAddTorque(m_FrontWheelBodyID,0,0,-dBodyGetAngularVel(m_FrontWheelBodyID)[2]*PHYS_ROLL_RESIST);
      else
        dBodyAddTorque(m_FrontWheelBodyID,0,0,-dBodyGetAngularVel(m_FrontWheelBodyID)[2]*PHYS_ROLL_RESIST_MAX);
    }
   
    m_BikeS.physicalUpdate();
  
    /* Update RPM */
    /* Simply map the wheel ang vel to the RPM (stupid, lame, lots of bad stuff) */
    if(m_BikeS.Dir == DD_RIGHT) {
      float f = -fRearWheelAngVel;
      if(f<0.0f) f=0.0f;      
      m_BikeS.fBikeEngineRPM = ENGINE_MIN_RPM + (ENGINE_MAX_RPM - ENGINE_MIN_RPM) * (f / PHYS_MAX_ROLL_VELOCITY) * m_BikeC.Drive();
      if(m_BikeS.fBikeEngineRPM < ENGINE_MIN_RPM) m_BikeS.fBikeEngineRPM = ENGINE_MIN_RPM;
      if(m_BikeS.fBikeEngineRPM > ENGINE_MAX_RPM) m_BikeS.fBikeEngineRPM = ENGINE_MAX_RPM;
    }
    else if(m_BikeS.Dir == DD_LEFT) {
      float f = fFrontWheelAngVel;
      if(f<0.0f) f=0.0f;      
      m_BikeS.fBikeEngineRPM = ENGINE_MIN_RPM + (ENGINE_MAX_RPM - ENGINE_MIN_RPM) * (f / PHYS_MAX_ROLL_VELOCITY) * m_BikeC.Drive();
      if(m_BikeS.fBikeEngineRPM < ENGINE_MIN_RPM) m_BikeS.fBikeEngineRPM = ENGINE_MIN_RPM;
      if(m_BikeS.fBikeEngineRPM > ENGINE_MAX_RPM) m_BikeS.fBikeEngineRPM = ENGINE_MAX_RPM;
    }

    /* Apply motor/brake torques */
    if(m_BikeC.Drive() < 0.0f) {
      /* Brake! */        
      if(!bSleep) {
        //printf("Brake!\n");
      
        dBodyAddTorque(m_RearWheelBodyID,0,0,dBodyGetAngularVel(m_RearWheelBodyID)[2]*PHYS_BRAKE_FACTOR*m_BikeC.Drive());
        dBodyAddTorque(m_FrontWheelBodyID,0,0,dBodyGetAngularVel(m_FrontWheelBodyID)[2]*PHYS_BRAKE_FACTOR*m_BikeC.Drive());
      }
    }
    else {
      /* Throttle? */
      if(m_BikeC.Drive() > 0.0f) {
        if(m_BikeS.Dir == DD_RIGHT) {
          if(fRearWheelAngVel > -PHYS_MAX_ROLL_VELOCITY) {
            m_nStillFrames=0;
            dBodyEnable(m_FrontWheelBodyID);
            dBodyEnable(m_RearWheelBodyID);
            dBodyEnable(m_FrameBodyID);
            
            dBodyAddTorque(m_RearWheelBodyID,0,0,-m_BikeS.Parameters().MaxEngine()*PHYS_ENGINE_DAMP*m_BikeC.Drive());    
            
            //printf("Drive!\n");
          }
        }
        else {
          if(fFrontWheelAngVel < PHYS_MAX_ROLL_VELOCITY) {
            m_nStillFrames=0;
            dBodyEnable(m_FrontWheelBodyID);
            dBodyEnable(m_RearWheelBodyID);
            dBodyEnable(m_FrameBodyID);

            dBodyAddTorque(m_FrontWheelBodyID,0,0,m_BikeS.Parameters().MaxEngine()*PHYS_ENGINE_DAMP*m_BikeC.Drive());    
          }
        }
      }
    }
      
    /* Perform collision detection between the bike and the level blocks */
    m_Collision.clearDynamicTouched();
    
    int nNumContacts;
    dContact Contacts[100];
    
    static int nSlipFrameInterlace =0; /* Okay, lazy approach again. Front wheel can generate particles
                                          even frames, rear wheel odd frames */
    nSlipFrameInterlace++;
        
    nNumContacts = _IntersectWheelLevel( m_BikeS.FrontWheelP,m_BikeS.Parameters().WheelRadius(),Contacts );
    if(nNumContacts>0) {
      if(bFrontWheelTouching == false) {
  bFrontWheelTouching = true;
  scriptCallVoidNumberArg("OnWheel1Touchs", 1);
      }
    } else {
      if(bFrontWheelTouching) {
  bFrontWheelTouching = false;
  scriptCallVoidNumberArg("OnWheel1Touchs", 0);
      }
    }
    if(m_Collision.isDynamicTouched()) {
      if(!dBodyIsEnabled(m_FrontWheelBodyID)) dBodyEnable(m_FrontWheelBodyID);
      if(!dBodyIsEnabled(m_RearWheelBodyID)) dBodyEnable(m_RearWheelBodyID);
      if(!dBodyIsEnabled(m_FrameBodyID)) dBodyEnable(m_FrameBodyID);
    }
    if(dBodyIsEnabled(m_FrontWheelBodyID)) {
      Vector2f WSP;
      for(int i=0;i<nNumContacts;i++) {
        dJointAttach(dJointCreateContact(m_WorldID,m_ContactGroup,&Contacts[i]),m_FrontWheelBodyID,0);                         
          
        WSP.x = Contacts[i].geom.pos[0];
        WSP.y = Contacts[i].geom.pos[1];
      }

      //if(nNumContacts > 0 && nSlipFrameInterlace&1  && sqrt(pfFrame[0]*pfFrame[0] + pfFrame[1]*pfFrame[1])>1.2f) {
      //  Vector2f WSPvel( (-(m_BikeS.FrontWheelP.y - WSP.y)),
      //                   ((m_BikeS.FrontWheelP.x - WSP.x)) );
      //  WSPvel.normalize();
      //  WSPvel *= -m_BikeP.WR * fFrontWheelAngVel;
      //  
      //  Vector2f Vs;
      //  Vs.x = WSPvel.x + pfRear[0];
      //  Vs.y = WSPvel.y + pfRear[1];
      //  
      //  if(Vs.length() > 1.25) {
      //    m_bWheelSpin = true;
      //    m_WheelSpinPoint = WSP;          
      //    m_WheelSpinDir = Vs * 0.1f;

      //    //Vector2f Zz = (m_WheelSpinDir + Vector2f(m_BikeS.FrontWheelP.x - WSP.x,m_BikeS.FrontWheelP.y - WSP.y)) / 2.0f;
      //    //m_WheelSpinDir = Zz;
      //  }        
      //}
      
      if(m_BikeS.Dir == DD_LEFT) {
        if(fabs(fFrontWheelAngVel) > 5 && m_BikeC.Drive()>0.0f && nNumContacts > 0) {
          m_bWheelSpin = true;
          m_WheelSpinPoint = WSP;
          m_WheelSpinDir.x = (((m_BikeS.FrontWheelP.y - WSP.y))*1 + (m_BikeS.FrontWheelP.x - WSP.x)) /2;
          m_WheelSpinDir.y = ((-(m_BikeS.FrontWheelP.x - WSP.x))*1 + (m_BikeS.FrontWheelP.y - WSP.y)) /2;
        }
      }
    }

    nNumContacts = _IntersectWheelLevel( m_BikeS.RearWheelP,m_BikeS.Parameters().WheelRadius(),Contacts );
    if(nNumContacts>0) {
      if(bRearWheelTouching == false) {
  bRearWheelTouching = true;
  scriptCallVoidNumberArg("OnWheel2Touchs", 1);
      }
    } else {
      if(bRearWheelTouching) {
  bRearWheelTouching = false;
  scriptCallVoidNumberArg("OnWheel2Touchs", 0);
      }
    }

    if(m_Collision.isDynamicTouched()) {
      if(!dBodyIsEnabled(m_FrontWheelBodyID)) dBodyEnable(m_FrontWheelBodyID);
      if(!dBodyIsEnabled(m_RearWheelBodyID)) dBodyEnable(m_RearWheelBodyID);
      if(!dBodyIsEnabled(m_FrameBodyID)) dBodyEnable(m_FrameBodyID);
    }
    if(dBodyIsEnabled(m_RearWheelBodyID)) {
      Vector2f WSP;
      for(int i=0;i<nNumContacts;i++) {
        dJointAttach(dJointCreateContact(m_WorldID,m_ContactGroup,&Contacts[i]),m_RearWheelBodyID,0);                         
        WSP.x = Contacts[i].geom.pos[0];
        WSP.y = Contacts[i].geom.pos[1];
      }
            
      /* Calculate wheel linear velocity at slip-point */
      //if(nNumContacts > 0 && !(nSlipFrameInterlace&1) && sqrt(pfFrame[0]*pfFrame[0] + pfFrame[1]*pfFrame[1])>1.2f) {
      //  Vector2f WSPvel( (-(m_BikeS.RearWheelP.y - WSP.y)),
      //                   ((m_BikeS.RearWheelP.x - WSP.x)) );
      //  WSPvel.normalize();
      //  WSPvel *= -m_BikeP.WR * fRearWheelAngVel;
      //  
      //  Vector2f Vs;
      //  Vs.x = WSPvel.x + pfRear[0];
      //  Vs.y = WSPvel.y + pfRear[1];
      //  
      //  if(Vs.length() > 1.25) {
      //    m_bWheelSpin = true;
      //    m_WheelSpinPoint = WSP;          
      //    m_WheelSpinDir = Vs * 0.1f;
      //    
      //    //Vector2f Zz = (m_WheelSpinDir + Vector2f(m_BikeS.RearWheelP.x - WSP.x,m_BikeS.RearWheelP.y - WSP.y)) / 2.0f;
      //    //m_WheelSpinDir = Zz;
      //  }        
      //}

      if(m_BikeS.Dir == DD_RIGHT) {
        if(fabs(fRearWheelAngVel) > 5 && m_BikeC.Drive()>0 && nNumContacts > 0) {
          m_bWheelSpin = true;
          m_WheelSpinPoint = WSP;
          m_WheelSpinDir.x = ((-(m_BikeS.RearWheelP.y - WSP.y))*1 + (m_BikeS.RearWheelP.x - WSP.x)) /2;
          m_WheelSpinDir.y = (((m_BikeS.RearWheelP.x - WSP.x))*1 + (m_BikeS.RearWheelP.y - WSP.y)) /2;
        }
      }
    }        
    
    /* body */
    if(m_bodyDetach) {
      /*
      // m_PlayerTorsoBody
      if(m_BikeS.Dir == DD_RIGHT) {
  nNumContacts = m_Collision.collideLine(m_BikeS.ShoulderP.x, m_BikeS.ShoulderP.y,
                 m_BikeS.LowerBodyP.x, m_BikeS.LowerBodyP.y,
                 Contacts, 100);
      } else {
  nNumContacts = m_Collision.collideLine(m_BikeS.Shoulder2P.x,  m_BikeS.Shoulder2P.y,
                 m_BikeS.LowerBody2P.x, m_BikeS.LowerBody2P.y,
                 Contacts, 100);
      }
      if(nNumContacts > 0) {
  //usleep(100000);
      }
      for(int i=0;i<nNumContacts;i++) {
        dJointAttach(dJointCreateContact(m_WorldID,
                 m_ContactGroup,
           &Contacts[i]),
         m_BikeS.Dir == DD_RIGHT ?
         m_PlayerTorsoBodyID : m_PlayerTorsoBodyID2, 0);
      }
      */
      /*
      // m_PlayerLArmBodyID
      if(m_BikeS.Dir == DD_RIGHT) {
  nNumContacts = m_Collision.collideLine(m_BikeS.ElbowP.x, m_BikeS.ElbowP.y,
                 m_BikeS.HandP.x, m_BikeS.HandP.y,
                 Contacts, 100);
      } else {
  nNumContacts = m_Collision.collideLine(m_BikeS.Elbow2P.x,  m_BikeS.Elbow2P.y,
                 m_BikeS.Hand2P.x, m_BikeS.Hand2P.y,
                 Contacts, 100);
      }
      for(int i=0;i<nNumContacts;i++) {
        dJointAttach(dJointCreateContact(m_WorldID,
                 m_ContactGroup,
           &Contacts[i]),
         m_BikeS.Dir == DD_RIGHT ?
         m_PlayerLArmBodyID : m_PlayerLArmBodyID2, 0);           
      }

      // m_PlayerUArmBodyID
      if(m_BikeS.Dir == DD_RIGHT) {
  nNumContacts = m_Collision.collideLine(m_BikeS.ElbowP.x, m_BikeS.ElbowP.y,
                 m_BikeS.ShoulderP.x, m_BikeS.ShoulderP.y,
                 Contacts, 100);
      } else {
  nNumContacts = m_Collision.collideLine(m_BikeS.Elbow2P.x,  m_BikeS.Elbow2P.y,
                 m_BikeS.Shoulder2P.x, m_BikeS.Shoulder2P.y,
                 Contacts, 100);
      }
      for(int i=0;i<nNumContacts;i++) {
        dJointAttach(dJointCreateContact(m_WorldID,
                 m_ContactGroup,
           &Contacts[i]),
         m_BikeS.Dir == DD_RIGHT ?
         m_PlayerUArmBodyID : m_PlayerUArmBodyID2, 0);           
      }  

      // m_PlayerULegBodyID
      if(m_BikeS.Dir == DD_RIGHT) {
  nNumContacts = m_Collision.collideLine(m_BikeS.LowerBodyP.x, m_BikeS.LowerBodyP.y,
                 m_BikeS.KneeP.x, m_BikeS.KneeP.y,
                 Contacts, 100);
      } else {
  nNumContacts = m_Collision.collideLine(m_BikeS.LowerBody2P.x,  m_BikeS.LowerBody2P.y,
                 m_BikeS.Knee2P.x, m_BikeS.Knee2P.y,
                 Contacts, 100);
      }
      for(int i=0;i<nNumContacts;i++) {
        dJointAttach(dJointCreateContact(m_WorldID,
                 m_ContactGroup,
           &Contacts[i]),
         m_BikeS.Dir == DD_RIGHT ?
         m_PlayerULegBodyID : m_PlayerULegBodyID2, 0);           
      } 

      // m_PlayerLLegBodyID
      if(m_BikeS.Dir == DD_RIGHT) {
  nNumContacts = m_Collision.collideLine(m_BikeS.KneeP.x, m_BikeS.KneeP.y,
                 m_BikeS.FootP.x, m_BikeS.FootP.y,
                 Contacts, 100);
      } else {
  nNumContacts = m_Collision.collideLine(m_BikeS.Knee2P.x,  m_BikeS.Knee2P.y,
                 m_BikeS.Foot2P.x, m_BikeS.Foot2P.y,
                 Contacts, 100);
      }
      for(int i=0;i<nNumContacts;i++) {
        dJointAttach(dJointCreateContact(m_WorldID,
                 m_ContactGroup,
           &Contacts[i]),
         m_BikeS.Dir == DD_RIGHT ?
         m_PlayerLLegBodyID : m_PlayerLLegBodyID2, 0);           
      }
      */

      /*
      // hand
      if(m_BikeS.Dir == DD_RIGHT) {
  nNumContacts = m_Collision.collideCircle(m_BikeS.HandP.x, m_BikeS.HandP.y, 0.1, Contacts, 100);
      } else {
  nNumContacts = m_Collision.collideCircle(m_BikeS.Hand2P.x, m_BikeS.Hand2P.y, 0.1, Contacts, 100);
      }
      for(int i=0;i<nNumContacts;i++) {
        dJointAttach(dJointCreateContact(m_WorldID,
                 m_ContactGroup,
           &Contacts[i]),
         m_BikeS.Dir == DD_RIGHT ?
         m_PlayerHandAnchorBodyID : m_PlayerHandAnchorBodyID2, 0);           
      }

      // foot
      if(m_BikeS.Dir == DD_RIGHT) {
  nNumContacts = m_Collision.collideCircle(m_BikeS.FootP.x, m_BikeS.FootP.y, 0.1, Contacts, 100);
      } else {
  nNumContacts = m_Collision.collideCircle(m_BikeS.Foot2P.x, m_BikeS.Foot2P.y, 0.1, Contacts, 100);
      }
      for(int i=0;i<nNumContacts;i++) {
        dJointAttach(dJointCreateContact(m_WorldID,
                 m_ContactGroup,
                 &Contacts[i]),
         m_BikeS.Dir == DD_RIGHT ?
         m_PlayerFootAnchorBodyID : m_PlayerFootAnchorBodyID2 , 0);           
      }
      */

    }

    /* Player head */
    if(m_BikeS.Dir == DD_RIGHT) {
      if(_IntersectHeadLevel(m_BikeS.HeadP,m_BikeS.Parameters().HeadSize(),m_PrevActiveHead)) {
        createGameEvent(new MGE_PlayerDies(getTime(), false));
      }
      
      m_PrevActiveHead = m_BikeS.HeadP;
    }
    else if(m_BikeS.Dir == DD_LEFT) {
      if(_IntersectHeadLevel(m_BikeS.Head2P,m_BikeS.Parameters().HeadSize(),m_PrevActiveHead)) {
        createGameEvent(new MGE_PlayerDies(getTime(), false));
      }

      m_PrevActiveHead = m_BikeS.Head2P;
    }
        
    //m_PrevFrontWheelP = m_BikeS.FrontWheelP;
    //m_PrevRearWheelP = m_BikeS.RearWheelP;
    m_PrevHeadP = m_BikeS.HeadP;
    m_PrevHead2P = m_BikeS.Head2P;
    
    /* Move rider along bike -- calculate handlebar and footpeg coords */
    Vector2f FootRP,HandRP;
    
    FootRP = m_BikeS.WantedFootP;
    HandRP = m_BikeS.WantedHandP;
        
    Vector2f PFq = FootRP - m_BikeS.FootP;
    Vector2f PFqv = PFq - m_BikeS.PrevPFq;
    Vector2f PFSpring = PFq * PHYS_RIDER_SPRING; 
    Vector2f PFDamp = PFqv * PHYS_RIDER_DAMP;
    Vector2f PFTotal = PFSpring + PFDamp;
    if(m_bodyDetach == false) {
      dBodyAddForce(m_PlayerFootAnchorBodyID,PFTotal.x,PFTotal.y,0);
    }

    m_BikeS.PrevPFq = PFq;    
           
    Vector2f PHq = HandRP - m_BikeS.HandP;
    Vector2f PHqv = PHq - m_BikeS.PrevPHq;
    Vector2f PHSpring = PHq * PHYS_RIDER_SPRING; 
    Vector2f PHDamp = PHqv * PHYS_RIDER_DAMP;
    Vector2f PHTotal = PHSpring + PHDamp;
    if(m_bodyDetach == false) {
      dBodyAddForce(m_PlayerHandAnchorBodyID,PHTotal.x,PHTotal.y,0);
    }
    m_BikeS.PrevPHq = PHq;    

    FootRP = m_BikeS.WantedFoot2P;
    HandRP = m_BikeS.WantedHand2P;
        
    PFq = FootRP - m_BikeS.Foot2P;
    PFqv = PFq - m_BikeS.PrevPFq2;
    PFSpring = PFq * PHYS_RIDER_SPRING; 
    PFDamp = PFqv * PHYS_RIDER_DAMP;
    PFTotal = PFSpring + PFDamp;  
    if(m_bodyDetach == false) {
      dBodyAddForce(m_PlayerFootAnchorBodyID2,PFTotal.x,PFTotal.y,0);
    }
    m_BikeS.PrevPFq2 = PFq;    
           
    PHq = HandRP - m_BikeS.Hand2P;
    PHqv = PHq - m_BikeS.PrevPHq2;
    PHSpring = PHq * PHYS_RIDER_SPRING; 
    PHDamp = PHqv * PHYS_RIDER_DAMP;
    PHTotal = PHSpring + PHDamp;  
    if(m_bodyDetach == false) {
      dBodyAddForce(m_PlayerHandAnchorBodyID2,PHTotal.x,PHTotal.y,0);
    }
    m_BikeS.PrevPHq2 = PHq;    
       
    /* Perform world simulation step */
    dWorldQuickStep(m_WorldID,fTimeStep*PHYS_SPEED);
    //dWorldStep(m_WorldID,fTimeStep*PHYS_SPEED);
    
    /* Empty contact joint group */
    dJointGroupEmpty(m_ContactGroup);
    
    m_bFirstPhysicsUpdate = false;
  }

  /*===========================================================================
    Update game state
    ===========================================================================*/
  void MotoGame::updateStateFromReplay(SerializedBikeState *pReplayState,
          BikeState *pBikeS) 
  {
    bool bUpdateRider=true,bUpdateAltRider=true;
    
    /* Replaying... fetch serialized state */
    pBikeS->RearWheelP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cRearWheelX); 
    pBikeS->RearWheelP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cRearWheelY);
    pBikeS->FrontWheelP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cFrontWheelX); 
    pBikeS->FrontWheelP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cFrontWheelY);
    pBikeS->CenterP.x = pReplayState->fFrameX; 
    pBikeS->CenterP.y = pReplayState->fFrameY;
    
    _16BitsToMatrix(pReplayState->nFrameRot,pBikeS->fFrameRot);
    _16BitsToMatrix(pReplayState->nFrontWheelRot,pBikeS->fFrontWheelRot);
    _16BitsToMatrix(pReplayState->nRearWheelRot,pBikeS->fRearWheelRot);
    
    /* Update engine stuff */
    pBikeS->fBikeEngineRPM = ENGINE_MIN_RPM + (ENGINE_MAX_RPM - ENGINE_MIN_RPM) * ((float)pReplayState->cBikeEngineRPM) / 255.0f;
    
    pBikeS->SwingAnchorP.x = pBikeS->Anchors().AR.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().AR.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->SwingAnchorP.y = pBikeS->Anchors().AR.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().AR.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->FrontAnchorP.x = pBikeS->Anchors().AF.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().AF.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->FrontAnchorP.y = pBikeS->Anchors().AF.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().AF.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    
    pBikeS->SwingAnchor2P.x = pBikeS->Anchors().AR2.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().AR2.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->SwingAnchor2P.y = pBikeS->Anchors().AR2.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().AR2.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->FrontAnchor2P.x = pBikeS->Anchors().AF2.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().AF2.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->FrontAnchor2P.y = pBikeS->Anchors().AF2.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().AF2.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    
    /* Calculate desired hand/foot positions */
    pBikeS->WantedFootP.x = pBikeS->Anchors().PFp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().PFp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->WantedFootP.y = pBikeS->Anchors().PFp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().PFp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->WantedHandP.x = pBikeS->Anchors().PHp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().PHp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->WantedHandP.y = pBikeS->Anchors().PHp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().PHp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;    
    
    pBikeS->WantedFoot2P.x = pBikeS->Anchors().PFp2.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().PFp2.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->WantedFoot2P.y = pBikeS->Anchors().PFp2.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().PFp2.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->WantedHand2P.x = pBikeS->Anchors().PHp2.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().PHp2.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->WantedHand2P.y = pBikeS->Anchors().PHp2.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().PHp2.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;    
    
    /* Get hinges from serialized state */
    if(pReplayState->cFlags & SER_BIKE_STATE_DIR_RIGHT) {
      pBikeS->HandP = pBikeS->WantedHandP;
      pBikeS->ElbowP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cElbowX);         
      pBikeS->ElbowP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cElbowY);
      pBikeS->ShoulderP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cShoulderX); 
      pBikeS->ShoulderP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cShoulderY);
      pBikeS->LowerBodyP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cLowerBodyX); 
      pBikeS->LowerBodyP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cLowerBodyY);
      pBikeS->KneeP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cKneeX); 
      pBikeS->KneeP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cKneeY);
      pBikeS->FootP = pBikeS->WantedFootP;
      
      pBikeS->Dir = DD_RIGHT;
      
      bUpdateAltRider = false;
    }
    else if(pReplayState->cFlags & SER_BIKE_STATE_DIR_LEFT) {
      pBikeS->Hand2P = pBikeS->WantedHand2P;
      pBikeS->Elbow2P.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cElbowX); 
      pBikeS->Elbow2P.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cElbowY);
      pBikeS->Shoulder2P.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cShoulderX); 
      pBikeS->Shoulder2P.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cShoulderY);
      pBikeS->LowerBody2P.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cLowerBodyX); 
      pBikeS->LowerBody2P.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cLowerBodyY);
      pBikeS->Knee2P.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cKneeX); 
      pBikeS->Knee2P.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cKneeY);
      pBikeS->Foot2P = pBikeS->WantedFoot2P;
      
      pBikeS->Dir = DD_LEFT;
      
      bUpdateRider = false;
    }
    
    Vector2f V;      
    
    if(bUpdateRider) {        
      /* Calculate head position */
      V = (pBikeS->ShoulderP - pBikeS->LowerBodyP);
      V.normalize();
      pBikeS->HeadP = pBikeS->ShoulderP + V*pBikeS->Parameters().fNeckLength;
    }
    
    if(bUpdateAltRider) {
      /* Calculate head position (Alt.) */
      V = (pBikeS->Shoulder2P - pBikeS->LowerBody2P);
      V.normalize();
      pBikeS->Head2P = pBikeS->Shoulder2P + V*pBikeS->Parameters().fNeckLength;
    }
    
    /* Internally we'd like to know the abs. relaxed position of the wheels */
    pBikeS->RFrontWheelP.x = pBikeS->Anchors().Fp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().Fp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->RFrontWheelP.y = pBikeS->Anchors().Fp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().Fp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;
    pBikeS->RRearWheelP.x = pBikeS->Anchors().Rp.x*pBikeS->fFrameRot[0] + pBikeS->Anchors().Rp.y*pBikeS->fFrameRot[1] + pBikeS->CenterP.x;
    pBikeS->RRearWheelP.y = pBikeS->Anchors().Rp.x*pBikeS->fFrameRot[2] + pBikeS->Anchors().Rp.y*pBikeS->fFrameRot[3] + pBikeS->CenterP.y;     
  }

  void MotoGame::_UpdateGameState(SerializedBikeState *pReplayState) {
    bool bUpdateRider=true,bUpdateAltRider=true;
    
    /* Replaying? */
    if(pReplayState == NULL) {
      /* Nope... Get current bike state */
      m_BikeS.RearWheelP.x = ((dReal *)dBodyGetPosition( m_RearWheelBodyID ))[0];     /* 4 bytes */
      m_BikeS.RearWheelP.y = ((dReal *)dBodyGetPosition( m_RearWheelBodyID ))[1];     /* 4 bytes */
      m_BikeS.FrontWheelP.x = ((dReal *)dBodyGetPosition( m_FrontWheelBodyID ))[0];   /* 4 bytes */
      m_BikeS.FrontWheelP.y = ((dReal *)dBodyGetPosition( m_FrontWheelBodyID ))[1];   /* 4 bytes */
      m_BikeS.CenterP.x = ((dReal *)dBodyGetPosition( m_FrameBodyID ))[0];            /* 4 bytes */
      m_BikeS.CenterP.y = ((dReal *)dBodyGetPosition( m_FrameBodyID ))[1];            /* 4 bytes */
                                                                                      /* ------- */
                                                                                      /* 24 bytes total */
      
      m_BikeS.fFrameRot[0] = ((dReal *)dBodyGetRotation( m_FrameBodyID ))[0];           
      m_BikeS.fFrameRot[1] = ((dReal *)dBodyGetRotation( m_FrameBodyID ))[1];
      m_BikeS.fFrameRot[2] = ((dReal *)dBodyGetRotation( m_FrameBodyID ))[4];
      m_BikeS.fFrameRot[3] = ((dReal *)dBodyGetRotation( m_FrameBodyID ))[5];           /* 16 bytes */
      
      m_BikeS.fFrontWheelRot[0] = ((dReal *)dBodyGetRotation( m_FrontWheelBodyID ))[0];
      m_BikeS.fFrontWheelRot[1] = ((dReal *)dBodyGetRotation( m_FrontWheelBodyID ))[1];
      m_BikeS.fFrontWheelRot[2] = ((dReal *)dBodyGetRotation( m_FrontWheelBodyID ))[4];
      m_BikeS.fFrontWheelRot[3] = ((dReal *)dBodyGetRotation( m_FrontWheelBodyID ))[5]; /* 16 bytes */
      
      m_BikeS.fRearWheelRot[0] = ((dReal *)dBodyGetRotation( m_RearWheelBodyID ))[0];
      m_BikeS.fRearWheelRot[1] = ((dReal *)dBodyGetRotation( m_RearWheelBodyID ))[1];
      m_BikeS.fRearWheelRot[2] = ((dReal *)dBodyGetRotation( m_RearWheelBodyID ))[4];
      m_BikeS.fRearWheelRot[3] = ((dReal *)dBodyGetRotation( m_RearWheelBodyID ))[5];   /* 16 bytes */
                                                                                        /* -------- */
                                                                                        /* 48 bytes total */
    }
    else {
      /* Replaying... fetch serialized state */
      m_BikeS.RearWheelP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cRearWheelX); 
      m_BikeS.RearWheelP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cRearWheelY);
      m_BikeS.FrontWheelP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cFrontWheelX); 
      m_BikeS.FrontWheelP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cFrontWheelY);
      m_BikeS.CenterP.x = pReplayState->fFrameX; 
      m_BikeS.CenterP.y = pReplayState->fFrameY;
      
      _16BitsToMatrix(pReplayState->nFrameRot,m_BikeS.fFrameRot);
      _16BitsToMatrix(pReplayState->nFrontWheelRot,m_BikeS.fFrontWheelRot);
      _16BitsToMatrix(pReplayState->nRearWheelRot,m_BikeS.fRearWheelRot);
      
      /* Update engine stuff */
      m_BikeS.fBikeEngineRPM = ENGINE_MIN_RPM + (ENGINE_MAX_RPM - ENGINE_MIN_RPM) * ((float)pReplayState->cBikeEngineRPM) / 255.0f;
    }                                                                                        
    
    m_BikeS.SwingAnchorP.x = m_BikeS.Anchors().AR.x*m_BikeS.fFrameRot[0] + m_BikeS.Anchors().AR.y*m_BikeS.fFrameRot[1] + m_BikeS.CenterP.x;
    m_BikeS.SwingAnchorP.y = m_BikeS.Anchors().AR.x*m_BikeS.fFrameRot[2] + m_BikeS.Anchors().AR.y*m_BikeS.fFrameRot[3] + m_BikeS.CenterP.y;

    m_BikeS.SwingAnchor2P.x = m_BikeS.Anchors().AR2.x*m_BikeS.fFrameRot[0] + m_BikeS.Anchors().AR2.y*m_BikeS.fFrameRot[1] + m_BikeS.CenterP.x;
    m_BikeS.SwingAnchor2P.y = m_BikeS.Anchors().AR2.x*m_BikeS.fFrameRot[2] + m_BikeS.Anchors().AR2.y*m_BikeS.fFrameRot[3] + m_BikeS.CenterP.y;

    m_BikeS.FrontAnchorP.x = m_BikeS.Anchors().AF.x*m_BikeS.fFrameRot[0] + m_BikeS.Anchors().AF.y*m_BikeS.fFrameRot[1] + m_BikeS.CenterP.x;
    m_BikeS.FrontAnchor2P.x = m_BikeS.Anchors().AF2.x*m_BikeS.fFrameRot[0] + m_BikeS.Anchors().AF2.y*m_BikeS.fFrameRot[1] + m_BikeS.CenterP.x;

    m_BikeS.FrontAnchorP.y = m_BikeS.Anchors().AF.x*m_BikeS.fFrameRot[2] + m_BikeS.Anchors().AF.y*m_BikeS.fFrameRot[3] + m_BikeS.CenterP.y;
    m_BikeS.FrontAnchor2P.y = m_BikeS.Anchors().AF2.x*m_BikeS.fFrameRot[2] + m_BikeS.Anchors().AF2.y*m_BikeS.fFrameRot[3] + m_BikeS.CenterP.y;

    /* Calculate desired hand/foot positions */
    m_BikeS.WantedFootP.x = m_BikeS.Anchors().PFp.x*m_BikeS.fFrameRot[0] + m_BikeS.Anchors().PFp.y*m_BikeS.fFrameRot[1] + m_BikeS.CenterP.x;
    m_BikeS.WantedFootP.y = m_BikeS.Anchors().PFp.x*m_BikeS.fFrameRot[2] + m_BikeS.Anchors().PFp.y*m_BikeS.fFrameRot[3] + m_BikeS.CenterP.y;
    m_BikeS.WantedHandP.x = m_BikeS.Anchors().PHp.x*m_BikeS.fFrameRot[0] + m_BikeS.Anchors().PHp.y*m_BikeS.fFrameRot[1] + m_BikeS.CenterP.x;
    m_BikeS.WantedHandP.y = m_BikeS.Anchors().PHp.x*m_BikeS.fFrameRot[2] + m_BikeS.Anchors().PHp.y*m_BikeS.fFrameRot[3] + m_BikeS.CenterP.y;    
        
    m_BikeS.WantedFoot2P.x = m_BikeS.Anchors().PFp2.x*m_BikeS.fFrameRot[0] + m_BikeS.Anchors().PFp2.y*m_BikeS.fFrameRot[1] + m_BikeS.CenterP.x;
    m_BikeS.WantedFoot2P.y = m_BikeS.Anchors().PFp2.x*m_BikeS.fFrameRot[2] + m_BikeS.Anchors().PFp2.y*m_BikeS.fFrameRot[3] + m_BikeS.CenterP.y;
    m_BikeS.WantedHand2P.x = m_BikeS.Anchors().PHp2.x*m_BikeS.fFrameRot[0] + m_BikeS.Anchors().PHp2.y*m_BikeS.fFrameRot[1] + m_BikeS.CenterP.x;
    m_BikeS.WantedHand2P.y = m_BikeS.Anchors().PHp2.x*m_BikeS.fFrameRot[2] + m_BikeS.Anchors().PHp2.y*m_BikeS.fFrameRot[3] + m_BikeS.CenterP.y;    

    /* Still a replay question... */
    if(pReplayState == NULL) {
      dVector3 T;
      
      dJointGetHingeAnchor(m_HandHingeID,T);
      m_BikeS.HandP.x = T[0]; m_BikeS.HandP.y = T[1];            /* 8 bytes */

      dJointGetHingeAnchor(m_ElbowHingeID,T);
      m_BikeS.ElbowP.x = T[0]; m_BikeS.ElbowP.y = T[1];          /* 8 bytes */

      dJointGetHingeAnchor(m_ShoulderHingeID,T);
      m_BikeS.ShoulderP.x = T[0]; m_BikeS.ShoulderP.y = T[1];    /* 8 bytes */

      dJointGetHingeAnchor(m_LowerBodyHingeID,T);
      m_BikeS.LowerBodyP.x = T[0]; m_BikeS.LowerBodyP.y = T[1];  /* 8 bytes */

      dJointGetHingeAnchor(m_KneeHingeID,T);
      m_BikeS.KneeP.x = T[0]; m_BikeS.KneeP.y = T[1];            /* 8 bytes */

      dJointGetHingeAnchor(m_FootHingeID,T);
      m_BikeS.FootP.x = T[0]; m_BikeS.FootP.y = T[1];            /* 8 bytes */
                                                                /* ------- */
                                                                /* 48 bytes total */
      
      dJointGetHingeAnchor(m_HandHingeID2,T);
      m_BikeS.Hand2P.x = T[0]; m_BikeS.Hand2P.y = T[1];

      dJointGetHingeAnchor(m_ElbowHingeID2,T);
      m_BikeS.Elbow2P.x = T[0]; m_BikeS.Elbow2P.y = T[1];

      dJointGetHingeAnchor(m_ShoulderHingeID2,T);
      m_BikeS.Shoulder2P.x = T[0]; m_BikeS.Shoulder2P.y = T[1];

      dJointGetHingeAnchor(m_LowerBodyHingeID2,T);
      m_BikeS.LowerBody2P.x = T[0]; m_BikeS.LowerBody2P.y = T[1];

      dJointGetHingeAnchor(m_KneeHingeID2,T);
      m_BikeS.Knee2P.x = T[0]; m_BikeS.Knee2P.y = T[1];

      dJointGetHingeAnchor(m_FootHingeID2,T);
      m_BikeS.Foot2P.x = T[0]; m_BikeS.Foot2P.y = T[1];
    }
    else {
      /* Get hinges from serialized state */
      if(pReplayState->cFlags & SER_BIKE_STATE_DIR_RIGHT) {
        m_BikeS.HandP = m_BikeS.WantedHandP;
        m_BikeS.ElbowP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cElbowX);         
        m_BikeS.ElbowP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cElbowY);
        m_BikeS.ShoulderP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cShoulderX); 
        m_BikeS.ShoulderP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cShoulderY);
        m_BikeS.LowerBodyP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cLowerBodyX); 
        m_BikeS.LowerBodyP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cLowerBodyY);
        m_BikeS.KneeP.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cKneeX); 
        m_BikeS.KneeP.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cKneeY);
        m_BikeS.FootP = m_BikeS.WantedFootP;
        
        m_BikeS.Dir = DD_RIGHT;
        
        bUpdateAltRider = false;
      }
      else if(pReplayState->cFlags & SER_BIKE_STATE_DIR_LEFT) {
        m_BikeS.Hand2P = m_BikeS.WantedHand2P;
        m_BikeS.Elbow2P.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cElbowX); 
        m_BikeS.Elbow2P.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cElbowY);
        m_BikeS.Shoulder2P.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cShoulderX); 
        m_BikeS.Shoulder2P.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cShoulderY);
        m_BikeS.LowerBody2P.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cLowerBodyX); 
        m_BikeS.LowerBody2P.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cLowerBodyY);
        m_BikeS.Knee2P.x = _Map8BitsToCoord(pReplayState->fFrameX,pReplayState->fMaxXDiff,pReplayState->cKneeX); 
        m_BikeS.Knee2P.y = _Map8BitsToCoord(pReplayState->fFrameY,pReplayState->fMaxYDiff,pReplayState->cKneeY);
        m_BikeS.Foot2P = m_BikeS.WantedFoot2P;

        m_BikeS.Dir = DD_LEFT;

        bUpdateRider = false;
      }
    }
          
    Vector2f V;      
    if(bUpdateRider) {        
      /* Calculate head position */
      V = (m_BikeS.ShoulderP - m_BikeS.LowerBodyP);
      V.normalize();
      m_BikeS.HeadP = m_BikeS.ShoulderP + V*m_BikeS.Parameters().fNeckLength;
    }

    if(bUpdateAltRider) {
      /* Calculate head position (Alt.) */
      V = (m_BikeS.Shoulder2P - m_BikeS.LowerBody2P);
      V.normalize();
      m_BikeS.Head2P = m_BikeS.Shoulder2P + V*m_BikeS.Parameters().fNeckLength;
    }

    /* Internally we'd like to know the abs. relaxed position of the wheels */
    m_BikeS.RFrontWheelP.x = m_BikeS.Anchors().Fp.x*m_BikeS.fFrameRot[0] + m_BikeS.Anchors().Fp.y*m_BikeS.fFrameRot[1] + m_BikeS.CenterP.x;
    m_BikeS.RFrontWheelP.y = m_BikeS.Anchors().Fp.x*m_BikeS.fFrameRot[2] + m_BikeS.Anchors().Fp.y*m_BikeS.fFrameRot[3] + m_BikeS.CenterP.y;
    m_BikeS.RRearWheelP.x = m_BikeS.Anchors().Rp.x*m_BikeS.fFrameRot[0] + m_BikeS.Anchors().Rp.y*m_BikeS.fFrameRot[1] + m_BikeS.CenterP.x;
    m_BikeS.RRearWheelP.y = m_BikeS.Anchors().Rp.x*m_BikeS.fFrameRot[2] + m_BikeS.Anchors().Rp.y*m_BikeS.fFrameRot[3] + m_BikeS.CenterP.y;       
  }

  /*===========================================================================
  Prepare rider
  ===========================================================================*/
  void MotoGame::_PrepareRider(Vector2f StartPos) {
    /* Allocate bodies */
    m_PlayerTorsoBodyID = dBodyCreate(m_WorldID);
    m_PlayerLLegBodyID = dBodyCreate(m_WorldID);
    m_PlayerULegBodyID = dBodyCreate(m_WorldID);
    m_PlayerLArmBodyID = dBodyCreate(m_WorldID);
    m_PlayerUArmBodyID = dBodyCreate(m_WorldID);
    m_PlayerFootAnchorBodyID = dBodyCreate(m_WorldID);
    m_PlayerHandAnchorBodyID = dBodyCreate(m_WorldID);    

    m_PlayerTorsoBodyID2 = dBodyCreate(m_WorldID);
    m_PlayerLLegBodyID2 = dBodyCreate(m_WorldID);
    m_PlayerULegBodyID2 = dBodyCreate(m_WorldID);
    m_PlayerLArmBodyID2 = dBodyCreate(m_WorldID);
    m_PlayerUArmBodyID2 = dBodyCreate(m_WorldID);
    m_PlayerFootAnchorBodyID2 = dBodyCreate(m_WorldID);
    m_PlayerHandAnchorBodyID2 = dBodyCreate(m_WorldID);    

    /* Place and define the player bodies */                
    dBodySetPosition(m_PlayerTorsoBodyID,StartPos.x + m_BikeS.Anchors().PTp.x,StartPos.y + m_BikeS.Anchors().PTp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerTorsoMass,m_BikeS.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerTorsoBodyID,&m_PlayerTorsoMass);
    dBodySetFiniteRotationMode(m_PlayerTorsoBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerTorsoBodyID,0,0,1);

    dBodySetPosition(m_PlayerLLegBodyID,StartPos.x + m_BikeS.Anchors().PLLp.x,StartPos.y + m_BikeS.Anchors().PLLp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerLLegMass,m_BikeS.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerLLegBodyID,&m_PlayerLLegMass);
    dBodySetFiniteRotationMode(m_PlayerLLegBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerLLegBodyID,0,0,1);

    dBodySetPosition(m_PlayerULegBodyID,StartPos.x + m_BikeS.Anchors().PULp.x,StartPos.y + m_BikeS.Anchors().PULp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerULegMass,m_BikeS.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerULegBodyID,&m_PlayerULegMass);
    dBodySetFiniteRotationMode(m_PlayerULegBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerULegBodyID,0,0,1);

    dBodySetPosition(m_PlayerLArmBodyID,StartPos.x + m_BikeS.Anchors().PLAp.x,StartPos.y + m_BikeS.Anchors().PLAp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerLArmMass,m_BikeS.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerLArmBodyID,&m_PlayerLArmMass);
    dBodySetFiniteRotationMode(m_PlayerLArmBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerLArmBodyID,0,0,1);

    dBodySetPosition(m_PlayerUArmBodyID,StartPos.x + m_BikeS.Anchors().PUAp.x,StartPos.y + m_BikeS.Anchors().PUAp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerUArmMass,m_BikeS.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerUArmBodyID,&m_PlayerUArmMass);
    dBodySetFiniteRotationMode(m_PlayerUArmBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerUArmBodyID,0,0,1);

    dBodySetPosition(m_PlayerFootAnchorBodyID,StartPos.x + m_BikeS.Anchors().PFp.x,StartPos.y + m_BikeS.Anchors().PFp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerFootAnchorMass,m_BikeS.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerFootAnchorBodyID,&m_PlayerFootAnchorMass);
    dBodySetFiniteRotationMode(m_PlayerFootAnchorBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerFootAnchorBodyID,0,0,1);
    
    dBodySetPosition(m_PlayerHandAnchorBodyID,StartPos.x + m_BikeS.Anchors().PHp.x,StartPos.y + m_BikeS.Anchors().PHp.y,0.0f);
    dMassSetSphereTotal(&m_PlayerHandAnchorMass,m_BikeS.Parameters().BPm,0.4);
    dBodySetMass(m_PlayerHandAnchorBodyID,&m_PlayerHandAnchorMass);
    dBodySetFiniteRotationMode(m_PlayerHandAnchorBodyID,1);
    dBodySetFiniteRotationAxis(m_PlayerHandAnchorBodyID,0,0,1);
    
    /* Place and define the player bodies (Alt.) */                
    dBodySetPosition(m_PlayerTorsoBodyID2,StartPos.x + m_BikeS.Anchors().PTp2.x,StartPos.y + m_BikeS.Anchors().PTp2.y,0.0f);
    dBodySetMass(m_PlayerTorsoBodyID2,&m_PlayerTorsoMass);
    dBodySetFiniteRotationMode(m_PlayerTorsoBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerTorsoBodyID2,0,0,1);

    dBodySetPosition(m_PlayerLLegBodyID2,StartPos.x + m_BikeS.Anchors().PLLp2.x,StartPos.y + m_BikeS.Anchors().PLLp2.y,0.0f);
    dBodySetMass(m_PlayerLLegBodyID2,&m_PlayerLLegMass);
    dBodySetFiniteRotationMode(m_PlayerLLegBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerLLegBodyID2,0,0,1);

    dBodySetPosition(m_PlayerULegBodyID2,StartPos.x + m_BikeS.Anchors().PULp2.x,StartPos.y + m_BikeS.Anchors().PULp2.y,0.0f);
    dBodySetMass(m_PlayerULegBodyID2,&m_PlayerULegMass);
    dBodySetFiniteRotationMode(m_PlayerULegBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerULegBodyID2,0,0,1);

    dBodySetPosition(m_PlayerLArmBodyID2,StartPos.x + m_BikeS.Anchors().PLAp2.x,StartPos.y + m_BikeS.Anchors().PLAp2.y,0.0f);
    dBodySetMass(m_PlayerLArmBodyID2,&m_PlayerLArmMass);
    dBodySetFiniteRotationMode(m_PlayerLArmBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerLArmBodyID2,0,0,1);

    dBodySetPosition(m_PlayerUArmBodyID2,StartPos.x + m_BikeS.Anchors().PUAp2.x,StartPos.y + m_BikeS.Anchors().PUAp2.y,0.0f);
    dBodySetMass(m_PlayerUArmBodyID2,&m_PlayerUArmMass);
    dBodySetFiniteRotationMode(m_PlayerUArmBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerUArmBodyID2,0,0,1);

    dBodySetPosition(m_PlayerFootAnchorBodyID2,StartPos.x + m_BikeS.Anchors().PFp2.x,StartPos.y + m_BikeS.Anchors().PFp2.y,0.0f);
    dBodySetMass(m_PlayerFootAnchorBodyID2,&m_PlayerFootAnchorMass);
    dBodySetFiniteRotationMode(m_PlayerFootAnchorBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerFootAnchorBodyID2,0,0,1);
    
    dBodySetPosition(m_PlayerHandAnchorBodyID2,StartPos.x + m_BikeS.Anchors().PHp2.x,StartPos.y + m_BikeS.Anchors().PHp2.y,0.0f);
    dBodySetMass(m_PlayerHandAnchorBodyID2,&m_PlayerHandAnchorMass);
    dBodySetFiniteRotationMode(m_PlayerHandAnchorBodyID2,1);
    dBodySetFiniteRotationAxis(m_PlayerHandAnchorBodyID2,0,0,1);
    
    float fERP = 0.3; float fCFM = 0.03f;

    /* Connect em */
    m_KneeHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_KneeHingeID,m_PlayerLLegBodyID,m_PlayerULegBodyID);
    dJointSetHingeAnchor(m_KneeHingeID,StartPos.x + m_BikeS.Parameters().PKVx,StartPos.y + m_BikeS.Parameters().PKVy,0.0f);
    dJointSetHingeAxis(m_KneeHingeID,0,0,1);
    dJointSetHingeParam(m_KneeHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_KneeHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_KneeHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_KneeHingeID,dParamStopCFM,fCFM);

    m_LowerBodyHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_LowerBodyHingeID,m_PlayerULegBodyID,m_PlayerTorsoBodyID);
    dJointSetHingeAnchor(m_LowerBodyHingeID,StartPos.x + m_BikeS.Parameters().PLVx,StartPos.y + m_BikeS.Parameters().PLVy,0.0f);
    dJointSetHingeAxis(m_LowerBodyHingeID,0,0,1);       
    dJointSetHingeParam(m_LowerBodyHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_LowerBodyHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_LowerBodyHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_LowerBodyHingeID,dParamStopCFM,fCFM);

    m_ShoulderHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_ShoulderHingeID,m_PlayerTorsoBodyID,m_PlayerUArmBodyID);
    dJointSetHingeAnchor(m_ShoulderHingeID,StartPos.x + m_BikeS.Parameters().PSVx,StartPos.y + m_BikeS.Parameters().PSVy,0.0f);
    dJointSetHingeAxis(m_ShoulderHingeID,0,0,1);       
    dJointSetHingeParam(m_ShoulderHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_ShoulderHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_ShoulderHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_ShoulderHingeID,dParamStopCFM,fCFM);

    m_ElbowHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_ElbowHingeID,m_PlayerUArmBodyID,m_PlayerLArmBodyID);
    dJointSetHingeAnchor(m_ElbowHingeID,StartPos.x + m_BikeS.Parameters().PEVx,StartPos.y + m_BikeS.Parameters().PEVy,0.0f);
    dJointSetHingeAxis(m_ElbowHingeID,0,0,1);       
    dJointSetHingeParam(m_ElbowHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_ElbowHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_ElbowHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_ElbowHingeID,dParamStopCFM,fCFM);
    
    m_FootHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_FootHingeID,m_PlayerFootAnchorBodyID,m_PlayerLLegBodyID);
    dJointSetHingeAnchor(m_FootHingeID,StartPos.x + m_BikeS.Parameters().PFVx,StartPos.y + m_BikeS.Parameters().PFVy,0.0f);
    dJointSetHingeAxis(m_FootHingeID,0,0,1);       
    dJointSetHingeParam(m_FootHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_FootHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_FootHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_FootHingeID,dParamStopCFM,fCFM);
    
    m_HandHingeID = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_HandHingeID,m_PlayerLArmBodyID,m_PlayerHandAnchorBodyID);
    dJointSetHingeAnchor(m_HandHingeID,StartPos.x + m_BikeS.Parameters().PHVx,StartPos.y + m_BikeS.Parameters().PHVy,0.0f);
    dJointSetHingeAxis(m_HandHingeID,0,0,1);       
    dJointSetHingeParam(m_HandHingeID,dParamLoStop,0);
    dJointSetHingeParam(m_HandHingeID,dParamHiStop,0);
    dJointSetHingeParam(m_HandHingeID,dParamStopERP,fERP);
    dJointSetHingeParam(m_HandHingeID,dParamStopCFM,fCFM);                

    /* Connect em (Alt.) */
    m_KneeHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_KneeHingeID2,m_PlayerLLegBodyID2,m_PlayerULegBodyID2);
    dJointSetHingeAnchor(m_KneeHingeID2,StartPos.x - m_BikeS.Parameters().PKVx,StartPos.y + m_BikeS.Parameters().PKVy,0.0f);
    dJointSetHingeAxis(m_KneeHingeID2,0,0,1);       
    dJointSetHingeParam(m_KneeHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_KneeHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_KneeHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_KneeHingeID2,dParamStopCFM,fCFM);

    m_LowerBodyHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_LowerBodyHingeID2,m_PlayerULegBodyID2,m_PlayerTorsoBodyID2);
    dJointSetHingeAnchor(m_LowerBodyHingeID2,StartPos.x - m_BikeS.Parameters().PLVx,StartPos.y + m_BikeS.Parameters().PLVy,0.0f);
    dJointSetHingeAxis(m_LowerBodyHingeID2,0,0,1);       
    dJointSetHingeParam(m_LowerBodyHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_LowerBodyHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_LowerBodyHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_LowerBodyHingeID2,dParamStopCFM,fCFM);

    m_ShoulderHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_ShoulderHingeID2,m_PlayerTorsoBodyID2,m_PlayerUArmBodyID2);
    dJointSetHingeAnchor(m_ShoulderHingeID2,StartPos.x - m_BikeS.Parameters().PSVx,StartPos.y + m_BikeS.Parameters().PSVy,0.0f);
    dJointSetHingeAxis(m_ShoulderHingeID2,0,0,1);       
    dJointSetHingeParam(m_ShoulderHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_ShoulderHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_ShoulderHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_ShoulderHingeID2,dParamStopCFM,fCFM);

    m_ElbowHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_ElbowHingeID2,m_PlayerUArmBodyID2,m_PlayerLArmBodyID2);
    dJointSetHingeAnchor(m_ElbowHingeID2,StartPos.x - m_BikeS.Parameters().PEVx,StartPos.y + m_BikeS.Parameters().PEVy,0.0f);
    dJointSetHingeAxis(m_ElbowHingeID2,0,0,1);       
    dJointSetHingeParam(m_ElbowHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_ElbowHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_ElbowHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_ElbowHingeID2,dParamStopCFM,fCFM);
    
    m_FootHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_FootHingeID2,m_PlayerFootAnchorBodyID2,m_PlayerLLegBodyID2);
    dJointSetHingeAnchor(m_FootHingeID2,StartPos.x - m_BikeS.Parameters().PFVx,StartPos.y + m_BikeS.Parameters().PFVy,0.0f);
    dJointSetHingeAxis(m_FootHingeID2,0,0,1);       
    dJointSetHingeParam(m_FootHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_FootHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_FootHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_FootHingeID2,dParamStopCFM,fCFM);
    
    m_HandHingeID2 = dJointCreateHinge(m_WorldID,0);
    dJointAttach(m_HandHingeID2,m_PlayerLArmBodyID2,m_PlayerHandAnchorBodyID2);
    dJointSetHingeAnchor(m_HandHingeID2,StartPos.x - m_BikeS.Parameters().PHVx,StartPos.y + m_BikeS.Parameters().PHVy,0.0f);
    dJointSetHingeAxis(m_HandHingeID2,0,0,1);       
    dJointSetHingeParam(m_HandHingeID2,dParamLoStop,0);
    dJointSetHingeParam(m_HandHingeID2,dParamHiStop,0);
    dJointSetHingeParam(m_HandHingeID2,dParamStopERP,fERP);
    dJointSetHingeParam(m_HandHingeID2,dParamStopCFM,fCFM);                
  }
  
  /*===========================================================================
  Set up bike physics
  ===========================================================================*/
  void MotoGame::_PrepareBikePhysics(Vector2f StartPos) {  
    /* Create bodies */
    m_FrontWheelBodyID = dBodyCreate(m_WorldID);
    m_RearWheelBodyID = dBodyCreate(m_WorldID);
    m_FrameBodyID = dBodyCreate(m_WorldID);
    
    /* Place and define the rear wheel */
    dBodySetPosition(m_RearWheelBodyID,StartPos.x + m_BikeS.Anchors().Rp.x,StartPos.y + m_BikeS.Anchors().Rp.y,0.0f);
/*    const dReal *pf;
    pf = dBodyGetAngularVel(m_RearWheelBodyID);
    printf("[%f %f %f]\n",pf[0],pf[1],pf[2]);*/
    dMassSetSphereTotal(&m_RearWheelMass,m_BikeS.Parameters().Wm,m_BikeS.Parameters().WheelRadius());
    dBodySetMass(m_RearWheelBodyID,&m_RearWheelMass);
    dBodySetFiniteRotationMode(m_RearWheelBodyID,1);
    dBodySetFiniteRotationAxis(m_RearWheelBodyID,0,0,1);

    /* Place and define the front wheel */
    dBodySetPosition(m_FrontWheelBodyID,StartPos.x + m_BikeS.Anchors().Fp.x,StartPos.y + m_BikeS.Anchors().Fp.y,0.0f);
    dMassSetSphereTotal(&m_FrontWheelMass,m_BikeS.Parameters().Wm,m_BikeS.Parameters().WheelRadius());
    dBodySetMass(m_FrontWheelBodyID,&m_FrontWheelMass);
    dBodySetFiniteRotationMode(m_FrontWheelBodyID,1);
    dBodySetFiniteRotationAxis(m_FrontWheelBodyID,0,0,1);
    
    /* Place and define the frame */
    dBodySetPosition(m_FrameBodyID,StartPos.x,StartPos.y,0.0f);    
    dMassSetBoxTotal(&m_FrameMass,m_BikeS.Parameters().Fm,m_BikeS.Parameters().IL,m_BikeS.Parameters().IH,DEPTH_FACTOR);
    dBodySetMass(m_FrameBodyID,&m_FrameMass);      
    dBodySetFiniteRotationMode(m_FrameBodyID,1);
    dBodySetFiniteRotationAxis(m_FrameBodyID,0,0,1);
    
    /* Prepare rider */
    _PrepareRider(StartPos);
  }
  
  /*===========================================================================
  Collision detection/handling
  ===========================================================================*/  
  bool MotoGame::_IntersectPointLevel(Vector2f Cp) {
    return false; /* TODO */
  }
  
  bool MotoGame::_IntersectHeadLevel(Vector2f Cp,float Cr,const Vector2f &LastCp) {
    if(m_Collision.checkCircle(Cp.x,Cp.y,Cr)) return true;
    
    if(!m_bFirstPhysicsUpdate) {
      dContact c[100];
      int nNumContacts = m_Collision.collideLine(LastCp.x,LastCp.y,Cp.x,Cp.y,c,100);
      if(nNumContacts > 0) {
  return true;
      }
    }
    
    return false;
  }

  int MotoGame::_IntersectWheelLevel(Vector2f Cp,float Cr,dContact *pContacts) {
    int nNumContacts = m_Collision.collideCircle(Cp.x,Cp.y,Cr,pContacts,100);
    if(nNumContacts == 0) {
      /* Nothing... but what if we are moving so fast that the circle has moved
         all the way through some geometry? Check it's path. */
      //nNumContacts = m_Collision.collideLine(
    }
    return nNumContacts;
  }
  
}
