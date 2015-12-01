// Copyright (c) 2012-2014, The CryptoNote developers, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include "include_base_utils.h"
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>
#include "serialization/binary_archive.h"

template<class T> class SwappedVector {
public:
	typedef T value_type;

	class const_iterator {
	public:
		typedef ptrdiff_t difference_type;
		typedef std::random_access_iterator_tag iterator_category;
		typedef const T* pointer;
		typedef const T& reference;
		typedef T value_type;

		const_iterator() {
		}

		const_iterator(SwappedVector* swappedVector, std::size_t index) : m_swappedVector(swappedVector), m_index(index) {
		}

		bool operator!=(const const_iterator& other) const {
			return m_index != other.m_index;
		}

		bool operator<(const const_iterator& other) const {
			return m_index < other.m_index;
		}

		bool operator<=(const const_iterator& other) const {
			return m_index <= other.m_index;
		}

		bool operator==(const const_iterator& other) const {
			return m_index == other.m_index;
		}

		bool operator>(const const_iterator& other) const {
			return m_index > other.m_index;
		}

		bool operator>=(const const_iterator& other) const {
			return m_index >= other.m_index;
		}

		const_iterator& operator++() {
			++m_index;
			return *this;
		}

		const_iterator operator++(int) {
			const_iterator i = *this;
			++m_index;
			return i;
		}

		const_iterator& operator--() {
			--m_index;
			return *this;
		}

		const_iterator operator--(int) {
			const_iterator i = *this;
			--m_index;
			return i;
		}

		const_iterator& operator+=(difference_type n) {
			m_index += n;
			return *this;
		}

		const_iterator& operator-=(difference_type n) {
			m_index -= n;
			return *this;
		}

		const_iterator operator+(difference_type n) const {
			return const_iterator(m_swappedVector, m_index + n);
		}

		friend const_iterator operator+(difference_type n, const const_iterator& i) {
			return const_iterator(i.m_swappedVector, n + i.m_index);
		}

		difference_type operator-(const const_iterator& other) const {
			return m_index - other.m_index;
		}

		const_iterator& operator-(difference_type n) const {
			return const_iterator(m_swappedVector, m_index - n);
		}

		const T& operator*() const {
			return (*m_swappedVector)[m_index];
		}

		const T* operator->() const {
			return &(*m_swappedVector)[m_index];
		}

		const T& operator[](difference_type offset) const {
			return (*m_swappedVector)[m_index + offset];
		}

		std::size_t index() const {
			return m_index;
		}

	private:
		SwappedVector* m_swappedVector;
		std::size_t m_index;
	};

	SwappedVector();
	//SwappedVector(const SwappedVector&) = delete;
	~SwappedVector();
	//SwappedVector& operator=(const SwappedVector&) = delete;

	bool open(const std::string& itemFileName, const std::string& indexFileName, size_t poolSize);
	void close();
	void hits();
	size_t get_prepared_read_count();

	bool empty() const;
	uint64_t size() const;
	const_iterator begin();
	const_iterator end();
	const T& operator[](uint64_t index);
	const T& front();
	const T& back();
	void clear();
	void pop_back();
	void push_back(const T& item);

private:
	struct ItemEntry;
	struct CacheEntry;

	struct ItemEntry {
	public:
		T item;
		typename std::list<CacheEntry>::iterator cacheIter;
	};

	struct CacheEntry {
	public:
		typename std::map<uint64_t, ItemEntry>::iterator itemIter;
	};

	std::fstream m_itemsFile;
	std::fstream m_indexesFile;
	size_t m_poolSize;
	std::vector<uint64_t> m_offsets;
	uint64_t m_itemsFileSize;
	std::map<uint64_t, ItemEntry> m_items;
	std::list<CacheEntry> m_cache;
	uint64_t m_cacheHits;
	uint64_t m_cacheMisses;

	T* prepare(uint64_t index);
};

template<class T> SwappedVector<T>::SwappedVector() {
}

template<class T> SwappedVector<T>::~SwappedVector() {
	close();
}

template<class T> bool SwappedVector<T>::open(const std::string& itemFileName, const std::string& indexFileName, size_t poolSize)
{
	if (poolSize == 0)
	{
		return false;
	}

	m_itemsFile.open(itemFileName, std::ios::in | std::ios::out | std::ios::binary);
	m_indexesFile.open(indexFileName, std::ios::in | std::ios::out | std::ios::binary);
	if (m_itemsFile && m_indexesFile)
	{
		uint64_t count;
		m_indexesFile.read(reinterpret_cast<char*>(&count), sizeof count);
		if (!m_indexesFile)
		{
			std::cout << "Cannot open index file." << std::endl;
			return false;
		}

		std::vector<uint64_t> offsets;
		uint64_t itemsFileSize = 0;
		for (uint64_t i = 0; i < count; ++i)
		{
			uint32_t itemSize;
			m_indexesFile.read(reinterpret_cast<char*>(&itemSize), sizeof itemSize);
			if (!m_indexesFile)
			{
				std::cout << "Read index file error, maybe it is corrupt." << std::endl;
				return false;
			}

			offsets.emplace_back(itemsFileSize);
			itemsFileSize += itemSize;
		}

		m_offsets.swap(offsets);
		m_itemsFileSize = itemsFileSize;
	}
	else
	{
		m_itemsFile.open(itemFileName, std::ios::out | std::ios::binary);
		m_itemsFile.close();
		m_itemsFile.open(itemFileName, std::ios::in | std::ios::out | std::ios::binary);
		m_indexesFile.open(indexFileName, std::ios::out | std::ios::binary);
		uint64_t count = 0;
		m_indexesFile.write(reinterpret_cast<char*>(&count), sizeof count);
		if (!m_indexesFile) 
		{
			std::cout << "Write count to index file error." << std::endl;
			return false;
		}

		m_indexesFile.close();
		m_indexesFile.open(indexFileName, std::ios::in | std::ios::out | std::ios::binary);
		m_offsets.clear();
		m_itemsFileSize = 0;
	}

	m_poolSize = poolSize;
	m_items.clear();
	m_cache.clear();
	m_cacheHits = 0;
	m_cacheMisses = 0;
	return true;
}

template<class T> void SwappedVector<T>::hits()
{
	LOG_PRINT_L1("SwappedVector size: " << m_offsets.size() <<  
				 ", poolsize: " << m_poolSize << 
				 ", prepared read count: " << get_prepared_read_count() <<
				 ", cache hits : " << m_cacheHits << 
				 ", misses : " << m_cacheMisses << " (" << std::fixed << std::setprecision(2)  
							   << static_cast<double>(m_cacheMisses) / (m_cacheHits + m_cacheMisses) * 100 << "%)");
}

template<class T> void SwappedVector<T>::close() 
{
	hits();
}

template<class T> bool SwappedVector<T>::empty() const 
{
	return m_offsets.empty();
}

template<class T> uint64_t SwappedVector<T>::size() const 
{
	return m_offsets.size();
}

template<class T> typename SwappedVector<T>::const_iterator SwappedVector<T>::begin() 
{
	return const_iterator(this, 0);
}

template<class T> typename SwappedVector<T>::const_iterator SwappedVector<T>::end() 
{
	return const_iterator(this, m_offsets.size());
}

#define MINIMUM_CACHE_LOAD_NUM 100
template<class T> size_t SwappedVector<T>::get_prepared_read_count()
{
	return (m_poolSize / 10 > MINIMUM_CACHE_LOAD_NUM ? m_poolSize / 10 : MINIMUM_CACHE_LOAD_NUM);
}

template<class T> const T& SwappedVector<T>::operator[](uint64_t index) 
{
	auto itemIter = m_items.find(index);
	if (itemIter != m_items.end()) 
	{
		if (itemIter->second.cacheIter != --m_cache.end()) 
		{
			m_cache.splice(m_cache.end(), m_cache, itemIter->second.cacheIter);
		}

		++m_cacheHits;
		if ((m_cacheMisses + m_cacheHits) % 1000 == 0) hits();
		return itemIter->second.item;
	}

	if (index >= m_offsets.size()) 
	{
		throw std::runtime_error("SwappedVector::operator[], index overflow.");
	}

	if (!m_itemsFile) 
	{
		throw std::runtime_error("SwappedVector::operator[], no items file.");
	}

	T* index_item = NULL;

	for (size_t i = 0; i < get_prepared_read_count(); i++)
	{
		if (index + i > m_offsets.size() - 1) break;

		m_itemsFile.seekg(m_offsets[index + i]);
		binary_archive<false> archive(m_itemsFile);
		T tempItem;

		if (!do_serialize(archive, tempItem))
		{
			throw std::runtime_error("SwappedVector::operator[], read item from items file error");
		}

		T* item = prepare(index + i);
		if (i == 0) index_item = item;

		std::swap(tempItem, *item);
	}

	++m_cacheMisses;
	if ((m_cacheMisses + m_cacheHits) % 1000 == 0) hits();
	return *index_item;
}

template<class T> T* SwappedVector<T>::prepare(uint64_t index)
{
	if (m_cache.size() == m_poolSize)
	{
		auto cacheIter = m_cache.begin();
		m_items.erase(cacheIter->itemIter);
		m_cache.erase(cacheIter);
	}

	auto itemIter = m_items.insert(std::make_pair(index, ItemEntry()));

	//already exists
	if (itemIter.second == false)
		return &itemIter.first->second.item;

	CacheEntry cacheEntry = { itemIter.first };
	auto cacheIter = m_cache.insert(m_cache.end(), cacheEntry);
	itemIter.first->second.cacheIter = cacheIter;

	if (m_cache.size() != m_items.size())
	{
		LOG_PRINT_RED("cache size: " << m_cache.size() << " is not equal to item size: " << m_items.size(), LOG_LEVEL_0);
	}

	return &itemIter.first->second.item;
}


template<class T> const T& SwappedVector<T>::front() 
{
	return operator[](0);
}

template<class T> const T& SwappedVector<T>::back() 
{
	return operator[](m_offsets.size() - 1);
}

template<class T> void SwappedVector<T>::clear() 
{
	if (!m_indexesFile) {
		throw std::runtime_error("SwappedVector::clear, no index file.");
	}

	m_indexesFile.seekp(0);
	uint64_t count = 0;
	m_indexesFile.write(reinterpret_cast<char*>(&count), sizeof count);
	if (!m_indexesFile) {
		throw std::runtime_error("SwappedVector::clear, write index file error.");
	}

	m_offsets.clear();
	m_itemsFileSize = 0;
	m_items.clear();
	m_cache.clear();
}

template<class T> void SwappedVector<T>::pop_back() 
{
	if (!m_indexesFile) 
	{
		throw std::runtime_error("SwappedVector::pop_back, no index file.");
	}

	m_indexesFile.seekp(0);
	uint64_t count = m_offsets.size() - 1;
	m_indexesFile.write(reinterpret_cast<char*>(&count), sizeof count);
	if (!m_indexesFile) 
	{
		throw std::runtime_error("SwappedVector::pop_back, write index file error.");
	}

	m_itemsFileSize = m_offsets.back();
	m_offsets.pop_back();
	auto itemIter = m_items.find(m_offsets.size());
	if (itemIter != m_items.end()) 
	{
		m_cache.erase(itemIter->second.cacheIter);
		m_items.erase(itemIter);
	}
}

template<class T> void SwappedVector<T>::push_back(const T& item) 
{
	uint64_t itemsFileSize;

	{
		if (!m_itemsFile) {
			throw std::runtime_error("SwappedVector::push_back, no items file.");
		}

		m_itemsFile.seekp(m_itemsFileSize);
		binary_archive<true> archive(m_itemsFile);
		if (!do_serialize(archive, *const_cast<T*>(&item))) {
			throw std::runtime_error("SwappedVector::push_back, save to item file error.");
		}

		itemsFileSize = m_itemsFile.tellp();
	}

	{
		if (!m_indexesFile) {
			throw std::runtime_error("SwappedVector::push_back, no index file.");
		}

		m_indexesFile.seekp(sizeof(uint64_t)+sizeof(uint32_t)* m_offsets.size());
		uint32_t itemSize = static_cast<uint32_t>(itemsFileSize - m_itemsFileSize);
		m_indexesFile.write(reinterpret_cast<char*>(&itemSize), sizeof itemSize);
		if (!m_indexesFile) {
			throw std::runtime_error("SwappedVector::push_back, write itemSize to index file, error.");
		}

		m_indexesFile.seekp(0);
		uint64_t count = m_offsets.size() + 1;
		m_indexesFile.write(reinterpret_cast<char*>(&count), sizeof count);
		if (!m_indexesFile) {
			throw std::runtime_error("SwappedVector::push_back, write count to index file, error.");
		}
	}

	m_offsets.push_back(m_itemsFileSize);
	m_itemsFileSize = itemsFileSize;

	T* newItem = prepare(m_offsets.size() - 1);
	*newItem = item;
}
