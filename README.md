# Toy Language

## The Language

This is toy language with following **functionalities**:

- Tensor-based language that allows you to define functions
- Perform math computations
- Output results of the computations

It has also following **limitations**:

- Limited to tensor of rank <= 2
- Only datatype is 64-bit floating point

And also it has following **properties**:

- All `Values` are immutable
- Deallocation is automatically managed

## Examples

Following is an example code that is executable by toy language:

```toy
def main() {
    # Define a variable `a` with shape <2,3>, initialized with the literal value.
    # The shape is **inferred** from the supplied value
    var a = [[1, 2, 3], [4, 5, 6]];

    # b is identical to a, the literal tensor is implicitly reshaped: defining new
    # variables is the way to reshape tensors (element count must match);
    var b<2,3> = [1,2,3,4,5,6];
    print(transpose(a)*transpose(b));
}
```

## Functions

Functions are generic: their parameters are unranked. In other words, we know they are tensors but we do not know their dimensions. We can also add function to our above example:

```toy
# User defined generic function that operates on unknown shaped arguments.
def multiply_transpose(a, b) {
  return transpose(a) * transpose(b);
}

def main() {
  # Define a variable `a` with shape <2, 3>, initialized with the literal value.
  var a = [[1, 2, 3], [4, 5, 6]];
  var b<2, 3> = [1, 2, 3, 4, 5, 6];

  # This call will specialize `multiply_transpose` with <2, 3> for both
  # arguments and deduce a return type of <3, 2> in initialization of `c`.
  var c = multiply_transpose(a, b);

  # A second call to `multiply_transpose` with <2, 3> for both arguments will
  # reuse the previously specialized and inferred version and return <3, 2>.
  var d = multiply_transpose(b, a);

  # A new call with <3, 2> (instead of <2, 3>) for both dimensions will
  # trigger another specialization of `multiply_transpose`.
  var e = multiply_transpose(c, d);

  # Finally, calling into `multiply_transpose` with incompatible shapes
  # (<2, 3> and <3, 2>) will trigger a shape inference error.
  var f = multiply_transpose(a, c);
}
```
