/**
 @file AtomicInt32.h

 @maintainer Morgan McGuire, http://graphics.cs.williams.edu

 @created 2005-09-01
 @edited  2006-06-21
 */
#ifndef G3D_ATOMICINT32_H
#define G3D_ATOMICINT32_H

#include "G3D/platform.h"
#include "G3D/g3dmath.h"
#include <atomic>

namespace G3D {

/**
 An integer that may safely be used on different threads without
 external locking.

 On Win32, Linux, FreeBSD, and Mac OS X this is implemented without locks.

 <B>BETA API</B>  This is unsupported and may change
 */
class AtomicInt32 {
private:
std::atomic<int32> m_value;


public:

    /** Initial value is undefined. */
    AtomicInt32() {}

    /** Atomic set */
    explicit AtomicInt32(const int32 x) {
        m_value = x;
    }

    /** Atomic set */
    AtomicInt32(const AtomicInt32& x) {
        m_value = x.m_value.load();
    }

    /** Atomic set */
    const AtomicInt32& operator=(const int32 x) {
        m_value = x;
        return *this;
    }

    /** Atomic set */
    void operator=(const AtomicInt32& x) {
        m_value = x.m_value.load();
    }

    /** Returns the current value */
    int32 value() const {
        return m_value.load();
    }

    /** Returns the old value, before the add. */
    int32 add(const int32 x) {
            int32 old = m_value.load();
            m_value += x;
            return old;
    }

    /** Returns old value. */
    int32 sub(const int32 x) {
        return add(-x);
    }

    void increment() {
        m_value++;
    }

    /** Returns zero if the result is zero after decrement, non-zero otherwise.*/
    int32 decrement() {
        return --m_value;
    }


    /** Atomic test-and-set:  if <code>*this == comperand</code> then <code>*this := exchange</code> else do nothing.
        In both cases, returns the old value of <code>*this</code>.

        Performs an atomic comparison of this with the Comperand value.
        If this is equal to the Comperand value, the Exchange value is stored in this.
        Otherwise, no operation is performed.

        Under VC6 the sign bit may be lost.
     */
    int32 compareAndSet(int32 comperand, const int32 exchange) {
        int32 oldValue = m_value.load();
        m_value.compare_exchange_strong(comperand, exchange);
        return oldValue;
    }

};

} // namespace

#endif
