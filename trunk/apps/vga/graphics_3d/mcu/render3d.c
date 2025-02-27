/*******************************************************************************
   render3D.c: scene rendering for 3D vector gfx demo
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Josef Strnadel <strnadel AT fit.vutbr.cz>

   LICENSE TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. All advertising materials mentioning features or use of this software
      or firmware must display the following acknowledgement:

        This product includes software developed by the University of
        Technology, Faculty of Information Technology, Brno and its
        contributors.

   4. Neither the name of the Company nor the names of its contributors
      may be used to endorse or promote products derived from this
      software without specific prior written permission.

   This software or firmware is provided ``as is'', and any express or implied
   warranties, including, but not limited to, the implied warranties of
   merchantability and fitness for a particular purpose are disclaimed.
   In no event shall the company or contributors be liable for any
   direct, indirect, incidental, special, exemplary, or consequential
   damages (including, but not limited to, procurement of substitute
   goods or services; loss of use, data, or profits; or business
   interruption) however caused and on any theory of liability, whether
   in contract, strict liability, or tort (including negligence or
   otherwise) arising in any way out of the use of this software, even
   if advised of the possibility of such damage.

   $Id$


*******************************************************************************/

#ifndef _DRAW3D_H_
#define _DRAW3D_H_

#include <fitkitlib.h>
#include "struct3d.h"
#include "transf3d.h"
#include "vga.h"

#include <stdlib.h>
#include <limits.h>

void line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned char color)
{
  VGA_line(x1, y1, x2, y2, color, 0);
}

void draw_4edge_surface(struct Surface *s, double d, double xbegin, double ybegin, bool wireframe)
{
  double x1, x2, y1, y2, t;
  int i=0;
  struct Edge *e = NULL;
  unsigned int x[4], y[4];

  for (e = s->edgelist; e; e = e->next)
  {
    // transformation 3D engine -> 2D screen
    t = d / (e->v1->z + d);
    x1 = e->v1->x * t;
    y1 = e->v1->y * t;

    t = d / (e->v2->z + d);
    x2 = e->v2->x * t;
    y2 = e->v2->y * t;

    // coord-system-start (0,0,0) into center of the window
    x1 = xbegin + x1;
    y1 = ybegin - y1;
    x2 = xbegin + x2;
    y2 = ybegin - y2;

    x[i] = x1;
    y[i] = y1;

    i++;
  }

  if (!wireframe)
  {
    SetTriangle(x[0], y[0], x[1], y[1], x[2], y[2], 100, 0);
    SetTriangle(x[2], y[2], x[3], y[3], x[0], y[0], 100, 0);
  }
}

void render(struct Scene *sc, unsigned int xwmin, unsigned int ywmin, unsigned int xwmax, unsigned int ywmax, double d, bool wireframe, bool drawborder)
{
      // d - distance from the begining of 3D coord system
  struct Object *ob;
  struct Surface *su;
  struct Edge *e;

  double x1, y1, x2, y2, t;
  double xbegin = (double)(xwmax-xwmin)/2,
         ybegin = (double)(ywmax-ywmin)/2;

  unsigned char border_color = 100;

  double k, q;
  int i;

  VGA_clear(0);  // nejprve smaz obrazovku

  // ### chybi test, jestli xwmin, ywmin, xwmax, ywmax padne do okna ###
  // virtual window border
  if (drawborder)
  {
    line(xwmin, ywmin, xwmax, ywmin, border_color);
    line(xwmin, ywmin, xwmin, ywmax, border_color);
    line(xwmax, ywmin, xwmax, ywmax, border_color);
    line(xwmin, ywmax, xwmax, ywmax, border_color);
  }

  if (sc)
  {
    for (ob = sc->objectlist; ob; ob = ob->next)
    {
      for (su = ob->surfacelist; su; su = su->next)
      {
        draw_4edge_surface(su, d, xbegin, ybegin, wireframe);
        for (e = su->edgelist; e; e = e->next)
        {
          switch (su->skintype)
          {
            case WIREFRAME:
            {
              // transformation 3D engine -> 2D screen
              t = d / (e->v1->z + d);
              x1 = e->v1->x * t;
              y1 = e->v1->y * t;

              t = d / (e->v2->z + d);
              x2 = e->v2->x * t;
              y2 = e->v2->y * t;

              // coord-system-start (0,0,0) into center of the window
              x1 = xbegin + x1;
              y1 = ybegin - y1;
              x2 = xbegin + x2;
              y2 = ybegin - y2;

              i = 0;
              // Conen & Sutherland line clipping
              if (x1 < xwmin) i += 1;
              if (x1 > xwmax) i += 2;
              if (y1 < ywmin) i += 4;
              if (y1 > ywmax) i += 8;

              if (x2 < xwmin) i += 16;
              if (x2 > xwmax) i += 32;
              if (y2 < ywmin) i += 64;
              if (y2 > ywmax) i += 128;

              if (i != 0)  // nektery z bodu je mimo okno
              {
                // rovnice primky y = k*x + q
                k = (y2 - y1) / (x2 - x1);
                q = y1 - k * x1;
              }

              // usecka je mimo okno -> nevykresli se
              if ((i & 1) && (i & 16)) ;
              else if ((i & 2) && (i & 32)) ;
              else if ((i & 4) && (i & 64)) ;
              else if ((i & 8) && (i & 128)) ;

              // usecka protina okno -> urceni pruseciku s oknem
              else
              {
                // x je mensi nez xwmin
                if (i & 1)
                {
                  x1 = xwmin;
                  y1 = k * x1 + q;
                  if (y1 > ywmax) y1 = ywmax;
                  else if (y1 < ywmin) y1 = ywmin;
                }

                if (i & 16)
                {
                  x2 = xwmin;
                  y2 = k * x2 + q;
                  if (y2 > ywmax) y2 = ywmax;
                  else if (y2 < ywmin) y2 = ywmin;
                }

                // x je vetsi nez xwmax
                if (i & 2)
                {
                  x1 = xwmax;
                  y1 = k * x1 + q;
                  if (y1 > ywmax) y1 = ywmax;
                  else if (y1 < ywmin) y1 = ywmin;
                }

                if (i & 32)
                {
                  x2 = xwmax;
                  y2 = k * x2 + q;
                  if (y2 > ywmax) y2 = ywmax;
                  else if (y2 < ywmin) y2 = ywmin;
                }

                // y je mensi nez ywmin
                if (i & 4)
                {
                  y1 = ywmin;
                  x1 = (y1 - q) / k;
                  if(x1 > xwmax) x1 = xwmax;
                  else if(x1 < xwmin) x1 = xwmin;
                }

                if (i & 64)
                {
                  y2 = ywmin;
                  x2 = (y2 - q) / k;
                  if (x2 > xwmax) x2 = xwmax;
                  else if (x2 < xwmin) x2 = xwmin;
                }

                // y je vetsi nez ywmax
                if (i & 8)
                {
                  y1 = ywmax;
                  x1 = (y1 - q) / k;
                  if (x1 > xwmax) x1 = xwmax;
                  else if (x1 < xwmin) x1 = xwmin;
                }

                if (i & 128)
                {
                  y2 = ywmax;
                  x2 = (y2-q)/k;
                  if (x2 > xwmax) x2 = xwmax;
                  else if (x2 < xwmin) x2 = xwmin;
                }

                line((int)(x1), (int)(y1), (int)(x2), (int)(y2), 255);
              } 
            }
            break;
          }      
        }
      }
    }
  }
  else
  {
    // error("No scene to render\n");
  }
}

#endif
