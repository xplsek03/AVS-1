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
	data = (int *)(_mm_malloc(height * width * sizeof(int), SPLIT));

	#pragma omp simd aligned(data: SPLIT)
	for (int i = 0; i < height * width; i++) {
		data[i] = limit;
	}
}

BatchMandelCalculator::~BatchMandelCalculator() {
	_mm_free(data);
	data = NULL;
}

// batch - implementace bez prehozeni smycek

int * BatchMandelCalculator::calculateMandelbrot () {

	int *pdata = data;
	int half_height = height / 2 + height % 2;
	float x, y, origx, origy, x2, y2;

	for (int i = 0; i < half_height; i++) {
		for (int g = 0; g < width; g += SPLIT) {

			# pragma omp simd aligned(data: SPLIT) simdlen(16)
			for (int j = g; j < g + SPLIT; j++) {
				origx = x = x_start + j * dx;
				origy = y = y_start + i * dy;
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
	}
	#pragma omp simd aligned(data: SPLIT)
	for (int i = 0; i < half_height; i++) {
		memcpy(pdata + width * (height - i - 1), pdata + width * i, width * sizeof(int));
	}
	return data;
}

///// batch - implementace s prehozenim smycek

// BatchMandelCalculator::BatchMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
// 	BaseMandelCalculator(matrixBaseSize, limit, "BatchMandelCalculator")
// {
// 	data = (int *)(_mm_malloc(height * width * sizeof(int), SPLIT));
// 	real = (float *)(_mm_malloc(width * sizeof(float), SPLIT));
// 	imag = (float *)(_mm_malloc(width * sizeof(float), SPLIT));
// 	#pragma omp simd aligned(data: SPLIT)
// 	for (int i = 0; i < height * width; i++) {
// 		data[i] = limit;
// 	}
// }

// BatchMandelCalculator::~BatchMandelCalculator() {
// 	_mm_free(data);
// 	data = NULL;
// 	_mm_free(real);
// 	real = NULL;
// 	_mm_free(imag);
// 	imag = NULL;
// }


// int * BatchMandelCalculator::calculateMandelbrot () {

// 	int *pdata = data;

// 	float x2, y2;
// 	float x, y;
// 	int iw, all, rs;

// 	int half_height = height / 2 + height % 2;

// 	for (int h = 0; h < half_height; h++) {

// 		rs = h * width;

// 		#pragma omp simd aligned(real, imag: SPLIT)
// 		for (int i = 0; i < width; i++) {
// 			real[i] = x_start + i * dx;
// 			imag[i] = y_start + h * dy;
// 		}

// 		for (int w = 0; w < width; w += SPLIT) {
// 			for (int l = 0; l < limit; l++) {

// 				all = 0;

// 				#pragma omp simd aligned(pdata, real, imag: SPLIT) reduction(+:all) simdlen(16)
// 				for (int i = 0; i < SPLIT; i++) {
// 					iw = i + w;
// 					if (pdata[rs + iw] == limit) {

// 						x = real[iw];
// 						y = imag[iw];

// 						x2 = x * x;
// 						y2 = y * y;

// 						if (x2 + y2 > 4.0f) {
// 							pdata[rs + iw] = l;
// 							all++;
// 							continue;
// 						}
// 						imag[iw] = 2.0f * x * y + y_start + h * dy;
// 						real[iw] = x2 - y2 + x_start + (iw) * dx;	

// 					}
					
// 				}

// 				// zbytecne by to pocitalo dalsi cisla
// 				if (all == SPLIT) {
// 					break;
// 				}

// 			}

// 		}

// 	}

// 	#pragma omp simd
// 	for (int i = 0; i < half_height; i++) {
// 		memcpy(pdata + width * (height - i - 1), pdata + width * i, width * sizeof(int));
// 	}

// 	return data;

// }