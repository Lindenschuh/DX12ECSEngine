#pragma once
#include "Default.h"

template <typename T>
class DynamicArray
{
public:

	DynamicArray(u32 startCount = 20);
	DynamicArray(const DynamicArray<T>& other);
	static void SwapArrays(DynamicArray<T>& a, DynamicArray<T>& b);
	void Add(T element);
	void AddAll(T* elemnts, u32 count);
	void AddAll(T* begin, T* end);
	void AddAll(DynamicArray<T> otherCollection);
	bool AddAt(u32 index, T element);
	DynamicArray<T>& operator= (const DynamicArray<T> & other);
	T& operator[] (u32 index);
	T operator[]  (u32 index) const;
	T& Get(u32 index);
	T Get(u32 index) const;
	void Delete(u32 index);
	void Resize(u32 reSize);
	u32 Count() const;
	u32 Capacity() const;
	void Reset();
	void Clear();
	T* GetData();
	void Pack();
private:
	u32 _capacity;
	u32 _count;
	T* _data;

	void _grow();
};

template<typename T>
DynamicArray<T>::DynamicArray(u32 startSize)
{
	_data = MALLOC(T, startSize);
	_capacity = startSize;
	_count = 0;
}

template<typename T>
DynamicArray<T>::DynamicArray(const DynamicArray<T>& other)
{
	_data = MALLOC(T, other.Capacity());
	_capacity = other.Capacity();
	_count = other.Count();
	for (int i = 0; i < _count; i++)
	{
		_data[i] = other[i];
	}
}

template<typename T>
inline void DynamicArray<T>::SwapArrays(DynamicArray<T>& a, DynamicArray<T>& b)
{
	u32 tmpCapa = a._capacity;
	u32 tmpCount = a._count;
	T* tmpData = a._data;

	a._capacity = b._capacity;
	a._count = b._count;
	a._data = b._data;

	b._capacity = tmpCapa;
	b._count = tmpCount;
	b._data = tmpData;
}

template<typename T>
void DynamicArray<T>::Add(T element)
{
	if (_capacity <= _count)
		_grow();

	_data[_count] = element;
	_count++;
}

template<typename T>
bool DynamicArray<T>::AddAt(u32 index, T element)
{
	if (index >= _count)
		return false;

	if (_capacity <= _count)
		_grow();

	for (u32 i = _count; i > index; i--)
	{
		_data[i] = _data[i - 1];
	}

	_data[index] = element;
	_count++;
	return true;
}

template<typename T>
DynamicArray<T>& DynamicArray<T>::operator=(const DynamicArray<T>& other)
{
	free(_data);
	_capacity = other.Capacity();
	_count = other.Count();
	_data = MALLOC(T, _capacity);
	for (int i = 0; i < _count; i++)
	{
		_data[i] = other[i];
	}
	return *this;
}

template<typename T>
T& DynamicArray<T>::operator[](u32 index)
{
	return _data[index];
}

template<typename T>
inline T DynamicArray<T>::operator[](u32 index) const
{
	return _data[index];
}

template<typename T>
T& DynamicArray<T>::Get(u32 index)
{
	return _data[index];
}

template<typename T>
inline T DynamicArray<T>::Get(u32 index) const
{
	return _data[index];
}

template<typename T>
void DynamicArray<T>::Delete(u32 index)
{
	if (index >= _count)
		return;

	for (u32 i = index; i < _count - 1; i++)
	{
		_data[i] = _data[i + 1];
	}
	_count--;
}

template<typename T>
u32 DynamicArray<T>::Count() const
{
	return _count;
}

template<typename T>
u32 DynamicArray<T>::Capacity() const
{
	return _capacity;
}

template<typename T>
void DynamicArray<T>::Reset()
{
	_count = 0;
}

template<typename T>
void DynamicArray<T>::Clear()
{
	free(_data);
	_data = MALLOC(T, 20);
	_capacity = 20;
	_count = 0;
}

template<typename T>
void DynamicArray<T>::_grow()
{
	u32 newSize = (_capacity + 3) * 1.2;
	Resize(newSize);
}

template<typename T>
void DynamicArray<T>::Resize(u32 reSize)
{
	u32 newSize = reSize;
	T* newData = MALLOC(T, newSize);
	u32 copySize = newSize > _count ? _count : newSize;

	for (u32 i = 0; i < copySize; i++)
	{
		newData[i] = _data[i];
	}
	free(_data);

	_data = newData;
	_capacity = newSize;
}

template<typename T>
T* DynamicArray<T>::GetData()
{
	return _data;
}

template<typename T>
void DynamicArray<T>::AddAll(T* begin, T* end)
{
	u32 range = end - begin;
	u32 newSize = (range + _count);
	if (newSize > _capacity)
		Resize(newSize);

	for (int i = 0; i < range; i++)
		Add(begin[i]);
}

template<typename T>
void DynamicArray<T>::AddAll(DynamicArray<T> otherCollection)
{
	if ((_count + otherCollection.Count()) > _capacity)
		Resize((_count + otherCollection.Count()));

	for (int i = 0; i < otherCollection.Count(); i++)
	{
		Add(otherCollection[i]);
	}
}

template<typename T>
void DynamicArray<T>::AddAll(T* elemnts, u32 count)
{
	if ((count + _count) > _capacity)
		Resize(count + _count);

	for (int i = 0; i < count; i++)
		Add(elemnts[i]);
}

template<typename T>
void DynamicArray<T>::Pack()
{
	Resize(_count);
}