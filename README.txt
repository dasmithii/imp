Overview

    Imp is a little scripting language I'm building for fun. It 
    is dynamic, duck-typed, and optimized for prototypal 
    programming.

      >  (import 'core/io')
      >  (io:writeLine 'Hello, world!')




Tutorial

                         Functions

    Above, we import the standard 'io' module and call its 
    'writeLine' function on a string. Resulting is an instance
    of software's greatest cliché.

    The following produces identical output:

      >  (import 'core/io')
      >  (def sayHello {io:writeLine 'Hello, World!'})
      >  (sayHello)

    This is because, in imp, curly braces define code blocks, 
    and because our code block contains only one statement, 
    parenthesis are implied. 



                       Object Literals

    Arbitrary objects are formed by surrounding key-value
    pairs with hard brackets:

      >  (def someObject [
             field1 'I am a field.'
             field2 'No you are not!'
             field3 'Shut it, you two.'
         ])

    Note that nesting is allowed:

      >  (def anotherObject [
             nestedObject [
                 field 'I am a field'
             ]
         ])



                      The Object System

    Imp is dynamic and duck-typed. There are no compile-time
    distinctions between objects, which contain arbitrary
    string keys that map to nested objects (except in the case
    of built-in data).

    Object fields are accessible via the colon operator and are
    most conveniently managed with the built-in 'def', 'set', 
    and 'rem' functions. 'rem' is rarely necessary, though.

      >  (def a 2)
      >  (set a 3)
      >  (rem a)

      >  (def myObject:mySlot 'hello')
      >  (rem myObject)

    Exceptions are thrown when code attempts to access fields
    and methods that do not exist. For example:

      >  (def myObject [])         # Create new object.
      >  (myObject:nonExistentMethod) # Error!

    Methods are defined and called as follows:

      >  (def myObject [greeting 'Hi,'])
      >  (def myObject:greet {
            (io:writeLine self:greeting (@:get 0) '!')
         })
      >  (myObject:greet)

    The 'self' variable is injected into scope. Likewise is
    the '@' arguments variable, an array of arbitrary
    length.

    The symbols '#' and '~' stand for "base object" and
    "clone", respectively. The former contains all sorts of
    functions built in to the Imp interpreter. And the latter
    is a primitive operation in prototype-oriented programming.
    You will see it often.

    A select few symbols are used to represent common
    operations. Besides '#' and '~', there is '$', for example,
    which can be read as "value" or "copy", and because all Imp
    objects are reference types, (obj:$) is often necessary.




Installation

    Note: Windows is not supported. Linux probably isn't
    either. I've only tested on my Mac thus far.
    
    1. Clone this repository.
    2. Enter root directory.
    3. `make release`
    4. `make install`

    `imp` will now be available in your command line.




Status

    Despite being slow and in need of space-optimization, the
    interpreter is functional. It is also extensible (via .c
    file imports). However, this early stage leaves us with a
    scarcity of modules. Therefor many expectedly-trivial
    Imp programs will require programmers to write C code. 

    What we HAVE: garbage collection, control structures,
    object literals, function blocks, function calls, variable
    definitions, an I/O library, some basic data structures,
    string manipulation functions, math stuff. Check /core for
    the standard library.

    What we DON'T have: networking, graphics, and mostly
    everything else.




Etymology 

    The word "imp" is defined as "a mischievous child" and 
    doubles as the acronym: I Make Programs. It is also a 
    substring of "simple". That's the best I could do.