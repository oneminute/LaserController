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

enum LaserPrimitiveType
{
    LPT_LINE,
    LPT_CIRCLE,
    LPT_ELLIPSE,
    LPT_RECT,
    LPT_POLYLINE,
    LPT_POLYGON,
    LPT_PATH,
    LPT_SHAPE,
    LPT_BITMAP
};

enum LaserLayerType
{
    LLT_ENGRAVING,
    LLT_CUTTING,
    LLT_BOTH
};

enum RelayAction
{
    RA_NONE = 0,
    RA_RELEASE = 1,
    RA_ORIGIN = 2,
    RA_MACHINING_1 = 3,
    RA_MACHINING_2 = 4,
    RA_MACHINING_3 = 5
};

enum PropertyType
{
    PT_Text = 0,
    PT_FinishRun
};

enum PropertyEditor
{
    PE_LineEdit = 0,
    PE_FinishRunWidget
};

enum WidgetUserData
{
    WUD_PropertyValue = Qt::UserRole + 1, 
    WUD_PropertyType, 
    WUD_PropertyEditor
};

struct FillStyleAndPixelsCount
{
    union {
        int code;
        struct {
            quint8 fillStyle;
            quint8 count0;
            quint8 count1;
            quint8 count2;
        };
    };

    FillStyleAndPixelsCount()
        : code(0)
    {}
};

#endif // COMMON_H