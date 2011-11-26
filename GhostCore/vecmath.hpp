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

#ifndef __VECMATH_HPP__
#define __VECMATH_HPP__

#include <complex>
using namespace std;

typedef complex<double> Point;

//���e�덷
#define EPS (1e-10)
//2�̃X�J���[�����������ǂ���
#define EQ(a,b) (abs((a)-(b)) < EPS)
//2�̃x�N�g�������������ǂ���
#define EQV(a,b) ( EQ((a).real(), (b).real()) && EQ((a).imag(), (b).imag()) )

class vecmath
{
public:

	//�x�N�g��a�̐�Βl
	static double length(Point a){ return abs(a); }

	//2�_�ԋ���
	static double distance(Point a, Point b){ return abs(a-b); }

	//�P�ʃx�N�g����
	static Point unit(Point a){ return a / abs(a); }

	//�x�N�g��a�̖@���x�N�g�������߂�
	static void normal(Point a, Point* n1, Point* n2){ (*n1) = a * Point(0,+1); (*n2) = a * Point(0,-1); }

	//�x�N�g��a�̒P�ʖ@���x�N�g�������߂�
	static void unitnormal(Point a, Point* n1, Point* n2){ (*n1) = a * Point(0,+1) / abs(a); (*n2) = a * Point(0,-1) / abs(a); }

	// ���� (dot product) : a�Eb = |a||b|cos��
	static double dot(Point a, Point b) {
		return (a.real() * b.real() + a.imag() * b.imag());
	}

	// �O�� (cross product) : a�~b = |a||b|sin��
	static double cross(Point a, Point b) {
		return (a.real() * b.imag() - a.imag() * b.real());
	}

	// 2�����̒��𔻒� : a��b <=> dot(a, b) = 0
	static int is_orthogonal(Point a1, Point a2, Point b1, Point b2) {
		return EQ( dot(a1-a2, b1-b2), 0.0 );
	}

	// 2�����̕��s���� : a//b <=> cross(a, b) = 0
	static int is_parallel(Point a1, Point a2, Point b1, Point b2) {
		return EQ( cross(a1-a2, b1-b2), 0.0 );
	}

	// �_c������a,b��ɂ��邩�Ȃ���
	static int is_point_on_line(Point a, Point b, Point c) {
		return EQ( cross(b-a, c-a), 0.0 );
	}

	// �_c������a,b��ɂ��邩�Ȃ���
	static int is_point_on_linestrip(Point a, Point b, Point c) {
		return (abs(a-c) + abs(c-b) < abs(a-b) + EPS);
	}

	// �_a,b��ʂ钼���Ɠ_c�Ƃ̋���
	static double distance_l_p(Point a, Point b, Point c) {
		return abs(cross(b-a, c-a)) / abs(b-a);
	}

	// �_a,b��[�_�Ƃ�������Ɠ_c�Ƃ̋���
	static double distance_ls_p(Point a, Point b, Point c) {
		if ( dot(b-a, c-a) < EPS ) return abs(c-a);
		if ( dot(a-b, c-b) < EPS ) return abs(c-b);
		return abs(cross(b-a, c-a)) / abs(b-a);
	}

	// a1,a2��[�_�Ƃ��������b1,b2��[�_�Ƃ�������̌�������
	static int is_intersected_ls(Point a1, Point a2, Point b1, Point b2) {
		return ( cross(a2-a1, b1-a1) * cross(a2-a1, b2-a1) < EPS ) &&
			   ( cross(b2-b1, a1-b1) * cross(b2-b1, a2-b1) < EPS );
	}

	// a1,a2��[�_�Ƃ��������b1,b2��[�_�Ƃ�������̌�_�v�Z
	static Point intersection_ls(Point a1, Point a2, Point b1, Point b2) {
		Point b = b2-b1;
		double d1 = abs(cross(b, a1-b1));
		double d2 = abs(cross(b, a2-b1));
		double t = d1 / (d1 + d2);

		return a1 + (a2-a1) * t;
	}

	// a1,a2��ʂ钼����b1,b2��ʂ钼���̌�������
	static int is_intersected_l(Point a1, Point a2, Point b1, Point b2) {
		return !EQ( cross(a1-a2, b1-b2), 0.0 );
	}

	// a1,a2��ʂ钼����b1,b2��ʂ钼���̌�_�v�Z
	static Point intersection_l(Point a1, Point a2, Point b1, Point b2) {
		Point a = a2 - a1; Point b = b2 - b1;
		return a1 + a * cross(b, b1-a1) / cross(b, a);
	}
};

#endif