//
// Created by zanpisotek on 10/15/26.
//

#include <iostream>
#include "Application.h"

int main() {
  Application app;
  try {
    app.Run();
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}
