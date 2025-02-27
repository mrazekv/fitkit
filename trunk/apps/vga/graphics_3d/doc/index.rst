.. article:: apps_vga_graphics_3d
    :author: Josef Strnadel <strnadel AT fit.vutbr.cz>
    :updated: 20090414

    Aplikace demonstruj�c� mo�nosti tvorby vektorov�ch objekt� ve 3D a jejich zobrazen�.

========================
3D vektorov� grafika
========================

.. contents:: Obsah

Popis aplikace
=================

Aplikace demonstruj�c� mo�nosti tvorby vektorov�ch objekt� ve 3D a vykreslov�n� 3D sc�ny v re�imech wireframe (dr�t�n� model) nebo solid (vypln�n� plochy) pomoc� rasteriz�ru trojuheln�k� implementovan�ho v FPGA. 
K dispozici jsou z�kladn� funkce pro transformace (posun, rotace, zm�na m���tka) vrchol�, hran, povrch�, objekt� a cel� sc�ny. 

.. figure:: graphic_3d_screen.png

   Uk�zka z aplikace
   
Zdrojov� k�dy aplikace je mo�n� nal�zt v `SVN <SVN_APP_DIR>`_.

Knihovna pro manipulaci s 3D objekty
=====================================
Sou��st� projektu je knihovna umo��uj�c� popis, zobrazen� a manipulaci s 3D objekty.

Z�kladn� datov� struktury
--------------------------

P�ed popisem tvorby vektorov�ch objekt� ve 3D se nejprve sezn�m�me se z�kladn�mi datov�mi strukturami, kter� pro tento ��el budeme nezbytn� pot�ebovat. Prvn� d�le�itou datovou strukturou je struktura ``Vertex``. Pomoc� n� m��eme vytvo�it bod (jeden z vrchol� budouc�ho vektorov�ho objektu) ve 3D, a to na sou�adnic�ch [x, y, z]. 3D body mohou b�t �et�zeny do jednosm�rn� v�zan�ho line�rn�ho seznamu, kter� je budov�n dynamicky za b�hu aplikace.

::

  struct Vertex
  {
      double x;
      double y;
      double z;
      struct Vertex *next;
  };

.. figure:: 3d_axis.png

   Ilustrace k 3D bodu a sou�adnicov�mu syst�mu

Ze dvou 3D bod� je mo�no vytvo�it hranu reprezentovanou strukturou ``Edge``. I hrany jsou �et�zeny do seznamu.

::

  struct Edge
  {
      struct Vertex *v1;
      struct Vertex *v2;
      struct Edge *next;
  };

.. figure:: 3d_edge.png

   Ilustrace k hran�

M�me-li vytvo�eny hrany, m��eme jimi (formou obecn�ho polygonu) ohrani�it plochu (tj. st�nu budouc�ho objektu). 

.. note:: Struktura SkinAttrib zat�m vyu�ita nen�, ale je p�ipravena pro p��padn� budouc� p�i�azen� vlastnost� ka�d� plo�e. Mezi tyto vlastnosti pat�� nap�. barva, pr�svitnost, tvrdost atp.

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

Plochu m��eme vytvo�it pomoc� n�sleduj�c� struktury (``Surface``). V�znamn� je zejm�na ukazatel \*edgelist ukazuj�c� na za��tek seznamu hran tvo��c�ch plochu.

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

M�me-li vytvo�enu alespo� jednu plochu, m��eme vytvo�it objekt a to tak, �e ve struktu�e Object ulo��me informaci o ploch�ch tvo��c�ch objekt.

::

  struct Object
  {
      struct Surface *surfacelist;
      struct Object *next;
  };

Pomoc� vytvo�en�ch objekt� m��eme vytvo�it sc�nu - objekty tvo��c� sc�nu z�et�z�me do line�rn�ho seznamu a ukazatel na za��tek tohoto seznamu ulo��me do \*objectlist struktury ``Scene``.

::

  struct Scene
  {
      struct Object *objectlist;
  };

Tvorba objekt� a sc�ny
--------------------------
P�ed vytvo�en�m objekt� a sc�ny je pot�eba deklarovat prostor pro pl�novan� vrcholy, hrany, povrchy, objekty a sc�nu. Trivi�ln� �e�en� viz n�e.

::

  struct Vertex *v1 = NULL;
  
  struct Edge *e1 = NULL;
  
  struct Surface *s1 = NULL;
  
  struct Object *o1 = NULL;
  
  struct Scene *scene = NULL;
  

Funkce pro vykreslen� sc�ny
-----------------------------

Je-li sc�na vytvo�ena, je mo�no si ji nechat vykreslit. K tomu slou�� funkce render. Parametry t�to funkce jsou ukazatel na strukturu Scene, d�le plocha VGA obrazu, kter� bude vyu�ita k vykreslen� sc�ny, um�st�n� kamery na ose Z (v bod� [0, 0, d], kamera je zam��ena do bodu [0, 0, 0]), informace o typu vykreslov�n� ploch (dr�t�n�/pln�) a o pot�eb� vykreslen� hranic plohy pro vykreslov�n�.

::
  
  void render(struct Scene *sc, 
              unsigned int xwmin, unsigned int ywmin, unsigned int xwmax, unsigned int ywmax, 
              double d, bool wireframe, bool drawborder);

Funkce pro transformaci objekt� a sc�ny
-----------------------------------------

Nad vytvo�en�mi objekty i sc�nou jako celkem je mo�n� prov�d�t v�echny typick� operace. Prvn� skupinou operac� jsou operace nad vrcholy. Vrcholy je mo�no posouvat (translate3Dv), m�nit jejich m���tko (scale3Dv), rotovat kolem sou�adnicov�ch os, pop�. rotovat kolem obecn� osy (rotate3Dv) zadan� dv�ma body [x1, y1, y1], [x2, y2, z2] na n� le��c�mi.

::

  void translate3Dv(struct Vertex *v, double tx, double ty, double tz);
  void scale3Dv(struct Vertex *v, double xf, double yf, double zf, double sx, double sy, double sz);
  void rotate3DXv(struct Vertex *v, double angle);
  void rotate3DYv(struct Vertex *v, double angle);
  void rotate3DZv(struct Vertex *v, double angle);
  void rotate3Dv(struct Vertex *v, double x1, double y1, double z1, double x2, double y2, double z2, double angle);

Obdobn� funkce existuj� pro hrany,

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

a pro celou sc�nu, tj. v�echny objekty j� n�le��c�.

::

  void translate3Ds(struct Scene *s, double tx, double ty, double tz);
  void scale3Ds(struct Scene *s, double sx, double sy, double sz);
  void rotate3DXs(struct Scene *s, double angle);
  void rotate3DYs(struct Scene *s, double angle);
  void rotate3DZs(struct Scene *s, double angle);
  void rotate3Ds(struct Scene *s, double x1, double y1, double z1, double x2, double y2, double z2, double angle);

Zobrazen� krychle krok za krokem
==================================

Demo aplikace demonstruje z�kladn� funk�nost 3D modelu, a to nejprve pomoc� rotuj�c� krychle vykreslovan� v dr�t�n�m (wireframe) modelu a pot� pomoc� krychle vykreslovan� ve vypl�ovan�m (solid) modelu. Krychle m� 8 vrchlol� s n�sleduj�c�mi sou�adnicemi:

::

  v5->x = -40;   v5->y = 40;   v5->z = 40;  
  v6->x = 40;  v6->y = 40;  v6->z = 40;  
  v7->x = 40;  v7->y = -40; v7->z = 40;  
  v8->x = -40;  v8->y = -40; v8->z = 40; 
  v9->x = -40;   v9->y = 40;   v9->z = -40;    
  v10->x = 40;  v10->y = 40;  v10->z = -40;  
  v11->x = 40;  v11->y = -40; v11->z = -40;  
  v12->x = -40;  v12->y = -40; v12->z = -40; 

Pomoc� vrchol� vytvo��me hrany, pomoc� nich� pot� vytvo��me plochy. Ka�d� plocha (strana) krychle je tvo�ena �tvercem a k definici �tverce sta�� 4 hrany. Jeliko� se jedn� o krychli, nemus�me vytv��et v�ech 6 stran, ale k vykteslen� krychle n�m posta�� strany 4.

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
 
Ka�dou hranu vytvo��me pomoc� dvou bod�.

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

Ka�d� stran� krychle p�i�ad�me seznam jej�ch hran, strany krychle (povrchy) prov�eme do seznamu.

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

Objekt (krychli) vytvo��me pomoc� seznamu jeho ploch.

::

  o2 = (struct Object *)malloc(sizeof(struct Object));
  o2->surfacelist = s1;
  o2->next = NULL;

Sc�nu vytvo��me pomoc� seznamu jej�ch objekt� (zde jen na�e krychle).

::

  scene = (struct Scene *)malloc(sizeof(struct Scene));
  scene->objectlist = o2;

Demonstra�n� funkce nejprve rotuje dr�t�nou krychl� kolem os X, Y a pot� pokra�uje rotac� pln�n� krychle kolem t�ch� os.

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
  
Zprovozn�n� aplikace
========================
 1. p�elo�te aplikaci 

 2. aktivujte propojku J6 pro povolen� periferi� PC
  
 3. naprogramujte MCU a FPGA a spus�te termin�lov� program. Zad�n�m p��kazu ``help`` se zobraz� dal�� mo�nosti.
    
