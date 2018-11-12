#pragma once
#include "Default.h"
#include <string.h>

template <typename t, u32 size>
class SimpleHashTable
{
public:
	SimpleHashTable();
	bool Insert(const char* key, t Value);
	bool Remove(const char* key);
	t Lookup(const char* key) const;
	t& Lookup(const char* key);
	t operator[](const char* key) const;
	t& operator[](const char* key);
	char** GetAllkeys();
	t* GetAllData();
	u32 GetCount();
private:
	u32 elementCount;
	const char** keys;
	t* data;

	u32 hashEntry(const char* key);
	u32 findEntry(const char* key);
};

template<typename t, u32 size>
SimpleHashTable<t, size>::SimpleHashTable() : elementCount(0)
{
	keys = new const char*[size];
	for (int i = 0; i < size; i++)
	{
		keys[i] = "";
	}
	data = new t[size];
}

template<typename t, u32 size>
bool SimpleHashTable<t, size>::Insert(const char * key, t value)
{
	if (elementCount >= size)
		return false;

	u32 hashValue = hashEntry(key);
	while (true)
	{
		const char* foundKey = keys[hashValue];
		if (!*foundKey)
		{
			data[hashValue] = value;
			keys[hashValue] = key;
			break;
		}
		if (strcmp(foundKey, key) == 0)
		{
			data[hashValue] = value;
			return true;
		}

		hashValue = ++hashValue % size;
	}
	elementCount++;
	return true;
}

template<typename t, u32 size>
u32 SimpleHashTable<t, size>::findEntry(const char* key)
{
	u32 hashval = hashEntry(key);
	u32 searchIndex = hashval;
	do
	{
		const char* foundKey = keys[searchIndex];

		if (!*foundKey)
			return size + 1;
		if (strcmp(foundKey, key) == 0)
		{
			return searchIndex;
		}

		searchIndex = ++searchIndex % size;
	} while (searchIndex != hashval);

	return size + 1;
}

template<typename t, u32 size>
bool SimpleHashTable<t, size>::Remove(const char * key)
{
	u32 index = findEntry(key);
	if (index > size)
		return false;

	data[index] = { 0 };
	keys[index][0] = 0;

	elementCount--;
	return true;
}

template<typename t, u32 size>
t SimpleHashTable<t, size>::Lookup(const char * key) const
{
	u32 index = findEntry(key);
	return data[index];
}

template<typename t, u32 size>
t& SimpleHashTable<t, size>::Lookup(const char * key)
{
	u32 index = findEntry(key);
	return data[index];
}

template<typename t, u32 size>
t SimpleHashTable<t, size>::operator[](const char * key) const
{
	u32 index = findEntry(key);
	return data[index];
}

template<typename t, u32 size>
t& SimpleHashTable<t, size>::operator[](const char * key)
{
	u32 index = findEntry(key);
	return data[index];
}

template<typename t, u32 size>
char ** SimpleHashTable<t, size>::GetAllkeys()
{
	return keys;
}

template<typename t, u32 size>
t * SimpleHashTable<t, size>::GetAllData()
{
	return data;
}

template<typename t, u32 size>
u32 SimpleHashTable<t, size>::GetCount()
{
	return elementCount;
}

//TODO: Implement a better hashfunction than DJB
template <typename t, u32 size>
u32 SimpleHashTable<t, size>::hashEntry(const char* key)
{
	u32 hashval = 5381;
	u32 c;
	while (c = *key++)
		hashval = ((hashval << 5) + hashval) + c;

	return hashval % size;
}