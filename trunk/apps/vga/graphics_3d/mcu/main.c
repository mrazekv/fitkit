/*******************************************************************************
   main.c: main application for 3D vector gfx demo
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
#include "vga.h"
#include "stdbool.h"
#include "struct3d.h"
#include "transf3d.h"
#include "render3d.h"
#include "utils.h"

#include <stdlib.h>

unsigned char Animate = 0;
unsigned char AnimateCnt = 0;
unsigned int  centerX = 640/2;
unsigned int  centerY = 480/2;

unsigned int xwmin=50, ywmin=50, xwmax=300, ywmax=300;
double distance = 100000.0;

struct Vertex *v5 = NULL;
struct Vertex *v6 = NULL;
struct Vertex *v7 = NULL;
struct Vertex *v8 = NULL;
struct Vertex *v9 = NULL;
struct Vertex *v10 = NULL;
struct Vertex *v11 = NULL;
struct Vertex *v12 = NULL;

struct Edge *e1 = NULL;
struct Edge *e2 = NULL;
struct Edge *e3 = NULL;
struct Edge *e4 = NULL;
struct Edge *e5 = NULL;
struct Edge *e6 = NULL;
struct Edge *e7 = NULL;
struct Edge *e8 = NULL;
struct Edge *e9 = NULL;
struct Edge *e10 = NULL;
struct Edge *e11 = NULL;
struct Edge *e12 = NULL;
struct Edge *e13 = NULL;
struct Edge *e14 = NULL;
struct Edge *e15 = NULL;
struct Edge *e16 = NULL;

struct Surface *s1 = NULL;
struct Surface *s2 = NULL;
struct Surface *s3 = NULL;
struct Surface *s4 = NULL;

struct Object *o2 = NULL;

struct Scene *scene = NULL;

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf(" DEMO........spusti demo aplikaci. Po jeho odeslani by mela byt na monitoru viditelna rodujici krychle, a to nejprve v rezimu wireframe a pote v rezimu solid.");
}


unsigned int mrand(unsigned int lim)
{
  unsigned int res = rand();
  return res % lim;
}


void demo()
{
  unsigned int i;
  bool border=0;
  bool wireframe=1;

  for (i = 0; i <= 40; i++)
  {
    WDG_reset();
    rotate3DXo(o2, -3);
    rotate3DYo(o2, -6);
    render(scene, xwmin, ywmin, xwmax, ywmax, distance, wireframe, border);
    delay_ms(1);
  }

  wireframe=0;

  for (i = 0; i <= 40; i++)
  {
    WDG_reset();
    rotate3DXo(o2, -3);
    rotate3DYo(o2, -6);
    render(scene, xwmin, ywmin, xwmax, ywmax, distance, wireframe, border);
    delay_ms(1);
  }
}

/*******************************************************************************
 * Dekodovani uzivatelskych prikazu a jejich vykonavani
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  if (strcmp4(cmd_ucase, "DEMO")) {
     demo();
  }
  else
    return CMD_UNKNOWN;

  return USER_COMMAND;
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized()
{
}

/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/

int main(void)
{
  initialize_hardware();
  srand(100);

  // alokuj pamet pro vrcholy
  v5 = (struct Vertex *)malloc(sizeof(struct Vertex));
  v6 = v5->next = (struct Vertex *)malloc(sizeof(struct Vertex));
  v7 = v6->next = (struct Vertex *)malloc(sizeof(struct Vertex));
  v8 = v7->next = (struct Vertex *)malloc(sizeof(struct Vertex));
  v9 = v8->next = (struct Vertex *)malloc(sizeof(struct Vertex));
  v10 = v9->next = (struct Vertex *)malloc(sizeof(struct Vertex));
  v11 = v10->next = (struct Vertex *)malloc(sizeof(struct Vertex));
  v12 = v11->next = (struct Vertex *)malloc(sizeof(struct Vertex));
  v12->next = NULL;

  // prirad kazdemu vrcholu 3D souradnici [x,y,z]
  v5->x = -40;   v5->y = 40;   v5->z = 40;   v5->lock = false;  
  v6->x = 40;    v6->y = 40;   v6->z = 40;   v6->lock = false; 
  v7->x = 40;    v7->y = -40;  v7->z = 40;   v7->lock = false; 
  v8->x = -40;   v8->y = -40;  v8->z = 40;   v8->lock = false; 
  v9->x = -40;   v9->y = 40;   v9->z = -40;  v9->lock = false;  
  v10->x = 40;   v10->y = 40;  v10->z = -40; v10->lock = false; 
  v11->x = 40;   v11->y = -40; v11->z = -40; v11->lock = false; 
  v12->x = -40;  v12->y = -40; v12->z = -40; v12->lock = false; 

  // alokuj pamet pro hrany
  e1 = (struct Edge *)malloc(sizeof(struct Edge));
  e2 = e1->next = (struct Edge *)malloc(sizeof(struct Edge));
  e3 = e2->next = (struct Edge *)malloc(sizeof(struct Edge));
  e4 = e3->next = (struct Edge *)malloc(sizeof(struct Edge));
  e4->next = NULL;

  e5 = (struct Edge *)malloc(sizeof(struct Edge));
  e6 = e5->next = (struct Edge *)malloc(sizeof(struct Edge));
  e7 = e6->next = (struct Edge *)malloc(sizeof(struct Edge));
  e8 = e7->next = (struct Edge *)malloc(sizeof(struct Edge));
  e8->next = NULL;

  e9 = (struct Edge *)malloc(sizeof(struct Edge));
  e10 = e9->next = (struct Edge *)malloc(sizeof(struct Edge));
  e11 = e10->next = (struct Edge *)malloc(sizeof(struct Edge));
  e12 = e11->next = (struct Edge *)malloc(sizeof(struct Edge));
  e12->next=NULL;

  e13 = (struct Edge *)malloc(sizeof(struct Edge));
  e14 = e13->next = (struct Edge *)malloc(sizeof(struct Edge));
  e15 = e14->next = (struct Edge *)malloc(sizeof(struct Edge));
  e16 = e15->next = (struct Edge *)malloc(sizeof(struct Edge));
  e16->next = NULL;

  // vytvor z vrcholu hrany
  e1->v1 = v6;   e1->v2 = v10;  //  e1 = (v6, v10)
  e2->v1 = v10;  e2->v2 = v11;  //  e2 = (v10, v11)
  e3->v1 = v11;  e3->v2 = v7;   //  e3 = (v11, v7)
  e4->v1 = v7;   e4->v2 = v6;   //  e4 = (v7, v6)

  e5->v1 = v5;   e5->v2 = v6;   //  e5 = (v5, v6)
  e6->v1 = v6;   e6->v2 = v7;   //  e6 = (v6, v7)
  e7->v1 = v7;   e7->v2 = v8;   //  e7 = (v7, v8)
  e8->v1 = v8;   e8->v2 = v5;   //  e8 = (v8, v5)

  e9->v1 = v5;   e9->v2 = v9;   //  e9 = (v5, v9)
  e10->v1 = v9;  e10->v2 = v12; //  e10 = (v9, v12)
  e11->v1 = v12; e11->v2 = v8;  //  e11 = (v12, v8)
  e12->v1 = v8;  e12->v2 = v5;  //  e12 = (v8, v5)

  e13->v1 = v9;  e13->v2 = v10; //  e13 = (v9, v10)
  e14->v1 = v10; e14->v2 = v11; //  e14 = (v10, v11)
  e15->v1 = v11; e15->v2 = v12; //  e15 = (v11, v12)
  e16->v1 = v12; e16->v2 = v9;  //  e16 = (v12, v9)

  // alokuj pamet pro povrchy
  s1 = (struct Surface *)malloc(sizeof(struct Surface));
  s2 = s1->next = (struct Surface *)malloc(sizeof(struct Surface));
  s3 = s2->next = (struct Surface *)malloc(sizeof(struct Surface));
  s4 = s3->next = (struct Surface *)malloc(sizeof(struct Surface));
  s4->next = NULL;
  
  // vytvor povrchy z hran
  s1->edgelist = e1;
  s1->skintype = WIREFRAME;
  s1->skinattrib = NULL;

  s2->edgelist = e5;
  s2->skintype = WIREFRAME;
  s2->skinattrib = NULL;

  s3->edgelist = e9;
  s3->skintype = WIREFRAME;
  s3->skinattrib = NULL;

  s4->edgelist = e13;
  s4->skintype = WIREFRAME;
  s4->skinattrib = NULL;

  // alokuj pamet pro objekt a prirad objektu povrchy
  o2 = (struct Object *)malloc(sizeof(struct Object));
  o2->surfacelist = s1;
  o2->next = NULL;

  // alokuj pamet pro scenu a vloz do sceny objekty
  scene = (struct Scene *)malloc(sizeof(struct Scene));
  scene->objectlist = o2;

  while (1)
  {
    terminal_idle();                       
  }         
}           
