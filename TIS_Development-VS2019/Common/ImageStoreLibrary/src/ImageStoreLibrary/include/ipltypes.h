#pragma once

typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
} IplRect;

typedef struct {
	unsigned int x;
	unsigned int y;
	unsigned int z;
	unsigned int width;
	unsigned int height;
	unsigned int depth;
} IplRect3D;

typedef struct {
	unsigned int x;
	unsigned int y;
} IplPoint;

typedef struct {
	unsigned int width;
	unsigned int height;
} IplSize;

enum ProjectionTypes
{
	PROJECTION_MAX = 0,
	PROJECTION_MIN = 1,
	PROJECTION_MEAN = 2,
};