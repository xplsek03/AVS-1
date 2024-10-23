/**
 * @file BatchMandelCalculator.cc
 * @author Michal Plsek <xplsek03@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over small batches
 * @date 2024-23-10
 */
#include <BaseMandelCalculator.h>

class BatchMandelCalculator : public BaseMandelCalculator
{
public:
    BatchMandelCalculator(unsigned matrixBaseSize, unsigned limit);
    ~BatchMandelCalculator();
    int *calculateMandelbrot();

private:
	int *data;
	//float *real;
	//float *imag;
};
