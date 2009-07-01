/*
Copyright (c) 2003-2006 Gino van den Bergen / Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/



#ifndef pybtTransform_H
#define pybtTransform_H


#include "pybtQuaternion.h"
#include "pybtVector3.h"
#include "pybtMatrix3x3.h"

namespace py
{
/**@brief The Transform class supports rigid transforms with only translation and rotation and no scaling/shear.
 *It can be used in combination with Vector3, Quaternion and Matrix3x3 linear algebra classes. */
class Transform {
	

public:
	
  /**@brief No initialization constructor */
	Transform() {}
  /**@brief Constructor from Quaternion (optional Vector3 )
   * @param q Rotation from quaternion 
   * @param c Translation from Vector (default 0,0,0) */
  explicit SIMD_FORCE_INLINE Transform(const py::Quaternion& q, 
		const Vector3& c = Vector3(btScalar(0), btScalar(0), btScalar(0))) 
		: m_basis(q),
		m_origin(c)
	{}

  /**@brief Constructor from Matrix3x3 (optional Vector3)
   * @param b Rotation from Matrix 
   * @param c Translation from Vector default (0,0,0)*/
  Transform(const py::Matrix3x3& b, 
		const Vector3& c = Vector3(btScalar(0), btScalar(0), btScalar(0)))
		: m_basis(b),
		m_origin(c)
	{}
  /**@brief Copy constructor */
	SIMD_FORCE_INLINE Transform (const Transform& other)
		: m_basis(other.m_basis),
		m_origin(other.m_origin)
	{
	}
  /**@brief Assignment Operator */
  // Comment for swig I'm not sure why it's complaining Warning 362
	SIMD_FORCE_INLINE Transform& operator=(const Transform& other)
	{
		m_basis = other.m_basis;
		m_origin = other.m_origin;
		return *this;
	}
  

  /**@brief Set the current transform as the value of the product of two transforms
   * @param t1 Transform 1
   * @param t2 Transform 2
   * This = Transform1 * Transform2 */
		SIMD_FORCE_INLINE void mult(const py::Transform& t1, const py::Transform& t2) {
			m_basis = t1.m_basis * t2.m_basis;
			m_origin = t1(t2.m_origin);
		}

/*		void multInverseLeft(const Transform& t1, const Transform& t2) {
			Vector3 v = t2.m_origin - t1.m_origin;
			m_basis = btMultTransposeLeft(t1.m_basis, t2.m_basis);
			m_origin = v * t1.m_basis;
		}
		*/

/**@brief Return the transform of the vector */
  SIMD_FORCE_INLINE Vector3 operator()(const py::Vector3& x) const
	{
          return py::Vector3(m_basis.getRow(0).dot(x) + m_origin.x(), 
                           m_basis.getRow(1).dot(x) + m_origin.y(), 
                           m_basis.getRow(2).dot(x) + m_origin.z());
	}

  /**@brief Return the transform of the vector */
	SIMD_FORCE_INLINE py::Vector3 operator*(const py::Vector3& x) const
	{
		return (*this)(x);
	}

  /**@brief Return the transform of the Quaternion */
  SIMD_FORCE_INLINE py::Quaternion operator*(const py::Quaternion& q) const
	{
		return getRotation() * q;
	}

  /**@brief Return the basis matrix for the rotation */
	SIMD_FORCE_INLINE py::Matrix3x3&       getBasis()          { return m_basis; }
  /**@brief Return the basis matrix for the rotation */
	SIMD_FORCE_INLINE const py::Matrix3x3& getBasis()    const { return m_basis; }

  /**@brief Return the origin vector translation */
	SIMD_FORCE_INLINE Vector3&         getOrigin()         { return m_origin; }
  /**@brief Return the origin vector translation */
	SIMD_FORCE_INLINE const py::Vector3&   getOrigin()   const { return m_origin; }

  /**@brief Return a quaternion representing the rotation */
	py::Quaternion getRotation() const { 
		py::Quaternion q;
		m_basis.getRotation(q);
		return q;
	}
	
	
  /**@brief Set from an array 
   * @param m A pointer to a 15 element array (12 rotation(row major padded on the right by 1), and 3 translation */
	void setFromOpenGLMatrix(const btScalar *m)
	{
		m_basis.setFromOpenGLSubMatrix(m);
		m_origin.setValue(m[12],m[13],m[14]);
	}

  /**@brief Fill an array representation
   * @param m A pointer to a 15 element array (12 rotation(row major padded on the right by 1), and 3 translation */
	void getOpenGLMatrix(btScalar *m) const 
	{
		m_basis.getOpenGLSubMatrix(m);
		m[12] = m_origin.x();
		m[13] = m_origin.y();
		m[14] = m_origin.z();
		m[15] = btScalar(1.0);
	}

  /**@brief Set the translational element
   * @param origin The vector to set the translation to */
	SIMD_FORCE_INLINE void setOrigin(const Vector3& origin) 
	{ 
		m_origin = origin;
	}

	SIMD_FORCE_INLINE Vector3 invXform(const Vector3& inVec) const;


  /**@brief Set the rotational element by Matrix3x3 */
	SIMD_FORCE_INLINE void setBasis(const Matrix3x3& basis)
	{ 
		m_basis = basis;
	}

  /**@brief Set the rotational element by Quaternion */
	SIMD_FORCE_INLINE void setRotation(const Quaternion& q)
	{
		m_basis.setRotation(q);
	}


  /**@brief Set this transformation to the identity */
	void setIdentity()
	{
		m_basis.setIdentity();
		m_origin.setValue(btScalar(0.0), btScalar(0.0), btScalar(0.0));
	}

  /**@brief Multiply this Transform by another(this = this * another) 
   * @param t The other transform */
	Transform& operator*=(const Transform& t) 
	{
		m_origin += m_basis * t.m_origin;
		m_basis *= t.m_basis;
		return *this;
	}

  /**@brief Return the inverse of this transform */
	Transform inverse() const
	{ 
		Matrix3x3 inv = m_basis.transpose();
		return Transform(inv, inv * -m_origin);
	}

  /**@brief Return the inverse of this transform times the other transform
   * @param t The other transform 
   * return this.inverse() * the other */
	Transform inverseTimes(const Transform& t) const;  

  /**@brief Return the product of this transform and the other */
	Transform operator*(const Transform& t) const;

  /**@brief Return an identity transform */
	static const Transform&	getIdentity()
	{
		static const Transform identityTransform(Matrix3x3::getIdentity());
		return identityTransform;
	}
	
  /**@brief Test if two transforms have all elements equal */
  bool operator==(const Transform& t2) const
  {
    return ( getBasis()  == t2.getBasis() &&
             getOrigin() == t2.getOrigin() );
  };

private:
  ///Storage for the rotation
	Matrix3x3 m_basis;
  ///Storage for the translation
	Vector3   m_origin;
};


SIMD_FORCE_INLINE Vector3
Transform::invXform(const Vector3& inVec) const
{
	Vector3 v = inVec - m_origin;
	return (m_basis.transpose() * v);
}

SIMD_FORCE_INLINE Transform 
Transform::inverseTimes(const Transform& t) const  
{
	Vector3 v = t.getOrigin() - m_origin;
		return Transform(m_basis.transposeTimes(t.m_basis),
                                   py::vecTimesMatrix(v, m_basis));
}

SIMD_FORCE_INLINE Transform 
Transform::operator*(const Transform& t) const
{
	return Transform(m_basis * t.m_basis, 
		(*this)(t.m_origin));
}


}

#endif






