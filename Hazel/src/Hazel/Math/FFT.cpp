#include "hzpch.h"
#include "FFT.h"
#include <tgmath.h>

using namespace std::literals::complex_literals;

namespace Hazel {

	
	
	void FFT::fft1d(std::complex<float>* f, std::complex<float>* F, uint32_t N, uint32_t s)
	{
		float lg2 = log2f((float)N);
		if (ceilf(lg2)!=lg2)
		{
			HZ_CORE_ERROR("Length N not a power of 2!");
		}
		if (N == 1)
		{
			F[0] = f[0];
		}	
		else
		{
			fft1d(f,		F ,			N / 2,	2 * s);
			fft1d(f + s,	F + N /2,	N / 2,	2 * s);
			for (int k = 0;k < N / 2;k++)
			{
				std::complex<float> t = *(F + k);
				*(F + k) =				t	+	(std::complex<float>)std::exp(1i * (-2 * M_PI * k / N))	*	(*(F + k + N / 2));
				*(F + k + N/2) =		t	-	(std::complex<float>)std::exp(1i * (-2 * M_PI * k / N))	*	(*(F + k + N / 2));
			}
		}

	}

	void FFT::fft2d(std::complex<float>* f, std::complex<float>* F, uint32_t N, uint32_t M)
	{
		//// ROWS
		//for (int m = 0;m < M;m++) 
		//{
		//	fft1d(f + m * N, F + m * N, N, 1);
		//}
		//for (int n = 0;n < N;n++)
		//{
		//	fft1d(F + n, F + n, M, N);
		//
		// works fine but might be slow: (23/05/20)
		//// ROWS
		std::complex<float>* r = new std::complex<float>[M];
		std::complex<float>* R = new std::complex<float>[M];
		for (int n = 0;n < N;n++)
		{
			// copy row
			for (int m = 0;m < M;m++)
			{
				*(r + m) = *(f + n * N + m);
			}
			// fft on row
			fft1d(r, R, M, 1);
			for (int m = 0;m < M;m++)
			{
				*(F + n * N + m) = *(R + m);
			}
		}

		// COLS
		std::complex<float>* c = new std::complex<float>[N];
		std::complex<float>* C = new std::complex<float>[N];
		for (int m = 0;m < M;m++)
		{
			// copy col
			for (int n = 0;n < N; n++)
			{
				*(c + n) = *(F + m + n * M);
				
			}
			fft1d(c, C, N, 1);
			// fft on col
			for (int n = 0;n < N;n++)
			{
				*(F + m + n * M) = *(C + n);
			}
		}
	}

	void FFT::dft2d(std::vector<std::vector<std::complex<double>>>& arr, bool inverse)
	{
		uint32_t N = arr.size();
		uint32_t M = arr[0].size();
		// rows
		transpose(arr);
		for (int n = 0;n < N;n++)
		{

			fshift(arr[n]);
			dft1d(arr[n], inverse);
		}
		// cols
		transpose(arr);
		for (int m = 0;m < M;m++)
		{

			fshift(arr[m]);
			dft1d(arr[m], inverse);
		}
	}

	void FFT::dft1d(std::vector<std::complex<double>>& vec,bool inverse)
	{
		uint32_t N = vec.size();
		fshift(vec);
		std::vector<std::complex<double>> res(N, std::complex<double>(0, 0));
		int dir = -1;
		if (inverse)
			dir = 1;

		for (int n = 0;n < N;n++)
		{
			std::complex<double> temp;
			temp = (0, 0);
			for (int k = 0;k < N;k++)
			{
				temp += vec[k] * (std::complex<double>)std::exp(1i * (dir * 2 * M_PI * (double)n * (double)k / (double)N));
			}
			res[n] = temp / std::sqrt((double)N);
		}

		vec.assign(res.begin(), res.end());
	}

	void FFT::transpose(std::vector<std::vector<std::complex<double>>>& arr)
	{
		uint32_t N = arr.size();
		uint32_t M = arr[0].size();
		if (N != M)
			HZ_CORE_ASSERT(0,"FT::Transpose currently only supported for square matrices!");

		for (int n = 0;n < N;n++)
		{
			for (int m = n + 1;m < M;m++)
			{
				std::swap(arr[n][m], arr[m][n]);
			}
		}
	}

	void FFT::fshift(std::vector<std::complex<double>>& vec)
	{
		uint32_t N = vec.size();
		for (int n = 0;n < N/2;n++)
		{
			std::swap(vec[n], vec[n + N / 2]);
		}
	}

}