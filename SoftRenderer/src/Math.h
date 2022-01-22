#pragma once
#include<initializer_list>
#include<assert.h>


#define Bit(x) (1<<x)

namespace TinyMath
{	
	//----------------------------------------------------------------
	//vector defination
	//----------------------------------------------------------------
	template<size_t N, typename T>
	struct Vector
	{
		T m_[N];
		inline Vector() { for (size_t i = 0; i < N; i++) m_[i] = T(); }
		inline Vector(const T* ptr) { for (size_t i = 0; i < N; i++)m_[i] = ptr[i]; }
		inline Vector(const Vector<N, T>& v) { for (size_t i = 0; i < N; i++)m_[i] = v.m_[i]; }
		inline Vector(const std::initializer_list<T>& list) { auto iter = list.begin(); for (size_t i = 0; i < N; i++)  m_[i] = *iter++; }
		inline const T& operator[](size_t i)const { assert(i < N); return m_[i]; }
		inline T& operator[](size_t i) { assert(i < N); return m_[i]; }
	};

	//2维向量 偏特化
	template<typename T>
	struct Vector<2, T>
	{
		union
		{
			struct { T x_, y_; };
			struct { T u_, v_; };
			T m_[2];
		};
		inline Vector() :x_(T()), y_(T()) {}
		inline Vector(T x, T y) : x_(x), y_(y) {}
		inline Vector(const Vector<2, T>& v) : x_(v.x_), y_(v.y_) {}
		inline Vector(const T* ptr) : x_(ptr[0]), y_(ptr[1]) {}
		inline const T& operator[](size_t i)const { assert(i < 2); return m_[i]; }
		inline T& operator[](size_t i) { assert(i < 2); return m_[i]; }

	};


	//3维向量 偏特化
	template<typename T>
	struct Vector<3, T>
	{
		union
		{
			struct { T x_, y_, z_; };
			struct { T r_, g_, b_; };
			T m_[3];
		};
		inline Vector() :x_(T()), y_(T()), z_(T()) {}
		inline Vector(T x, T y, T z) : x_(x), y_(y), z_(z) {}
		inline Vector(const Vector<2, T>& v, T z) : x(v.x_), y_(v.y_), z_(z) {}
		inline Vector(const Vector<3, T>& v) : x_(v.x_), y_(v.y_), z_(v.z_) {}
		inline Vector(const T* ptr) : x_(ptr[0]), y_(ptr[1]), z_(ptr[2]) {}
		inline const T& operator[](size_t i)const { assert(i < 3); return m_[i]; }
		inline T& operator[](size_t i) { assert(i < 3); return m_[i]; }
		inline operator Vector<2, T>()const { return Vector<2, T>(x_, y_); }
	};



	//4维向量 偏特化
	template<typename T>
	struct Vector<4, T>
	{
		union
		{
			struct { T x_, y_, z_, w_; };
			struct { T r_, g_, b_, a_; };
			T m_[4];
		};
		inline Vector() :x_(T()), y_(T()), z_(T()), w_(T()) {}
		inline Vector(T x, T y, T z, T w) : x_(x), y_(y), z_(z), w_(w) {}
		inline Vector(const Vector<3, T>& v, T w) : x_(v.x_), y_(v.y_), z_(v.z_), w_(w) {}
		inline Vector(const Vector<4, T>& v) : x_(v.x_), y_(v.y_), z_(v.z_), w_(v.w_) {}
		inline Vector(const T* ptr) : x_(ptr[0]), y_(ptr[1]), z_(ptr[2]), w_(ptr[3]) {}
		inline const T& operator[](size_t i)const { assert(i < 4); return m_[i]; }
		inline T& operator[](size_t i) { assert(i < 4); return m_[i]; }

		inline operator Vector<2, T>()const { return Vector<2, T>(x_, y_); }
		inline operator Vector<3, T>()const { return Vector<3, T>(x_, y_, w_); }

	};


	//----------------------------------------------------------------
	//vector operation
	//----------------------------------------------------------------
	// = (+a)
	template <size_t N, typename T>
	inline Vector<N, T> operator + (const Vector<N, T>& a) {
		return a;
	}

	// = (-a)

	template <size_t N, typename T>
	inline Vector<N, T> operator - (const Vector<N, T>& a) {
		Vector<N, T> b;
		for (size_t i = 0; i < N; i++) b[i] = -a[i];
		return b;
	}

	// = (a == b) ? true : false
	template <size_t N, typename T>
	inline bool operator == (const Vector<N, T>& a, const Vector<N, T>& b) {
		for (size_t i = 0; i < N; i++) if (a[i] != b[i]) return false;
		return true;
	}

	// = (a != b)? true : false
	template <size_t N, typename T>
	inline bool operator != (const Vector<N, T>& a, const Vector<N, T>& b) {
		return !(a == b);
	}

	// = a + b
	template <size_t N, typename T>
	inline Vector<N, T> operator + (const Vector<N, T>& a, const Vector<N, T>& b) {
		Vector<N, T> c;
		for (size_t i = 0; i < N; i++) c[i] = a[i] + b[i];
		return c;
	}

	// = a - b
	template <size_t N, typename T>
	inline Vector<N, T> operator - (const Vector<N, T>& a, const Vector<N, T>& b) {
		Vector<N, T> c;
		for (size_t i = 0; i < N; i++) c[i] = a[i] - b[i];
		return c;
	}

	// = a * b，不是点乘也不是叉乘，而是各个元素分别相乘，色彩计算时有用
	template <size_t N, typename T>
	inline Vector<N, T> operator * (const Vector<N, T>& a, const Vector<N, T>& b) {
		Vector<N, T> c;
		for (size_t i = 0; i < N; i++) c[i] = a[i] * b[i];
		return c;
	}

	// = a / b，各个元素相除
	template <size_t N, typename T>
	inline Vector<N, T> operator / (const Vector<N, T>& a, const Vector<N, T>& b) {
		Vector<N, T> c;
		for (size_t i = 0; i < N; i++) c[i] = a[i] / b[i];
		return c;
	}

	// = a * x
	template <size_t N, typename T>
	inline Vector<N, T> operator * (const Vector<N, T>& a, T x) {
		Vector<N, T> b;
		for (size_t i = 0; i < N; i++) b[i] = a[i] * x;
		return b;
	}

	// = x * a
	template <size_t N, typename T>
	inline Vector<N, T> operator * (T x, const Vector<N, T>& a) {
		Vector<N, T> b;
		for (size_t i = 0; i < N; i++) b[i] = a[i] * x;
		return b;
	}

	// = a / x
	template <size_t N, typename T>
	inline Vector<N, T> operator / (const Vector<N, T>& a, T x) {
		Vector<N, T> b;
		for (size_t i = 0; i < N; i++) b[i] = a[i] / x;
		return b;
	}

	// = x / a
	template <size_t N, typename T>
	inline Vector<N, T> operator / (T x, const Vector<N, T>& a) {
		Vector<N, T> b;
		for (size_t i = 0; i < N; i++) b[i] = x / a[i];
		return b;
	}

	// a += b
	template <size_t N, typename T>
	inline Vector<N, T>& operator += (Vector<N, T>& a, const Vector<N, T>& b) {
		for (size_t i = 0; i < N; i++) a[i] += b[i];
		return a;
	}

	// a -= b
	template <size_t N, typename T>
	inline Vector<N, T>& operator -= (Vector<N, T>& a, const Vector<N, T>& b) {
		for (size_t i = 0; i < N; i++) a[i] -= b[i];
		return a;
	}

	// a *= b
	template <size_t N, typename T>
	inline Vector<N, T>& operator *= (Vector<N, T>& a, const Vector<N, T>& b) {
		for (size_t i = 0; i < N; i++) a[i] *= b[i];
		return a;
	}

	// a /= b
	template <size_t N, typename T>
	inline Vector<N, T>& operator /= (Vector<N, T>& a, const Vector<N, T>& b) {
		for (size_t i = 0; i < N; i++) a[i] /= b[i];
		return a;
	}

	// a *= x
	template <size_t N, typename T>
	inline Vector<N, T>& operator *= (Vector<N, T>& a, T x) {
		for (size_t i = 0; i < N; i++) a[i] *= x;
		return a;
	}

	// a /= x
	template <size_t N, typename T>
	inline Vector<N, T>& operator /= (Vector<N, T>& a, T x) {
		for (size_t i = 0; i < N; i++) a[i] /= x;
		return a;
	}


	typedef Vector<2, float> Vec2f;
	typedef Vector<3, float> Vec3f;
	typedef Vector<4, float> Vec4f;

	typedef Vector<2, double> Vec2d;
	typedef Vector<3, double> Vec3d;
	typedef Vector<4, double> Vec4d;
		 
	typedef Vector<2, int> Vec2i;
	typedef Vector<3, int> Vec3i;
	typedef Vector<4, int> Vec4i;
}
