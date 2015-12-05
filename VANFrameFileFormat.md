This page describes the format that's used in van-frame.txt
to describe the frame's function.

The file itself is available in the SVN.

Each line is formatted as:
iden:[filter](filter.md):range:mask:type:[expression](expression.md):description

Leading and trailing spaces in the description field will be removed by the parsing software.

  * iden
    * The iden you want to match on

  * filter
    * A filter specification to match against frame content

  * range
    * A
    * A-
    * A-B

  * type
    * iden
    * ascii
    * int
    * assert
    * bcd
    * cmp
    * bit

  * expression
    * with types int, assert and bcd, this field will be evaluated before presenting the result to the user. Any X in the field will be substituted with the current field's value.