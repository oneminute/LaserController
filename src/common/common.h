#ifndef COMMON_H
#define COMMON_H

#include <Qt>
#include <QDebug>
#include <QTransform>

#define qLogD qDebug().noquote().nospace()
#define qsLogD qDebug().nospace()
#define qnLogD qDebug().nospace()
#define qLogW qWarning().noquote().nospace()
#define qsLogW qWarning().nospace()
#define qnLogW qWarning().nospace()
#define qLogI qInfo().noquote().nospace()
#define qsLogI qInfo().nospace()
#define qnLogI qInfo().nospace()
#define qLogC qCritical().noquote().nospace()
#define qsLogC qCritical().nospace()
#define qnLogC qCritical().nospace()
#define qLogF qFatal().noquote().nospace()
#define qsLogF qFatal().nospace()
#define qnLogF qFatal().nospace()

class QWidget;

enum SizeUnit {
    SU_PERCENT,
    SU_PX,
    SU_PC,
    SU_PT,
    SU_MM,
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
    LPT_UNKNOWN,
    LPT_LINE,
    LPT_CIRCLE,
    LPT_ELLIPSE,
    LPT_RECT,
    LPT_POLYLINE,
    LPT_POLYGON,
    LPT_PATH,
    LPT_SHAPE,
    LPT_BITMAP,
	LPT_TEXT
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

enum QUADRANT
{
    QUADRANT_BL = 1,
    QUADRANT_1 = 1,
    QUADRANT_BR = 2,
    QUADRANT_2 = 2,
    QUADRANT_TR = 3,
    QUADRANT_3 = 3,
    QUADRANT_TL = 4,
    QUADRANT_4 = 4
};

enum RELATION
{
    A_CONTAINS_B,
    B_CONTAINS_A,
    INTERSECTION,
    NONE
};

class Global
{
public:
	static int dpiX;
	static int dpiY;
	static SizeUnit unit;
	static QWidget* mainWindow;

    static int mm2PixelsX(float mm);
	static qreal Global::mm2PixelsXF(float mm);
    static int mm2PixelsY(float mm);
	static qreal mm2PixelsYF(float mm);
    static float pixels2mmX(int pixels);
    static float pixels2mmY(int pixels);

	static float convertUnit(SizeUnit from, SizeUnit to, float num, Qt::Orientation orientation = Qt::Horizontal);
	static float convertToMM(SizeUnit from, float num, Qt::Orientation orientation = Qt::Horizontal);
	static float convertFromMM(SizeUnit to, float num, Qt::Orientation orientation = Qt::Horizontal);
	static QTransform matrixToMM(SizeUnit from, float hScale = 1.f, float vScale = 1.f);
	static QTransform matrix(SizeUnit from, SizeUnit to, float hScale = 1.0f, float vScale = 1.0f);
};

struct FillStyleAndPixelsCount
{
    int code;

    int count() const
    {
        return code & 0x7fffffff;
    }

    void setCount(int count)
    {
        code &= 0x80000000;
        int t = count & 0x7fffffff;
        code |= t;
    }

    void setSame(bool same = true)
    {
        if (same)
        {
            code |= 0x80000000;
        }
        else
        {
            code &= 0x7fffffff;
        }
    }

    bool same()
    {
        return code & 0x80000000;
    }

    FillStyleAndPixelsCount()
        : code(0)
    {}
};

Q_DECLARE_METATYPE(FillStyleAndPixelsCount);

struct FinishRun
{
public:
    FinishRun()
        : code(0)
    {}

    union
    {
        quint16 code;
        struct
        {
            quint8 action;
            quint8 relays;
        };
    };

    void setRelays(const QList<int>& relays, bool enabled = true)
    {
        for (int no : relays)
        {
            setRelay(no, enabled);
        }
    }

    void setRelay(int no, bool enabled = true)
    {
        int relayNo = no;
        if (relayNo < 0 || relayNo > 7)
            return;

        int relayBit = 1 << relayNo;

        if (enabled)
        {
            relays |= relayBit;
        }
        else
        {
            relays &= ~relayBit;
        }
    }

    void setAction(int action)
    {
        this->action = action;
    }

    bool isEnabled(int no)
    {
        int relayNo = no;
        int relayBit = 1 << relayNo;
        return relayBit & relays;
    }

    QString toString();
};

Q_DECLARE_METATYPE(FinishRun);

#endif // COMMON_H