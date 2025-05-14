// gcc -S -m32 stackFrames.c

// no stack allocation... for local variables
// => the parameters are not considered while 
// allocating stack! As these are already
// taken care off by the caller procedure
int bar(int a, int b) {
  a = b * a;
  return a;
}

void foo() {
  int x = 10;
  bar(x, 20);
  return;
}

struct MyStruct {
  int x;
  int y;
  int z;
};

struct MyStruct baz() {
  struct MyStruct s;
  s.x = 10;
  s.y = 20;
  s.z = 30;

  // This is so stupid in the asm
  return s;
}

int main() {
  int a = 10;
  a = bar(a, 10);
}