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

#include "audioAnalyzer.hpp"

const bool AudioAnalyzer::__debug = false;

/*!
 * \brief
 * ケフレンシー解析の結果からF0を返す
 * 
 * \param v
 * ケフレンシー解析結果
 * 
 * \param samplerate
 * サンプリングレート
 * 
 * \param N
 * 解析ウィンドウサイズ
 * 
 * \returns
 * F0周波数[Hz]
 * 
 */
double AudioAnalyzer::F0(QVector<double> v, int samplerate, int N)
{
	//最大値を取るだけです
	double maxCefrencyBin = 0;
	double max = 0;
	for(int i=10;i<N/2.-10;i++){
		if(max < qAbs(v[i])){
			max = qAbs(v[i]);
			//最大値を取ったときのケフレンシー
			maxCefrencyBin = i;
		}
	}

	//サンプリングレートに対してウィンドウサイズは何秒に相当？
	//double unit_sec = (double) N / (double) samplerate;
	
	//ケフレンシービンあたりの周波数は？
	//double unit_freq = unit_sec / (double) samplerate;

	//従って今回のケフレンシーピークの周波数は？
	//double peak_freq = unit_freq * maxCefrencyBin;


	//横軸最大値（今回の N は何秒に相当？）
	//samplerate サンプルで 1 秒，では N/2 サンプルでは？
	double cutsec   = ((double)N/2.) / (double) samplerate;        // [sec]
	double cursec   = cutsec * ( maxCefrencyBin / ((double)N/2.)); // [sec]
	double peak_freq = 1.0 / cursec; // [Hz]

	//ケフレンシーのフロアリング処理
	if(max < 0.05){ //実測値を見た
		peak_freq = 0;
	}
	//std::cout << max << std::endl;
	
	return peak_freq;
}


/*!
 * \brief
 * 対数振幅スペクトルを返す
 * 
 * \param v
 * 解析区間
 * 
 * \param length
 * 解析区間の長さ
 *
 * \param N
 * FFT解析長
 * 
 * \returns
 * 対数振幅スペクトル
 * 
 * \remarks
 * CPSもALOGも同一のFFT処理を行っているため、重い処理が非常に冗長になってしまっていることに注意
 * 
 */
QVector<double> AudioAnalyzer::ALOG(double *v, int length, int N)
{
	if(N%2 != 0){
		std::cerr << "[ FZERO ] FFTサイズが2の累乗ではないため結果が正しくない可能性があります" << std::endl;
	}
	if(length < N){
		std::cerr << "[ FZERO ] FFTサイズがサンプル長を上回っているため結果が正しくない可能性があります" << std::endl;
	}
		
	//// FFT解析用一時変数

	double *x_real = (double*)calloc(N, sizeof(double));  //実部
	double *x_imag = (double*)calloc(N, sizeof(double));  //虚部

	double *w      = (double*)calloc(N, sizeof(double));  //窓関数

	//// スペクトラム

	//振幅スペクトル
	double *A      = (double*)calloc(N, sizeof(double));
	//位相スペクトル
	double *T      = (double*)calloc(N, sizeof(double));
	//対数振幅スペクトル
	double *A_log  = (double*)calloc(N, sizeof(double));

	//対数パワースペクトルを返す
	QVector<double>   alog;
	for(int i=0;i<N;i++) alog.append(0); // 初期化

	//ハニング窓
	HanningWindow(w, N);
		
	//オーバーラップ
	int overwrap = N/2.;

	//サブフレーム数
	int number_of_frame = length / N;

	//各サブフレームについて
	for (int frame = 0; frame < number_of_frame; frame++){
		int offset = overwrap * frame;
			
		for (int n = 0; n < N; n++){
			x_real[n] = v[offset + n] * w[n];
			x_imag[n] = 0.0;
		}
		//FFT実行
		FFT(x_real, x_imag, N);

		//振幅スペクトルを求める
		for (int k = 0; k < N; k++){
			//振幅スペクトル
			A[k] = sqrt(x_real[k] * x_real[k] + x_imag[k] * x_imag[k]);
			//位相スペクトル
			T[k] = atan2(x_imag[k], x_real[k]);
			//対数振幅スペクトル
			A_log[k] = log10(A[k]);			

			//平均を求めるために蓄積
			//alog[k] += A_log[k];
			alog[k] += A[k];
		}		
	}

	//平均を求める
	for(int k=0;k<N;k++){
		alog[k] = alog[k] / (double) number_of_frame;
	}

	free(x_real);
	free(x_imag);
	free(w);
	free(A);
	free(T);
	free(A_log);

	return alog;
}


/*!
 * \brief
 * ケプストラム解析
 * 
 * \param v
 * 解析対象データ．[0,1]に正規化されていることが必要．
 * 
 * \param N
 * FFT/IFFT他ウィンドウサイズ
 * 
 * \returns
 * ケプストラム解析結果．含まれる各ウィンドウの平均値を返す．
 * 
 * \remarks
 * ・解析対象データの長さがウィンドウサイズより小さいのはダメですよ．変なときは標準エラーに警告を表示するだけ．
 * ・ハニング窓を使っていますよ．
 * 
 * \see
 * Separate items with the '|' character.
 */
QVector<double> AudioAnalyzer::CPS(double* v,int length, int N)
{
	if(N%2 != 0){
		std::cerr << "[ FZERO ] FFTサイズが2の累乗ではないため結果が正しくない可能性があります" << std::endl;
	}
	if(length < N){
		std::cerr << "[ FZERO ] FFTサイズがサンプル長を上回っているため結果が正しくない可能性があります" << std::endl;
	}
		
	//// FFT解析用一時変数

	double *x_real = (double*)calloc(N, sizeof(double));  //実部
	double *x_imag = (double*)calloc(N, sizeof(double));  //虚部

	double *y_real = (double*)calloc(N, sizeof(double));  //実部
	double *y_imag = (double*)calloc(N, sizeof(double));  //虚部

	double *w      = (double*)calloc(N, sizeof(double));  //窓関数

	//// スペクトラム

	//振幅スペクトル
	double *A      = (double*)calloc(N, sizeof(double));
	//位相スペクトル
	double *T      = (double*)calloc(N, sizeof(double));
	//パワースペクトル
	double *P      = (double*)calloc(N, sizeof(double));
	//対数振幅スペクトル
	double *A_log  = (double*)calloc(N, sizeof(double));
	//対数パワースペクトル
	double *P_log  = (double*)calloc(N, sizeof(double));

	//// ケプストラム

	double *cps      = (double*)calloc(N, sizeof(double));  //位相（周波数方向）
	
	//// 平均ケプストラム（戻り値）

	QVector<double>   cepstrum; 
	for(int i=0;i<N;i++) cepstrum.append(0); // 初期化

	//ハニング窓
	HanningWindow(w, N);
		
	//オーバーラップ
	int overwrap = N/2.;

	//サブフレーム数
	int number_of_frame = length / N;

	//各サブフレームについて
	for (int frame = 0; frame < number_of_frame; frame++){
		int offset = overwrap * frame;
			
		for (int n = 0; n < N; n++){
			x_real[n] = v[offset + n] * w[n];
			x_imag[n] = 0.0;
		}
		//FFT実行
		FFT(x_real, x_imag, N);

		//振幅スペクトルを求める
		for (int k = 0; k < N; k++){
			//振幅スペクトル
			A[k] = sqrt(x_real[k] * x_real[k] + x_imag[k] * x_imag[k]);
			//位相スペクトル
			T[k] = atan2(x_imag[k], x_real[k]);
			//パワースペクトル
			P[k] = A[k]*A[k];
			//対数振幅スペクトル
			A_log[k] = log10(A[k]);
			//対数パワースペクトル
			P_log[k] = log10(P[k]);				
			
			//ケプストラム解析の準備
			y_real[k] = A_log[k];
			y_imag[k] = 0;
		}
			
		//対数振幅スペクトルに対して逆フーリエ変換実行
		IFFT(y_real, y_imag, N);

		//ケプストラムを記録
		for (int k = 0; k < N; k++){
			cps[k] = y_real[k];
		}

		//平均を求めるために累積
		for(int k = 0; k < N; k++){
			cepstrum[k] += cps[k];
		}
	}

	//平均を求める
	for(int k=0;k<N;k++){
		cepstrum[k] = cepstrum[k] / (double) number_of_frame;
	}

	free(x_real);
	free(x_imag);
	free(y_real);
	free(y_imag);
	free(w);
	free(A);
	free(T);
	free(P);
	free(A_log);
	free(P_log);
	free(cps);

	return cepstrum;
}