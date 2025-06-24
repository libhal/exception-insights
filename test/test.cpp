#include <print>
using namespace std;

int main () {
  try
  {
    throw 20;
  }
  catch (int e)
  {
    println("Exception at {}", e)  ;
  }
  return 0;
}