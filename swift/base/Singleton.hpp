#ifndef __SWIFT_BASE_SINGLETON_HPP__
#define __SWIFT_BASE_SINGLETON_HPP__

#include <mutex>
#include <swift/base/noncopyable.hpp>

// used example
// T& instance = Singleton<T>::Instance ();

namespace swift {

template <typename T>
class Singleton : swift::noncopyable
{
public:
    static T& Instance ()
    {
        std::call_once (ponce_, &Singleton::Init);

        return *value_;
    }

private:
    Singleton () {};
    ~Singleton () {};

    static void Init ()
    {
        if (0 == value_) {
            value_ = new T ();
            ::atexit (Destroy);
        }
    }

    static void Destroy ()
    {
        // this typedef is to avoid T is not a complete type
        typedef char T_must_be_complete_type[sizeof (T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy; (void) dummy;
        if (nullptr != value_) {
            delete value_;
            value_ = nullptr;
        }
    }

private:
    static std::once_flag ponce_;
    static T* volatile value_;
};

template <typename T>
std::once_flag Singleton<T>::ponce_;

template <typename T>
T* volatile Singleton<T>::value_ = nullptr;

} // end of name space swift

#endif // __SWIFT_BASE_SINGLETON_HPP__
