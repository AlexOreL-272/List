#include "tests.cpp"

int main() {
  OTHER();
  STATIC_ASSERTS();
  CONSTRUCTOR();
  OPERATORS();
  BASIC_FUNC();
  PROPAGATE(); // TODO: fuck dingus
  ACCOUNTANT();
  EXCEPTS();
}
