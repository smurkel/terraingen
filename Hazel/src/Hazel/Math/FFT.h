#pragma once

#include <vector>
#include <complex>

namespace Hazel {

	class FFT
	{
	public:
		// 18 May:
		static void fft1d(std::complex<float>* f, std::complex<float>* F, uint32_t N, uint32_t s);
		static void fft2d(std::complex<float>* f, std::complex<float>* F, uint32_t N, uint32_t M);
		// April:
		static void dft2d(std::vector<std::vector<std::complex<double>>>& arr, bool inverse);
		static void dft1d(std::vector<std::complex<double>>& vec, bool inverse);
		static void transpose(std::vector<std::vector<std::complex<double>>>& arr);
		static void fshift(std::vector<std::complex<double>>& vec);
	};

}