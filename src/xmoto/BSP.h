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

#ifndef __BSP_H__
#define __BSP_H__

#include "common/VTexture.h"
#include "helpers/VMath.h"

class BSPLine {
public:
  BSPLine(const Vector2f &i_p0, const Vector2f &i_p1);
  BSPLine(const BSPLine &i_line);
  ~BSPLine();

  // public, for avoiding calls (perf)
  Vector2f P0, P1; /* Line */
  Vector2f Normal; /* Linenormal (meaningless, but hey :P)*/

private:
  void computeNormal();
};

class BSPPoly {
public:
  BSPPoly(int size);
  BSPPoly(const BSPPoly &i_poly);
  ~BSPPoly();

  const std::vector<Vector2f> &Vertices() const;
  void addVertice(const Vector2f &i_vertice);
  void addVerticesOf(const BSPPoly *i_poly);

private:
  std::vector<Vector2f> m_vertices;
};

class BSP {
public:
  BSP();
  ~BSP();

  int getNumErrors();
  void addLineDefinition(const Vector2f &i_p0, const Vector2f &i_p1);

  /* build some polygons ; don't delete the result, it's in memory in the class
   * BSP */
  std::vector<BSPPoly *> *compute();

private:
  int m_nNumErrors; /* Number of errors found */
  std::vector<BSPLine *> m_lines; /* Input data set */
  std::vector<BSPPoly *> m_polys; /* Output data set */

  void recurse(BSPPoly *pSubSpace, std::vector<BSPLine *> &Lines);

  BSPLine *findBestSplitter(std::vector<BSPLine *> &i_lines);
  /* if bProbe is true, pnNumFront, pnNumBack and pnNumSplits must not be NULL
   * to be filled AND Front and Back will not be filled */
  void splitLines(std::vector<BSPLine *> &Lines,
                  std::vector<BSPLine *> &Front,
                  std::vector<BSPLine *> &Back,
                  BSPLine *pLine,
                  bool bProbe = false,
                  int *pnNumFront = NULL,
                  int *pnNumBack = NULL,
                  int *pnNumSplits = NULL);
  void splitPoly(BSPPoly *pPoly,
                 BSPPoly *pFront,
                 BSPPoly *pBack,
                 BSPLine *pLine);
};

#endif
