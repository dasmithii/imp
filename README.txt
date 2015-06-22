Overview

    Imp is a little scripting language I'm building for fun. It 
    is dynamic, duck-typed, and optimized for prototypal 
    programming. Its syntax is characterized by prefix notation
    and heavy use of grouping operators.

      >  (import 'core/io')
      >  (io:writeLine 'Hello, world!')




Tutorial

                        Grouping Operators

    Above, parentheses activate the printer object with a
    single argument. Resulting is an instance of software's
    greatest cliché.

    The following produces identical output:

      >  (io:writeLine [{(return 'Hello, world')}])

    This is because, in imp, parentheses are function calls;
    curly braces define code blocks; and hard brackets expand
    macros. The macro expands to 'Hello, world', and thus we
    have (io:writeLine 'Hello, world').

    Take note of the similarity between function calls and
    macro expansions. Each requires one activate-able object
    and may accept arbitrary arguments. Accordingly, activate-
    able objects can be used for either purpose. There is no
    difference in semantics between macro code and function
    code, as there is in C.



                      The Object System

    Imp is dynamic and duck-typed (meaning that there are no
    compile-time distinctions between object types, and that,
    instead, objects are permitted wherever they are capable of
    going). So, essentially, there exists only one type: the
    object, which contains arbitrary fields that are themselves 
    nested objects (except in the case of built-in data).

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

      >  (def myObject (#:~))         # Create new object.
      >  (myObject:nonExistentMethod) # Error!

    Methods are defined and called as follows:

      >  (def myObject (#:~))
      >  (def myObject:greeting 'Hi,')
      >  (def myObject:greet {
            (io:writeLine self:greeting (@:get 0) '!')
         })
      >  (myObject:greet)

    The 'self' variable is injected into block scope. Likewise
    is the '@' arguments variable, an array of arbitrary
    length.

    The symbols '#' and '~' stand for "base object" and
    "clone", respectively. The former contains all sorts of
    functions
    built in to the Imp interpreter. And the latter is a 
    primitive operation in prototype-oriented programming. You
    will see it often.

    A select few symbols are used to represent common
    operations. Besides '#' and '~', there is '$', for example,
    which can be read as "value". It is short for 'copy'. And
    because all Imp objects are reference objects, (obj:$) is
    often necessary.



                 Special and Internal Slots

    Where many languages have reserved keywords, imp has a
    reserved keyword space, which comes in two parts.

    Anything prefixed with a single underscore is considered
    'special' and should be handled with care. The _collect
    method, for example, is activated upon garbage collection
    of the specific object. _mark is activated during the mark
    phase of mark & sweep garbage collection. Then their is
    _each and _begin and others. The interpreter and standard
    library have the power to access and activate these 
    special slots, so don't make slots special unless you 
    intend on capitalizing on something like a foreach loop,
    iterators, or activat-able objects.
    
    Double underscore prefixes are indicative of internal 
    slots which, in function, are no different than special
    slots. However their is a key difference: internal slots
    store internal data, rather than objects. Accordingly, one
    cannot access them from within the Imp interpreter. Only C
    functions can access them. Regardless of access, though,
    all slot names of an object can be listed as follows:

      >  (io:writeLine (someObject:slotNames))
    



Object Activation

    The process of object activation, whether in a function call
    or macro expansion, is as follows:

      1. If object has an '_activate' field, that field is
         executed as a method on its parent object.

      2. If object has an '__activate' (double underscore)
         internal field, it is assumed that this field points to
         a C function, which is executed with the arguments: (1)
         runtime, (2) context, (3) calling object, (4) argc, and
         (5) argv.

      3. An error is thrown. Object cannot be activated.
 



Disclaimer

    Note that Imp remains in early-stage development. The
    language, though powerful, is incomplete and quite rough
    around the edges. There are many bugs; documentation is
    scarce; and API changes ought to be expected. Accordingly, 
    serious projects should choose a more mature language to
    work with. No guarantee here (see LICENSE.txt).




Manifesto 

    The word "imp" is defined as "a mischievous child" and 
    doubles as the acronym: I Make Programs. It is also a 
    substring of "simple". 

    I hope that users of Imp will write code with this etymology
    in mind. Mess with language internals if you'd like. And
    please, don't worry about breaking things, because to hyper-
    focus on "idioms" and "best practices" is to abandon your 
    potential as as programmer. To look to others for hints is
    fine, but intellectual dependence is no good. So think like
    a child and play around. Code instinctively. Experiment!
    That's how learning is done. Things are more fun that way,
    too.

    Mistakes are inevitably made, of course. However, if you
    trust yourself, eventually you may deserve to be trusted.
    Contrarily, self-doubt brings about nothing but hesitance 
    and mediocrity. And so, like a young bird, you must 
    take a faithful leap. Only in the hands of gravity will you
    realize your true potential - flight (or not). I encourage
    you to take your leap if you haven't already.



