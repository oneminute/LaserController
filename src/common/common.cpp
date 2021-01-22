#include "common.h"
#include "common.h"
#include "common.h"
#include "common.h"

int Global::dpiX(96);
int Global::dpiY(96);
SizeUnit Global::unit(SU_MM);

int Global::mm2PixelsX(float mm)
{
	return mm / 25.4f * dpiX;
}

int Global::mm2PixelsY(float mm)
{
	return mm / 25.4f * dpiY;
}

float Global::pixels2mmX(int pixels)
{
	return pixels / dpiX * 25.4f;
}

float Global::pixels2mmY(int pixels)
{
	return pixels / dpiY * 25.4f;
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
	case SU_MM100:
		factor = 0.01f;
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
		factor = dpi * 25.4f;
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
	case SU_MM100:
		factor = 100.f;
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
