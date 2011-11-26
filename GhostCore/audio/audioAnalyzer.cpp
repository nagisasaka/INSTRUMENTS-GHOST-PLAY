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
 * �P�t�����V�[��͂̌��ʂ���F0��Ԃ�
 * 
 * \param v
 * �P�t�����V�[��͌���
 * 
 * \param samplerate
 * �T���v�����O���[�g
 * 
 * \param N
 * ��̓E�B���h�E�T�C�Y
 * 
 * \returns
 * F0���g��[Hz]
 * 
 */
double AudioAnalyzer::F0(QVector<double> v, int samplerate, int N)
{
	//�ő�l����邾���ł�
	double maxCefrencyBin = 0;
	double max = 0;
	for(int i=10;i<N/2.-10;i++){
		if(max < qAbs(v[i])){
			max = qAbs(v[i]);
			//�ő�l��������Ƃ��̃P�t�����V�[
			maxCefrencyBin = i;
		}
	}

	//�T���v�����O���[�g�ɑ΂��ăE�B���h�E�T�C�Y�͉��b�ɑ����H
	//double unit_sec = (double) N / (double) samplerate;
	
	//�P�t�����V�[�r��������̎��g���́H
	//double unit_freq = unit_sec / (double) samplerate;

	//�]���č���̃P�t�����V�[�s�[�N�̎��g���́H
	//double peak_freq = unit_freq * maxCefrencyBin;


	//�����ő�l�i����� N �͉��b�ɑ����H�j
	//samplerate �T���v���� 1 �b�C�ł� N/2 �T���v���ł́H
	double cutsec   = ((double)N/2.) / (double) samplerate;        // [sec]
	double cursec   = cutsec * ( maxCefrencyBin / ((double)N/2.)); // [sec]
	double peak_freq = 1.0 / cursec; // [Hz]

	//�P�t�����V�[�̃t���A�����O����
	if(max < 0.05){ //�����l������
		peak_freq = 0;
	}
	//std::cout << max << std::endl;
	
	return peak_freq;
}


/*!
 * \brief
 * �ΐ��U���X�y�N�g����Ԃ�
 * 
 * \param v
 * ��͋��
 * 
 * \param length
 * ��͋�Ԃ̒���
 *
 * \param N
 * FFT��͒�
 * 
 * \returns
 * �ΐ��U���X�y�N�g��
 * 
 * \remarks
 * CPS��ALOG�������FFT�������s���Ă��邽�߁A�d�����������ɏ璷�ɂȂ��Ă��܂��Ă��邱�Ƃɒ���
 * 
 */
QVector<double> AudioAnalyzer::ALOG(double *v, int length, int N)
{
	if(N%2 != 0){
		std::cerr << "[ FZERO ] FFT�T�C�Y��2�̗ݏ�ł͂Ȃ����ߌ��ʂ��������Ȃ��\��������܂�" << std::endl;
	}
	if(length < N){
		std::cerr << "[ FZERO ] FFT�T�C�Y���T���v�����������Ă��邽�ߌ��ʂ��������Ȃ��\��������܂�" << std::endl;
	}
		
	//// FFT��͗p�ꎞ�ϐ�

	double *x_real = (double*)calloc(N, sizeof(double));  //����
	double *x_imag = (double*)calloc(N, sizeof(double));  //����

	double *w      = (double*)calloc(N, sizeof(double));  //���֐�

	//// �X�y�N�g����

	//�U���X�y�N�g��
	double *A      = (double*)calloc(N, sizeof(double));
	//�ʑ��X�y�N�g��
	double *T      = (double*)calloc(N, sizeof(double));
	//�ΐ��U���X�y�N�g��
	double *A_log  = (double*)calloc(N, sizeof(double));

	//�ΐ��p���[�X�y�N�g����Ԃ�
	QVector<double>   alog;
	for(int i=0;i<N;i++) alog.append(0); // ������

	//�n�j���O��
	HanningWindow(w, N);
		
	//�I�[�o�[���b�v
	int overwrap = N/2.;

	//�T�u�t���[����
	int number_of_frame = length / N;

	//�e�T�u�t���[���ɂ���
	for (int frame = 0; frame < number_of_frame; frame++){
		int offset = overwrap * frame;
			
		for (int n = 0; n < N; n++){
			x_real[n] = v[offset + n] * w[n];
			x_imag[n] = 0.0;
		}
		//FFT���s
		FFT(x_real, x_imag, N);

		//�U���X�y�N�g�������߂�
		for (int k = 0; k < N; k++){
			//�U���X�y�N�g��
			A[k] = sqrt(x_real[k] * x_real[k] + x_imag[k] * x_imag[k]);
			//�ʑ��X�y�N�g��
			T[k] = atan2(x_imag[k], x_real[k]);
			//�ΐ��U���X�y�N�g��
			A_log[k] = log10(A[k]);			

			//���ς����߂邽�߂ɒ~��
			//alog[k] += A_log[k];
			alog[k] += A[k];
		}		
	}

	//���ς����߂�
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
 * �P�v�X�g�������
 * 
 * \param v
 * ��͑Ώۃf�[�^�D[0,1]�ɐ��K������Ă��邱�Ƃ��K�v�D
 * 
 * \param N
 * FFT/IFFT���E�B���h�E�T�C�Y
 * 
 * \returns
 * �P�v�X�g������͌��ʁD�܂܂��e�E�B���h�E�̕��ϒl��Ԃ��D
 * 
 * \remarks
 * �E��͑Ώۃf�[�^�̒������E�B���h�E�T�C�Y��菬�����̂̓_���ł���D�ςȂƂ��͕W���G���[�Ɍx����\�����邾���D
 * �E�n�j���O�����g���Ă��܂���D
 * 
 * \see
 * Separate items with the '|' character.
 */
QVector<double> AudioAnalyzer::CPS(double* v,int length, int N)
{
	if(N%2 != 0){
		std::cerr << "[ FZERO ] FFT�T�C�Y��2�̗ݏ�ł͂Ȃ����ߌ��ʂ��������Ȃ��\��������܂�" << std::endl;
	}
	if(length < N){
		std::cerr << "[ FZERO ] FFT�T�C�Y���T���v�����������Ă��邽�ߌ��ʂ��������Ȃ��\��������܂�" << std::endl;
	}
		
	//// FFT��͗p�ꎞ�ϐ�

	double *x_real = (double*)calloc(N, sizeof(double));  //����
	double *x_imag = (double*)calloc(N, sizeof(double));  //����

	double *y_real = (double*)calloc(N, sizeof(double));  //����
	double *y_imag = (double*)calloc(N, sizeof(double));  //����

	double *w      = (double*)calloc(N, sizeof(double));  //���֐�

	//// �X�y�N�g����

	//�U���X�y�N�g��
	double *A      = (double*)calloc(N, sizeof(double));
	//�ʑ��X�y�N�g��
	double *T      = (double*)calloc(N, sizeof(double));
	//�p���[�X�y�N�g��
	double *P      = (double*)calloc(N, sizeof(double));
	//�ΐ��U���X�y�N�g��
	double *A_log  = (double*)calloc(N, sizeof(double));
	//�ΐ��p���[�X�y�N�g��
	double *P_log  = (double*)calloc(N, sizeof(double));

	//// �P�v�X�g����

	double *cps      = (double*)calloc(N, sizeof(double));  //�ʑ��i���g�������j
	
	//// ���σP�v�X�g�����i�߂�l�j

	QVector<double>   cepstrum; 
	for(int i=0;i<N;i++) cepstrum.append(0); // ������

	//�n�j���O��
	HanningWindow(w, N);
		
	//�I�[�o�[���b�v
	int overwrap = N/2.;

	//�T�u�t���[����
	int number_of_frame = length / N;

	//�e�T�u�t���[���ɂ���
	for (int frame = 0; frame < number_of_frame; frame++){
		int offset = overwrap * frame;
			
		for (int n = 0; n < N; n++){
			x_real[n] = v[offset + n] * w[n];
			x_imag[n] = 0.0;
		}
		//FFT���s
		FFT(x_real, x_imag, N);

		//�U���X�y�N�g�������߂�
		for (int k = 0; k < N; k++){
			//�U���X�y�N�g��
			A[k] = sqrt(x_real[k] * x_real[k] + x_imag[k] * x_imag[k]);
			//�ʑ��X�y�N�g��
			T[k] = atan2(x_imag[k], x_real[k]);
			//�p���[�X�y�N�g��
			P[k] = A[k]*A[k];
			//�ΐ��U���X�y�N�g��
			A_log[k] = log10(A[k]);
			//�ΐ��p���[�X�y�N�g��
			P_log[k] = log10(P[k]);				
			
			//�P�v�X�g������͂̏���
			y_real[k] = A_log[k];
			y_imag[k] = 0;
		}
			
		//�ΐ��U���X�y�N�g���ɑ΂��ċt�t�[���G�ϊ����s
		IFFT(y_real, y_imag, N);

		//�P�v�X�g�������L�^
		for (int k = 0; k < N; k++){
			cps[k] = y_real[k];
		}

		//���ς����߂邽�߂ɗݐ�
		for(int k = 0; k < N; k++){
			cepstrum[k] += cps[k];
		}
	}

	//���ς����߂�
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