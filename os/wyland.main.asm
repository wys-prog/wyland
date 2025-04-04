@wyland.string.hello: .ascii "wyland" 0x0A 0x00

@wyland_start:
  # prepare call for "strlen"
  # wyland.strlen(&string.hello)
  lea qmm0 %wyland.string.hello
  lea qmm15 @calc: %here + 10
  jmp wyland.strlen

  