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

int Global::mm2PixelsX(qreal mm)
{
	return qRound(mm * dpiX / 25.4);
}

qreal Global::mm2PixelsXF(qreal mm)
{
	return mm * dpiX / 25.4;
}

int Global::mm2PixelsY(qreal mm)
{
	return qRound(mm * dpiY / 25.4);
}
qreal Global::mm2PixelsYF(qreal mm)
{
	return mm * dpiY / 25.4;
}

qreal Global::pixels2mmX(int pixels)
{
	return pixels * 25.4 / dpiX;
}

qreal Global::pixels2mmY(int pixels)
{
	return pixels * 25.4 / dpiY;
}
qreal Global::pixelsF2mmX(qreal pixels)
{
	return pixels * 25.4 / dpiX;
}

qreal Global::pixelsF2mmY(qreal pixels)
{
	return pixels * 25.4 / dpiY;
}

qreal Global::convertUnit(qreal num, SizeUnit from, SizeUnit to, Qt::Orientation orientation)
{
	if (from == to)
		return num;

	qreal output = convertToMM(num, from, orientation);
	output = convertFromMM(num, to, orientation);

	return output;
}

qreal Global::convertToMM(qreal num, SizeUnit from, Qt::Orientation orientation)
{
	qreal factor = 1.0;
	qreal dpi = orientation == Qt::Horizontal ? dpiX : dpiY;
	switch (from)
	{
	case SU_PX:
		factor = 1 * 25.4 / dpi;
		break;
	case SU_PT:
		factor = 25.4 / 72;
		break;
	case SU_PC:
		factor = 25.4 / 6;
		break;
	case SU_MM:
		factor = 1.0;
		break;
	case SU_CM:
		factor = 10.0;
		break;
	case SU_IN:
		factor = 25.4;
		break;
	}
	return num * factor;
}

qreal Global::convertToMmH(qreal num, SizeUnit from)
{
	return convertToMM(num, from, Qt::Horizontal);
}

qreal Global::convertToMmV(qreal num, SizeUnit from)
{
	return convertToMM(num, from, Qt::Vertical);
}

qreal Global::convertFromMM(qreal num, SizeUnit to, Qt::Orientation orientation)
{
	qreal factor = 1.0;
	qreal dpi = orientation == Qt::Horizontal ? dpiX : dpiY;
	switch (to)
	{
	case SU_PX:
		factor = dpi / 25.4;
		break;
	case SU_PT:
		factor = 72 / 25.4;
		break;
	case SU_PC:
		factor = 6 / 25.4;
		break;
	case SU_MM:
		factor = 1.0;
		break;
	case SU_CM:
		factor = 0.1;
		break;
	case SU_IN:
		factor = 1.0 / 25.4;
		break;
	}
	return num * factor;
}

qreal Global::convertFromMmH(qreal num, SizeUnit to)
{
	return convertFromMM(num, to, Qt::Horizontal);
}

qreal Global::convertFromMmV(qreal num, SizeUnit to)
{
	return convertFromMM(num, to, Qt::Vertical);
}

qreal Global::convertToMachining(qreal num, SizeUnit from, Qt::Orientation orientation)
{
	return convertToMM(num, from, orientation) * 1000;
}

QTransform Global::matrixToMM(SizeUnit from, qreal hScale, qreal vScale)
{
	return QTransform::fromScale(Global::convertToMM(1.0, from) * hScale, 
		Global::convertToMM(1.0, from, Qt::Vertical) * vScale);
}

QTransform Global::matrix(SizeUnit from, SizeUnit to, qreal hScale, qreal vScale)
{
	return QTransform::fromScale(Global::convertUnit(1.0, from, to) * hScale, 
		Global::convertUnit(1.0, from, to, Qt::Vertical) * vScale);
}

QTransform Global::matrixToMachining(SizeUnit from)
{
	return QTransform::fromScale(Global::convertToMM(1000.0, from), 
		Global::convertToMM(1000.0, from, Qt::Vertical));
}

