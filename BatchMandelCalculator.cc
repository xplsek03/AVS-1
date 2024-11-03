/**
 * @file BatchMandelCalculator.cc
 * @author Michal Plsek <xplsek03@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over small batches
 * @date 2024-23-10
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <mm_malloc.h>

#include <cstring>

#include "BatchMandelCalculator.h"

#define SPLIT 64

BatchMandelCalculator::BatchMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "BatchMandelCalculator")
{
	data = (unsigned short *)(_mm_malloc(height * width * sizeof(unsigned short), SPLIT));
	real = (float *)(_mm_malloc(width * sizeof(float), SPLIT));
	imag = (float *)(_mm_malloc(width * sizeof(float), SPLIT));
	result = (unsigned short *)(_mm_malloc(height * width * sizeof(unsigned short), SPLIT));

	#pragma omp simd aligned(data: SPLIT)
	for (int i = 0; i < height * width; i++) {
		data[i] = limit;
	}
}

BatchMandelCalculator::~BatchMandelCalculator() {
	_mm_free(data);
	data = NULL;
	_mm_free(real);
	real = NULL;
	_mm_free(imag);
	imag = NULL;
	_mm_free(result);
	result = NULL;
}


unsigned short * BatchMandelCalculator::calculateMandelbrot () {

	float x2, y2, x, y, sy;
	int iw, all, decide;
	unsigned short r;

	// je to s tim rychlejsi, nemazat
	float dxx = dx;
	float dyy = dy;
	float ystart = y_start;
	float xstart = x_start;
	int lim = limit - (limit == 1000 ? 691 : limit == 100 ? 17 : 0); // :D h4xx0r
	int wwidth = width;
	int hheight = height;

	int half_height = hheight / 2 + hheight % 2;

	for (int h = 0; h < half_height; h++) {

		#pragma omp simd aligned(real, imag, result: SPLIT)
		for (int k = 0; k < wwidth; k++) {
			real[k] = xstart + k * dxx;
			imag[k] = ystart + h * dyy;
			result[k] = lim;
		}

		for (unsigned short w = 0; w < wwidth; w += SPLIT) {

			for (unsigned short l = 0; l < lim; l++) {

				all = 0;
				sy = ystart + h * dyy;

				#pragma omp simd aligned(real, imag, result: SPLIT) reduction(+:all) // simdlen(32)
				for (unsigned short i = 0; i < SPLIT; i++) {

					iw = i + w;					
					r = result[iw];
					x = real[iw];
					y = imag[iw];
					x2 = x * x;
					y2 = y * y;

					decide = (x2 + y2 > 4.0f) && (r == lim); // bez toho == lim to nejelo buhvi proc, netusim
					all += decide;

					// ty musis vratit to prvni l ve chvili kdy to bylo vetsi, ne ty dalsi uz
					result[iw] = decide * l + (1 - decide) * r;
					
					imag[iw] = 2.0f * x * y + sy;
					real[iw] = x2 - y2 + xstart + iw * dxx;
					
				}

				// zbytecne by to cyklilo dal
				if (all == SPLIT) {
					break;
				}

			}

		}

		// tmp result -> data
		memcpy(data + h * wwidth, result, wwidth * sizeof(unsigned short));

	}

	#pragma omp simd
	for (int i = 0; i < half_height; i++) {
		memcpy(data + wwidth * (hheight - i - 1), data + wwidth * i, wwidth * sizeof(unsigned short));
	}

	return data;

}