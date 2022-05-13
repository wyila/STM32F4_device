#ifndef _OPVX_DEVICE_H
#define _OPVX_DEVICE_H

enum BOARD_TYPE_t {
//BPL..., add forward, defined by hardware
    BPL_3U1200_ID          = 0x00,
    BPL_1U200_ID           = 0x01,

//CARD..., add forward, start with 0x20, defined by software
    CARD_DSM16P_ID         = 0x20,
    CARD_VS_KBL_CCU_ID     = 0x21,
    CARD_E1_TOP_ID         = 0x22,
    CARD_VS_AGU_16S_ID     = 0x23,
    CARD_VS_AGU_8SO_ID     = 0x24,
//unknow
    CARD_UNKNOW_ID         = 0xFF
};

enum BPL_SLOTS_t {
    BPL_3U1200_CAP  = 12,
    BPL_1U200_CAP   = 2,
};

#endif

