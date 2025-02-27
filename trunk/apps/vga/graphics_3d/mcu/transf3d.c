/*******************************************************************************
   transf3D.c: basic 3D operations for 3D vector gfx demo
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

#include <fitkitlib.h>
#include "struct3d.h"
#include "transf3d.h"

#include <math.h>

//---------------------------- 3D-point operations -----------------------------

// translate point by vector [tx,ty,tz]
void translate3Dv(struct Vertex *v, double tx, double ty, double tz)
{
  v->x += tx;
  v->y += ty;
  v->z += tz;
}

// scale point
// (xf,yf,zf) represents fixed point for scale
void scale3Dv(struct Vertex *v, double xf, double yf, double zf, double sx, double sy, double sz)
{
  translate3Dv(v, -xf, -yf, -zf);
  v->x *= sx;
  v->y *= sy;
  v->z *= sz;
  translate3Dv(v, xf, yf, zf);
}

// rotate point around X-axis
void rotate3DXv(struct Vertex *v, double angle)
{
  double si, co;
  double y, z;

  angle *= M_PI / 180;
  si = sin(angle);
  co = cos(angle);

  y = v->y;
  z = v->z;

  v->y = y * co - z * si;
  v->z = y * si + z * co;
}

// rotate point around Y-axis
void rotate3DYv(struct Vertex *v, double angle)
{
  double si, co;
  double x, z;

  x = v->x;
  z = v->z;

  angle *= M_PI / 180;
  si = sin(angle);
  co = cos(angle);

  v->x = x * co + z * si;
  v->z = -x * si + z * co;
}

// rotate point around Z-axis
void rotate3DZv(struct Vertex *v, double angle)
{
  double si, co;
  double x, y;

  x = v->x;
  y = v->y;

  angle *= M_PI / 180;
  si = sin(angle);
  co = cos(angle);

  v->x = x * co - y * si;
  v->y = x * si + y * co;
}

// rotate point around general 3D-axis
// 3D-axis is defined by [x1,y1,z1] and [x2,y2,z2]
void rotate3Dv(struct Vertex *v, double x1, double y1, double z1, double x2, double y2, double z2, double angle)
{
  double a, b, c, d, l;
  double x, y, z;
  double i, j;
  double dx = x2 - x1,
         dy = y2 - y1,
         dz = z2 - z1;

  l = sqrtf(dx * dx + dy * dy + dz * dz);

  a = dx / l;
  b = dy / l;
  c = dz / l;
  d = sqrtf(b * b + c * c);

  // T
  translate3Dv(v, -x1, -y1, -z1);

  // Rx(alpha)
  y = v->y;   z = v->z;
  i = c / d;  j = b / d;
  v->y = y * i - z * j;
  v->z = y * j + z * i;

  // Ry(beta)
  x = v->x;   z = v->z;
  v->x = x * d - z * a;
  v->z = x * a + z * d;

  // Rz(gama = angle)
  rotate3DZv(v, angle);

  // Ry(beta)^-1
  x = v->x;   z = v->z;
  v->x = x * d + z * a;
  v->z = -x * a + z * d;

  // Rx(alpha)^-1
  y = v->y;   z = v->z;
  i = c / d;  j = b / d;
  v->y = y * i + z * j;
  v->z = -y * j + z * i;

  // T^-1
  translate3Dv(v, x1, y1, z1);
}


// ------------------------------- Edge operations ------------------------------

void translate3De(struct Edge *e, double tx, double ty, double tz)
{
  translate3Dv(e->v1, tx, ty, tz);
  translate3Dv(e->v2, tx, ty, tz);
}

void scale3De(struct Edge *e, double sx, double sy, double sz)
{
  double xf, yf, zf;                  // [xf,yf,zf] is point in the half of the edge e

  xf = (e->v1->x + e->v2->x) / 2;
  yf = (e->v1->y + e->v2->y) / 2;
  zf = (e->v1->z + e->v2->z) / 2;

  scale3Dv(e->v1, xf, yf, zf, sx, sy, sz);
  scale3Dv(e->v2, xf, yf, zf, sx, sy, sz);
}

void rotate3DXe(struct Edge *e, double angle)
{
  rotate3DXv(e->v1, angle);
  rotate3DXv(e->v2, angle);
}

void rotate3DYe(struct Edge *e, double angle)
{
  rotate3DYv(e->v1, angle);
  rotate3DYv(e->v2, angle);
}

void rotate3DZe(struct Edge *e, double angle)
{
  rotate3DZv(e->v1, angle);
  rotate3DZv(e->v2, angle);
}

void rotate3De(struct Edge *e, double x1, double y1, double z1, double x2, double y2, double z2, double angle)
{
  rotate3Dv(e->v1, x1, y1, z1, x2, y2, z2, angle);
  rotate3Dv(e->v2, x1, y1, z1, x2, y2, z2, angle);
}


//------------------------- Polygon-surface operations -------------------------
//
// operace se provadi pouze nad prvnim vrcholem kazde hrany, protoze vsechny prvni
// vrcholy hran jsou zaroven vsechny vrcholy polygon-povrchu a tim je operace
// provedena nad celym povrchem

void translate3Dp(struct Surface *s, double tx, double ty, double tz)
{
  struct Edge *e;

  for (e = s->edgelist; e; e = e->next)
  {
    translate3Dv(e->v1, tx, ty, tz);
  }
}

void scale3Dp(struct Surface *s, double sx, double sy, double sz)
{
  struct Edge *e;

  double xf, yf, zf;                  // [xf,yf,zf] is point in the half of the edge e

  for (e = s->edgelist; e; e = e->next)
  {
    xf = (e->v1->x + e->v2->x) / 2;
    yf = (e->v1->y + e->v2->y) / 2;
    zf = (e->v1->z + e->v2->z) / 2;

    // lock vertex after operation applied on it
    if (e->v1->lock == false)
    {
      scale3Dv(e->v1, xf, yf, zf, sx, sy, sz);
      e->v1->lock = true;
    }

    if(e->v2->lock == false)
    {
      scale3Dv(e->v2, xf, yf, zf, sx, sy, sz);
      e->v2->lock = true;
    }
  }

  // unlock all object vertexs
  for (e = s->edgelist; e; e = e->next)
    e->v1->lock = e->v2->lock = false;
}

void rotate3DXp(struct Surface *s, double angle)
{
  struct Edge *e;

  for (e = s->edgelist; e; e = e->next)
  {
    rotate3DXv(e->v1, angle);
  }
}

void rotate3DYp(struct Surface *s, double angle)
{
  struct Edge *e;

  for (e = s->edgelist; e; e = e->next)
  {
    rotate3DYv(e->v1, angle);
  }
}

void rotate3DZp(struct Surface *s, double angle)
{
  struct Edge *e;

  for (e = s->edgelist; e; e = e->next)
  {
    rotate3DZv(e->v1, angle);
  }
}

void rotate3Dp(struct Surface *s, double x1, double y1, double z1, double x2, double y2, double z2, double angle)
{
  struct Edge *e;

  for (e = s->edgelist; e; e = e->next)
  {
    rotate3Dv(e->v1, x1, y1, z1, x2, y2, z2, angle);
  }
}


//------------------------------ Object operations -----------------------------

void translate3Do(struct Object *o, double tx, double ty, double tz)
{
  struct Surface *s;
  struct Edge *e;

  for (s = o->surfacelist; s; s = s->next)
  {
    for (e = s->edgelist; e; e = e->next)
    {
      // lock vertex after operation applied on it
      if (e->v1->lock == false)
      {
        translate3Dv(e->v1, tx, ty, tz);
        e->v1->lock = true;
      }

      if (e->v2->lock == false)
      {
        translate3Dv(e->v2, tx, ty, tz);
        e->v2->lock = true;
      }
    }
  }

  // unlock all object vertexs
  for (s = o->surfacelist; s; s = s->next)
  {
    for (e = s->edgelist; e; e = e->next)
      e->v1->lock = e->v2->lock = false;
  }
}

void scale3Do(struct Object *o, double sx, double sy, double sz)
{
  struct Surface *s;
  struct Edge *e;

  double xf, yf, zf;                  // [xf,yf,zf] is point in the half of the edge e

  for (s = o->surfacelist; s; s = s->next)
  {
    for (e = s->edgelist; e; e = e->next)
    {
      xf = (e->v1->x + e->v2->x) / 2;
      yf = (e->v1->y + e->v2->y) / 2;
      zf = (e->v1->z + e->v2->z) / 2;

      // lock vertex after operation applied on it
      if (e->v1->lock == false)
      {
        scale3Dv(e->v1, xf, yf, zf, sx, sy, sz);
        e->v1->lock = true;
      }

      if (e->v2->lock == false)
      {
        scale3Dv(e->v2, xf, yf, zf, sx, sy, sz);
        e->v2->lock = true;
      }
    }
  }

      // unlock all object vertexs
  for (s = o->surfacelist; s; s = s->next)
  {
    for (e = s->edgelist; e; e = e->next)
      e->v1->lock = e->v2->lock = false;
  }
}

void rotate3DXo(struct Object *o, double angle)
{
  struct Surface *s;
  struct Edge *e;

  for (s = o->surfacelist; s; s = s->next)
  {
    for (e = s->edgelist; e; e = e->next)
    {
      // lock vertex after operation applied on it
      if(e->v1->lock == false)
      {
        rotate3DXv(e->v1, angle);
        e->v1->lock = true;
      }

      if (e->v2->lock == false)
      {
        rotate3DXv(e->v2, angle);
        e->v2->lock = true;
      }
    }
  }

  // unlock all object vertexs
  for (s = o->surfacelist; s; s = s->next)
  {
    for (e = s->edgelist; e; e = e->next)
      e->v1->lock = e->v2->lock = false;
  }
}

void rotate3DYo(struct Object *o, double angle)
{
  struct Surface *s;
  struct Edge *e;

  for (s = o->surfacelist; s; s = s->next)
  {
    for (e = s->edgelist; e; e = e->next)
    {
      // lock vertex after operation applied on it
      if (e->v1->lock == false)
      {
        rotate3DYv(e->v1, angle);
        e->v1->lock = true;
      }

      if (e->v2->lock == false)
      {
        rotate3DYv(e->v2, angle);
        e->v2->lock = true;
      }
    }
  }

  // unlock all object vertexs
  for (s=o->surfacelist; s; s=s->next)
  {
    for (e=s->edgelist; e; e=e->next)
      e->v1->lock = e->v2->lock = false;
  }
}

void rotate3DZo(struct Object *o, double angle)
{
  struct Surface *s;
  struct Edge *e;

  for (s = o->surfacelist; s; s = s->next)
  {
    for (e = s->edgelist; e; e = e->next)
    {
      // lock vertex after operation applied on it
      if (e->v1->lock == false)
      {
        rotate3DZv(e->v1, angle);
        e->v1->lock = true;
      }

      if (e->v2->lock == false)
      {
        rotate3DZv(e->v2, angle);
        e->v2->lock = true;
      }
    }
  }

  // unlock all object vertexs
  for (s=o->surfacelist; s; s=s->next)
  {
    for (e=s->edgelist; e; e=e->next)
      e->v1->lock = e->v2->lock = false;
  }
}

void rotate3Do(struct Object *o, double x1, double y1, double z1, double x2, double y2, double z2, double angle)
{
  struct Surface *s;
  struct Edge *e;

  for (s = o->surfacelist; s; s = s->next)
  {
    for (e = s->edgelist; e; e = e->next)
    {
      // lock vertex after operation applied on it
      if (e->v1->lock == false)
      {
        rotate3Dv(e->v1, x1, y1, z1, x2, y2, z2, angle);
        e->v1->lock = true;
      }

      if (e->v2->lock == false)
      {
        rotate3Dv(e->v2, x1, y1, z1, x2, y2, z2, angle);
        e->v2->lock = true;
      }
    }
  }

      // unlock all object vertexs
  for (s = o->surfacelist; s; s = s->next)
  {
    for (e = s->edgelist; e; e = e->next)
      e->v1->lock = e->v2->lock = false;
  }
}


//------------------------------- Scene operations -----------------------------

void translate3Ds(struct Scene *s, double tx, double ty, double tz)
{
  struct Object *o;

  for (o = s->objectlist; o; o = o->next)
  {
    translate3Do(o, tx, ty, tz);
  }
}

void scale3Ds(struct Scene *s, double sx, double sy, double sz)
{
  struct Object *o;

  for (o = s->objectlist; o; o = o->next)
  {
    scale3Do(o, sx, sy, sz);
  }
}

void rotate3DXs(struct Scene *s, double angle)
{
  struct Object *o;

  for (o = s->objectlist; o; o = o->next)
  {
    rotate3DXo(o, angle);
  }
}

void rotate3DYs(struct Scene *s, double angle)
{
  struct Object *o;

  for (o = s->objectlist; o; o = o->next)
  {
    rotate3DYo(o, angle);
  }
}

void rotate3DZs(struct Scene *s, double angle)
{
  struct Object *o;

  for (o = s->objectlist; o; o = o->next)
  {
    rotate3DZo(o, angle);
  }
}

void rotate3Ds(struct Scene *s, double x1, double y1, double z1, double x2, double y2, double z2, double angle)
{
  struct Object *o;

  for (o = s->objectlist; o; o = o->next)
  {
    rotate3Do(o, x1, y1, z1, x2, y2, z2, angle);
  }
}
