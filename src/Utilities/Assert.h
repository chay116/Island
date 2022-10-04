//
//  Assert.h
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

#ifndef Assert_h
#define Assert_h

#include <Foundation/NSError.hpp>
#include <Foundation/NSString.hpp>
#include <iostream>

#define CLASS_PTR(klassName) \
class klassName; \
using klassName ## UPtr = std::unique_ptr<klassName>; \
using klassName ## SPtr = std::shared_ptr<klassName>; \
using klassName ## WPtr = std::weak_ptr<klassName>;

static inline void CHA_PRINT()
{
    std::cout << std::endl;
}

template< typename T, typename... Args >
static void CHA_PRINT( const T& value, Args&&... args )
{
    std::cout << value << " ";
    CHA_PRINT( args... );
}

template< typename... Args >
void CHA_ASSERT( bool condition, Args&&... args )
{
    if ( !condition )
    {
        CHA_PRINT( args... );
        __builtin_trap();
    }
}

template< typename... Args >
void CHA_ASSERT_NULL_ERROR( NS::Error* pError, Args&&... args )
{
    if ( pError )
    {
        CHA_PRINT( args..., pError->localizedDescription()->utf8String() );
        __builtin_trap();
    }
}

template< typename... Args >
void CHA_ASSERT( NS::Error* pError, Args&&... args ) = delete;

#define CHASTR( str ) reinterpret_cast< const NS::String* >(__builtin___CFStringMakeConstantString( str ))

#endif /* Assert_h */
