/*******************************************************************************
   transf3D.h: header file of basic 3D operations for 3D vector gfx demo
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

#ifndef _TRANSF3D_H_
#define _TRANSF3D_H_

/*
 *   Vertex (3D-pixel) operations
*******************************************************************************/
void translate3Dv(struct Vertex *v, double tx, double ty, double tz);
void scale3Dv(struct Vertex *v, double xf, double yf, double zf, double sx, double sy, double sz);
void rotate3DXv(struct Vertex *v, double angle);
void rotate3DYv(struct Vertex *v, double angle);
void rotate3DZv(struct Vertex *v, double angle);
void rotate3Dv(struct Vertex *v, double x1, double y1, double z1, double x2, double y2, double z2, double angle);

/*
 *   Edge operations
*******************************************************************************/
void translate3De(struct Edge *e, double tx, double ty, double tz);
void scale3De(struct Edge *e, double sx, double sy, double sz);
void rotate3DXe(struct Edge *e, double angle);
void rotate3DYe(struct Edge *e, double angle);
void rotate3DZe(struct Edge *e, double angle);
void rotate3De(struct Edge *e, double x1, double y1, double z1, double x2, double y2, double z2, double angle);

/*
 *   Polygon-surface operations
*******************************************************************************/
void translate3Dp(struct Surface *s, double tx, double ty, double tz);
void scale3Dp(struct Surface *s, double sx, double sy, double sz);
void rotate3DXp(struct Surface *s, double angle);
void rotate3DYp(struct Surface *s, double angle);
void rotate3DZp(struct Surface *s, double angle);
void rotate3Dp(struct Surface *s, double x1, double y1, double z1, double x2, double y2, double z2, double angle);

/*
 *   Object operations
*******************************************************************************/
void translate3Do(struct Object *o, double tx, double ty, double tz);
void scale3Do(struct Object *o, double sx, double sy, double sz);
void rotate3DXo(struct Object *o, double angle);
void rotate3DYo(struct Object *o, double angle);
void rotate3DZo(struct Object *o, double angle);
void rotate3Do(struct Object *o, double x1, double y1, double z1, double x2, double y2, double z2, double angle);

/*
 *   Scene operations
*******************************************************************************/
void translate3Ds(struct Scene *s, double tx, double ty, double tz);
void scale3Ds(struct Scene *s, double sx, double sy, double sz);
void rotate3DXs(struct Scene *s, double angle);
void rotate3DYs(struct Scene *s, double angle);
void rotate3DZs(struct Scene *s, double angle);
void rotate3Ds(struct Scene *s, double x1, double y1, double z1, double x2, double y2, double z2, double angle);

#endif /* _TRANSF3D_H_ */
