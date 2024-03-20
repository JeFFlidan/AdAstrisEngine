#pragma once

#include "platform.h"
#include <cassert>

namespace ad_astris
{
	class PlatformResourcePtr
	{
		public:
			~PlatformResourcePtr() = default;
	};
	
#ifdef _WIN32
	template <typename WinApiType>
	struct CleanupResource
	{
		void operator()(typename WinApiType::Resource) const { }
	};

	struct HModuleTag
	{
		using Resource = HMODULE;
	};

	struct HandleTag
	{
		using Resource = HANDLE;
	};

	template<>
	struct CleanupResource<HModuleTag>
	{
		void operator()(HModuleTag::Resource handle) const
		{
			assert(FreeLibrary(handle) != 0);
		}
	};

	template<>
	struct CleanupResource<HandleTag>
	{
		void operator()(HandleTag::Resource handle) const
		{
			assert(CloseHandle(handle) != 0);
		}
	};

	template<typename Tag, typename Tag::Resource Invalid, typename Cleanup = CleanupResource<Tag>>
	class WinApiResourcePtr : public PlatformResourcePtr
	{
		private:
			using Resource = typename Tag::Resource;
		
		public:
			WinApiResourcePtr() : _resource{ Invalid } { }
			WinApiResourcePtr(const WinApiResourcePtr&) = delete;
			void operator=(const WinApiResourcePtr&) = delete;
		
			~WinApiResourcePtr()
			{
				if (is_valid())
					_deleter(_resource);
			}

			void reset(Resource newResource = Resource())
			{
				if (is_valid())
					_deleter(_resource);
				_resource = newResource;
			}
		
			bool is_valid() const
			{
				return _resource != Resource() && _resource != Invalid;
			}
		
			const Resource& get_handle() const { return _resource; }
			operator const Resource&() const { return _resource; }
			operator bool() const { return is_valid(); }
		
		private:
			Resource _resource;
			Cleanup _deleter;
	};

	using HModulePtr = WinApiResourcePtr<HModuleTag, NULL>;
	using HandlePtr = WinApiResourcePtr<HandleTag, INVALID_HANDLE_VALUE>;
#endif
}
