#include "common.h"
#include "common.h"
#include "common.h"
#include "common.h"

int Global::dpiX(96);
int Global::dpiY(96);
SizeUnit Global::unit(SU_MM);
//QWidget* Global::mainWindow(nullptr);

int Global::mm2PixelsX(float mm)
{
	return mm / 25.4f * dpiX;
}

qreal Global::mm2PixelsXF(float mm)
{
	return mm / 25.4f * dpiX;
}

int Global::mm2PixelsY(float mm)
{
	return mm / 25.4f * dpiY;
}
qreal Global::mm2PixelsYF(float mm)
{
	return mm / 25.4f * dpiY;
}

float Global::pixels2mmX(int pixels)
{
	return pixels * 25.4f / dpiX;
}

float Global::pixels2mmY(int pixels)
{
	return pixels * 25.4f / dpiY;
}

float Global::convertUnit(SizeUnit from, SizeUnit to, float num, Qt::Orientation orientation)
{
	if (from == to)
		return num;

	float output = convertToMM(from, num, orientation);
	output = convertFromMM(to, num, orientation);

	return output;
}

float Global::convertToMM(SizeUnit from, float num, Qt::Orientation orientation)
{
	float factor = 1.0f;
	float dpi = orientation == Qt::Horizontal ? dpiX : dpiY;
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

float Global::convertFromMM(SizeUnit to, float num, Qt::Orientation orientation)
{
	float factor = 1.0f;
	float dpi = orientation == Qt::Horizontal ? dpiX : dpiY;
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

QTransform Global::matrixToMM(SizeUnit from, float hScale, float vScale)
{
	QTransform transform;
	transform.scale(Global::convertToMM(from, 1) * hScale, Global::convertToMM(from, 1, Qt::Vertical) * vScale);
	return transform;
}

QTransform Global::matrix(SizeUnit from, SizeUnit to, float hScale, float vScale)
{
	QTransform transform;
	transform.scale(Global::convertUnit(from, to, 1) * hScale, Global::convertUnit(from, to, 1, Qt::Vertical) * vScale);
	return transform;
}
