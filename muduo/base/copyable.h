#ifndef MUDUO_BASE_COPYABLE_H
#define MUDUO_BASE_COPYABLE_H

namespace muduo {
    
    // Tag class to emphasis the objects are copyable.
    class copyable {
    protected:
        copyable() = default;
        ~copyable() = default;
    };
}

#endif