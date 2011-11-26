/*
 * Ghost Play - Autonomous Violin-Player Imitation Device
 * 
 * Copyright (C) 2009-2012  Masato Fujino <fujino@fairydevices.jp>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __AUDIO_ANALYZER_HPP__
#define __AUDIO_ANALYZER_HPP__

#include "../fftw-3.3/fftw3.h"
#pragma comment(lib,"fftw-3.3/libfftw3-3.lib")
#pragma comment(lib,"fftw-3.3/libfftw3f-3.lib")
#pragma comment(lib,"fftw-3.3/libfftw3l-3.lib")

#ifndef M_PI
#define M_PI 3.14159265358979323846   /* pi   */
#define M_PI_2 1.57079632679489661923 /* pi/2 */
#define M_PI_4 0.78539816339744830962 /* pi/4 */
#endif

#include <QVector>

#include <cmath>
#include <iostream>

class AudioAnalyzer
{
public:

	static double F0(QVector<double> v, int samplerate, int N);

	static QVector<double> CPS(double *v,int length, int N);

	static QVector<double> ALOG(double *v,int length, int N);

private:
	static void FFT(double *real, double *imag, int N)
	{
		fftw(real, imag, N, true);
	}
	
	static void IFFT(double *real, double *imag, int N)
	{
		fftw(real, imag, N, false);
	}

	static void fftw(double *real, double *imag, int N, bool flag)
	{
	    fftw_complex *in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*N);
		fftw_complex *out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*N);
		fftw_plan p;
		if(flag){
			p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	    }else{
			p = fftw_plan_dft_1d(N, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
		}
	    for(int i=0;i<N;i++){
			in[i][0] = real[i];
			in[i][1] = imag[i];
		}
	    fftw_execute(p);
		if(flag){
			for(int i=0;i<N;i++){
				real[i] = out[i][0];
		        imag[i] = out[i][1];
			}
	    }else{
			for(int i=0;i<N;i++){
				real[i] = out[i][0]/(double)N;
				imag[i] = out[i][1]/(double)N;
			}
		}
	    fftw_destroy_plan(p);
		fftw_free(in);
		fftw_free(out);
	}
	
	static void HanningWindow(double *w, int N)
	{
		int n;
		if (N % 2 == 0){
			for (n = 0; n < N; n++){
				w[n] = 0.5 - 0.5 * cos(2.0 * M_PI * n / N);
			}
		}else{
			for (n = 0; n < N; n++){
				w[n] = 0.5 - 0.5 * cos(2.0 * M_PI * (n + 0.5) / N);
			}
		}
	}

	static const bool __debug;
};

#endif