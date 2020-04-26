# integral_io

This is a small header-only library which helps ensure C++ streams read/write integers of all sizes
consistently. Commonly, 1-byte integers are handled as characters, while all other integers are
handled as numbers. Using this library, you can make sure they are all handled as numbers.

This library has a permissive licence (MIT). You are free to use it in whatever way you like,
including in closed-source / commerical projects.


## Quick example
To try out this library, simply copy the header file into your project. You can then use it like
this:

```c++
#include "integral_io.hpp"
#include <iostream>

using integral_io::as_integer;

int main()
{
    std::int8_t value;
    std::cout << "Enter a number: ";
    std::cin >> as_integer(value);
    std::cout << "The number was: " << as_integer(value) << std::endl;
    return 0;
}
```

The main public interface of this library is the `as_integer()` function. It will take any integer
you give it and ensure the stream treats it as a number (not a character), no matter what size it
is. It should work for input and output using any standard streams, including `cin`, `cout`,
`fstream`, and `stringstream`.

The example above is quite simple and contrived. In practice, I expect it will be more useful in
situations where the integer type is a template parameter. In these cases, it prevents the need to
write your own conditional logic for single-byte types.


## C++ version
This library requires C++11 or later.


## Background
### The problem I wanted to solve
Consider the following simple C++ program:

```c++
#include <iostream>

int main()
{
    std::int8_t value1 = 42;
    std::int16_t value2 = 42;
    std::cout << value1 << " " << value2;
    return 0;
}
```

At first glance, it looks like it should print "`42 42`" to the console. However, it actually prints
"`* 42`" on many compilers.

The reason is that the 1-byte integer (`value1`) is treated as a character not a number. It renders
an asterisk (`*`) because that is the ASCII/UTF-8 character with value 42. Meanwhile, the 2-byte
integer is rendered as the number we'd expect.

A similar problem exists when reading from an input stream. When reading into a 1-byte type, the
stream will not parse the number. Instead, it will read a single character and store its ASCII/UTF-8
code.

This library aims to provide a readable way to resolve this inconsistency.

### Why does the problem exist?
The C++ standard does not require compilers to distinguish between 1-byte integers and characters.
In many cases (perhaps most), all 1-byte integers are aliases of an equivalent `char` type (i.e. a
1-byte character). This means the stream operators cannot tell them apart so they prefer the
well-known character insertion/extraction procedures.

It's an annoying quirk, but we're probably stuck with it. Changing it now would likely cause a lot
of existing code to break. Additionally, streams are likely to be somewhat superseded by
[format strings in C++20](https://en.cppreference.com/w/cpp/utility/format/format) anyway.

### Aren't there easier ways to do this?
Yes. If you know the limits of your integer when you're writing your code then you can make it much
simpler. A static cast and (in the case of input) a temporary variable are all you need. (Or just
don't use streams in the first place.)

The complexity of this library's implementation therefore *massively* outweighs the scale of the
problem. It's really more of an interesting exercise than something I'd expect anyone to use for
anything serious.


## TODO list

* Implement unit tests.
* Automatically build/test on every push.
* Ensure code only uses C++11 features if possible.
* Ensure wide streams work correctly.
* Ensure code works on a variety of compilers/platforms.
* Avoid doing anything if the compiler treats 1-byte integers as distinct from chars.
