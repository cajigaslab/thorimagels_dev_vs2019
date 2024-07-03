#include "GRController.h"
#include "stdafx.h"

GRController::GRController():_hasGRController(false)
{
}

GRController::~GRController()
{
}

long GRController::StartGR()
{
	return TRUE;
}

long GRController::StopGR()
{
	return TRUE;
}

bool GRController::IsAvaliable()
{
	return true;
}

long GRController::MoveGalvoToParkPosition()
{
	return TRUE;
}

long GRController::MoveGalvoToPostion(double pos)
{
	return TRUE;
}