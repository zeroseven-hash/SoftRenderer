#pragma once
#include<initializer_list>
#include<cmath>
#include<assert.h>
#include<stdint.h>

#define BIT(x) (1<<x)


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

		inline T Cross(const Vector<2, T>& other)const
		{
			return x_*other.y_-y_*other.x_;
		}
		
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
		inline Vector(const Vector<2, T>& v, T z) : x_(v.x_), y_(v.y_), z_(z) {}
		inline Vector(const Vector<3, T>& v) : x_(v.x_), y_(v.y_), z_(v.z_) {}
		inline Vector(const T* ptr) : x_(ptr[0]), y_(ptr[1]), z_(ptr[2]) {}

		inline Vector<3, T> Cross(const Vector<3, T>& other)const
		{
			return Vector<3, T>(y_ * other.z_ - z_ * other.y_, z_ * other.x_ - x_ * other.z_, x_ * other.y_ - y_ * other.x_);
		}

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
		
		inline Vector(const Vector<3, T>& v, T w=1.0f) : x_(v.x_), y_(v.y_), z_(v.z_), w_(w) {}
		inline Vector(const Vector<4, T>& v) : x_(v.x_), y_(v.y_), z_(v.z_), w_(v.w_) {}
		inline Vector(const T* ptr) : x_(ptr[0]), y_(ptr[1]), z_(ptr[2]), w_(ptr[3]) {}

		inline Vector<4, T> Cross(const Vector<4, T>& other)const
		{
			return Vector<4, T>(y_ * other.z_ - z_ * other.y_, z_ * other.x_ - x_ * other.z_, x_ * other.y_ - y_ * other.x_, w_);
		}


		inline const T& operator[](size_t i)const { assert(i < 4); return m_[i]; }
		inline T& operator[](size_t i) { assert(i < 4); return m_[i]; }

		
		inline operator Vector<2, T>()const { return Vector<2, T>(x_, y_); }
		inline operator Vector<3, T>()const { return Vector<3, T>(x_, y_, z_); }
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

	typedef Vector<4, uint8_t> Vec4u8i;
	

	//vector function
	// = |a| ^ 2
	template<size_t N, typename T>
	inline T VectorLengthSquare(const Vector<N, T>& a) {
		T sum = 0;
		for (size_t i = 0; i < N; i++) sum += a[i] * a[i];
		return sum;
	}

	// = |a|
	template<size_t N, typename T>
	inline T VectorLength(const Vector<N, T>& a) {
		return sqrt(VectorLengthSquare(a));
	}

	// = |a| , 特化 float 类型，使用 sqrtf
	template<size_t N>
	inline float VectorLength(const Vector<N, float>& a) {
		return sqrtf(VectorLengthSquare(a));
	}

	// = a / |a|
	template<size_t N, typename T>
	inline Vector<N, T> Normalize(const Vector<N, T>& a) {
		return a / VectorLength(a);
	}

	// dot
	template<size_t N, typename T>
	inline T VectorDot(const Vector<N, T>& a, const Vector<N, T>b)
	{
		T sum = 0;
		for (size_t i = 0; i < N; i++) sum += a[i] * b[i];
		return sum;
	}

	///Utils function
	template<typename T>
	inline T Between(T xmin, T xmax, T x)
	{
		return std::min<T>(std::max<T>(xmin, x), xmax);
	}

	template<typename T>
	inline T LinerInterpolation(const T& v1, const T& v2, float t)
	{
		return static_cast<T>(v1 + (v2 - v1) * t);
	}

	template<size_t N,typename T>
	inline  Vector<N, T> LinerInterpolation(const Vector<N, T>& v1, const Vector<N, T>& v2, float t)
	{
		Vector<N,T> res;
		for (int i = 0;i < N;i++)
		{
			res.m_[i] = static_cast<T>(v1.m_[i] + (v2.m_[i] - v1.m_[i]) * t);
		}
		return res;
	}

	template<size_t N, typename T>
	inline  Vector<N, T> Mix(const Vector<N, T>& v1, const Vector<N, T>& v2, float a)
	{
		return (1 - a) * v1 + a * v2;
	}


	template<size_t N,typename T>
	inline Vector<N, T> Pow(const Vector<N,T>& v,float pow)
	{
		Vector<N, T> res;
		for (int i = 0; i < N; i++)
		{
			res.m_[i] = std::pow(v.m_[i], pow);
		}
		return res;
	}
	template<size_t N, typename T>
	inline Vector<N, T> Exp(const Vector<N, T>& v)
	{
		Vector<N, T> res;
		for (int i = 0; i < N; i++)
		{
			res.m_[i] = std::exp(v.m_[i]);
		}
		return res;
	}

	// 判断一条边是不是三角形的左上边 (Top-Left Edge)
	inline bool IsTopLeft(const Vec2i& a, const Vec2i& b)
	{
		return ((a.y_ == b.y_) && (a.x_ < b.x_)) || (a.y_ > b.y_);
	}

	inline Vec4f TransformToVec4(const Vector<4, uint8_t>& col)
	{
		Vec4f res;
		res.r_ = TinyMath::Between(0.0f, 1.0f, (float)col.r_ / 255);
		res.g_ = TinyMath::Between(0.0f, 1.0f, (float)col.g_ / 255);
		res.b_ = TinyMath::Between(0.0f, 1.0f, (float)col.b_ / 255);
		res.a_ = TinyMath::Between(0.0f, 1.0f, (float)col.a_ / 255);
		return res;
	}
	inline Vector<4,uint8_t> TransformToColor(const Vec4f& col)
	{
		Vector<4, uint8_t> color;
		color.r_ = (uint8_t)TinyMath::Between(0, 255, (int)(col.r_ * 255));
		color.g_ = (uint8_t)TinyMath::Between(0, 255, (int)(col.g_ * 255));
		color.b_ = (uint8_t)TinyMath::Between(0, 255, (int)(col.b_ * 255));
		color.a_ = (uint8_t)TinyMath::Between(0, 255, (int)(col.a_ * 255));
		return color;
	}





	//----------------------------------------------------------------
	//matrix defination
	//----------------------------------------------------------------

	template<size_t ROW, size_t COL, typename T>
	struct Matrix
	{
		T m_[ROW][COL];
		inline Matrix() {}

		inline Matrix(const Matrix<ROW, COL, T>& other)
		{
			for (size_t r = 0;r < ROW;r++)for (size_t c = 0;c < COL;c++)
				m_[r][c] = other.m_[r][c];
		}


		inline Matrix(const std::initializer_list<Vector<COL, T>>& lists)
		{
			auto iter = lists.begin();
			for (size_t i = 0;i < ROW;i++) SetRow(i, *iter++);
		}

		inline const T* operator[](size_t row)const { assert(row < ROW); return m_[row]; }
		inline T* operator[](size_t row) { assert(row < ROW); return m_[row]; }

		inline Vector<COL,T> Row(size_t row)const
		{
			assert(row < ROW);
			Vector<COL, T> res;
			for (size_t i = 0;i < COL;i++)res[i] = m_[row][i];
			return res;
		}

		inline Vector<ROW,T> Col(size_t col)const 
		{
			assert(col < COL);
			Vector<ROW, T> a;
			for (size_t i = 0; i < ROW; i++) a[i] = m_[i][col];
			return a;
		}

		inline void SetRow(size_t row, const Vector<COL, T>& a)
		{
			assert(row < ROW);
			for (size_t i = 0;i < COL;i++)m_[row][i] = a[i];
		}

		inline void SetCol(size_t col, const Vector<ROW, T>& a)
		{
			assert(col < COL);
			for (size_t i = 0; i < ROW; i++) m_[i][col] = a[i];
		}

		inline Matrix<ROW - 1, COL - 1, T>
		GetMinor(size_t row, size_t col)const
		{
			Matrix<ROW - 1, COL - 1, T> ret;
			for (size_t r = 0;r < ROW - 1;r++)for (size_t c = 0;c < COL - 1;c++)
			{
				ret.m_[r][c] = m_[r < row ? r : r + 1][c < col ? c : c + 1];
			}
			return ret;
		}

		inline Matrix<COL, ROW, T> 
		Transpose()const
		{
			Matrix<COL, ROW, T>ret;
			for (size_t r = 0;r < ROW;r++)for (size_t c = 0;c < COL;c++)
				ret.m_[c][r] = m_[r][c];
			return ret;
		}

		inline static Matrix<ROW, COL, T>
		GetZero()
		{
			Matrix<ROW, COL, T> ret;
			for (size_t r = 0;r < ROW;r++)for (size_t c = 0;c < COL;c++)
				ret.m_[r][c] = 0;
			return ret;
		}

		inline static Matrix<ROW, COL, T>
		GetIdentity()
		{
			Matrix<ROW, COL, T> ret;
			for (size_t r = 0;r < ROW;r++)for (size_t c = 0;c < COL;c++)
				ret.m_[r][c] = (r == c) ? static_cast<T>(1) : static_cast<T>(0);
			return ret;
		}

	};

	
	

	//----------------------------------------------------------------------
	//数学库：矩阵运算
	//----------------------------------------------------------------------
	template<size_t ROW, size_t COL, typename T>
	inline bool operator ==(const Matrix<ROW, COL, T>& a, const Matrix<ROW, COL, T>& b)
	{
		for (size_t r = 0;r < ROW;r++)for (size_t c = 0;c < COL;c++)
			if (a.m_[r][c] != b.m_[r][c])return false;

		return true;
	}

	template<size_t ROW, size_t COL, typename T>
	inline bool operator!=(const Matrix<ROW, COL, T>& a, const Matrix<ROW, COL, T>& b)
	{
		return !(a == b);
	}

	template<size_t ROW, size_t COL, typename T>
	inline Matrix<ROW, COL, T> operator - (const Matrix<ROW, COL, T>& src) {
		Matrix<ROW, COL, T> out;
		for (size_t j = 0; j < ROW; j++) {
			for (size_t i = 0; i < COL; i++)
				out.m_[j][i] = -src.m_[j][i];
		}
		return out;
	}

	template<size_t ROW, size_t COL, typename T>
	inline Matrix<ROW, COL, T> operator + (const Matrix<ROW, COL, T>& a, const Matrix<ROW, COL, T>& b) {
		Matrix<ROW, COL, T> out;
		for (size_t j = 0; j < ROW; j++) {
			for (size_t i = 0; i < COL; i++)
				out.m_[j][i] = a.m_[j][i] + b.m_[j][i];
		}
		return out;
	}

	template<size_t ROW, size_t COL, typename T>
	inline Matrix<ROW, COL, T> operator - (const Matrix<ROW, COL, T>& a, const Matrix<ROW, COL, T>& b) {
		Matrix<ROW, COL, T> out;
		for (size_t j = 0; j < ROW; j++) {
			for (size_t i = 0; i < COL; i++)
				out.m_[j][i] = a.m_[j][i] - b.m_[j][i];
		}
		return out;
	}

	template<size_t ROW, size_t COL, size_t NEWCOL, typename T>
	inline Matrix<ROW, NEWCOL, T> operator * (const Matrix<ROW, COL, T>& a, const Matrix<COL, NEWCOL, T>& b) {
		Matrix<ROW, NEWCOL, T> out;
		for (size_t j = 0; j < ROW; j++) {
			for (size_t i = 0; i < NEWCOL; i++) {
				out.m_[j][i] = VectorDot(a.Row(j), b.Col(i));
			}
		}
		return out;
	}

	template<size_t ROW, size_t COL, typename T>
	inline Matrix<ROW, COL, T> operator * (const Matrix<ROW, COL, T>& a, T x) {
		Matrix<ROW, COL, T> out;
		for (size_t j = 0; j < ROW; j++) {
			for (size_t i = 0; i < COL; i++) {
				out.m_[j][i] = a.m_[j][i] * x;
			}
		}
		return out;
	}

	template<size_t ROW, size_t COL, typename T>
	inline Matrix<ROW, COL, T> operator / (const Matrix<ROW, COL, T>& a, T x) {
		Matrix<ROW, COL, T> out;
		for (size_t j = 0; j < ROW; j++) {
			for (size_t i = 0; i < COL; i++) {
				out.m_[j][i] = a.m_[j][i] / x;
			}
		}
		return out;
	}

	template<size_t ROW, size_t COL, typename T>
	inline Matrix<ROW, COL, T> operator * (T x, const Matrix<ROW, COL, T>& a) {
		return (a * x);
	}

	template<size_t ROW, size_t COL, typename T>
	inline Matrix<ROW, COL, T> operator / (T x, const Matrix<ROW, COL, T>& a) {
		Matrix<ROW, COL, T> out;
		for (size_t j = 0; j < ROW; j++) {
			for (size_t i = 0; i < COL; i++) {
				out.m_[j][i] = x / a.m_[j][i];
			}
		}
		return out;
	}

	template<size_t ROW, size_t COL, typename T>
	inline Vector<COL, T> operator * (const Vector<ROW, T>& a, const Matrix<ROW, COL, T>& m) {
		Vector<COL, T> b;
		for (size_t i = 0; i < COL; i++)
			b[i] = vector_dot(a, m.Col(i));
		return b;
	}

	template<size_t ROW, size_t COL, typename T>
	inline Vector<ROW, T> operator * (const Matrix<ROW, COL, T>& m, const Vector<COL, T>& a) {
		Vector<ROW, T> b;
		
		for (size_t i = 0; i < ROW; i++)
		{
			T sum = static_cast<T>(0);
			for (size_t j = 0; j < COL; j++) sum += a[j] * m[i][j];
			b[i] = sum;
		}
		return b;
	}

	typedef Matrix<2, 2, float> Mat2f;
	typedef Matrix<3, 3, float> Mat3f;
	typedef Matrix<4, 4, float> Mat4f;


	//空间运算

	template<typename T>
	inline Matrix<4, 4, T> Tranlate(const Matrix<4, 4, T>& m, const Vector<3, T>& v)
	{
		Matrix<4, 4, T> res(m);
		res[0][3] = res[0][0] * v[0] + res[0][1] * v[1] + res[0][2] * v[2] + res[0][3];
		res[1][3] = res[1][0] * v[0] + res[1][1] * v[1] + res[1][2] * v[2] + res[1][3];
		res[2][3] = res[2][0] * v[0] + res[2][1] * v[1] + res[2][2] * v[2] + res[2][3];
		return res;
	}

	template<typename T>
	inline Matrix<4, 4, T> Scale(const Matrix<4, 4, T>& m, const Vector<3, T>& v)
	{
		Matrix<4, 4, T> res(m);
		for (int i = 0;i < 4;i++)
		{
			res[0][i] *= v[0];
			res[1][i] *= v[1];
			res[2][i] *= v[2];
		}
		return res;
	}
	
	/*
	*** m:变换举证
	*** angle:弧度制
	*** v：旋转轴
 	*/
	template<typename T>
	inline Matrix<4, 4, T> Rotate(const Matrix<4, 4, T>& m,T angle, const Vector<3, T>& v)
	{
		//fomula: 													(0, -v[2], v[1])
		//	R(n,angle)=cos(angle)*I+(1-cos(angle))vvT+sin(angle)	(v[2], 0, -v[0])
		//															(-v[1],v[0] ,0 )
		angle = angle / 180 * 3.1415926;
		T cosa = (T)std::cos(angle);
		T sina = (T)std::sin(angle);

		Vector<3, T> n = Normalize(v);
		Vector<3, T> temp((T(1) - cosa) * n);

		Matrix<4,4,T> rotate;
		rotate[0][0] = cosa + temp[0] * n[0];
		rotate[1][0] = temp[0] * n[1] + sina * n[2];
		rotate[2][0] = temp[0] * n[2] - sina * n[1];
		rotate[3][0] = 0;
		
		rotate[0][1] = temp[1] * n[0] - sina * n[2];
		rotate[1][1] = cosa + temp[1] * n[1];
		rotate[2][1] = temp[1] * n[2] + sina * n[0];
		rotate[3][1] = 0;
		
		rotate[0][2] = temp[2] * n[0] + sina * n[1];
		rotate[1][2] = temp[2] * n[1] - sina * n[0];
		rotate[2][2] = cosa + temp[2] * n[2];
		rotate[3][2] = 0;

		rotate[0][3] = 0;
		rotate[1][3] = 0;
		rotate[2][3] = 0;
		rotate[3][3] = 1;


		Matrix<4, 4, T> result = m*rotate;
		return result;
	}

	inline Mat4f LookAt(const Vec3f& eye, const Vec3f& at, const Vec3f& up) {
		const Vec3f f(Normalize(at - eye));
		const Vec3f s(Normalize(f.Cross(up)));
		const Vec3f u(s.Cross(f));

		Mat4f result = Mat4f::GetIdentity();
		result[0][0] = s.x_;
		result[0][1] = s.y_;
		result[0][2] = s.z_;

		result[1][0] = u.x_;
		result[1][1] = u.y_;
		result[1][2] = u.z_;

		result[2][0] = -f.x_;
		result[2][1] = -f.y_;
		result[2][2] = -f.z_;

		result[0][3] = -VectorDot(s,eye);
		result[1][3] = -VectorDot(u, eye);
		result[2][3] = VectorDot(f,eye);
		return result;

	}

	//right-hand!!
	inline Mat4f Perspective(float fovy, float aspect, float zn, float zf) {
		fovy = fovy / 180.f * 3.1415926f;
		const float tan_half_fov = std::tan(fovy*0.5f);
		Mat4f result = Mat4f::GetZero();
		result[0][0] = 1.0f / (aspect * tan_half_fov);
		result[1][1] = 1.0f / tan_half_fov;
		result[2][2] = -(zf + zn) / (zf - zn);


		result[3][2] = - 1.0f;
		result[2][3] = -(2.0f * zf * zn) / (zf - zn);

		return result;
	}

	inline Mat4f ortho(float left, float right, float bottom, float top,float near,float far)
	{
		Mat4f result = Mat4f::GetIdentity();
		result[0][0] = static_cast<float>(2) / (right - left);
		result[1][1] = static_cast<float>(2) / (top - bottom);
		result[0][3] = -(right + left) / (right - left);
		result[1][3] = -(top + bottom) / (top - bottom);

		result[2][2] = -static_cast<float>(2) / (far - near);
		result[2][3] = -(far + near) / (far - near);
		return result;
	}




	template<typename T>
	inline bool IsEqual(T a, T b, T elpsilon)
	{
		T lh = std::abs(a - b);
		return lh <= elpsilon;
	}




	//----------------------------------------------------------------------
	//四元数
	//----------------------------------------------------------------------
	struct Quaternion
	{
		Quaternion() = delete;
		Quaternion(float x, float y, float z, float w):x_(x),y_(y),z_(z),w_(w){}
		Quaternion(const Vec3f& axis, float angle)
		{
			const float ha = angle * 0.5f;
			const float sa = std::sin(ha);
			x_ = axis.x_*sa;
			y_ = axis.y_*sa;
			z_ = axis.z_*sa;
			w_ = std::cos(ha);
		}


		inline float Dot(const Quaternion& other)const
		{
			return x_ * other.x_ + y_ * other.y_ + z_ * other.z_ + w_ * other.w_;
		}

		inline static Quaternion GetIdentity()
		{
			return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
		}
		inline static Quaternion GetZero()
		{
			return Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
		}
		
		float x_, y_, z_,w_;
	};



	inline Quaternion operator*(const Quaternion& l, const Quaternion& r)
	{
		const float lx = l.x_;
		const float ly = l.y_;
		const float lz = l.z_;
		const float lw = l.w_;

		const float rx = r.x_;
		const float ry = r.y_;
		const float rz = r.z_;
		const float rw = r.w_;

		return
		{
			lw * rx + lx * rw + ly * rz - lz * ry,
			lw * ry - lx * rz + ly * rw + lz * rx,
			lw * rz + lx * ry - ly * rx + lz * rw,
			lw * rw - lx * rx - ly * ry - lz * rz,
		};
	}

	inline Quaternion Normalize(const Quaternion& a)
	{
		const float norm = a.Dot(a);
		if (0.0f < norm)
		{
			const float inv_norm = 1/std::sqrt(norm);
			return Quaternion(a.x_ * inv_norm, a.y_ * inv_norm, a.z_ * inv_norm, a.w_ * inv_norm);
		}
		return Quaternion::GetIdentity();
	}

	inline Vec3f ToXAxis(const Quaternion& a)
	{
		const float xx = a.x_;
		const float yy = a.y_;
		const float zz = a.z_;
		const float ww = a.w_;
		const float ysq = yy * yy;
		const float zsq = zz * zz;

		return
		{
			1.0f - 2.0f * ysq - 2.0f * zsq,
			2.0f * xx * yy + 2.0f * zz * ww,
			2.0f * xx * zz - 2.0f * yy * ww
		};
	}
	inline Vec3f ToYAxis(const Quaternion& a)
	{
		const float xx = a.x_;
		const float yy = a.y_;
		const float zz = a.z_;
		const float ww = a.w_;
		const float xsq = xx*xx;
		const float zsq = zz*zz;

		return
		{
			2.0f * xx * yy - 2.0f * zz * ww,
			1.0f - 2.0f * xsq - 2.0f * zsq,
			2.0f * yy * zz + 2.0f * xx * ww
		};
	}

	inline Vec3f ToZAxis(const Quaternion& a)
	{
		const float xx = a.x_;
		const float yy = a.y_;
		const float zz = a.z_;
		const float ww = a.w_;
		const float xsq = xx * xx;
		const float ysq = yy * yy;

		return
		{
			2.0f * xx * zz + 2.0f * yy * ww,
			2.0f * yy * zz - 2.0f * xx * ww,
			1.0f - 2.0f * xsq - 2.0f * ysq
		};
	}






	
}
typedef TinyMath::Vec4u8i Color;


