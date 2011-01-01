/* Copyright (c) 2007 Scott Lembcke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include "stdio.h"
#include "math.h"

#include "chipmunk.h"

cpFloat
cpvlength(const cpVect v)
{
	return sqrtf( cpvdot(v, v) );
}

cpFloat
cpvlengthsq(const cpVect v)
{
	return cpvdot(v, v);
}

cpVect
cpvnormalize(const cpVect v)
{
	return cpvmult( v, 1.0f/cpvlength(v) );
}

cpVect
cpvforangle(const cpFloat a)
{
	return cpv(cos(a), sin(a));
}

cpFloat
cpvtoangle(const cpVect v)
{
	return atan2(v.x, v.y);
}

char*
cpvstr(const cpVect v)
{
	static char str[256];
	sprintf(str, "(% .3f, % .3f)", v.x, v.y);
	return str;
}
