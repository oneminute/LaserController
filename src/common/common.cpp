#include "common.h"
#include "common/Config.h"

int Global::dpiX(96);
int Global::dpiY(96);
//grid
qreal Global::lowPen1(224);
qreal Global::lowPen2(236);
qreal Global::mediumPen1(208);
qreal Global::mediumPen2(228);
qreal Global::highPen1(192);
qreal Global::highPen2(220);
SizeUnit Global::unit(SU_MM);
//QWidget* Global::mainWindow(nullptr);

int Global::mm2PixelsX(qreal mm)
{
	return mm / 25.4f * dpiX;
}

qreal Global::mm2PixelsXF(qreal mm)
{
	return mm / 25.4f * dpiX;
}

int Global::mm2PixelsY(qreal mm)
{
	return mm / 25.4f * dpiY;
}
qreal Global::mm2PixelsYF(qreal mm)
{
	return mm / 25.4f * dpiY;
}

qreal Global::pixels2mmX(int pixels)
{
	return pixels * 25.4f / dpiX;
}

qreal Global::pixels2mmY(int pixels)
{
	return pixels * 25.4f / dpiY;
}
qreal Global::pixelsF2mmX(qreal pixels)
{
	return pixels * 25.4f / dpiX;
}

qreal Global::pixelsF2mmY(qreal pixels)
{
	return pixels * 25.4f / dpiY;
}

qreal Global::convertUnit(SizeUnit from, SizeUnit to, qreal num, Qt::Orientation orientation)
{
	if (from == to)
		return num;

	qreal output = convertToMM(from, num, orientation);
	output = convertFromMM(to, num, orientation);

	return output;
}

qreal Global::convertToMM(SizeUnit from, qreal num, Qt::Orientation orientation)
{
	qreal factor = 1.0f;
	qreal dpi = orientation == Qt::Horizontal ? dpiX : dpiY;
	switch (from)
	{
	case SU_PX:
		factor = 1 / dpi * 25.4f;
		break;
	case SU_PT:
		factor = 25.4f / 72;
		break;
	case SU_PC:
		factor = 25.4f / 6;
		break;
	case SU_MM:
		factor = 1.0f;
		break;
	case SU_CM:
		factor = 10.0f;
		break;
	case SU_IN:
		factor = 25.4f;
		break;
	}
	return num * factor;
}

qreal Global::convertFromMM(SizeUnit to, qreal num, Qt::Orientation orientation)
{
	qreal factor = 1.0f;
	qreal dpi = orientation == Qt::Horizontal ? dpiX : dpiY;
	switch (to)
	{
	case SU_PX:
		factor = dpi / 25.4f;
		break;
	case SU_PT:
		factor = 72 / 25.4f;
		break;
	case SU_PC:
		factor = 6 / 25.4f;
		break;
	case SU_MM:
		factor = 1.0f;
		break;
	case SU_CM:
		factor = 0.1f;
		break;
	case SU_IN:
		factor = 1 / 25.4f;
		break;
	}
	return num * factor;
}

qreal Global::convertToMachining(SizeUnit from, Qt::Orientation orientation)
{
	return convertToMM(from, Config::General::machiningUnit(), orientation);
}

QTransform Global::matrixToMM(SizeUnit from, qreal hScale, qreal vScale)
{
	return QTransform::fromScale(Global::convertToMM(from, 1) * hScale, 
		Global::convertToMM(from, 1, Qt::Vertical) * vScale);
}

QTransform Global::matrix(SizeUnit from, SizeUnit to, qreal hScale, qreal vScale)
{
	return QTransform::fromScale(Global::convertUnit(from, to, 1) * hScale, 
		Global::convertUnit(from, to, 1, Qt::Vertical) * vScale);
}

QTransform Global::matrixToMachining(SizeUnit from)
{
	return QTransform::fromScale(Global::convertToMM(from, Config::General::machiningUnit()), 
		Global::convertToMM(from, Config::General::machiningUnit(), Qt::Vertical));
}

int Global::pxToMachiningH(qreal x)
{
	return qRound(convertToMM(SU_PX, x * Config::General::machiningUnit()));
}

int Global::pxToMachiningV(qreal y)
{
	return qRound(convertToMM(SU_PX, y * Config::General::machiningUnit(), Qt::Vertical));
}
