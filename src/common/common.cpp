#include "common.h"
#include "common/Config.h"

int Global::dpiX(96);
int Global::dpiY(96);
qreal Global::mmToInchCoef(0.03937007874);
qreal Global::inchToMmCoef(25.4);
//grid
qreal Global::lowPen1(224);
qreal Global::lowPen2(236);
qreal Global::mediumPen1(208);
qreal Global::mediumPen2(228);
qreal Global::highPen1(192);
qreal Global::highPen2(220);
SizeUnit Global::unit(SU_MM);

int Global::mechToSceneH(qreal value)
{
	return qRound(value * dpiX * 0.001 / 25.4);
}

qreal Global::mechToSceneHF(qreal value)
{
	return value * dpiX * 0.001 / 25.4;
}

int Global::mechToSceneV(qreal value)
{
	return qRound(value * dpiY * 0.001 / 25.4);
}

qreal Global::mechToSceneVF(qreal value)
{
	return value * dpiY * 0.001 / 25.4;
}

qreal Global::mmToSceneHF(qreal value)
{
	return value * dpiX / 25.4;
}

qreal Global::mmToSceneVF(qreal value)
{
	return value * dpiY / 25.4;
}

qreal Global::sceneToMechH(int pixels)
{
	return pixels * 25400.0 / dpiX;
}

qreal Global::sceneToMechV(int pixels)
{
	return pixels * 25400.0 / dpiY;
}

qreal Global::sceneToMmH(int pixels)
{
	return pixels * 25.4 / dpiX;
}

qreal Global::sceneToMmV(int pixels)
{
	return pixels * 25.4 / dpiY;
}

qreal Global::convertUnit(qreal num, SizeUnit from, SizeUnit to, Qt::Orientation orientation)
{
	if (from == to)
		return num;

	qreal output = convertToUm(num, from, orientation);
	output = convertFromUm(num, to, orientation);

	return output;
}

qreal Global::convertToUm(qreal num, SizeUnit from, Qt::Orientation orientation)
{
	qreal factor = 1.0;
	qreal dpi = orientation == Qt::Horizontal ? dpiX : dpiY;
	switch (from)
	{
	case SU_PX:
		factor = 25400.0 / dpi;
		break;
	case SU_PT:
		factor = 25400.0 / 72;
		break;
	case SU_PC:
		factor = 25400.0 / 6;
		break;
	case SU_UM:
		factor = 1.0;
		break;
	case SU_MM:
		factor = 1000.0;
		break;
	case SU_CM:
		factor = 10000.0;
		break;
	case SU_IN:
		factor = 25400;
		break;
	}
	return num * factor;
}

qreal Global::convertToUmH(qreal num, SizeUnit from)
{
	return convertToUm(num, from, Qt::Horizontal);
}

qreal Global::convertToUmV(qreal num, SizeUnit from)
{
	return convertToUm(num, from, Qt::Vertical);
}

qreal Global::convertFromUm(qreal num, SizeUnit to, Qt::Orientation orientation)
{
	qreal factor = 1.0;
	qreal dpi = orientation == Qt::Horizontal ? dpiX : dpiY;
	switch (to)
	{
	case SU_PX:
		factor = dpi * 0.001 / 25.4;
		break;
	case SU_PT:
		factor = 0.072 / 25.4;
		break;
	case SU_PC:
		factor = 0.006 / 25.4;
		break;
	case SU_UM:
		factor = 1.0;
		break;
	case SU_MM:
		factor = 0.001;
		break;
	case SU_CM:
		factor = 0.0001;
		break;
	case SU_IN:
		factor = 0.001 / 25.4;
		break;
	}
	return num * factor;
}

qreal Global::convertFromUmH(qreal num, SizeUnit to)
{
	return convertFromUm(num, to, Qt::Horizontal);
}

qreal Global::convertFromUmV(qreal num, SizeUnit to)
{
	return convertFromUm(num, to, Qt::Vertical);
}

QTransform Global::matrixToUm(SizeUnit from)
{
	return QTransform::fromScale(
		Global::convertToUmH(1.0, from),
		Global::convertToUmV(1.0, from));
}

QTransform Global::matrixFromUm(SizeUnit to)
{
	return QTransform::fromScale(
		Global::convertFromUmH(1.0, to),
		Global::convertFromUmV(1.0, to));
}

QTransform Global::matrix(SizeUnit from, SizeUnit to)
{
	return QTransform::fromScale(
		Global::convertUnit(1.0, from, to), 
		Global::convertUnit(1.0, from, to, Qt::Vertical));
}

