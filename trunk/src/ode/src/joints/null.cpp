/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/


#include "../config.h"
#include "null.h"
#include "joint_internal.h"



//****************************************************************************
// null joint
dxJointNull::dxJointNull( dxWorld *w ) :
        dxJoint( w )
{
}

void 
dxJointNull::getSureMaxInfo( SureMaxInfo* info )
{
    info->max_m = 0;
}


void
dxJointNull::getInfo1( dxJoint::Info1 *info )
{
    info->m = 0;
    info->nub = 0;
}


void
dxJointNull::getInfo2( dxJoint::Info2 *info )
{
    dDebug( 0, "this should never get called" );
}

dJointType
dxJointNull::type() const
{
    return dJointTypeNull;
}

size_t
dxJointNull::size() const
{
    return sizeof( *this );
}


