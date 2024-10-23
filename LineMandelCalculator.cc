/**
 * @file BatchMandelCalculator.cc
 * @author Michal Plsek <xplsek03@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date 2024-23-10
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>
#include <mm_malloc.h>
#include <cstring>

#include "LineMandelCalculator.h"

#define BLOCK 64

LineMandelCalculator::LineMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "LineMandelCalculator")
{
	data = (int *)(_mm_malloc(height * width * sizeof(int), BLOCK));
	
	#pragma omp simd aligned(data:BLOCK)
	for (int i = 0; i < height * width; i++) {
		data[i] = limit;
	}
}

LineMandelCalculator::~LineMandelCalculator() {
	_mm_free(data);
	data = NULL;
}


int * LineMandelCalculator::calculateMandelbrot () {
	
	int *pdata = data;
	int half_height = height / 2 + height % 2;

	float x, y, origx, origy, x2, y2;

	for (int i = 0; i < half_height; i++) {

		# pragma omp simd aligned(data: BLOCK) simdlen(16)
		for (int j = 0; j < width; j++) {  // jdi po radku

			x = origx = x_start + j * dx;
			y = origy = y_start + i * dy;

			for(int k = 0; k < limit; k++) {
				x2 = x * x;
				y2 = y * y;
				if (x2 + y2 > 4.0f) {
					*(pdata + i * width + j) = k;
					break;
				}			
				y = 2.0f * x * y + origy;
				x = x2 - y2 + origx;
			}
		}
	}

	#pragma omp simd aligned(data: BLOCK)
	for (int i = 0; i < half_height; i++) {
		memcpy(pdata + width * (height - i - 1), pdata + width * i, width * sizeof(int));
	}
	return data;
}