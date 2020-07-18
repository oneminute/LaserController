#ifndef COMMON_H
#define COMMON_H

#include <QDebug>

enum SizeUnit {
    SU_PERCENT,
    SU_PX,
    SU_PC,
    SU_PT,
    SU_MM,
    SU_MM100,
    SU_CM,
    SU_IN,
    SU_OTHER
};

enum PageType {
    PT_A0,
    PT_A1,
    PT_A2,
    PT_A3,
    PT_A4,
    PT_A5,
    PT_A6,
    PT_A7,
    PT_A8,
    PT_A9,
    PT_A10,
    PT_B0,
    PT_B1,
    PT_B2,
    PT_B3,
    PT_B4,
    PT_B5,
    PT_B6,
    PT_B7,
    PT_B8,
    PT_B9,
    PT_B10,
    PT_C0,
    PT_C1,
    PT_C2,
    PT_C3,
    PT_C4,
    PT_C5,
    PT_C6,
    PT_C7,
    PT_C8,
    PT_C9,
    PT_C10,
    PT_D0,
    PT_D1,
    PT_D2,
    PT_D3,
    PT_D4,
    PT_D5,
    PT_D6,
    PT_LETTER,
    PT_LEGAL,
    PT_JUNIOR_LEGAL,
    PT_LEDGER,
    PT_TABLOID,
    PT_UNDEFINED
};

enum LaserItemType
{
    LIT_LINE,
    LIT_CIRCLE,
    LIT_ELLIPSE,
    LIT_RECT,
    LIT_POLYLINE,
    LIT_POLYGON,
    LIT_PATH,
    LIT_SHAPE,
    LIT_BITMAP
};

#endif // COMMON_H