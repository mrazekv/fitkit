/*******************************************************************************
   struct3D.h: data structures for 3D vector gfx demo
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

#ifndef _STRUCT3D_H_
#define _STRUCT3D_H_

#include <stdbool.h>

/*
 *   Surface skin type
*******************************************************************************/
enum SkinType { WIREFRAME, SOLID, SHADED, TEXTURED };

/*   Color representation
 *
*******************************************************************************/
struct Color
{
  long r;
  long g;
  long b;
};

/*
 *   Vertex representation
*******************************************************************************/
struct Vertex
{
  double x;
  double y;
  double z;
  struct Vertex *next;
  bool lock;
};

/*
 *   Edge representation
*******************************************************************************/
struct Edge
{
  struct Vertex *v1;
  struct Vertex *v2;
  struct Edge *next;
};

/*
 *   Surface skin attributes
*******************************************************************************/
struct SkinAttrib
{
  bool phong;
  bool light;
  struct Color *color;
  struct Color *reflect;
  struct Color *filter;
  struct Color *specular;
  long hardness;
  long roughness;
  long shininess;
  long brightness;
  // bitmap *texture;
};

/*
 *   Surface (polygon) is created by edges
*******************************************************************************/
struct Surface
{
  struct Edge *edgelist;
  enum SkinType skintype;
  struct SkinAttrib *skinattrib;
  struct Surface *next;
};

/*
 *   Object is created by surfaces
*******************************************************************************/
struct Object
{
  // char name[256];
  struct Surface *surfacelist;
  struct Object *next;
};

/*
 *   Camera position [x,y,z]
 *   Vector [vx,vy,vz] of camera view
 *   Rotation angle around vector of camera view
 *
*******************************************************************************/
struct Camera
{
  // char name[256];
  double x;
  double y;
  double z;
  double vx;
  double vy;
  double vz;
  double angle;
};


/*
 *   Eye position [x,y,z]
 *   Vector [vx,vy,vz] of eye view
 *   Rotation angle around vector of eye view
 *
*******************************************************************************/
struct Eye
{
  // char name[256];
  double x;
  double y;
  double z;
  double vx;
  double vy;
  double vz;
  double angle;
};

/*
 *   Light source representation
*******************************************************************************/
struct Light
{
  // char name[256];
  double x;
  double y;
  double z;
  struct Color color;
  // #######################
  // ###  dodelat atributy pro ruzne typy svetel (bodove, kuzelove (konicke),
  // ###  valcove,...
  // #######################
};

/*
 *   Scene is created by objects
*******************************************************************************/
struct Scene
{
  // char name[256];
  struct Object *objectlist;
};

#endif /* _STRUCT3D_H_ */
