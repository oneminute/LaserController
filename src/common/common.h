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
    SU_UM,
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
    LPT_NULL,
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
	LPT_TEXT,
    LPT_NURBS
};

enum LaserLayerType
{
    LLT_ENGRAVING = 0,
    LLT_CUTTING = 1,
    LLT_FILLING = 2
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
//selected
enum SelectionOriginal
{
	SelectionOriginalTopLeft = 0,
	SelectionOriginalTopCenter = 1,
	SelectionOriginalTopRight = 2 ,
	SelectionOriginalLeftCenter = 3,
	SelectionOriginalCenter = 4,
	SelectionOriginalRightCenter = 5,
	SelectionOriginalLeftBottom = 6,
	SelectionOriginalBottomCenter = 7,
	SelectionOriginalBottomRight = 8
};
//selected
enum SelectionTransformType {
	Transform_MOVE = 0,
	Transform_SCALE = 1,
	Transform_ROTATE = 2,
	Transform_RESIZE = 3
};
/// <summary>
/// 当前选项的修改方式
/// </summary>
enum StoreStrategy
{
    SS_AS_IS,

    /// <summary>
    /// 直接修改
    /// </summary>
    SS_DIRECTLY,

    SS_REGISTER,

    SS_NORMAL,

    /// <summary>
    /// 延后保存，一般是与寄存器相关，由板卡返回寄存器的值后再修改当前内存中的值
    /// </summary>
    SS_LAZY
};

enum InputWidgetType
{
    IWT_Custom,
    IWT_CheckBox,
    IWT_ComboBox,
    IWT_LineEdit,
    IWT_TextEdit,
    IWT_PlainTextEdit,
    IWT_SpinBox,
    IWT_DoubleSpinBox,
    IWT_TimeEdit,
    IWT_DateEdit,
    IWT_DateTimeEdit,
    IWT_Dial,
    IWT_EditSlider,
    IWT_FloatEditSlider,
    IWT_Vector2DWidget,
    //IWT_LimitationWidget
};

enum DataType
{
    DT_INT,
    DT_FLOAT,
    DT_DOUBLE,
    DT_REAL,
    DT_BOOL,
    DT_STRING,
    DT_DATETIME,
    DT_RECT,
    DT_POINT,
    DT_SIZE,
    DT_LIST,
    DT_CUSTOM
};

enum LaserNodeType
{
    LNT_DOCUMENT,
    LNT_LAYER,
    LNT_PRIMITIVE,
    LNT_VIRTUAL,
};

enum StartFromType
{
    SFT_CurrentPosition,
    SFT_UserOrigin,
    SFT_AbsoluteCoords
};

//enum ModifiedBy
//{
//    MB_Manual,
//    MB_ConfigFile,
//    MB_Widget,
//    MB_Register,
//    MB_RegisterConfirmed
//};

enum PrimitiveProperty
{
    PP_Width,
    PP_Height,
    PP_PosX,
    PP_PosY,
    PP_ScaleX,
    PP_ScaleY,
    PP_Other
};

enum FillingType
{
    FT_Line,
    FT_Pixel
};

enum FinishRunType
{
    FT_CurrentPos,
    FT_Unload,
    FT_BackToOrigin,
    FT_BackToUserOrigin
};

enum ArrangeType
{
    AT_SameWidth = 5,
    AT_SameHeight = 6,
    AT_Left = 7,
    AT_Right = 8,
    AT_Top = 9,
    AT_Bottom = 10,
    AT_Center = 11,
    AT_VSpaced = 12,
    AT_VCentered = 13,
    AT_HSpaced = 14,
    AT_HCentered = 15
};

enum CalibrationPattern
{
    CP_CHESSBOARD,
    CP_CIRCLES_GRID,
    CP_ASYMMETRIC_CIRCLES_GRID,
    CP_CHARUCO_BOARD
};
Q_DECLARE_METATYPE(CalibrationPattern)

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

class Global
{
public:
	static int dpiX;
	static int dpiY;
	static SizeUnit unit;
    static qreal mmToInchCoef;
    static qreal inchToMmCoef;
	//static QWidget* mainWindow;
	//grid pen
	static qreal lowPen1;
	static qreal lowPen2;

	static qreal mediumPen1;
	static qreal mediumPen2;

	static qreal highPen1;
	static qreal highPen2;

    static int mechToSceneH(qreal value);
	static qreal mechToSceneHF(qreal value);
    static int mechToSceneV(qreal value);
	static qreal mechToSceneVF(qreal value);
    static qreal sceneToMechH(int pixels);
    static qreal sceneToMechV(int pixels);
	static qreal mmToSceneHF(qreal value);
	static qreal mmToSceneVF(qreal value);
    static qreal sceneToMmH(int pixels);
    static qreal sceneToMmV(int pixels);

	static qreal convertUnit(qreal num, SizeUnit from, SizeUnit to, Qt::Orientation orientation = Qt::Horizontal);

	static qreal convertToUm(qreal num, SizeUnit from = SU_PX, Qt::Orientation orientation = Qt::Horizontal);
	static qreal convertToUmH(qreal num, SizeUnit from = SU_PX);
	static qreal convertToUmV(qreal num, SizeUnit from = SU_PX);

	static qreal convertFromUm(qreal num, SizeUnit to = SU_PX, Qt::Orientation orientation = Qt::Horizontal);
	static qreal convertFromUmH(qreal num, SizeUnit to = SU_PX);
	static qreal convertFromUmV(qreal num, SizeUnit to = SU_PX);

	static QTransform matrixToUm(SizeUnit from = SU_PX);
	static QTransform matrix(SizeUnit from, SizeUnit to);
    static QTransform matrixFromUm(SizeUnit to = SU_PX);
};

typedef QPair<QPointF, QPointF> PointPair;
typedef QList<PointPair> PointPairList;

#define SAFE_DELETE(ptr) \
    if (ptr) \
    { \
        delete ptr; \
        ptr = nullptr; \
    }

#endif // COMMON_H