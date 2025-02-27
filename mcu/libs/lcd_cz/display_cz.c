/*******************************************************************************
   display_cz.c: Knihovna pro obsluhu LCD displeje, v�etn� �esk�ch znak�.
   Copyright (C) 2008 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Pavel Bartos <xbarto41 AT stud.fit.vutbr.cz>

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

*******************************************************************************/

#include <fitkitlib.h>
#include "display_cz.h"

//matice českých znaků
Tcharmatrix _lcd_cz_Sv = {0x0A, 0x04, 0x0F, 0x10, 0x0E, 0x01, 0x1E, 0x00}; //Š
Tcharmatrix _lcd_cz_Tv = {0x0A, 0x04, 0x1F, 0x04, 0x04, 0x04, 0x04, 0x00}; //Ť
Tcharmatrix _lcd_cz_Zv = {0x0A, 0x04, 0x1F, 0x02, 0x04, 0x08, 0x1F, 0x00}; //Ž
Tcharmatrix _lcd_cz_sv = {0x0A, 0x04, 0x0F, 0x10, 0x0E, 0x01, 0x1E, 0x00}; //š
Tcharmatrix _lcd_cz_tv = {0x09, 0x1D, 0x08, 0x08, 0x08, 0x09, 0x06, 0x00}; //ť
Tcharmatrix _lcd_cz_zv = {0x0A, 0x04, 0x1F, 0x02, 0x04, 0x08, 0x1F, 0x00}; //ž
Tcharmatrix _lcd_cz_A_ = {0x02, 0x04, 0x0E, 0x11, 0x1F, 0x11, 0x11, 0x00}; //Á
Tcharmatrix _lcd_cz_Cv = {0x0A, 0x04, 0x0E, 0x10, 0x10, 0x11, 0x0E, 0x00}; //Č
Tcharmatrix _lcd_cz_E_ = {0x02, 0x04, 0x1F, 0x10, 0x1F, 0x10, 0x1F, 0x00}; //É
Tcharmatrix _lcd_cz_Ev = {0x0A, 0x04, 0x1F, 0x10, 0x1F, 0x10, 0x1F, 0x00}; //Ě
Tcharmatrix _lcd_cz_I_ = {0x02, 0x04, 0x0E, 0x04, 0x04, 0x04, 0x0E, 0x00}; //Í
Tcharmatrix _lcd_cz_Dv = {0x0A, 0x04, 0x1E, 0x11, 0x11, 0x11, 0x1E, 0x00}; //Ď
Tcharmatrix _lcd_cz_Nv = {0x0A, 0x04, 0x11, 0x19, 0x15, 0x13, 0x11, 0x00}; //Ň
Tcharmatrix _lcd_cz_O_ = {0x02, 0x04, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00}; //Ó
Tcharmatrix _lcd_cz_Rv = {0x0A, 0x04, 0x1E, 0x11, 0x1E, 0x14, 0x12, 0x00}; //Ř
Tcharmatrix _lcd_cz_Uo = {0x06, 0x06, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00}; //Ů
Tcharmatrix _lcd_cz_U_ = {0x02, 0x04, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00}; //Ú
Tcharmatrix _lcd_cz_Y_ = {0x02, 0x04, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x00}; //Ý
Tcharmatrix _lcd_cz_a_ = {0x02, 0x04, 0x0E, 0x01, 0x0F, 0x11, 0x0F, 0x00}; //á
Tcharmatrix _lcd_cz_cv = {0x0A, 0x04, 0x0E, 0x10, 0x10, 0x11, 0x0E, 0x00}; //č
Tcharmatrix _lcd_cz_e_ = {0x02, 0x04, 0x0E, 0x11, 0x1F, 0x10, 0x0E, 0x00}; //é
Tcharmatrix _lcd_cz_ev = {0x0A, 0x04, 0x0E, 0x11, 0x1F, 0x10, 0x0E, 0x00}; //ě
Tcharmatrix _lcd_cz_i_ = {0x02, 0x04, 0x0C, 0x04, 0x04, 0x04, 0x0E, 0x00}; //í
Tcharmatrix _lcd_cz_dv = {0x03, 0x03, 0x0A, 0x16, 0x12, 0x16, 0x0A, 0x00}; //ď
Tcharmatrix _lcd_cz_nv = {0x0A, 0x04, 0x16, 0x19, 0x11, 0x11, 0x11, 0x00}; //ň
Tcharmatrix _lcd_cz_o_ = {0x02, 0x04, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00}; //ó
Tcharmatrix _lcd_cz_rv = {0x0A, 0x04, 0x16, 0x19, 0x10, 0x10, 0x10, 0x00}; //ř
Tcharmatrix _lcd_cz_uo = {0x06, 0x06, 0x11, 0x11, 0x11, 0x13, 0x0D, 0x00}; //ů
Tcharmatrix _lcd_cz_u_ = {0x02, 0x04, 0x11, 0x11, 0x11, 0x13, 0x0D, 0x00}; //ú
Tcharmatrix _lcd_cz_y_ = {0x02, 0x04, 0x11, 0x11, 0x0F, 0x01, 0x0E, 0x00}; //ý



//tabulka převodu znaku(>127) na ukazatel na pole s maticí znaku, kodovani iso-8859-2
Tcodetable _lcd_cz = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, _lcd_cz_Sv, 0, _lcd_cz_Tv, 0, 0, _lcd_cz_Zv, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, _lcd_cz_sv, 0, _lcd_cz_tv, 0, 0, _lcd_cz_zv, 0,
0, _lcd_cz_A_, 0, 0, 0, 0, 0, 0, _lcd_cz_Cv, _lcd_cz_E_, 0, 0, _lcd_cz_Ev, _lcd_cz_I_, 0, _lcd_cz_Dv,
0, 0, _lcd_cz_Nv, _lcd_cz_O_, 0, 0, 0, 0, _lcd_cz_Rv, _lcd_cz_Uo, _lcd_cz_U_, 0, 0, _lcd_cz_Y_, 0, 0,
0, _lcd_cz_a_, 0, 0, 0, 0, 0, 0, _lcd_cz_cv, _lcd_cz_e_, 0, 0, _lcd_cz_ev, _lcd_cz_i_, 0, _lcd_cz_dv,
0, 0, _lcd_cz_nv, _lcd_cz_o_, 0, 0, 0, 0, _lcd_cz_rv, _lcd_cz_uo, _lcd_cz_u_, 0, 0, _lcd_cz_y_, 0, 0
};




char _lcd_pos;
Tcodetable *_code_table = &_lcd_cz;



//pole s aktu�ln�m obsahem cgram
const char *_cgram[] = {0,0,0,0,0,0,0,0};

/**
 \brief Nahraje �esk�/speci�ln� znak do cgram a vr�t� jeho adresu
 */
unsigned char lcd_cz_char(unsigned char ch)
{
  const char *cz_char_data = (*_code_table)[ch-128]; //zji�t�n� ukazatele na matici znaku
  unsigned char i = 0;
  
  if (cz_char_data == 0)
  { //nen� matice pro znak
    return ch;
  }
  while (_cgram[i] != 0 && i < 8)
  {
    if (_cgram[i] == cz_char_data) 
    {
      return i;  //znak je ji� v cgram nahr�n
    }
    i++;
  }
  if (i >= 8)
  {
    return ' '; //pam�� cgram je pln�, m�sto znaku se vyp��e mezera
  }
  else
  {
    lcd_cz_load_char(i, cz_char_data); //nahraje znak do cgram
    return i;
  }
}

/**
 \brief Zneplatn� cgram 
 */
void lcd_cz_clear_cgram()
{
  unsigned char i;
  for (i=0; i < 8; i++)
    _cgram[i] = 0;
}

/**
 \brief Nahraje znak do cgram
 \param address Adresa v cgram, kam se znak m� nahr�t
 \param data Matice znaku
 */
void lcd_cz_load_char(unsigned char address, const char *data)
{
  unsigned char i;
  unsigned char pos = lcd_get_pos(); //uschov�n� aktu�ln� pozice kurzoru 
  _cgram[address] = data;
  //adres m� tvar 01 "adresa znaku" "adresa ��dku"
  address = address << 3; 
  address |= 0x40;
  address &= 0xF8;
 
  for (i=0; i <8; i++)
  {
    lcd_raw(address|i,0); //set CGRAM address
    lcd_raw(data[i],1);
  }
  lcd_set_pos(pos);
}


/**
 \brief Inicializace LCD
 */
void lcd_init()
{
  delay_ms(10);
  lcd_raw(0x01,0);  // vymaz�n� displeje
  lcd_raw(0x02,0);  // nastaveni kurzoru na za��tek
  lcd_raw(0x38,0);  // nastaven� m�du
  lcd_raw(0x0E,0);  // zobrazeni kurzoru
  lcd_raw(0x06,0);  // nastaven� inkrementu adresy
  _lcd_pos=0;
  lcd_set_pos(0);
  lcd_cz_clear_cgram();
}

/**
 \brief Vymaz�n� dipleje
 */
void lcd_clear()
{
  lcd_raw(0x01,0);  //v�maz dipleje
  lcd_set_pos(0);   //nastaven� pozice na 0
  lcd_cz_clear_cgram(); //vymaz�n� cgram
}

/**
 \brief P��m� z�pis do LCD �adi�e
 \param data Data
 \param RS �rove� RS vodi�e
 */
/**
 \brief Odesl�n� 8-bitov�ho p��kazu na LCD (data + RS bit).
 */

void lcd_raw(unsigned char data, unsigned char rs) {
  unsigned char pkt[6];

  pkt[1] = data; pkt[0] = (rs & 0x1);       //EN=0
  pkt[3] = data; pkt[2] = 0x2 | (rs & 0x1); //EN=1
  pkt[5] = data; pkt[4] = (rs & 0x1);       //EN=0

  FPGA_SPI_RW_A8_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_DISPLAY, &(pkt[0]), 6);
  if (((data == 0x01)||(data == 0x02))&&((rs & 0x01) == 0)) delay_ms(2);
}

/**
 \brief Vr�t� aktu�ln� pozici kurzoru
 */
char lcd_get_pos()
{
  if (_lcd_pos & 0x40)
    return _lcd_pos - 0x40 + 8;
  return _lcd_pos;
}

/**
 \brief Nastav� pozici kurzoru
 */
void lcd_set_pos(char pos)
{
  if (pos >= 8) pos = 0x40 + pos - 8;
  lcd_raw(pos | 0x80, 0);
  _lcd_pos = pos;
}

/**
 \brief Ode�le do LCD displeje �et�zec
 */
void lcd_string(const char *string)
{
  unsigned char i=0;
  while (string[i] != 0)
  {
    lcd_char(string[i]);
    i++;
  }
}

/**
 \brief Ode�le znak na LCD
 */
void lcd_char(unsigned char ch)
{
  if (ch > 127) 
  {
    ch = lcd_cz_char(ch); //nahraje �esk�/speci�ln� znak do cgram a vr�t� jeho adresu
  }
  lcd_raw(ch, 1); //zap��e znak
  _lcd_pos++; //aktualizace pozice
  if(_lcd_pos == 8)
  {
    lcd_raw(0xC0,0);
    _lcd_pos=0x40;
  } 
}

void lcd_set_code_table(Tcodetable *code_table)
{
  _code_table = code_table;
}
