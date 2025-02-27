/*******************************************************************************
   enc28j60_string.c: Knihovna funkcí pro práci s øetìzci a pamìtí.
   Copyright (C) 2010 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Martin Musil <xmusil34 AT fit.vutbr.cz>

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
#include "enc28j60_string.h"

/**
 \brief vlastní implementace fce MEMCPY
 **/
inline void* enc_memcpy(void* dst, void* src, unsigned int count){
	char* dst_tmp = (char*) dst;
	char* src_tmp = (char*) src;
	
	while(count--)
		*dst_tmp++ = *src_tmp++;
	
return dst;
}

/**
 \brief vlastní implementace fce MEMCMP
 **/
inline unsigned int enc_memcmp(void* str1, void* str2,unsigned int count){
	char* str1_tmp = (char*) str1;
	char* str2_tmp = (char*) str2;

	while(count--){
		if(*str1_tmp++ != *str2_tmp++)
			return 1;
	}
	
return 0;
}

/**
 \brief vlastní implementace fce MEMSET
 **/
inline void* enc_memset(void* ptr, char value, unsigned int count){
	char* ptr_tmp = (char*) ptr;
	
	while(count--)
		*ptr_tmp++ = value;
	
return ptr;
}

/**
 \brief vlastní implementace fce STRLEN
 **/
inline unsigned int enc_strlen(void* ptr){
	unsigned int len = 0;
	char* ptr_tmp = (char*) ptr;
	
	while((*ptr_tmp) != 0){
		len++;
		ptr_tmp++;
	}
return len;
}

/**
 \brief Funkce pøevede øetìzec na èísla reprezentující IP adresu a port.
 		\n Øetìzec musí být ve formátu "IP_adresa:port" nebo "IP_adresa". Ve druhém pøípadì bude navrácené èíslo portu 0. 
 **/
unsigned char str_to_ip_port(char* ptr, char max_len, unsigned long* ip, unsigned int* port){
	
	
	while(( (! is_digit(*ptr))) & (max_len > 0)){
		max_len--;
		ptr++;
		
	}
	
	char part = 1;
	unsigned int temp = 0;
	unsigned long temp_port = 0;
	
	while(max_len > 0){
	
		switch(part){
			case 1:
			case 2:
			case 3:
				if( is_digit(*ptr) & (temp <= 255) ){		//další èíslice - pøiètu do temp
					temp *= 10;
					temp += (*ptr) - 48;
					
				}
				else{
					if( ((*ptr) == '.') & ( temp <= 255 )){	// teèka - zápis èísla temp do parametru ip a pøechod na 
						((char*)ip)[4-part] = (char)temp;	// další èást IP adresy
						part++;
						temp = 0;
					}
					else{
						return 0;	
					}	
				}
				break;
				
			case 4:
				if( is_digit(*ptr) & (temp <= 255) ){		//další èíslice - pøiètu do temp
					temp *= 10;
					temp += (*ptr) - 48;
				}
				else{
					if( ((*ptr) == ':') & ( temp <= 255 ) ){	// dvojteèka - zápis èísla temp do parametru ip a pøechod na 
						((char*)ip)[4-part] = (char)temp;		// ètení portu
						part++;
						temp = 0;
					}
					else{
						if( temp <= 255){						// jiný znak než dvojteèka - IP pøeèteno, vracím úspìch a port = 0
							*port = 0;
							((char*)ip)[4-part] = (char)temp;
							return 1;
						}
						else	
							return 0;
					}	
				}
				break;
			//port
			case 5:
				if( is_digit(*ptr) & (temp <= 255) ){		//další èíslice - pøiètu do temp_port
					temp_port *= 10;
					temp_port += (*ptr) - 48;
				}
				else{
					if( temp_port <= 65535){				//jiný znak - konec portu, vracím úspìch,IP a port 
						*port = temp_port;
						return 1;
					}
					else{
						return 0;
					}
				}
				
		}

		//!!!!
		ptr++;
		max_len--;	

	}
	
	//dosazen konec retezce
	switch(part){
		case 1:
		case 2:
		case 3:
			return 0;									// nejsou pøeèteny všechny èásti IP - neúspìch
		case 4:
			if(temp <= 255){
				((char*)ip)[4-part] = (char)temp;		//pøeètena 4. èást IP - vracím úspìch a pouze IP adresu 
				*port = 0;
				return 1;
			}
			else
				return 0;
			
		case 5:
			if( temp_port <= 65535){				//pøeèten i port, vracím úspìch, IP a port
				*port = temp_port;
				return 1;
			}	
			else{
				*port = 0;
				return 1;
			}
	}
	
return 0;
}

/**
 \brief Funkce pøevede èíslo reprezentující IP adresu na øetìzec zakonèený znakem null.
 **/
unsigned char ip_to_str(char * ptr, char max_len, unsigned long ip){
	
	unsigned char tmp[3];
	char counter = max_len;
	unsigned char segment;	
	
	int i = 3;
	while( i >= 0){
	
		segment = ((char*)&ip)[i];	

		tmp[0] = (segment / 100) + 48;
		segment %= 100;
		tmp[1] = (segment / 10) + 48;
		segment %= 10;
		tmp[2] = segment + 48;

		int k = 0;
		char print = 0;
		while(k < 3){

			if(tmp[k] > '0'){			//pocatecni nuly nezapisujem
				print = 1;
				if(counter > 0){
					counter--;
					*(ptr) = tmp[k];
					ptr++;
				}
			}
			else if(print == 1){		//nuly uprostred cisla samozrejme zapisujem
				if(counter > 0){
					counter--;
					*(ptr) = tmp[k];
					ptr++;
				}	
			}
			
			k++;	
		}
		
		if(print == 0){				//byly same nuly, musime zapsat alespon jednu
			if(counter > 0){
				counter--;
				*(ptr) = '0';
				ptr++;
			}
		}
		
		if((i != 0) & ( counter > 0)){		//prvni 3 segmenty IP konci teckou,ctvrty ne
			counter--;
			*(ptr) = '.';
			ptr++;
		}	
			
		i--;
		print = 0;
		
	} 
	//ukonceni retezce
	if(counter == 0){
		ptr--;
		*(ptr) = 0;
	}
	else
		*(ptr) = 0;
	
return max_len - counter;
}
