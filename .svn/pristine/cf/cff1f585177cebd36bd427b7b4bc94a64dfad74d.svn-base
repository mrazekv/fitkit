.. article:: apps_vga_graphics_3d
    :author: Josef Strnadel <strnadel AT fit.vutbr.cz>
    :updated: 20090414

    Aplikace demonstrující možnosti tvorby vektorových objektù ve 3D a jejich zobrazení.

========================
3D vektorová grafika
========================

.. contents:: Obsah

Popis aplikace
=================

Aplikace demonstrující možnosti tvorby vektorových objektù ve 3D a vykreslování 3D scény v režimech wireframe (drátìný model) nebo solid (vyplnìné plochy) pomocí rasterizéru trojuhelníkù implementovaného v FPGA. 
K dispozici jsou základní funkce pro transformace (posun, rotace, zmìna mìøítka) vrcholù, hran, povrchù, objektù a celé scény. 

.. figure:: graphic_3d_screen.png

   Ukázka z aplikace
   
Zdrojové kódy aplikace je možné nalézt v `SVN <SVN_APP_DIR>`_.

Knihovna pro manipulaci s 3D objekty
=====================================
Souèástí projektu je knihovna umožòující popis, zobrazení a manipulaci s 3D objekty.

Základní datové struktury
--------------------------

Pøed popisem tvorby vektorových objektù ve 3D se nejprve seznámíme se základními datovými strukturami, které pro tento úèel budeme nezbytnì potøebovat. První dùležitou datovou strukturou je struktura ``Vertex``. Pomocí ní mùžeme vytvoøit bod (jeden z vrcholù budoucího vektorového objektu) ve 3D, a to na souøadnicích [x, y, z]. 3D body mohou být øetìzeny do jednosmìrnì vázaného lineárního seznamu, který je budován dynamicky za bìhu aplikace.

::

  struct Vertex
  {
      double x;
      double y;
      double z;
      struct Vertex *next;
  };

.. figure:: 3d_axis.png

   Ilustrace k 3D bodu a souøadnicovému systému

Ze dvou 3D bodù je možno vytvoøit hranu reprezentovanou strukturou ``Edge``. I hrany jsou øetìzeny do seznamu.

::

  struct Edge
  {
      struct Vertex *v1;
      struct Vertex *v2;
      struct Edge *next;
  };

.. figure:: 3d_edge.png

   Ilustrace k hranì

Máme-li vytvoøeny hrany, mùžeme jimi (formou obecného polygonu) ohranièit plochu (tj. stìnu budoucího objektu). 

.. note:: Struktura SkinAttrib zatím využita není, ale je pøipravena pro pøípadné budoucí pøiøazení vlastností každé ploše. Mezi tyto vlastnosti patøí napø. barva, prùsvitnost, tvrdost atp.

::

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
  };

Plochu mùžeme vytvoøit pomocí následující struktury (``Surface``). Významný je zejména ukazatel \*edgelist ukazující na zaèátek seznamu hran tvoøících plochu.

::

  struct Surface
  {
      struct Edge *edgelist;
      enum SkinType skintype;
      struct SkinAttrib *skinattrib;
      struct Surface *next;
  };

.. figure:: 3d_surface.png

   Ilustrace k povrchu

Máme-li vytvoøenu alespoò jednu plochu, mùžeme vytvoøit objekt a to tak, že ve struktuøe Object uložíme informaci o plochách tvoøících objekt.

::

  struct Object
  {
      struct Surface *surfacelist;
      struct Object *next;
  };

Pomocí vytvoøených objektù mùžeme vytvoøit scénu - objekty tvoøící scénu zøetìzíme do lineárního seznamu a ukazatel na zaèátek tohoto seznamu uložíme do \*objectlist struktury ``Scene``.

::

  struct Scene
  {
      struct Object *objectlist;
  };

Tvorba objektù a scény
--------------------------
Pøed vytvoøením objektù a scény je potøeba deklarovat prostor pro plánované vrcholy, hrany, povrchy, objekty a scénu. Triviální øešení viz níže.

::

  struct Vertex *v1 = NULL;
  
  struct Edge *e1 = NULL;
  
  struct Surface *s1 = NULL;
  
  struct Object *o1 = NULL;
  
  struct Scene *scene = NULL;
  

Funkce pro vykreslení scény
-----------------------------

Je-li scéna vytvoøena, je možno si ji nechat vykreslit. K tomu slouží funkce render. Parametry této funkce jsou ukazatel na strukturu Scene, dále plocha VGA obrazu, která bude využita k vykreslení scény, umístìní kamery na ose Z (v bodì [0, 0, d], kamera je zamìøena do bodu [0, 0, 0]), informace o typu vykreslování ploch (drátìný/plný) a o potøebì vykreslení hranic plohy pro vykreslování.

::
  
  void render(struct Scene *sc, 
              unsigned int xwmin, unsigned int ywmin, unsigned int xwmax, unsigned int ywmax, 
              double d, bool wireframe, bool drawborder);

Funkce pro transformaci objektù a scény
-----------------------------------------

Nad vytvoøenými objekty i scénou jako celkem je možné provádìt všechny typické operace. První skupinou operací jsou operace nad vrcholy. Vrcholy je možno posouvat (translate3Dv), mìnit jejich mìøítko (scale3Dv), rotovat kolem souøadnicových os, popø. rotovat kolem obecné osy (rotate3Dv) zadané dvìma body [x1, y1, y1], [x2, y2, z2] na ní ležícími.

::

  void translate3Dv(struct Vertex *v, double tx, double ty, double tz);
  void scale3Dv(struct Vertex *v, double xf, double yf, double zf, double sx, double sy, double sz);
  void rotate3DXv(struct Vertex *v, double angle);
  void rotate3DYv(struct Vertex *v, double angle);
  void rotate3DZv(struct Vertex *v, double angle);
  void rotate3Dv(struct Vertex *v, double x1, double y1, double z1, double x2, double y2, double z2, double angle);

Obdobné funkce existují pro hrany,

::

  void translate3De(struct Edge *e, double tx, double ty, double tz);
  void scale3De(struct Edge *e, double sx, double sy, double sz);
  void rotate3DXe(struct Edge *e, double angle);
  void rotate3DYe(struct Edge *e, double angle);
  void rotate3DZe(struct Edge *e, double angle);
  void rotate3De(struct Edge *e, double x1, double y1, double z1, double x2, double y2, double z2, double angle);

pro povrchy,

::

  void translate3Dp(struct Surface *s, double tx, double ty, double tz);
  void scale3Dp(struct Surface *s, double sx, double sy, double sz);
  void rotate3DXp(struct Surface *s, double angle);
  void rotate3DYp(struct Surface *s, double angle);
  void rotate3DZp(struct Surface *s, double angle);
  void rotate3Dp(struct Surface *s, double x1, double y1, double z1, double x2, double y2, double z2, double angle);

pro objekty,

::

  void translate3Do(struct Object *o, double tx, double ty, double tz);
  void scale3Do(struct Object *o, double sx, double sy, double sz);
  void rotate3DXo(struct Object *o, double angle);
  void rotate3DYo(struct Object *o, double angle);
  void rotate3DZo(struct Object *o, double angle);
  void rotate3Do(struct Object *o, double x1, double y1, double z1, double x2, double y2, double z2, double angle);

a pro celou scénu, tj. všechny objekty jí náležící.

::

  void translate3Ds(struct Scene *s, double tx, double ty, double tz);
  void scale3Ds(struct Scene *s, double sx, double sy, double sz);
  void rotate3DXs(struct Scene *s, double angle);
  void rotate3DYs(struct Scene *s, double angle);
  void rotate3DZs(struct Scene *s, double angle);
  void rotate3Ds(struct Scene *s, double x1, double y1, double z1, double x2, double y2, double z2, double angle);

Zobrazení krychle krok za krokem
==================================

Demo aplikace demonstruje základní funkènost 3D modelu, a to nejprve pomocí rotující krychle vykreslované v drátìném (wireframe) modelu a poté pomocí krychle vykreslované ve vyplòovaném (solid) modelu. Krychle má 8 vrchlolù s následujícími souøadnicemi:

::

  v5->x = -40;   v5->y = 40;   v5->z = 40;  
  v6->x = 40;  v6->y = 40;  v6->z = 40;  
  v7->x = 40;  v7->y = -40; v7->z = 40;  
  v8->x = -40;  v8->y = -40; v8->z = 40; 
  v9->x = -40;   v9->y = 40;   v9->z = -40;    
  v10->x = 40;  v10->y = 40;  v10->z = -40;  
  v11->x = 40;  v11->y = -40; v11->z = -40;  
  v12->x = -40;  v12->y = -40; v12->z = -40; 

Pomocí vrcholù vytvoøíme hrany, pomocí nichž poté vytvoøíme plochy. Každá plocha (strana) krychle je tvoøena ètvercem a k definici ètverce staèí 4 hrany. Jelikož se jedná o krychli, nemusíme vytváøet všech 6 stran, ale k vykteslení krychle nám postaèí strany 4.

::

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
 
Každou hranu vytvoøíme pomocí dvou bodù.

::

  e1->v1 = v6;   e1->v2 = v10;  //  e1 = (v6, v10)
  e2->v1 = v10;   e2->v2 = v11;  //  e2 = (v10, v11)
  e3->v1 = v11;   e3->v2 = v7;  //  e3 = (v11, v7)
  e4->v1 = v7;   e4->v2 = v6;  //  e4 = (v7, v6)
  
  e5->v1 = v5;   e5->v2 = v6;  //  e5 = (v5, v6)
  e6->v1 = v6;   e6->v2 = v7;  //  e6 = (v6, v7)
  e7->v1 = v7;   e7->v2 = v8;  //  e7 = (v7, v8)
  e8->v1 = v8;   e8->v2 = v5;  //  e8 = (v8, v5)
  
  e9->v1 = v5;   e9->v2 = v9;  //  e9 = (v5, v9)
  e10->v1 = v9;   e10->v2 = v12;  //  e10 = (v9, v12)
  e11->v1 = v12;   e11->v2 = v8;  //  e11 = (v12, v8)
  e12->v1 = v8;   e12->v2 = v5;  //  e12 = (v8, v5)
  
  e13->v1 = v9;   e13->v2 = v10;  //  e13 = (v9, v10)
  e14->v1 = v10;   e14->v2 = v11; //  e14 = (v10, v11)
  e15->v1 = v11;   e15->v2 = v12; //  e15 = (v11, v12)
  e16->v1 = v12;   e16->v2 = v9;  //  e16 = (v12, v9)

Každé stranì krychle pøiøadíme seznam jejích hran, strany krychle (povrchy) provážeme do seznamu.

::

  s1 = (struct Surface *)malloc(sizeof(struct Surface));
  s2 = s1->next = (struct Surface *)malloc(sizeof(struct Surface));
  s3 = s2->next = (struct Surface *)malloc(sizeof(struct Surface));
  s4 = s3->next = (struct Surface *)malloc(sizeof(struct Surface));
  s4->next = NULL;
    
  s1->edgelist = e1;
  s2->edgelist = e5;
  s3->edgelist = e9;
  s4->edgelist = e13;

Objekt (krychli) vytvoøíme pomocí seznamu jeho ploch.

::

  o2 = (struct Object *)malloc(sizeof(struct Object));
  o2->surfacelist = s1;
  o2->next = NULL;

Scénu vytvoøíme pomocí seznamu jejích objektù (zde jen naše krychle).

::

  scene = (struct Scene *)malloc(sizeof(struct Scene));
  scene->objectlist = o2;

Demonstraèní funkce nejprve rotuje drátìnou krychlí kolem os X, Y a poté pokraèuje rotací plnìné krychle kolem týchž os.

::

  void demo()
  {
    unsigned int i, j;
    bool border=0;
    bool wireframe=1;
  
    for(i=0; i<=40; i++)
    {
      rotate3DXo(o2, -3); 
      rotate3DYo(o2, -6); 
      render(scene, xwmin, ywmin, xwmax, ywmax, distance, wireframe, border); DelayMS(1);
    }
  
    wireframe=0;
  
    for(i=0; i<=40; i++)
    {
      rotate3DXo(o2, -3);  
      rotate3DYo(o2, -6); 
      render(scene, xwmin, ywmin, xwmax, ywmax, distance, wireframe, border); DelayMS(1);
    }
  }
  
Zprovoznìní aplikace
========================
 1. pøeložte aplikaci 

 2. aktivujte propojku J6 pro povolení periferií PC
  
 3. naprogramujte MCU a FPGA a spuste terminálový program. Zadáním pøíkazu ``help`` se zobrazí další možnosti.
    
