# Hunt Configuration Guide

## C Standards, Compilers and Microcontrollers

The embedded software world contains its challenges. Compilers support different
revisions of the C Standard. They ignore requirements in places, sometimes to
make the language more usable in some special regard. Sometimes it's to simplify
their support. Sometimes it's due to specific quirks of the microcontroller they
are targeting. Simulators add another dimension to this menagerie.

Hunt is designed to run on almost anything that is targeted by a C compiler. It
would be awesome if this could be done with zero configuration. While there are
some targets that come close to this dream, it is sadly not universal. It is
likely that you are going to need at least a couple of the configuration options
described in this document.

All of Hunt's configuration options are `#defines`. Most of these are simple
definitions. A couple are macros with arguments. They live inside the
hunt_internals.h header file. We don't necessarily recommend opening that file
unless you really need to. That file is proof that a cross-platform library is
challenging to build. From a more positive perspective, it is also proof that a
great deal of complexity can be centralized primarily to one place to
provide a more consistent and simple experience elsewhere.


### Using These Options

It doesn't matter if you're using a target-specific compiler and a simulator or
a native compiler. In either case, you've got a couple choices for configuring
these options:

1. Because these options are specified via C defines, you can pass most of these
options to your compiler through command line compiler flags. Even if you're
using an embedded target that forces you to use their overbearing IDE for all
configuration, there will be a place somewhere in your project to configure
defines for your compiler.
2. You can create a custom `hunt_config.h` configuration file (present in your
toolchain's search paths). In this file, you will list definitions and macros
specific to your target. All you must do is define `HUNT_INCLUDE_CONFIG_H` and
Hunt will rely on `hunt_config.h` for any further definitions it may need.


## The Options

### Integer Types

If you've been a C developer for long, you probably already know that C's
concept of an integer varies from target to target. The C Standard has rules
about the `int` matching the register size of the target microprocessor. It has
rules about the `int` and how its size relates to other integer types. An `int`
on one target might be 16 bits while on another target it might be 64. There are
more specific types in compilers compliant with C99 or later, but that's
certainly not every compiler you are likely to encounter. Therefore, Hunt has a
number of features for helping to adjust itself to match your required integer
sizes. It starts off by trying to do it automatically.


##### `HUNT_EXCLUDE_STDINT_H`

The first thing that Hunt does to guess your types is check `stdint.h`.
This file includes defines like `UINT_MAX` that Hunt can use to
learn a lot about your system. It's possible you don't want it to do this
(um. why not?) or (more likely) it's possible that your system doesn't
support `stdint.h`. If that's the case, you're going to want to define this.
That way, Hunt will know to skip the inclusion of this file and you won't
be left with a compiler error.

_Example:_
```C
#define HUNT_EXCLUDE_STDINT_H
```


##### `HUNT_EXCLUDE_LIMITS_H`

The second attempt to guess your types is to check `limits.h`. Some compilers
that don't support `stdint.h` could include `limits.h` instead. If you don't
want Hunt to check this file either, define this to make it skip the inclusion.

_Example:_
```C
#define HUNT_EXCLUDE_LIMITS_H
```

If you've disabled both of the automatic options above, you're going to have to
do the configuration yourself. Don't worry. Even this isn't too bad... there are
just a handful of defines that you are going to specify if you don't like the
defaults.


##### `HUNT_INT_WIDTH`

Define this to be the number of bits an `int` takes up on your system. The
default, if not autodetected, is 32 bits.

_Example:_
```C
#define HUNT_INT_WIDTH 16
```


##### `HUNT_LONG_WIDTH`

Define this to be the number of bits a `long` takes up on your system. The
default, if not autodetected, is 32 bits. This is used to figure out what kind
of 64-bit support your system can handle. Does it need to specify a `long` or a
`long long` to get a 64-bit value. On 16-bit systems, this option is going to be
ignored.

_Example:_
```C
#define HUNT_LONG_WIDTH 16
```


##### `HUNT_POINTER_WIDTH`

Define this to be the number of bits a pointer takes up on your system. The
default, if not autodetected, is 32-bits. If you're getting ugly compiler
warnings about casting from pointers, this is the one to look at.

_Hint:_ In order to support exotic processors (for example TI C55x with a pointer 
width of 23-bit), choose the next power of two (in this case 32-bit).

_Supported values:_ 16, 32 and 64 

_Example:_
```C
// Choose on of these #defines to set your pointer width (if not autodetected)
//#define HUNT_POINTER_WIDTH 16
//#define HUNT_POINTER_WIDTH 32
#define HUNT_POINTER_WIDTH 64 // Set HUNT_POINTER_WIDTH to 64-bit
```


##### `HUNT_SUPPORT_64`

Hunt will automatically include 64-bit support if it auto-detects it, or if
your `int`, `long`, or pointer widths are greater than 32-bits. Define this to
enable 64-bit support if none of the other options already did it for you. There
can be a significant size and speed impact to enabling 64-bit support on small
targets, so don't define it if you don't need it.

_Example:_
```C
#define HUNT_SUPPORT_64
```


### Floating Point Types

In the embedded world, it's not uncommon for targets to have no support for
floating point operations at all or to have support that is limited to only
single precision. We are able to guess integer sizes on the fly because integers
are always available in at least one size. Floating point, on the other hand, is
sometimes not available at all. Trying to include `float.h` on these platforms
would result in an error. This leaves manual configuration as the only option.


##### `HUNT_INCLUDE_FLOAT`

##### `HUNT_EXCLUDE_FLOAT`

##### `HUNT_INCLUDE_DOUBLE`

##### `HUNT_EXCLUDE_DOUBLE`

By default, Hunt guesses that you will want single precision floating point
support, but not double precision. It's easy to change either of these using the
include and exclude options here. You may include neither, either, or both, as
suits your needs. For features that are enabled, the following floating point
options also become available.

_Example:_
```C
//what manner of strange processor is this?
#define HUNT_EXCLUDE_FLOAT
#define HUNT_INCLUDE_DOUBLE
```


##### `HUNT_EXCLUDE_FLOAT_PRINT`

Hunt aims for as small of a footprint as possible and avoids most standard
library calls (some embedded platforms don’t have a standard library!). Because
of this, its routines for printing integer values are minimalist and hand-coded.
Therefore, the display of floating point values during a failure are optional.
By default, Hunt will print the actual results of floating point assertion
failure (e.g. ”Expected 4.56 Was 4.68”). To not include this extra support, you
can use this define to instead respond to a failed assertion with a message like
”Values Not Within Delta”. If you would like verbose failure messages for floating
point assertions, use these options to give more explicit failure messages.

_Example:_
```C
#define HUNT_EXCLUDE_FLOAT_PRINT
```


##### `HUNT_FLOAT_TYPE`

If enabled, Hunt assumes you want your `FLOAT` asserts to compare standard C
floats. If your compiler supports a specialty floating point type, you can
always override this behavior by using this definition.

_Example:_
```C
#define HUNT_FLOAT_TYPE float16_t
```


##### `HUNT_DOUBLE_TYPE`

If enabled, Hunt assumes you want your `DOUBLE` asserts to compare standard C
doubles. If you would like to change this, you can specify something else by
using this option. For example, defining `HUNT_DOUBLE_TYPE` to `long double`
could enable gargantuan floating point types on your 64-bit processor instead of
the standard `double`.

_Example:_
```C
#define HUNT_DOUBLE_TYPE long double
```


##### `HUNT_FLOAT_PRECISION`

##### `HUNT_DOUBLE_PRECISION`

If you look up `HUNT_ASSERT_EQUAL_FLOAT` and `HUNT_ASSERT_EQUAL_DOUBLE` as
documented in the big daddy Hunt Assertion Guide, you will learn that they are
not really asserting that two values are equal but rather that two values are
"close enough" to equal. "Close enough" is controlled by these precision
configuration options. If you are working with 32-bit floats and/or 64-bit
doubles (the normal on most processors), you should have no need to change these
options. They are both set to give you approximately 1 significant bit in either
direction. The float precision is 0.00001 while the double is 10-12.
For further details on how this works, see the appendix of the Hunt Assertion
Guide.

_Example:_
```C
#define HUNT_FLOAT_PRECISION 0.001f
```


### Miscellaneous

##### `HUNT_EXCLUDE_STDDEF_H`

Hunt uses the `NULL` macro, which defines the value of a null pointer constant,
defined in `stddef.h` by default. If you want to provide
your own macro for this, you should exclude the `stddef.h` header file by adding this
define to your configuration.

_Example:_
```C
#define HUNT_EXCLUDE_STDDEF_H
```


#### `HUNT_INCLUDE_PRINT_FORMATTED`

Hunt provides a simple (and very basic) printf-like string output implementation,
which is able to print a string modified by the following format string modifiers:

- __%d__ - signed value (decimal)
- __%i__ - same as __%i__
- __%u__ - unsigned value (decimal)
- __%f__ - float/Double (if float support is activated)
- __%g__ - same as __%f__
- __%b__ - binary prefixed with "0b"
- __%x__ - hexadecimal (upper case) prefixed with "0x"
- __%X__ - same as __%x__
- __%p__ - pointer (same as __%x__ or __%X__)
- __%c__ - a single character
- __%s__ - a string (e.g. "string")
- __%%__ - The "%" symbol (escaped)

_Example:_
```C
#define HUNT_INCLUDE_PRINT_FORMATTED

int a = 0xfab1;
HuntPrintFormatted("Decimal   %d\n", -7);
HuntPrintFormatted("Unsigned  %u\n", 987);
HuntPrintFormatted("Float     %f\n", 3.1415926535897932384);
HuntPrintFormatted("Binary    %b\n", 0xA);
HuntPrintFormatted("Hex       %X\n", 0xFAB);
HuntPrintFormatted("Pointer   %p\n", &a);
HuntPrintFormatted("Character %c\n", 'F');
HuntPrintFormatted("String    %s\n", "My string");
HuntPrintFormatted("Percent   %%\n");
HuntPrintFormatted("Color Red \033[41mFAIL\033[00m\n");
HuntPrintFormatted("\n");
HuntPrintFormatted("Multiple (%d) (%i) (%u) (%x)\n", -100, 0, 200, 0x12345);
```


### Toolset Customization

In addition to the options listed above, there are a number of other options
which will come in handy to customize Hunt's behavior for your specific
toolchain. It is possible that you may not need to touch any of these... but
certain platforms, particularly those running in simulators, may need to jump
through extra hoops to run properly. These macros will help in those
situations.


##### `HUNT_OUTPUT_CHAR(a)`

##### `HUNT_OUTPUT_FLUSH()`

##### `HUNT_OUTPUT_START()`

##### `HUNT_OUTPUT_COMPLETE()`

By default, Hunt prints its results to `stdout` as it runs. This works
perfectly fine in most situations where you are using a native compiler for
testing. It works on some simulators as well so long as they have `stdout`
routed back to the command line. There are times, however, where the simulator
will lack support for dumping results or you will want to route results
elsewhere for other reasons. In these cases, you should define the
`HUNT_OUTPUT_CHAR` macro. This macro accepts a single character at a time (as
an `int`, since this is the parameter type of the standard C `putchar` function
most commonly used). You may replace this with whatever function call you like.

_Example:_
Say you are forced to run your test suite on an embedded processor with no
`stdout` option. You decide to route your test result output to a custom serial
`RS232_putc()` function you wrote like thus:
```C
#include "RS232_header.h"
...
#define HUNT_OUTPUT_CHAR(a)    RS232_putc(a)
#define HUNT_OUTPUT_START()    RS232_config(115200,1,8,0)
#define HUNT_OUTPUT_FLUSH()    RS232_flush()
#define HUNT_OUTPUT_COMPLETE() RS232_close()
```

_Note:_
`HUNT_OUTPUT_FLUSH()` can be set to the standard out flush function simply by
specifying `HUNT_USE_FLUSH_STDOUT`. No other defines are required.


##### `HUNT_WEAK_ATTRIBUTE`

##### `HUNT_WEAK_PRAGMA`

##### `HUNT_NO_WEAK`

For some targets, Hunt can make the otherwise required setUp() and tearDown()
functions optional. This is a nice convenience for test writers since setUp and
tearDown don’t often actually do anything. If you’re using gcc or clang, this
option is automatically defined for you. Other compilers can also support this
behavior, if they support a C feature called weak functions. A weak function is
a function that is compiled into your executable unless a non-weak version of
the same function is defined elsewhere. If a non-weak version is found, the weak
version is ignored as if it never existed. If your compiler supports this feature,
you can let Hunt know by defining HUNT_WEAK_ATTRIBUTE or HUNT_WEAK_PRAGMA as
the function attributes that would need to be applied to identify a function as
weak. If your compiler lacks support for weak functions, you will always need to
define setUp and tearDown functions (though they can be and often will be just
empty). You can also force Hunt to NOT use weak functions by defining
HUNT_NO_WEAK. The most common options for this feature are:

_Example:_
```C
#define HUNT_WEAK_ATTRIBUTE weak
#define HUNT_WEAK_ATTRIBUTE __attribute__((weak))
#define HUNT_WEAK_PRAGMA
#define HUNT_NO_WEAK
```


##### `HUNT_PTR_ATTRIBUTE`

Some compilers require a custom attribute to be assigned to pointers, like
`near` or `far`. In these cases, you can give Hunt a safe default for these by
defining this option with the attribute you would like.

_Example:_
```C
#define HUNT_PTR_ATTRIBUTE __attribute__((far))
#define HUNT_PTR_ATTRIBUTE near
```

##### `HUNT_PRINT_EOL`

By default, Hunt outputs \n at the end of each line of output. This is easy
to parse by the scripts, by Ceedling, etc, but it might not be ideal for YOUR
system. Feel free to override this and to make it whatever you wish.

_Example:_
```C
#define HUNT_PRINT_EOL { HUNT_OUTPUT_CHAR('\r'); HUNT_OUTPUT_CHAR('\n') }
```


##### `HUNT_EXCLUDE_DETAILS`

This is an option for if you absolutely must squeeze every byte of memory out of
your system. Hunt stores a set of internal scratchpads which are used to pass
extra detail information around. It's used by systems like CMock in order to
report which function or argument flagged an error. If you're not using CMock and
you're not using these details for other things, then you can exclude them.

_Example:_
```C
#define HUNT_EXCLUDE_DETAILS
```


##### `HUNT_EXCLUDE_SETJMP`

If your embedded system doesn't support the standard library setjmp, you can
exclude Hunt's reliance on this by using this define. This dropped dependence
comes at a price, though. You will be unable to use custom helper functions for
your tests, and you will be unable to use tools like CMock. Very likely, if your
compiler doesn't support setjmp, you wouldn't have had the memory space for those
things anyway, though... so this option exists for those situations.

_Example:_
```C
#define HUNT_EXCLUDE_SETJMP
```

##### `HUNT_OUTPUT_COLOR`

If you want to add color using ANSI escape codes you can use this define.

_Example:_
```C
#define HUNT_OUTPUT_COLOR
```

##### `HUNT_SHORTHAND_AS_INT`
##### `HUNT_SHORTHAND_AS_MEM`
##### `HUNT_SHORTHAND_AS_RAW`
##### `HUNT_SHORTHAND_AS_NONE`

These options  give you control of the `TEST_ASSERT_EQUAL` and the
`TEST_ASSERT_NOT_EQUAL` shorthand assertions. Historically, Hunt treated the
former as an alias for an integer comparison. It treated the latter as a direct
comparison using `!=`. This assymetry was confusing, but there was much
disagreement as to how best to treat this pair of assertions. These four options
will allow you to specify how Hunt will treat these assertions.

  - AS INT - the values will be cast to integers and directly compared. Arguments
             that don't cast easily to integers will cause compiler errors.
  - AS MEM - the address of both values will be taken and the entire object's
             memory footprint will be compared byte by byte. Directly placing
             constant numbers like `456` as expected values will cause errors.
  - AS_RAW - Hunt assumes that you can compare the two values using `==` and `!=`
             and will do so. No details are given about mismatches, because it
             doesn't really know what type it's dealing with.
  - AS_NONE - Hunt will disallow the use of these shorthand macros altogether,
             insisting that developers choose a more descriptive option.

## Getting Into The Guts

There will be cases where the options above aren't quite going to get everything
perfect. They are likely sufficient for any situation where you are compiling
and executing your tests with a native toolchain (e.g. clang on Mac). These
options may even get you through the majority of cases encountered in working
with a target simulator run from your local command line. But especially if you
must run your test suite on your target hardware, your Hunt configuration will
require special help. This special help will usually reside in one of two
places: the `main()` function or the `RUN_TEST` macro. Let's look at how these
work.


##### `main()`

Each test module is compiled and run on its own, separate from the other test
files in your project. Each test file, therefore, has a `main` function. This
`main` function will need to contain whatever code is necessary to initialize
your system to a workable state. This is particularly true for situations where
you must set up a memory map or initialize a communication channel for the
output of your test results.

A simple main function looks something like this:

```C
int main(void) {
    HUNT_BEGIN();
    RUN_TEST(test_TheFirst);
    RUN_TEST(test_TheSecond);
    RUN_TEST(test_TheThird);
    return HUNT_END();
}
```

You can see that our main function doesn't bother taking any arguments. For our
most barebones case, we'll never have arguments because we just run all the
tests each time. Instead, we start by calling `HUNT_BEGIN`. We run each test
(in whatever order we wish). Finally, we call `HUNT_END`, returning its return
value (which is the total number of failures).

It should be easy to see that you can add code before any test cases are run or
after all the test cases have completed. This allows you to do any needed
system-wide setup or teardown that might be required for your special
circumstances.


##### `RUN_TEST`

The `RUN_TEST` macro is called with each test case function. Its job is to
perform whatever setup and teardown is necessary for executing a single test
case function. This includes catching failures, calling the test module's
`setUp()` and `tearDown()` functions, and calling `HuntConcludeTest()`. If
using CMock or test coverage, there will be additional stubs in use here. A
simple minimalist RUN_TEST macro looks something like this:

```C
#define RUN_TEST(testfunc) \
    HUNT_NEW_TEST(#testfunc) \
    if (TEST_PROTECT()) { \
        setUp(); \
        testfunc(); \
    } \
    if (TEST_PROTECT() && (!TEST_IS_IGNORED)) \
        tearDown(); \
    HuntConcludeTest();
```

So that's quite a macro, huh? It gives you a glimpse of what kind of stuff Hunt
has to deal with for every single test case. For each test case, we declare that
it is a new test. Then we run `setUp` and our test function. These are run
within a `TEST_PROTECT` block, the function of which is to handle failures that
occur during the test. Then, assuming our test is still running and hasn't been
ignored, we run `tearDown`. No matter what, our last step is to conclude this
test before moving on to the next.

Let's say you need to add a call to `fsync` to force all of your output data to
flush to a file after each test. You could easily insert this after your
`HuntConcludeTest` call. Maybe you want to write an xml tag before and after
each result set. Again, you could do this by adding lines to this macro. Updates
to this macro are for the occasions when you need an action before or after
every single test case throughout your entire suite of tests.


## Happy Porting

The defines and macros in this guide should help you port Hunt to just about
any C target we can imagine. If you run into a snag or two, don't be afraid of
asking for help on the forums. We love a good challenge!


*Find The Latest of This And More at [ThrowTheSwitch.org](https://throwtheswitch.org)*
