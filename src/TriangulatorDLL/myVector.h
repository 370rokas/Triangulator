#ifndef	__VECTOR__
#define	__VECTOR__

#include	<math.h>
#include <assert.h>

#define M_PI        3.14159265358979323846
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.785398163397448309616
#define MAXDOUBLE   1.7976931348623158E+308
#define MAXFLOAT    3.40282347E+38F
#define MINDOUBLE   2.2250738585072014E-308
#define MINFLOAT    1.17549435E-38F
#define MAXLDOUBLE  1.1897314953572317649E+4932L
#define MINLDOUBLE  3.362103143112094E-4917L 

class	Vector
{
public:
	double	x, y, z;

	Vector() {};
	Vector(double v) {x = y = z = v;};
	Vector(const Vector& v) {x = v.x; y = v.y; z = v.z;};
	Vector(double vx, double vy, double vz) {x = vx; y = vy; z = vz;};
	Vector(int vx, int vy, int vz = 0) {x = vx; y = vy; z = vz;};

	Vector&	operator=(const Vector& v ) { x = v.x; y = v.y; z = v.z; return *this; };
	Vector&  operator=(double f ) { x = y = z = f; return *this; };
	Vector   operator-() const;

	Vector&	operator += ( const Vector& );
	Vector& operator -= ( const Vector& );
	Vector&	operator *= ( const Vector& );
	Vector& operator *= ( double );
	Vector&	operator /= ( double );

	friend	Vector operator + ( const Vector&, const Vector& );
	friend	Vector operator - ( const Vector&, const Vector& );
	friend	Vector operator * ( const Vector&, const Vector& );
	friend	Vector operator * ( double, const Vector&  );
	friend	Vector operator * ( const Vector&, double  );
	friend	Vector operator / ( const Vector&, double  );
	friend	Vector operator / ( const Vector&, const Vector& );
	friend	double operator & ( const Vector& u, const Vector& v ) { return u.x*v.x + u.y*v.y + u.z*v.z; };
	friend	Vector operator ^ ( const Vector&, const Vector& );

	double	operator !  () { return (double) sqrt ( x*x + y*y + z*z ); };
	double&	operator [] ( int n ) { return * ( &x + n ); };
	int	operator <  ( double v ) { return x < v && y < v && z < v; };
	int	operator >  ( double v ) { return x > v && y > v && z > v; };
};

//////////////////// implementation /////////////////////////

inline Vector Vector::operator-() const
{
	return Vector(-x, -y, -z);
}

inline Vector operator+(const Vector& u, const Vector& v)
{
	return Vector(u.x + v.x, u.y + v.y, u.z + v.z);
}

inline Vector operator-(const Vector& u, const Vector& v)
{
	return Vector(u.x - v.x, u.y - v.y, u.z - v.z);
}

inline Vector operator*(const Vector& u, const Vector& v)
{
	return Vector(u.x * v.x, u.y * v.y, u.z * v.z);
}

inline Vector operator*(const Vector& u, double f)
{
	return Vector(u.x * f, u.y * f, u.z * f);
}

inline Vector operator*(double f, const Vector& v)
{
	return Vector(f * v.x, f * v.y, f * v.z);
}

inline Vector operator/(const Vector& v, double f)
{
   assert(f);
	return Vector(v.x / f, v.y / f, v.z / f);
}

inline Vector operator/(const Vector& u, const Vector& v)
{
	return Vector(u.x / v.x, u.y / v.y, u.z / v.z);
}

inline Vector&	Vector::operator+=(const Vector& v)
{
	x += v.x;
	y += v.y;
	z += v.z;

	return *this;
}

inline Vector&	Vector::operator-=(const Vector& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;

	return *this;
}

inline Vector&	Vector::operator*=(double v)
{
	x *= v;
	y *= v;
	z *= v;

	return *this;
}

inline Vector&	Vector::operator*=(const Vector& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;

	return *this;
}

inline Vector&	Vector::operator/=(double v)
{
	x /= v;
	y /= v;
	z /= v;

	return *this;
}

class	Ray {
public:
	Vector Org;
	Vector Dir;				// direction must be normalyzed

	Ray(){};
	Ray(Vector& o, Vector& d) {Org = o; Dir = d;};

	Vector Point(double t) {return Org + Dir*t;};
};

/////////////////////////// Functions /////////////////////////////////

inline	Vector Normalize(Vector& v)
{
   return v/!v;
};
Vector	RndVector();
Vector&	Clip(Vector&);

// 3D кубик (€чейка)
typedef struct {
   Vector p[8];   // координаты вершин кубика
   double val[8]; // значени€ функции в вершинах
} GRIDCELL;

Vector operator^(const Vector& u, const Vector& v)
{
	return Vector(u.y*v.z - u.z*v.y,
					  u.z*v.x - u.x*v.z,
					  u.x*v.y - u.y*v.x);
}

Vector RndVector()
{
	Vector v(rand() - 0.5*RAND_MAX, rand() - 0.5*RAND_MAX, rand() - 0.5*RAND_MAX);
	return Normalize(v);
}

Vector& Clip(Vector& v)
{
	if(v.x < 0.0)
		v.x = 0.0;
	else
	if(v.x > 1.0)
		v.x = 1.0;

	if(v.y < 0.0)
		v.y = 0.0;
	else
	if(v.y > 1.0)
		v.y = 1.0;

	if(v.z < 0.0)
		v.z = 0.0;
	else
	if(v.z > 1.0)
		v.z = 1.0;

	return v;
}

#endif
