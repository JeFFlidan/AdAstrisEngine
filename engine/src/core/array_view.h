#pragma once

#include <shared_mutex>
#include <cassert>
#include <memory>

namespace ad_astris
{
	template<typename T>
	class ArrayView
	{
		public:
			ArrayView(T* data, size_t size) : _data(data), _size(size) { }

			T& operator[](size_t index)
			{
				assert(index < _size);
				return _data[index];
			}

			T* data()
			{
				return _data;
			}

			size_t size()
			{
				return _size;
			}

		private:
			T* _data;
			size_t _size;
	};

	template<typename T>
	class ConstArrayView
	{
		public:
			ConstArrayView(const T* data, size_t size) : _data(data), _size(size) { }
		
			T operator[](size_t index) const
			{
				assert(index < _size);
				return _data[index];
			}

			const T* data() const
			{
				return _data;
			}

			size_t size() const
			{
				return _size;
			}

		private:
			const T* _data;
			size_t _size;
	};

	// If object of ThreadSafeArrayView is copied, mutex is moved and previous object is invalid
	template<typename T>
	class ThreadSafeArrayView
	{
		public:
			ThreadSafeArrayView(T* data, size_t size) : _data(data), _size(size)
			{
				_mutex = std::make_unique<std::shared_mutex>();
			}
		
			ThreadSafeArrayView(const ThreadSafeArrayView& other)
			{
				_mutex = std::move(other._mutex);
				_data = other._data;
				_size = other._size;
			}

			T operator[](size_t index) const
			{
				std::shared_lock<std::shared_mutex> lock(*_mutex);
				assert(index < _size);
				return _data[index];
			}

			void set(const T& value, size_t index)
			{
				std::unique_lock<std::shared_mutex> lock(*_mutex);
				assert(index < _size);
				_data[index] = value;
			}

			size_t size() const
			{
				std::shared_lock<std::shared_mutex> lock(*_mutex);
				return _size;
			}

			ThreadSafeArrayView& operator=(const ThreadSafeArrayView& other)
			{
				if (this != &other)
				{
					_mutex = std::move(other._mutex);
					_data = other._data;
					_size = other._size;
				}
				return *this;
			}
		
		private:
			T* _data;
			size_t _size;
			mutable std::unique_ptr<std::shared_mutex> _mutex;
	};
}