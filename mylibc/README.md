I want to use `NULL` across various files without having to `#define` it
every time I want to use it. Placing the `#define` in a header in this
directory is my solution.


* [] Is there a better place for `NULL`?
* [] What is the point of defining `NULL` as `((void *) 0)` vs. `(0)`?