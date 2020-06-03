# Result&lt;T, E&gt;

## Overview ##
C++17 version of [Rust's Result](https://doc.rust-lang.org/std/result/enum.Result.html).
The implementation is motivated by approach used in [Oktal's Result](https://github.com/oktal/result).


## Supported operations ##
the examples how to use the supported operations are placed in result_test.cpp file in the test subdirectory.

| Operation          | Description                                     |
| :---               | :---:                                           |
|is_ok               |returns true if the result is Ok                 |
|is_err              |returns true if the result is Err                |
|ok                  |converts from Result&lt;T, E&gt; to std&colon;&colon;optional&lt;T&gt; |
|err                 |converts from Result&lt;T, E&gt; to std&colon;&colon;optional&lt;E&gt; |
|expect              |returns the contained Ok value                   |
|expect_err          |returns the contained Err value                  |
|map                 |maps a Result&lt;T, E&gt; to Result&lt;U, E&gt; by applying a function to a contained Ok value, leaving an Err value untouched |
|map_err             |maps a Result&lt;T, E&gt; to Result&lt;T, F&gt; by applying a function to a contained Err value, leaving an Ok value untouched |
|map_or              |applies a function to the contained value (if any), or returns the provided default (if not) |
|map_or_else         |maps a Result&lt;T, E&gt; to U by applying a function to a contained Ok value, or a fallback function to a contained Err value |
|and (as operator&amp;&amp;) |returns res if the result is Ok, otherwise returns the Err value |
|and_then            |calls op if the result is Ok, otherwise returns the Err value |
|or (as operator&verbar;&verbar;) | returns res if the result is Err, otherwise returns the Ok value |
|or_else             |calls op if the result is Err, otherwise returns the Ok value |
|unwrap              |returns the contained Ok value |
|unwrap_err          |returns the contained Err value |
|unwrap_or           |returns the contained Ok value or a provided default |
|unwrap_or_default   |returns the contained Ok value or a default |
|unwrap_or_else      |returns the contained Ok value or computes it from a closure |
|contains            |returns true if the result is an Ok value containing the given value |
|contains_err        |returns true if the result is an Err value containing the given value |
|transpose           |transposes a Result&lt;std&colon;&colon;optional&lt;T&gt;, E&gt; into a std&colon;&colon;optional&lt;Result&lt;T, E&gt;&gt;. Ok(std&colon;&colon;nullopt) will be mapped to std&colon;&colon;nullopt |
|equality/inequality operators | compares the euality/inequality of two Results or Result and Err or Result and Ok |





