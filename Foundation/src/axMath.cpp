#include "Foundation/axMath.h"
#include "Foundation/detail/axVector3.h"
#include "Foundation/detail/axVector4.h"

namespace apex {
namespace math {

	// Vector2

	Vector2::Vector2(f32 _x, f32 _y)
		: x(_x), y(_y)
	{
	}

	Vector2::Vector2(f32 values[2])
		: x(values[0]), y(values[1])
	{
	}

	Vector2::Vector2(f32 value)
		: x(value), y(value)
	{
	}

	f32& Vector2::operator[](size_t index)
	{
		return m_values[index];
	}

	const f32& Vector2::operator[](size_t index) const
	{
		return m_values[index];
	}

	// Vector3

	Vector3::Vector3(f32 _x, f32 _y, f32 _z)
		: x(_x), y(_y), z(_z)
	{
	}

	Vector3::Vector3(f32 values[3])
		: x(values[0]), y(values[1]), z(values[2])
	{
	}

	Vector3::Vector3(f32 value)
		: x(value), y(value), z(value)
	{
	}

	Vector3::Vector3(Vector2 const& _xy, f32 _z)
		: x(_xy.x), y(_xy.y), z(_z)
	{
	}

	Vector3::Vector3(f32 _x, Vector2 const& _yz)
		: x(_x), y(_yz.x), z(_yz.y)
	{
	}

	f32& Vector3::operator[](size_t index)
	{
		return m_values[index];
	}

	const f32& Vector3::operator[](size_t index) const
	{
		return m_values[index];
	}

	// Vector4

	Vector4::Vector4(f32 _x, f32 _y, f32 _z, f32 _w)
		: x(_x), y(_y), z(_z), w(_w)
	{
	}

	Vector4::Vector4(f32 values[4])
		: x(values[0]), y(values[1]), z(values[2]), w(values[3])
	{
	}

	Vector4::Vector4(f32 value)
		: x(value), y(value), z(value), w(value)
	{
	}

	Vector4::Vector4(Vector3 const& _xyz, f32 _w)
		: x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w)
	{
	}

	Vector4::Vector4(f32 _x, Vector3 const& _yzw)
		: x(_x), y(_yzw.x), z(_yzw.y), w(_yzw.z)
	{
	}

	Vector4::Vector4(Vector2 const& _xy, f32 _z, f32 _w)
		: x(_xy.x), y(_xy.y), z(_z), w(_w)
	{
	}

	Vector4::Vector4(f32 _x, Vector2 const& _yz, f32 _w)
		: x(_x), y(_yz.x), z(_yz.y), w(_w)
	{
	}

	Vector4::Vector4(f32 _x, f32 _y, Vector2 const& _zw)
		: x(_x), y(_y), z(_zw.x), w(_zw.y)
	{
	}

	Vector4::Vector4(Vector2 const& _xy, Vector2 const& _zw)
		: x(_xy.x), y(_xy.y), z(_zw.x), w(_zw.y)
	{
	}

	f32& Vector4::operator[](size_t index)
	{
		return m_values[index];
	}

	const f32& Vector4::operator[](size_t index) const
	{
		return m_values[index];
	}
}
}
