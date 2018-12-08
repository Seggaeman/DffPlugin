#pragma once

#include <iostream>
#include <Windows.h>
#define RELEASE(x)		{ if (x != NULL) delete x;		x = NULL; }
#define RELEASEARRAY(x)	{ if (x != NULL) delete []x;	x = NULL; }

#include "RevisitedRadix.h"
#include "CustomArray.h"
#include "Adjacency.h"
#include "Striper.h"