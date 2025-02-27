.. article:: apps_audio_ring_buffer_fpga
    :author: Karel Slanı <slany AT fit.vutbr.cz>
    :updated: 14.10.2009

    Práce s digitálním rozhraním kodeku TLV320AIC23B

================================
Digitální rozhraní TLV320AIC23B
================================
.. contents:: Obsah
   :depth: 1

Popis aplikace
==================

V FPGA je pomocí BRAM pamìti realizován kruhovı buffer o kapacitì 512 32-bitovıch poloek. Kruhovı buffer je pøipojenı ke kodeku TLV320AIC23B. 
V této aplikaci je buffer vyuit jako drát - tzn. slouí pouze k propojení digitálního vıstupu na digitální vstup kodeku.

Obsluná aplikace pracuje s audio vstupem JP6 a vıstupem JP7.

Návrh aplikace (MCU)
=========================

Podporované funkce
-------------------

Audio kodek TLV320AIC23B se skládá ze dvou èástí - analogové (obsahuje zesilovaèe, mixer) a digitální (obsahuje A/D a D/A sigma delta pøevodník a dále digitální seriové rozhraní). Analogová èást umoòuje vynechat digitální èást (bypass) pro úèely testování.  Aktivace digitálního rozhraní se provádí pomocí pøíkazu ``DSP_ON``, bypass poté pomocí ``BYPASS_DSP``. Pokud neslyšíte zvuk ani po zadání pøíkazu ``BYPASS_DSP``, ujistìte se, e jste správnì zapojili linkovı vstup.

Pøíkazem ``DSP_ON`` se pøes SPI rozhraní v kodeku aktivují A/D a D/A pøevodníky, nastaví se komunikaèní protokol na DSP reim, nastaví se vzorkovací frekvence a bitová šíøka slova.

Pøíkaz ``BYPASS_DSP`` deaktivuje digitální pøevodníky a v kodeku aktivuje analogovı bypass, kterı propojí analogovı vstup s analogovım vıstupem.

V obou pøedchozích reimech je aktivní vıstup JP7.

Pøíkazem ``MUTE`` se v sobou reimech ztlumí hlasitost vıstupu na minimum. Pøíkazem ``RESTORE`` se obnoví pùvodní hlasitost vıstupu.

Práce s DA pøevodníkem
-----------------------

**Konfigurace kodeku**

Konfigurace kodeku se provádí pomocí rozhraní SPI. K zápisu konfiguraèního slova se pouívá funkce ``codec_SPI_write(reg, content)``, kde ``reg`` je sedmibitová adresa konfiguraèního registru a ``content`` je devítibitovı obsah registru.

.. note:: 

   Popis funkce a registrù lze najít v souborech `mcu/libs/codec/audio_codec.h <SVN_DIR/trunk/mcu/libs/codec/audio_codec.h>`_ a `mcu/libs/codec/audio_codec.c <SVN_DIR/trunk/mcu/libs/codec/audio_codec.c>`_.

V ukázkové aplikaci se DSP reim kodeku aktivuje posloupností následujících pøíkazù:

.. listing::
   :language: c

    term_send_str("codec setup: reset\n");
    codec_SPI_write(RST_REG, 0);
    term_send_str("codec setup: analog path control\n");
    codec_SPI_write(ALOG_PATH_CTRL, ST_disable | DAC_on | BYP_off | INSEL_line | MICM_on | MICB_off);
    term_send_str("codec setup: digital interface format\n");
    // LRP_on must be set, MSB must be available after LRCIN/CLRCOUT falling edge
    codec_SPI_write(DTAL_IFACE_FMAT, MS_master |  LRSWAP_off | LRP_on | IWL_16bit | FOR_DSP);
    term_send_str("codec setup: sample rate format\n");
    codec_SPI_write(SRATE_CTRL, CLKOUT_full | CLKIN_full | USB_ADC48k_DAC48k | CLKMOD_USB);
    term_send_str("codec setup: digital interface activation\n");
    codec_SPI_write(DTAL_IFACE_ACT, ACT_on);
    term_send_str("codec setup: line-in left and right\n");
    codec_SPI_write(LINE_IN_VOL_L, LRS_off | LIM_off | (LIV_GAIN_Z));
    codec_SPI_write(LINE_IN_VOL_R, RLS_off | LIM_off | (LIV_GAIN_Z));
    term_send_str("codec setup: headphone left and right\n");
    codec_SPI_write(HPHONE_OUT_VOL_L, LZC_on | (LHV_GAIN_Z - 0));
    codec_SPI_write(HPHONE_OUT_VOL_R, RZC_on | (RHV_GAIN_Z - 0));
    term_send_str("codec setup: digital path control\n");
    codec_SPI_write(DTAL_PATH_CTRL, DACM_off | DEEMP_disabled | ADCHP_on);
    term_send_str("codec setup: power control\n");
    codec_SPI_write(PWR_DWN_CTRL, CODEC_ON | CODEC_CLK_on | CODEC_OSC_on |
                                  CODEC_OUT_on | CODEC_DAC_on | CODEC_ADC_on | CODEC_MIC_off | CODEC_LINE_on);

Zprovoznìní aplikace
========================
1. Pøed spuštìním kodeku se ujistìte, e propojka J3, J4 je aktivní a J2 neaktivní.

2. Do konektoru JP7 zapojte sluchátka a do JP6 zdroj signálu (napø. MP3 pøehrávaè).

3. Pøelote aplikaci.

4. Naprogramujte MCU a FPGA a spuste terminálovı program.

   Nyní byste mìli ve sluchátkách slyšet zvuk pøicházející na linkovı vstup JP6.
