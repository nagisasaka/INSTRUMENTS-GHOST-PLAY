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

//許容誤差
#define EPS (1e-10)
//2つのスカラーが等しいかどうか
#define EQ(a,b) (abs((a)-(b)) < EPS)
//2つのベクトルが等しいかどうか
#define EQV(a,b) ( EQ((a).real(), (b).real()) && EQ((a).imag(), (b).imag()) )

class vecmath
{
public:

	//ベクトルaの絶対値
	static double length(Point a){ return abs(a); }

	//2点間距離
	static double distance(Point a, Point b){ return abs(a-b); }

	//単位ベクトル化
	static Point unit(Point a){ return a / abs(a); }

	//ベクトルaの法線ベクトルを求める
	static void normal(Point a, Point* n1, Point* n2){ (*n1) = a * Point(0,+1); (*n2) = a * Point(0,-1); }

	//ベクトルaの単位法線ベクトルを求める
	static void unitnormal(Point a, Point* n1, Point* n2){ (*n1) = a * Point(0,+1) / abs(a); (*n2) = a * Point(0,-1) / abs(a); }

	// 内積 (dot product) : a・b = |a||b|cosΘ
	static double dot(Point a, Point b) {
		return (a.real() * b.real() + a.imag() * b.imag());
	}

	// 外積 (cross product) : a×b = |a||b|sinΘ
	static double cross(Point a, Point b) {
		return (a.real() * b.imag() - a.imag() * b.real());
	}

	// 2直線の直交判定 : a⊥b <=> dot(a, b) = 0
	static int is_orthogonal(Point a1, Point a2, Point b1, Point b2) {
		return EQ( dot(a1-a2, b1-b2), 0.0 );
	}

	// 2直線の平行判定 : a//b <=> cross(a, b) = 0
	static int is_parallel(Point a1, Point a2, Point b1, Point b2) {
		return EQ( cross(a1-a2, b1-b2), 0.0 );
	}

	// 点cが直線a,b上にあるかないか
	static int is_point_on_line(Point a, Point b, Point c) {
		return EQ( cross(b-a, c-a), 0.0 );
	}

	// 点cが線分a,b上にあるかないか
	static int is_point_on_linestrip(Point a, Point b, Point c) {
		return (abs(a-c) + abs(c-b) < abs(a-b) + EPS);
	}

	// 点a,bを通る直線と点cとの距離
	static double distance_l_p(Point a, Point b, Point c) {
		return abs(cross(b-a, c-a)) / abs(b-a);
	}

	// 点a,bを端点とする線分と点cとの距離
	static double distance_ls_p(Point a, Point b, Point c) {
		if ( dot(b-a, c-a) < EPS ) return abs(c-a);
		if ( dot(a-b, c-b) < EPS ) return abs(c-b);
		return abs(cross(b-a, c-a)) / abs(b-a);
	}

	// a1,a2を端点とする線分とb1,b2を端点とする線分の交差判定
	static int is_intersected_ls(Point a1, Point a2, Point b1, Point b2) {
		return ( cross(a2-a1, b1-a1) * cross(a2-a1, b2-a1) < EPS ) &&
			   ( cross(b2-b1, a1-b1) * cross(b2-b1, a2-b1) < EPS );
	}

	// a1,a2を端点とする線分とb1,b2を端点とする線分の交点計算
	static Point intersection_ls(Point a1, Point a2, Point b1, Point b2) {
		Point b = b2-b1;
		double d1 = abs(cross(b, a1-b1));
		double d2 = abs(cross(b, a2-b1));
		double t = d1 / (d1 + d2);

		return a1 + (a2-a1) * t;
	}

	// a1,a2を通る直線とb1,b2を通る直線の交差判定
	static int is_intersected_l(Point a1, Point a2, Point b1, Point b2) {
		return !EQ( cross(a1-a2, b1-b2), 0.0 );
	}

	// a1,a2を通る直線とb1,b2を通る直線の交点計算
	static Point intersection_l(Point a1, Point a2, Point b1, Point b2) {
		Point a = a2 - a1; Point b = b2 - b1;
		return a1 + a * cross(b, b1-a1) / cross(b, a);
	}
};

#endif