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
 * This fragment shader is part of the ghost motion blur effect
 */
 
uniform sampler2D tex;

void main() {
	vec4 texel = texture2D(tex,gl_TexCoord[0].st);
	
	float intensity = (texel.r + texel.g + texel.b) / 3.0;
	
	if(intensity < 0.015) /* this fixes a problem where old blur isn't
	                         removed totally */
		discard;
	
	gl_FragColor = 0.8*vec4(intensity,intensity,intensity*1.5,1);
}

