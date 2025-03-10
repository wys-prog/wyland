
%macro add_values A, B
    dq $A
    dd $B
%endmacro

start:
  add_values 89, 90