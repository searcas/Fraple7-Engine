#pragma once


namespace Math
{
	template<typename T>
	inline T DivideByMultiple(T value, size_t alignment)
	{
		return (T)((value + alignment - 1) / alignment);
	}
}