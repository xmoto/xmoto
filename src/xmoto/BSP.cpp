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
 *  Convex polygon generation using BSP trees.
 */
#include "BSP.h"
#include "helpers/Log.h"

#define PLANE_LIMIT_VALUE 0.0001
#define T_LIMIT_VALUE 0.0001

BSPLine::BSPLine(const Vector2f &i_p0, const Vector2f &i_p1) {
  P0 = i_p0;
  P1 = i_p1;

  computeNormal();
}

BSPLine::BSPLine(const BSPLine &i_line) {
  P0 = i_line.P0;
  P1 = i_line.P1;
  Normal = i_line.Normal;
}

BSPLine::~BSPLine() {}

void BSPLine::computeNormal() {
  Vector2f v;

  v = P1 - P0;
  Normal.x = v.y;
  Normal.y = -v.x;
  Normal.normalize();
}

BSPPoly::BSPPoly(int size) {
  m_vertices.reserve(size);
}

BSPPoly::BSPPoly(const BSPPoly &i_poly) {
  m_vertices.reserve(i_poly.Vertices().size());
  addVerticesOf(&i_poly);
}

void BSPPoly::addVerticesOf(const BSPPoly *i_poly) {
  for (unsigned int i = 0; i < i_poly->m_vertices.size(); i++) {
    m_vertices.push_back(i_poly->m_vertices[i]);
  }
}

BSPPoly::~BSPPoly() {}

const std::vector<Vector2f> &BSPPoly::Vertices() const {
  return m_vertices;
}

void BSPPoly::addVertice(const Vector2f &i_vertice) {
  m_vertices.push_back(i_vertice);
}

BSP::BSP() {
  m_nNumErrors = 0;
}

BSP::~BSP() {
  for (unsigned int i = 0; i < m_lines.size(); i++) {
    delete m_lines[i];
  }

  for (unsigned int i = 0; i < m_polys.size(); i++) {
    delete m_polys[i];
  }
}

void BSP::addLineDefinition(const Vector2f &i_p0, const Vector2f &i_p1) {
  try {
    m_lines.push_back(new BSPLine(i_p0, i_p1));
  } catch (Exception &e) {
    /* can't compute normal because points are too near */
    /* just forget this line */
  }
}

int BSP::getNumErrors() {
  return m_nNumErrors;
}

std::vector<BSPPoly *> *BSP::compute() {
  /* Start by creating the root polygon - i.e. the quad that covers
     the entire region enclosed by the input linedefs */
  AABB GlobalBox;

  for (unsigned int i = 0; i < m_lines.size(); i++) {
    GlobalBox.addPointToAABB2f(m_lines[i]->P0);
    GlobalBox.addPointToAABB2f(m_lines[i]->P1);
  }

  BSPPoly RootPoly(m_lines.size());

  // try this root polygon to see if it fixes the graphic problem due to the
  // AABB one
  for (unsigned int i = 0; i < m_lines.size(); i++) {
    RootPoly.addVertice(m_lines[i]->P0);
  }

  /* Start the recursion */
  recurse(&RootPoly, m_lines);

  return &m_polys;
}

/*===========================================================================
  Recursive generation
  ===========================================================================*/
void BSP::recurse(BSPPoly *pSubSpace, std::vector<BSPLine *> &Lines) {
  BSPLine *pBestSplitter = findBestSplitter(Lines);

  if (pBestSplitter == NULL) {
    /* We've found a final convex hull! :D  stop here -- add it to output (after
     * cutting it of course) */
    BSPPoly *pPoly = new BSPPoly(*pSubSpace);

    /* Cut the polygon by each lines */
    for (unsigned int i = 0; i < Lines.size(); i++) {
      /* Define splitting plane */
      BSPLine Splitter = *(Lines[i]);

      /* Cut */
      BSPPoly *pTempPoly = new BSPPoly(pPoly->Vertices().size());

      if (pPoly->Vertices().empty()) {
        LogWarning("recurse(), try to split an empty poly (no BestSplitter)");
        LogWarning("Line causing the trouble is (%.5f %.5f ; %.5f %.5f)",
                   Lines[i]->P0.x,
                   Lines[i]->P0.y,
                   Lines[i]->P1.x,
                   Lines[i]->P1.y);
      } else {
        splitPoly(pPoly, NULL, pTempPoly, &Splitter);
      }
      delete pPoly;
      pPoly = pTempPoly;
    }

    if (pPoly->Vertices().size() > 0) {
      m_polys.push_back(pPoly);
    } else {
      LogWarning("Lines causing the trouble are :");
      for (unsigned int i = 0; i < Lines.size(); i++) {
        LogInfo("%.5f %.5f ; %.5f %.5f",
                Lines[i]->P0.x,
                Lines[i]->P0.y,
                Lines[i]->P1.x,
                Lines[i]->P1.y);
      }

      delete pPoly;
      LogWarning("BSP::recurse() - empty final polygon ignored");

      // m_nNumErrors++; not considered as an error now
    }
  } else {
    /* Split the mess */

    std::vector<BSPLine *> Front, Back;
    Front.reserve(Lines.size());
    Back.reserve(Lines.size());
    splitLines(Lines, Front, Back, pBestSplitter);

    /* Also split the convex subspace */
    BSPPoly FrontSpace(pSubSpace->Vertices().size());
    BSPPoly BackSpace(pSubSpace->Vertices().size());
    if (pSubSpace->Vertices().empty()) {
      LogWarning("recurse(), try to split an empty poly (BestSplitter is set)");
    }
    splitPoly(pSubSpace, &FrontSpace, &BackSpace, pBestSplitter);

    /* Continue recursion */
    if (FrontSpace.Vertices().empty() == false) {
      recurse(&FrontSpace, Front);
    }
    if (BackSpace.Vertices().empty() == false) {
      recurse(&BackSpace, Back);
    }

    /* Clean up */
    for (unsigned int i = 0; i < Front.size(); i++)
      delete Front[i];
    for (unsigned int i = 0; i < Back.size(); i++)
      delete Back[i];
  }
}

/*===========================================================================
  Find best splitter in line soup
  ===========================================================================*/
BSPLine *BSP::findBestSplitter(std::vector<BSPLine *> &i_lines) {
  BSPLine *pBest = NULL;
  std::vector<BSPLine *> Dummy1, Dummy2;
  Dummy1.reserve(i_lines.size());
  Dummy2.reserve(i_lines.size());
  int nBestScore = -1;
  int nNumFront, nNumBack, nNumSplits, nScore;

  /* Try splitting all lines with each of the lines */
  for (unsigned int i = 0; i < i_lines.size(); i++) {
    splitLines(i_lines,
               Dummy1,
               Dummy2,
               i_lines[i],
               true,
               &nNumFront,
               &nNumBack,
               &nNumSplits);

    /* Only qualify if both front and back is larger than 0 */
    if (nNumFront > 0 && nNumBack > 0) {
      /* Compute the score (smaller the better) */
      nScore = abs(nNumBack - nNumFront) + nNumSplits * 2;
      if (nScore < nBestScore || nBestScore == -1) {
        pBest = i_lines[i];
        nBestScore = nScore;
      }
    }
  }

  return pBest;
}

/*===========================================================================
  Split polygon
  ===========================================================================*/
void BSP::splitPoly(BSPPoly *pPoly,
                    BSPPoly *pFront,
                    BSPPoly *pBack,
                    BSPLine *pLine) {
  /* Split the given convex polygon to produce two new convex polygons */
  enum SplitPolyRel { SPR_ON_PLANE, SPR_IN_FRONT, SPR_IN_BACK };

  /* Empty? */
  if (pPoly->Vertices().empty()) {
    LogError("BSP::splitPoly() - empty polygon encountered");
    m_nNumErrors++;
    return;
  }

  /* Look at each corner of the polygon -- how does each of them relate to the
   * plane? */
  std::vector<int> Rels;
  Rels.reserve(pPoly->Vertices().size());
  int nNumInFront = 0, nNumInBack = 0, nNumOnPlane = 0;

  for (unsigned int i = 0; i < pPoly->Vertices().size(); i++) {
    double d = pLine->Normal.dot(pLine->P0 - pPoly->Vertices()[i]);

    if (fabs(d) < PLANE_LIMIT_VALUE) {
      Rels.push_back(SPR_ON_PLANE);
      nNumOnPlane++;
    } else if (d < 0.0f) {
      Rels.push_back(SPR_IN_BACK);
      nNumInBack++;
    } else {
      Rels.push_back(SPR_IN_FRONT);
      nNumInFront++;
    }
  }

  /* Do we need a split, or can we draw a simple conclusion to this madness? */
  if (nNumInBack == 0 && nNumInFront == 0) {
    /* Everything is on the line */
    LogError("BSP::splitPoly() - polygon fully plane aligned");
    m_nNumErrors++;
  } else if (nNumInBack == 0 && nNumInFront > 0) {
    /* Polygon can be regarded as being in front */
    if (pFront != NULL) {
      pFront->addVerticesOf(pPoly);
    }
  } else if (nNumInBack > 0 && nNumInFront == 0) {
    /* Polygon can be regarded as being behind the line */
    if (pBack != NULL) {
      pBack->addVerticesOf(pPoly);
    }
  } else {
    /* We need to divide the polygon */
    for (unsigned int i = 0; i < pPoly->Vertices().size(); i++) {
      bool bSplit = false;

      /* Next vertex? */
      unsigned int j = i + 1;
      if (j == pPoly->Vertices().size())
        j = 0;

      /* Which sides should we add this corner to? */
      if (Rels[i] == SPR_ON_PLANE) {
        /* Both */
        if (pFront) {
          pFront->addVertice(pPoly->Vertices()[i]);
        }
        if (pBack) {
          pBack->addVertice(pPoly->Vertices()[i]);
        }
      } else if (Rels[i] == SPR_IN_FRONT) {
        /* In front */
        if (pFront) {
          pFront->addVertice(pPoly->Vertices()[i]);
        }

        if (Rels[j] == SPR_IN_BACK)
          bSplit = true;
      } else if (Rels[i] == SPR_IN_BACK) {
        /* In back */
        if (pBack) {
          pBack->addVertice(pPoly->Vertices()[i]);
        }

        if (Rels[j] == SPR_IN_FRONT)
          bSplit = true;
      }

      /* Check split? */
      if (bSplit) {
        /* Calculate */
        Vector2f v = pPoly->Vertices()[j] - pPoly->Vertices()[i];
        float den = v.dot(pLine->Normal);
        if (den == 0.0f) {
          /* This should REALLY not be the case... warning! */
          LogError("BSP::splitPoly() - impossible case (1)");
          m_nNumErrors++;

          /* Now it's best simply to ignore this */
          continue;
        }

        float t = -(pLine->Normal.dot(pPoly->Vertices()[i] - pLine->P0)) / den;

        if (t > -(T_LIMIT_VALUE) && t < 1.0 + (T_LIMIT_VALUE)) {
          Vector2f Sect = pPoly->Vertices()[i] + v * t;

          /* We have now calculated the intersection point.
             Add it to both front/back */
          if (pFront) {
            pFront->addVertice(Sect);
          }
          if (pBack) {
            pBack->addVertice(Sect);
          }
        }
      }
    }
  }
}

void BSP::splitLines(std::vector<BSPLine *> &Lines,
                     std::vector<BSPLine *> &Front,
                     std::vector<BSPLine *> &Back,
                     BSPLine *pLine,
                     bool bProbe,
                     int *pnNumFront,
                     int *pnNumBack,
                     int *pnNumSplits) {
  enum SplitLineRel { SLR_ON_PLANE, SLR_IN_FRONT, SLR_IN_BACK };

  if (bProbe) {
    *pnNumFront = *pnNumBack = *pnNumSplits = 0;
  }

  /* Try splitting all the lines -- and collect a bunch of stats about it */
  unsigned int v_size = Lines.size(); // perf cause a high number of calls
  for (unsigned int i = 0; i < v_size; i++) {
    /* Look at this... determined the signed point-plane distance for each ends
     * of the line to split */
    double d0 = pLine->Normal.dot(pLine->P0 - Lines[i]->P0);
    double d1 = pLine->Normal.dot(pLine->P0 - Lines[i]->P1);

    /* Now decide how it relates to the splitter */
    SplitLineRel r0, r1;

    if (fabs(d0) < PLANE_LIMIT_VALUE)
      r0 = SLR_ON_PLANE;
    else if (d0 < 0.0f)
      r0 = SLR_IN_BACK;
    else
      r0 = SLR_IN_FRONT;

    if (fabs(d1) < PLANE_LIMIT_VALUE)
      r1 = SLR_ON_PLANE;
    else if (d1 < 0.0f)
      r1 = SLR_IN_BACK;
    else
      r1 = SLR_IN_FRONT;

    /* If we are lucky we don't need to perform the split */
    if ((r0 == SLR_IN_FRONT && r1 != SLR_IN_BACK) ||
        (r1 == SLR_IN_FRONT && r0 != SLR_IN_BACK)) {
      /* The entire line can be regarded as being IN FRONT */
      if (bProbe) {
        *pnNumFront = *pnNumFront + 1;
      } else {
        Front.push_back(new BSPLine(*(Lines[i])));
      }
    } else if ((r0 == SLR_IN_BACK && r1 != SLR_IN_FRONT) ||
               (r1 == SLR_IN_BACK && r0 != SLR_IN_FRONT)) {
      /* The entire line can be regarded as being IN BACK */
      if (bProbe) {
        *pnNumBack = *pnNumBack + 1;
      } else {
        Back.push_back(new BSPLine(*(Lines[i])));
      }
    } else if (r0 == SLR_ON_PLANE && r1 == SLR_ON_PLANE) {
      /* The line is approximately on the plane... find out which way it faces
       */
      if (Lines[i]->Normal.almostEqual(pLine->Normal)) {
        /* In back then */
        if (bProbe) {
          *pnNumBack = *pnNumBack + 1;
        } else {
          Back.push_back(new BSPLine(*(Lines[i])));
        }
      } else {
        /* In front then */
        if (bProbe) {
          *pnNumFront = *pnNumFront + 1;
        } else {
          Front.push_back(new BSPLine(*(Lines[i])));
        }
      }
    } else {
      /* We need to perform a split :( */
      if (bProbe) {
        *pnNumFront = *pnNumFront + 1;
        *pnNumBack = *pnNumBack + 1;
        *pnNumSplits = *pnNumSplits + 1;
      } else {
        Vector2f v = Lines[i]->P1 - Lines[i]->P0;
        float den = v.dot(pLine->Normal);
        if (den == 0.0f) {
          /* This should REALLY not be the case... warning! */
          LogError("BSP::_SplitLines() - impossible case (1)");
          m_nNumErrors++;

          /* Now it's best simply to ignore this */
          continue;
        }

        double t = -(pLine->Normal.dot(Lines[i]->P0 - pLine->P0)) / den;

        if (t > -(T_LIMIT_VALUE) && t < 1.0 + (T_LIMIT_VALUE)) {
          Vector2f Sect = Lines[i]->P0 + v * t;

          if (r0 == SLR_IN_FRONT) {
            Front.push_back(new BSPLine(Lines[i]->P0, Sect));
          } else {
            Front.push_back(new BSPLine(Sect, Lines[i]->P1));
          }

          if (r0 == SLR_IN_FRONT) {
            Back.push_back(new BSPLine(Sect, Lines[i]->P1));
          } else {
            Back.push_back(new BSPLine(Lines[i]->P0, Sect));
          }
        } else {
          /* Another thing we should just ignore */
          LogError("BSP::_SplitLines() - impossible case (2)");
          m_nNumErrors++;

          continue;
        }
      }
    }
  }
}
