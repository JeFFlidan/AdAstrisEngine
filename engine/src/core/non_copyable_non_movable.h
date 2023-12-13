#pragma once

namespace ad_astris
{
	class NonCopyableNonMovable
	{
		public:
			NonCopyableNonMovable() = default;
			NonCopyableNonMovable(const NonCopyableNonMovable&) = delete;
			NonCopyableNonMovable& operator=(const NonCopyableNonMovable&) = delete;
			NonCopyableNonMovable(NonCopyableNonMovable&&) = delete;
			NonCopyableNonMovable& operator=(NonCopyableNonMovable&&) = delete;
	};
}