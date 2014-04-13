#ifndef __SWIFT_BASE_NONCOPYABLE_HPP__
#define __SWIFT_BASE_NONCOPYABLE_HPP__

namespace swift {

//  Private copy constructor and copy assignment ensure classes derived from
//  class noncopyable cannot be copied.

namespace noncopyable_  // protection from unintended ADL
{
	class noncopyable
	{
	protected:
		noncopyable () {}
		~noncopyable () {}

	private:  // emphasize the following members are private
		noncopyable (const noncopyable&);
		const noncopyable& operator= (const noncopyable&);
	};
}

typedef noncopyable_::noncopyable noncopyable;

} // namespace swift

#endif  // __SWIFT_BASE_NONCOPYABLE_HPP__
